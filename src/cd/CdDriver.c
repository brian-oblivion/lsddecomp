/*
 * CdDriver.c -- the CD driver (class CdDriver, include/CdDriver.h). Five
 * parts, in ROM order:
 *   1. the lifecycle: the allocator, the constructor, the finalizer and the
 *      empty slot +0x040;
 *   2. the request methods (below);
 *   3. the module level: the three request methods that are not
 *      per-request, the queue's front end, the driver mode, the file table
 *      and the service tick;
 *   4. the read state machines, the queue's nodes, the file-table lookups;
 *   5. the blocking file calls.
 *
 * Part 2: CdDriver's request methods, open, close, seek, read and loadFile
 * (slots +0x044..+0x058 of gCdDriverMethods, with the empty slot +0x050)
 * and runRequestQueue (+0x068), which feeds the queued requests back to
 * them.
 *
 * `self` is never a CdDriver of its own: SetActiveDataSource copies these
 * slots into every FileResource client's table, so `self` is the TimImage,
 * TodSet, ... that called its own `open`/`read`, and every field used here
 * is FileResource's.
 *
 * Every method but runRequestQueue has one shape, chosen by the driver mode
 * (SetCdDriverMode):
 *   - sCdAsyncEnabled and sCdSyncQueueMode both 0: forward to the blocking
 *     OpenCdFile / CloseCdFile / GetCdFileSize / ReadCdFile, or for loadFile
 *     to FileResource__LoadFile, and return.
 *   - otherwise, called from outside the queue (inQueueDispatch 0): append a
 *     CD_OP_* request for `self` with EnqueueCdRequest.
 *   - called back by runRequestQueue (inQueueDispatch 1), and the drive not
 *     busy: StartCdOperation, then either hand the work to the state machine
 *     through sCdSeekParam / sCdReadSectorCount / sCdReadBuffer and
 *     sCdTickStep (async mode), or do it on the spot as a CdControl(CdlSetloc)
 *     / CdSync / CdRead / CdReadSync spin and ResetCdStateMachine (queued
 *     synchronous mode).
 *
 * runRequestQueue looks only at the head node: a node not yet started is
 * dispatched to its owner's slot; a started one, once the drive is idle, is
 * reported to its owner as CD_FLAG_* bits in `flags` (then onRequestDone) and
 * freed, and the service stops when the queue empties.
 */
#include "common.h"
#include <libcd.h>
#include "CdDriver.h"
#include <libetc.h>
#include "DrawSystem.h"
#include <strings.h>
#include "GameApplicationFileResource.h"

/* `pos` (self->pos, CdFileEntry::pos) is Sony's CdlLOC; CdControl takes it
 * as the u_char * parameter bytes, hence those casts. */


extern void CloseCdFile(CdDriver *self);
extern void LockCd(void);
/* op is a CD_OPERATION_* and state the first CD_STATE_* (CdDriver.h). */
extern void StartCdOperation(s32 op, s32 state);
extern void ResetCdStateMachine(void);
extern void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1);
extern void UnlockCd(void);

extern void *FindCdFileEntry(char *name);
extern s32 FindCdFileIndex(char *name);

extern void OpenCdFile(CdDriver *self, char *name);
extern char *BuildCdFilePath(char *dest, char *name);

/* ---- part 1: the lifecycle ---- */

/* Nothing calls New_CdDriver: SetActiveDataSource copies the driver's slots
 * into its clients' tables, so the request methods below run on other
 * objects. The ctor runs InitCdDrive (part 3) once per boot. */

/* The game's pool allocator, src/app/BMemPMgr.c. */
extern void *BMemPMgrAlloc(s32 size);
/* Defined in CdDriver.c. */
extern void InitCdDrive(void);

CdDriver *New_CdDriver(void) {
    CdDriver *self;

    self = BMemPMgrAlloc(sizeof(CdDriver));
    if (self != NULL) {
        GetCdDriverMethods()->ctor(self);
        return self;
    }
    return NULL;
}

void CdDriver__CdDriver(CdDriver *self) {
    GetFileResourceMethods()->ctor((FileResource *)self);
    self->methods = GetCdDriverMethods();
    self->inQueueDispatch = 0;
    InitCdDrive();
}

void CdDriver__Finalize(CdDriver *self) {
    self->methods->cancelRequests(self);
    self->methods->freeBuffer(self);
}

void CdDriver__NoOpSlot40(void) {}

/* ---- part 2: the request methods ---- */

void CdDriver__Open(CdDriver *self, char *name, s32 param0, s32 param1) {
    char path[CD_PATH_SIZE];
    CdlFILE statBuf;
    CdFileEntry *entry;
    s32 size; /* MATCHING: not shared with status: one local adds a move in the CdSync loop */
    s32 status;

    if (sCdAsyncEnabled == 0 && sCdSyncQueueMode == 0) {
        OpenCdFile(self, name);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (sCdBusy == 0 && self->isOpen == 0) {
            StartCdOperation(CD_OPERATION_OPEN, CD_STATE_SETLOC);
            if (sCdAsyncEnabled != 0) {
                entry = FindCdFileEntry(name);
                sCdSeekParam = entry;
                if (entry == NULL) {
                    return;
                }
                self->pos = entry->pos;
                /* MATCHING: re-read through sCdSeekParam, here, not entry->size later */
                size = sCdSeekParam->size;
                sCdTickStep = CD_TICK_STATE_MACHINE;
                self->isOpen = 1;
                self->size = size;
            } else {
                BuildCdFilePath(path, name);
                do {
                } while (CdSearchFile(&statBuf, path) == NULL);
                self->pos = statBuf.pos;
                self->size = statBuf.size;
                do {
                    CdControl(CdlSetloc, (u_char *)&self->pos, 0);
                    do {
                        status = CdSync(0, 0);
                    } while (status == CdlNoIntr);
                } while (status == CdlDiskError);
                self->isOpen = 1;
                ResetCdStateMachine();
            }
        }
    } else {
        EnqueueCdRequest(self, FindCdFileIndex(name), CD_OP_OPEN, param0, param1);
    }
    UnlockCd();
}

void CdDriver__Close(CdDriver *self) {
    if (sCdAsyncEnabled == 0 && sCdSyncQueueMode == 0) {
        CloseCdFile(self);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (sCdBusy == 0) {
            StartCdOperation(CD_OPERATION_CLOSE, CD_STATE_IDLE);
            self->isOpen = 0;
            ResetCdStateMachine();
        }
    } else {
        EnqueueCdRequest(self, 0, CD_OP_CLOSE, 0, 0);
    }
    UnlockCd();
}

extern CdlLOC sCdSeekLoc;

extern s32 GetCdFileSize(CdDriver *self);

s32 CdDriver__Seek(CdDriver *self, u32 offset, s32 mode) {
    s32 status;
    u32 sectors;

    if (sCdAsyncEnabled == 0 && sCdSyncQueueMode == 0) {
        return GetCdFileSize(self);
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (sCdBusy == 0 && self->isOpen != 0) {
            StartCdOperation(CD_OPERATION_SEEK, CD_STATE_SETLOC);
            sectors = offset >> CD_SECTOR_SHIFT;
            if ((offset & (CD_SECTOR_SIZE - 1)) != 0) {
                sectors = sectors + 1;
            }
            CdIntToPos(CdPosToInt(&self->pos) + sectors, &sCdSeekLoc);
            if (mode == 0) {
                if (sCdAsyncEnabled != 0) {
                    /* the state machine seeks to &sCdSeekParam->pos: aim it
                     * at a pretend entry whose pos is sCdSeekLoc */
                    sCdSeekParam = (CdFileEntry *)((u8 *)&sCdSeekLoc - offsetof(CdFileEntry, pos));
                    sCdTickStep = CD_TICK_STATE_MACHINE;
                } else {
                    do {
                        CdControl(CdlSetloc, (u_char *)&sCdSeekLoc, 0);
                        do {
                            status = CdSync(0, 0);
                        } while (status == CdlNoIntr);
                    } while (status == CdlDiskError);
                    ResetCdStateMachine();
                }
            } else {
                ResetCdStateMachine();
                UnlockCd();
                if ((self->size & (CD_SECTOR_SIZE - 1)) != 0) {
                    return ((self->size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
                }
                return self->size;
            }
        }
    } else {
        EnqueueCdRequest(self, 0, CD_OP_SEEK, (s32)offset, mode);
    }
    UnlockCd();
    return 0;
}

void CdDriver__NoOpSlot50(void) {}

extern s32 ReadCdFile(CdDriver *self, void *buf, s32 size);

s32 CdDriver__Read(CdDriver *self, void *buf, u32 size) {
    s32 status;

    if (sCdAsyncEnabled == 0 && sCdSyncQueueMode == 0) {
        ReadCdFile(self, buf, size);
        return 0;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (sCdBusy == 0 && self->isOpen != 0) {
            StartCdOperation(CD_OPERATION_READ, CD_STATE_READ);
            if (sCdAsyncEnabled != 0) {
                sCdReadSectorCount = size >> CD_SECTOR_SHIFT;
                sCdReadBuffer = buf;
                sCdTickStep = CD_TICK_STATE_MACHINE;
            } else {
            /* MATCHING: a goto, not do-while: a do-while hoists the -1 into a saved register */
            retry:
                CdRead(size >> CD_SECTOR_SHIFT, buf, CdlModeSpeed);
                do {
                    status = CdReadSync(0, 0);
                } while (status > 0);
                if (status == -1) {
                    goto retry;
                }
                ResetCdStateMachine();
            }
        }
    } else {
        EnqueueCdRequest(self, 0, CD_OP_READ, (s32)buf, size);
    }
    UnlockCd();
    return 0;
}

/* FileResource__LoadFile takes (self, name), and loadFile's direct path
 * hands it this method's own self and name by leaving them where they
 * arrived. MATCHING: called through a no-argument type, so no argument is
 * reloaded before the jal. */
typedef void (*LoadFileNoArgsFn)(void);

extern void *BMemPMgrAlloc(s32 size);

void CdDriver__LoadFile(CdDriver *self, char *name) {
    CdFileEntry *entry;
    s32 sectorCount;
    s32 readSize;
    void *buffer;
    s32 status;

    if (sCdAsyncEnabled == 0 && sCdSyncQueueMode == 0) {
        ((LoadFileNoArgsFn)FileResource__LoadFile)();
        self->flags |= CD_FLAG_LOAD_FILE_DONE;
        self->methods->onRequestDone(self);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (sCdBusy == 0 && (self->buffer == NULL || self->freeGuard != 0)) {
            StartCdOperation(CD_OPERATION_LOAD_FILE, CD_STATE_SETLOC);
            sCdSavedSeekParam = sCdSeekParam;
            entry = FindCdFileEntry(name);
            sCdSeekParam = entry;
            if (entry == NULL) {
                return;
            }
            /* MATCHING: entry->size spelled at both uses; one local merges the two loads */
            sectorCount = entry->size >> CD_SECTOR_SHIFT;
            sCdReadSectorCount = sectorCount;
            if ((entry->size & (CD_SECTOR_SIZE - 1)) != 0) {
                sCdReadSectorCount = sectorCount + 1;
            }
            readSize = sCdReadSectorCount << CD_SECTOR_SHIFT;
            if (self->buffer == NULL) {
                buffer = BMemPMgrAlloc(readSize);
                if (buffer == NULL) {
                    self->methods->close(self);
                    return;
                }
                sCdReadBuffer = buffer;
                self->buffer = buffer;
            } else {
                sCdReadBuffer = self->buffer;
            }
            if (sCdAsyncEnabled != 0) {
                self->bufferSize = readSize;
                sCdTickStep = CD_TICK_LOAD_FILE;
            } else {
            /* MATCHING: a goto, not do-while, as in CdDriver__Read */
            retry:
                do {
                    CdControl(CdlSetloc, (u_char *)&sCdSeekParam->pos, 0);
                    do {
                        status = CdSync(0, 0);
                    } while (status == CdlNoIntr);
                } while (status == CdlDiskError);
                CdRead(sCdReadSectorCount, self->buffer, CdlModeSpeed);
                do {
                    status = CdReadSync(0, 0);
                } while (status > 0);
                if (status == -1) {
                    goto retry;
                }
                self->bufferSize = readSize;
                sCdRequestQueue->active = 1;
                ResetCdStateMachine();
            }
        }
    } else {
        EnqueueCdRequest(self, FindCdFileIndex(name), CD_OP_LOAD_FILE, 0, 0);
    }
    UnlockCd();
}

extern void FreeCdRequestNode(CdRequestNode *node);
/* An entry's name is its first member, so the entry is also the name that
 * open and loadFile take. */
extern void *GetCdFileEntry(s32 index);

void CdDriver__RunRequestQueue(void) {
    CdRequestNode *node;
    CdDriver *self;
    s32 op;

    LockCd();
    node = sCdRequestQueue;
    if (node != NULL) {
        self = node->owner;
        op = node->op;
        if (node->active == 0) {
            self->inQueueDispatch = 1;
            switch (op) {
                case CD_OP_OPEN:
                    self->methods->open(self, GetCdFileEntry(node->fileIndex), node->param0, node->param1);
                    break;
                case CD_OP_CLOSE:
                    self->methods->close(self);
                    break;
                case CD_OP_SEEK:
                    self->methods->seek(self, node->param0, node->param1);
                    break;
                case CD_OP_READ:
                    self->methods->read(self, (void *)node->param0, node->param1);
                    break;
                case CD_OP_LOAD_FILE:
                    self->methods->loadFile(self, GetCdFileEntry(node->fileIndex));
                    break;
            }
            self->inQueueDispatch = 0;
        } else if (sCdIdle != 0) {
            if (node->unk4 != 0) {
                self->flags |= 1;
            }
            self->pendingRequests -= 1;
            /* MATCHING: the two volatile re-reads; plain |= is 3 words short */
            self->flags = *(volatile s32 *)&self->flags | CD_FLAG_DONE;
            if (self->pendingRequests == 0) {
                self->flags = *(volatile s32 *)&self->flags | CD_FLAG_NONE_PENDING;
            }
            switch (op) {
                case CD_OP_OPEN:
                    self->flags |= CD_FLAG_OPEN_DONE;
                    break;
                case CD_OP_CLOSE:
                    self->flags |= CD_FLAG_CLOSE_DONE;
                    break;
                case CD_OP_SEEK:
                    self->flags |= CD_FLAG_SEEK_DONE;
                    break;
                case CD_OP_READ:
                    self->flags |= CD_FLAG_READ_DONE;
                    break;
                case CD_OP_LOAD_FILE:
                    self->flags |= CD_FLAG_LOAD_FILE_DONE;
                    break;
            }
            self->methods->onRequestDone(self);
            FreeCdRequestNode(node);
            if (sCdRequestQueue == NULL) {
                self->methods->stopService(self);
            }
        }
    }
    UnlockCd();
}

/* ---- part 3: the module level ---- */

/*
 * The CD driver's module level: the three request methods that are not
 * per-request (requestLoadFile, stopService, cancelRequests), the queue's
 * front end, the driver mode, the file table and the service tick. The
 * class, the queue's types and the shared module state are
 * include/CdDriver.h's; the constructor is in part 1, the other
 * request methods in part 2, the state machines and the queue's nodes in
 * part 4, the blocking file calls in part 5.
 *
 *   - The driver mode. SetCdDriverMode sets sCdAsyncEnabled (requests are
 *     queued and run in the background), sCdSyncQueueMode (queued, but each
 *     run as a blocking spin) and gCdUseVSyncCallback (ServiceCdDriver is
 *     installed with VSyncCallback; otherwise it becomes the DrawSystem
 *     singleton's callback). It refuses while sCdBusy. InitCdDrive puts the
 *     drive in double speed once; the Is/Get functions read the state
 *     machine's flags back.
 *   - The queue's front end. EnqueueCdRequest fills a node
 *     AllocCdRequestNode has linked, counts it in its owner's
 *     pendingRequests and calls StartCdService. CdDriver__CancelRequests
 *     frees every node an owner has queued, first stopping the drive
 *     (CdFlush, ResetCdStateMachine) if that owner's request is running.
 *   - The service tick. ServiceCdDriver steps the state machine sCdTickStep
 *     names and then runs the queue (runRequestQueue) while sCdQueueEnabled;
 *     StartCdService installs it and enables the queue, StopCdServiceIfIdle
 *     removes it once no state machine runs, DisableCdQueue only stops the
 *     queue. The tick does nothing while sCdLock is set (LockCd/UnlockCd)
 *     or while the BMemPMgr allocator is busy.
 *   - The file table. SetFileTable / SetFileTableCount install a
 *     CdFileEntry array; ResolveFileEntries fills each entry's disc position
 *     and size with CdSearchFile, CD_SEARCH_ATTEMPTS tries per name.
 *
 * The game reaches all of it through GameApplicationFileResource.c's
 * wrappers while the active data source is this class's id, DATASOURCE_CD
 * (0x13); the other source is the null driver, DATASOURCE_NULL (0x23).
 */

/* This unit's own functions, called before their definitions. */
void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1);
void InitCdDrive(void);
void LockCd(void);
void UnlockCd(void);
s32 ServiceCdDriver(void);
void StopCdServiceIfIdle(void);

/* Part 3, below: the request list, the state machines, the file-table lookup. */
extern CdRequestNode *AllocCdRequestNode(void);     /* allocate and link at the tail */
extern void FreeCdRequestNode(CdRequestNode *node); /* unlink and free */
extern void ResetCdStateMachine(void);              /* end the operation, mark idle */
extern void TickCdStateMachine(void);               /* CD_TICK_STATE_MACHINE */
extern void TickCdLoadFileStateMachine(void);       /* CD_TICK_LOAD_FILE */
extern s32 FindCdFileIndex(char *name);             /* name -> gFileTable index */

/* Part 4, below. */
extern char *BuildCdFilePath(char *dest, char *name); /* "\\<data directory><name>;1" */

/* TmdRenderer.c */
extern s32 GetBMemPMgrBusy(void);

extern void printf(const char *fmt, ...);

extern const char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */

/* Part 2's module state; parts 2 and 4 see it only through the
 * functions below. */
extern s32 sCdDriveInited;       /* InitCdDrive has set the drive's mode */
extern s32 sCdLock;              /* LockCd / UnlockCd */
extern s32 sCdCallbackInstalled; /* StartCdService has run, StopCdServiceIfIdle not since */
extern s32 sCdQueueEnabled;      /* ServiceCdDriver runs the request queue */

void CdDriver__RequestLoadFile(CdDriver *self, char *name) {
    s32 *unassigned; /* never assigned */
    s32 fileIndex;

    LockCd();

    if (name != NULL) {
        if (sCdAsyncEnabled != 0) {
            /* MATCHING: retail's bug, a store through the caller's $s2 */
            unassigned[1] = 1;
            fileIndex = FindCdFileIndex(name);
            EnqueueCdRequest(self, fileIndex, CD_OP_LOAD_FILE, 0, 0);
        } else {
            self->methods->loadFile(self, name);

            if (self->pendingRequests == 0) {
                self->flags |= CD_FLAG_NONE_PENDING;
            }
        }
    }

    UnlockCd();
}

void CdDriver__StopService(void) {
    LockCd();
    StopCdServiceIfIdle();
    UnlockCd();
}

void CdDriver__CancelRequests(CdDriver *self) {
    CdRequestNode *head;
    CdRequestNode *node;
    CdRequestNode *next;
    CdFileEntry *saved;

    LockCd();

    head = sCdRequestQueue;

    if (head != NULL && self->pendingRequests != 0) {
        self->flags = 0;

        if (head->owner == self && head->active != 0 && sCdIdle == 0) {
            CdFlush();
            ResetCdStateMachine();
            saved = sCdSavedSeekParam;
            sCdSavedSeekParam = NULL;
            sCdSeekParam = saved;
        }

        for (node = sCdRequestQueue; node != NULL; node = next) {
            next = node->next;
            if (node->owner == self) {
                FreeCdRequestNode(node);
                self->pendingRequests--;
            }
        }
    }

    UnlockCd();
}

CdDriverMethods *GetCdDriverMethods(void) {
    return &gCdDriverMethods;
}

void InitCdDrive(void) {
    u8 mode;

    if (sCdDriveInited != 0) {
        return;
    }

    CdSetDebug(0);
    mode = CdlModeSpeed;
    while (CdControlB(CdlSetmode, &mode, NULL) == 0) {
    }
    sCdDriveInited = 1;
}

s32 IsCdBusy(void) {
    return sCdBusy;
}

s32 IsCdIdle(void) {
    return sCdIdle;
}

s32 GetCdOperation(void) {
    return sCdOperation;
}

s32 GetCdState(void) {
    return sCdState;
}

/* sCdSyncQueueMode is written only by SetCdDriverMode's second argument.
 * With it set and sCdAsyncEnabled clear, the request methods
 * (part 2) still queue each request but run it as a blocking spin
 * when runRequestQueue dispatches it; with both clear they skip the queue. */

s32 GetCdDriverMode(s32 *outSyncQueueMode) {
    if (outSyncQueueMode != NULL) {
        *outSyncQueueMode = sCdSyncQueueMode;
    }
    return sCdAsyncEnabled;
}

s32 SetCdDriverMode(s32 async, s32 syncQueueMode, s32 useVSyncCallback) {
    DrawSystem *drawSystem;

    if (sCdBusy == 0) {
        if (useVSyncCallback == 0) {
            drawSystem = GetDrawSystem();

            if (sCdAsyncEnabled == 0) {
                if (async != 0) {
                    drawSystem->methods->setCallback(drawSystem, (void (*)(void))ServiceCdDriver);
                }
            } else {
                if (async == 0) {
                    drawSystem->methods->setCallback(drawSystem, NULL);
                }
            }
        }

        gCdUseVSyncCallback = useVSyncCallback;
        sCdAsyncEnabled = async;
        sCdSyncQueueMode = syncQueueMode;

        return 1;
    }

    return 0;
}

void SetFileTable(CdFileEntry *table) {
    gFileTable = table;
}

void SetFileTableCount(s32 count) {
    gFileTableCount = count;
}

s32 GetFileTableCount(void) {
    return gFileTableCount;
}

s32 ResolveFileEntries(CdFileEntry *entries, s32 count) {
    CdFileEntry *end;
    char path[CD_PATH_SIZE];
    CdlFILE info;
    s32 tries;

    end = entries + count;

    InitCdDrive();

    for (; entries < end; entries++) {
        BuildCdFilePath(path, entries->name);

        for (tries = 0; tries < CD_SEARCH_ATTEMPTS; tries++) {
            if (CdSearchFile(&info, path) != 0) {
                goto found;
            }
        }

        printf(sFileNotFoundMsg, path);

    found:
        entries->pos = info.pos;
        entries->size = info.size;
    }

    return 1;
}

/* The driver's re-entrancy lock, and its one reader is ServiceCdDriver
 * below: the service tick returns immediately while sCdLock is set, so every
 * public entry point in parts 1 to 3 brackets its body
 * with LockCd()/UnlockCd() to keep the VSync-driven tick out of a
 * half-updated queue. Not a mutex -- nothing spins or blocks on it. */

void LockCd(void) {
    sCdLock = 1;
}

void UnlockCd(void) {
    sCdLock = 0;
}

s32 ServiceCdDriver(void) {
    if (sCdLock != 0) {
        return 0;
    }

    if (GetBMemPMgrBusy() != 0) {
        return 0;
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback(NULL);
    }

    if (sCdTickStep == CD_TICK_STATE_MACHINE) {
        TickCdStateMachine();
    } else if (sCdTickStep == CD_TICK_LOAD_FILE) {
        TickCdLoadFileStateMachine();
    }

    if (sCdQueueEnabled != 0) {
        GetCdDriverMethods()->runRequestQueue();
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback((void (*)(void))ServiceCdDriver);
    }

    return 0;
}

void StartCdService(void) {
    LockCd();

    if (sCdCallbackInstalled == 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback((void (*)(void))ServiceCdDriver);
        }
        sCdCallbackInstalled = 1;
    }

    sCdQueueEnabled = 1;
    UnlockCd();
}

void StopCdServiceIfIdle(void) {
    LockCd();

    if (sCdTickStep == CD_TICK_NONE && sCdCallbackInstalled != 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback(NULL);
        }
        sCdCallbackInstalled = 0;
        sCdQueueEnabled = 0;
    }

    UnlockCd();
}

void DisableCdQueue(void) {
    LockCd();
    sCdQueueEnabled = 0;
    UnlockCd();
}

/* Fills a node AllocCdRequestNode has already linked onto sCdRequestQueue,
 * counts it against its owner and starts the service tick. */
void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op, s32 param0, s32 param1) {
    CdRequestNode *node = AllocCdRequestNode();

    /* MATCHING: retail's store order; cc1 keeps statement order here */
    node->op = op;
    node->param0 = param0;
    node->owner = owner;
    node->fileIndex = fileIndex;
    node->param1 = param1;

    owner->pendingRequests++;
    owner->flags = 0;
    StartCdService();
}

/* ---- part 4: the state machines, the queue's nodes, the file table ---- */

/*
 * The CD driver's read state machine, its request-queue nodes and its
 * file-table lookups (the class, the queue and the table are
 * include/CdDriver.h's).
 *
 * The state machine takes one step each time ServiceCdDriver
 * (part 3, a VSync callback) runs: it calls TickCdStateMachine or
 * TickCdLoadFileStateMachine as sCdTickStep says, after a CdDriver method
 * (part 2) has started an operation with StartCdOperation and set
 * sCdSeekParam, sCdReadSectorCount and sCdReadBuffer. sCdState walks
 * CD_STATE_SETLOC (CdControlF(CdlSetloc) to sCdSeekParam->pos),
 * CD_STATE_SETLOC_WAIT (poll CdSync), CD_STATE_READ (CdRead) and
 * CD_STATE_READ_WAIT (poll CdReadSync); SetCdState moves it and
 * ResetCdStateMachine ends the operation and marks the driver idle. A seek
 * that errors or stays unanswered for CD_WAIT_TIMEOUT polls, and a read that
 * errors, go back to CD_STATE_SETLOC.
 *
 * The two tick functions differ in three places. TickCdStateMachine, which
 * Open and Seek use (Read starts it at CD_STATE_READ), ends the operation
 * when the seek completes and CdFlushes after a read error;
 * TickCdLoadFileStateMachine, LoadFile's, goes on to read after the seek,
 * and when the read is done restores the sCdSeekParam LoadFile saved in
 * sCdSavedSeekParam.
 *
 * AllocCdRequestNode appends a zeroed node to sCdRequestQueue and
 * FreeCdRequestNode unlinks one; EnqueueCdRequest (part 3) fills
 * them and CdDriver__RunRequestQueue (part 2) consumes them from the
 * head. FindCdFileEntry and FindCdFileIndex look a name up in gFileTable by
 * substring, GetCdFileEntry indexes it. Every function but the three
 * state setters brackets its body with LockCd / UnlockCd, which makes
 * ServiceCdDriver skip its tick in between.
 */

/* Part 2's, above. */
extern void LockCd(void);
extern void UnlockCd(void);

/* CdSync / CdReadSync mode: return the current status at once (0 waits).
 * CdReadSync then answers -1 for an error, 0 when the read is done, and
 * otherwise the sectors still to come. */
#define CD_SYNC_POLL 1

/* Polls of CdSync that answer CdlNoIntr before the seek is issued again. */
#define CD_WAIT_TIMEOUT 601

/* The game's pool allocator, src/app/BMemPMgr.c. */
extern void *BMemPMgrAlloc(s32 size);
/* BMemPMgrFree is GameApplicationFileResource.h's, included above. */

extern s32 gCdTimeoutCounter; /* CD_STATE_SETLOC_WAIT's polls; SetCdState clears it */

CdRequestNode *AllocCdRequestNode(void) {
    CdRequestNode *node;
    CdRequestNode *head;
    CdRequestNode *cur;

    LockCd();
    node = BMemPMgrAlloc(sizeof(CdRequestNode));
    if (node != NULL) {
        head = sCdRequestQueue;
        node->prev = NULL;
        node->next = NULL;
        node->active = 0;
        node->unk4 = 0;
        if (head != NULL) {
            cur = head;
            while (cur->next != NULL) {
                cur = cur->next;
            }
            cur->next = node;
            node->prev = cur;
        } else {
            sCdRequestQueue = node;
        }
    }
    UnlockCd();
    return node;
}

void FreeCdRequestNode(CdRequestNode *node) {
    CdRequestNode *prev;
    CdRequestNode *next;

    LockCd();
    if (node != NULL) {
        prev = node->prev;
        if (prev != NULL) {
            prev->next = node->next;
        } else {
            sCdRequestQueue = node->next;
        }
        next = node->next;
        if (next != NULL) {
            next->prev = node->prev;
        }
        BMemPMgrFree(node);
    }
    UnlockCd();
}

/* The first entry whose name contains `name`, or NULL. The not-found return
 * skips UnlockCd, as FindCdFileIndex's does, so ServiceCdDriver stays off
 * until the next UnlockCd anywhere. */
void *FindCdFileEntry(char *name) {
    CdFileEntry *cur = gFileTable;
    s32 i = 0;

    LockCd();
    do {
        if (strstr(cur->name, name) != NULL) {
            UnlockCd();
            return cur;
        }
        i++;
        cur++;
    } while (i < gFileTableCount);
    return NULL;
}

/* The index of the first entry whose name contains `name`, or -1. */
s32 FindCdFileIndex(char *name) {
    CdFileEntry *cur = gFileTable;
    s32 i = 0;

    LockCd();
    while (strstr(cur->name, name) == NULL) {
        i++;
        if (i >= gFileTableCount) {
            return -1;
        }
        cur++;
    }
    UnlockCd();
    return i;
}

void *GetCdFileEntry(s32 index) {
    CdFileEntry *table = gFileTable;
    CdFileEntry *entry;

    LockCd();
    entry = &table[index];
    UnlockCd();
    return entry;
}

/* Defined below. */
extern void ResetCdStateMachine(void);
extern void SetCdState(s32 state);

void TickCdStateMachine(void) {
    s32 result;
    s32 newState;

    LockCd();
    switch (sCdState) {
        case CD_STATE_SETLOC:
            if (CdControlF(CdlSetloc, (u_char *)&sCdSeekParam->pos) == 0) {
                goto unlock;
            }
            newState = CD_STATE_SETLOC_WAIT;
            break;
        case CD_STATE_SETLOC_WAIT:
            switch (CdSync(CD_SYNC_POLL, NULL)) {
                /* MATCHING: CdlDiskError's case first; last, its test flips polarity */
                case CdlDiskError:
                    newState = CD_STATE_SETLOC;
                    break;
                case CdlComplete:
                    ResetCdStateMachine();
                    goto unlock;
                case CdlNoIntr:
                    gCdTimeoutCounter++;
                    if (gCdTimeoutCounter < CD_WAIT_TIMEOUT) {
                        goto unlock;
                    }
                    newState = CD_STATE_SETLOC;
                    break;
                default:
                    goto unlock;
            }
            break;
        case CD_STATE_READ:
            if (CdRead(sCdReadSectorCount, sCdReadBuffer, CdlModeSpeed) == 0) {
                goto unlock;
            }
            newState = CD_STATE_READ_WAIT;
            break;
        case CD_STATE_READ_WAIT:
            result = CdReadSync(CD_SYNC_POLL, NULL);
            if (result == -1) {
                CdFlush();
                newState = CD_STATE_SETLOC;
                break;
            }
            if (result == 0) {
                ResetCdStateMachine();
            }
            goto unlock;
        default:
            goto unlock;
    }
    SetCdState(newState);
unlock:
    UnlockCd();
}

void TickCdLoadFileStateMachine(void) {
    s32 result;
    s32 newState;

    LockCd();
    switch (sCdState) {
        case CD_STATE_SETLOC:
            if (CdControlF(CdlSetloc, (u_char *)&sCdSeekParam->pos) == 0) {
                goto unlock;
            }
            newState = CD_STATE_SETLOC_WAIT;
            break;
        case CD_STATE_SETLOC_WAIT:
            switch (CdSync(CD_SYNC_POLL, NULL)) {
                case CdlComplete:
                    newState = CD_STATE_READ;
                    break;
                case CdlNoIntr:
                    gCdTimeoutCounter++;
                    if (gCdTimeoutCounter < CD_WAIT_TIMEOUT) {
                        goto unlock;
                    }
                    newState = CD_STATE_SETLOC;
                    break;
                case CdlDiskError:
                    newState = CD_STATE_SETLOC;
                    break;
                default:
                    goto unlock;
            }
            break;
        case CD_STATE_READ:
            if (CdRead(sCdReadSectorCount, sCdReadBuffer, CdlModeSpeed) == 0) {
                goto unlock;
            }
            newState = CD_STATE_READ_WAIT;
            break;
        case CD_STATE_READ_WAIT:
            result = CdReadSync(CD_SYNC_POLL, NULL);
            if (result == -1) {
                newState = CD_STATE_SETLOC;
                break;
            }
            if (result == 0) {
                ResetCdStateMachine();
                sCdSeekParam = sCdSavedSeekParam;
                sCdSavedSeekParam = NULL;
            }
            goto unlock;
        default:
            goto unlock;
    }
    SetCdState(newState);
unlock:
    UnlockCd();
}

/* op is a CD_OPERATION_* and state the first CD_STATE_*; the operation is the
 * queue's head request, which is marked active. */
void StartCdOperation(s32 op, s32 state) {
    sCdBusy = 1;
    sCdOperation = op;
    sCdState = state;
    sCdIdle = 0;
    sCdRequestQueue->active = 1;
}

/* No operation, no tick step: the driver is idle. */
void ResetCdStateMachine(void) {
    sCdOperation = 0;
    sCdState = CD_STATE_IDLE;
    sCdTickStep = CD_TICK_NONE;
    sCdIdle = 1;
    gCdTimeoutCounter = 0;
    sCdBusy = 0;
}

void SetCdState(s32 state) {
    sCdState = state;
    gCdTimeoutCounter = 0;
}

/* ---- part 5: the blocking file calls ---- */

/*
 * The CD driver's blocking file access, and a pair that turns a FileResource
 * into a CD driver.
 *
 * OpenCdFile, CloseCdFile, GetCdFileSize and ReadCdFile are what CdDriver's
 * open, close, seek and read slots (part 2) call when the driver is
 * not in async mode. Like those slots they run on whichever FileResource
 * object called them (CdDriver.h's banner) and use only its isOpen, pos and
 * size. OpenCdFile looks the name up with CdSearchFile under the path
 * BuildCdFilePath makes ("\\<data directory><name>;1") and records where
 * the file starts and how long it is; ReadCdFile seeks to that start and
 * reads whole sectors, starting over from the seek on a disk error. In this
 * mode a seek does not move: CdDriver__Seek returns GetCdFileSize and every
 * read begins at the start of the file.
 *
 * FileResource__InstallCdReadDriver runs FileResource's ctor, then gives the
 * object gCdDriverMethods and marks it closed; FileResource__DestroyCdReadDriver
 * runs FileResource's finalize. GetCdUseVSyncCallback is the CD half of
 * GameApplicationFileResource.c's GetActiveDataSourceUseVSyncCallback.
 *
 * Nothing in the executable calls the install/destroy pair or NoOp2, NoOp3
 * and NoOp4 (no jal, stored pointer or built address reaches them).
 */
/* FileResource and its table come from include/FileResource.h, through GameApplicationFileResource.h. */

/* Defined in other units: GetDataDirectory (GameApplicationFileResource.c) returns the data
 * directory's name; strcpy and strcat are Sony's libc2. */
extern char *GetDataDirectory(void);
extern char *strcpy(char *dest, char *src);
extern char *strcat(char *dest, char *src);

extern char sCdFileNotFoundFmt[];   /* "File not found. path = %s\n" */
extern char sCdFileVersionSuffix[]; /* ";1", the ISO9660 CD file-version suffix */

/* Defined below, after its caller OpenCdFile: functions stay in ROM order. */
char *BuildCdFilePath(char *dest, char *name);

void FileResource__InstallCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->ctor(self);
    self->methods = (FileResourceMethods *)GetCdDriverMethods();
    self->isOpen = 0;
}

void FileResource__DestroyCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->finalize(self);
}

void NoOp2(void) {}

/* Resolves `name` once and marks the object open; an open object is left
 * alone. After CD_SEARCH_ATTEMPTS failed lookups it prints the path and
 * returns with the object still closed.
 * MATCHING: the retry is a label and goto; a while or for loop hoists &path
 * out of it and rotates the saved registers. */
void OpenCdFile(CdDriver *self, char *name) {
    s32 retries;
    CdlFILE file;
    char path[CD_PATH_SIZE];

    retries = 0;
    if (self->isOpen == 0) {
        BuildCdFilePath(path, name);
    retry:
        if (CdSearchFile(&file, path) == 0) {
            if (retries++ < CD_SEARCH_ATTEMPTS - 1) {
                goto retry;
            }
            printf(sCdFileNotFoundFmt, path);
            return;
        }
        self->pos = file.pos;
        self->size = file.size;
        self->isOpen = 1;
    }
}

char *BuildCdFilePath(char *dest, char *name) {
    dest[0] = '\\';
    strcpy(dest + 1, GetDataDirectory());
    strcat(dest, name);
    strcat(dest, sCdFileVersionSuffix);
    return dest;
}

void CloseCdFile(CdDriver *self) {
    if (self->isOpen != 0) {
        self->isOpen = 0;
    }
}

/* The open file's size in whole sectors, one sector over when it is
 * already a multiple (CdDriver__Seek's async path rounds up only when it is
 * not); 0 when the object is closed. */
s32 GetCdFileSize(CdDriver *self) {
    u32 result;

    if (self->isOpen == 0) {
        result = 0;
    } else {
        result = ((self->size >> CD_SECTOR_SHIFT) + 1) << CD_SECTOR_SHIFT;
    }
    return result;
}

void NoOp3(void) {}

/* Reads `size` bytes, rounded down to whole sectors, from the start of the
 * open file into `buf`, and returns 0. A closed object is sent to its own
 * close slot instead.
 * MATCHING: the seek retry and the CdSync wait are label and goto loops and
 * only the CdReadSync wait is a do-while; any other loop kind moves the
 * branch targets. */
s32 ReadCdFile(CdDriver *self, void *buf, s32 size) {
    s32 sectors;
    s32 status;
    char scratch[2048]; /* MATCHING: never used; it sizes the frame so syncResult sits where retail's does */
    u_char syncResult[16]; /* CdSync writes 8 bytes; 16 is the size retail reserved */

    if (self->isOpen != 0) {
    retry:
        sectors = (u32)size >> CD_SECTOR_SHIFT;
        CdControl(CdlSetloc, (u_char *)&self->pos, 0);
    sync:
        status = CdSync(0, syncResult);
        if (status == CdlNoIntr) {
            goto sync;
        }
        if (status == CdlDiskError) {
            goto retry;
        }
        if (sectors != 0) {
            CdRead(sectors, (u_long *)buf, CdlModeSpeed);
            do {
                status = CdReadSync(0, 0);
            } while (status > 0);
            if (status == -1) {
                goto retry;
            }
            return 0;
        }
    } else {
        self->methods->close(self);
    }
    return 0;
}

void NoOp4(void) {}

s32 GetCdUseVSyncCallback(void) {
    return gCdUseVSyncCallback;
}
