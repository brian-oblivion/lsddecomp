/*
 * cd_driver.c -- the CD-ROM data-source driver, CdDriver (include/cd_driver.h,
 * whose file documentation describes the driver mode, the service tick, the
 * state machines and the blocking calls). Six parts, in address order:
 *   1. the lifecycle: the allocator, the constructor, the finalizer and the
 *      empty slot +0x040;
 *   2. the request methods open, close, seek, read and loadFile (slots
 *      +0x044..+0x058, with the empty +0x050) and runRequestQueue (+0x068),
 *      which feeds the queued requests back to them;
 *   3. the module level: requestLoadFile, stopService and cancelRequests,
 *      the queue's front end, the driver mode, the file table, the lock and
 *      the service tick;
 *   4. the read state machines, the queue's nodes, the file-table lookups;
 *   5. the blocking file calls;
 *   6. the method table and the pretend file entry a seek aims at.
 *
 * `self` in a method is never a CdDriver of its own but whichever
 * FileResource client called its own slot, so every field used is
 * FileResource's.
 */
#include "common.h"
#include <libcd.h>
#include "cd_driver.h"
#include <libetc.h>
#include "draw_system.h"
#include <strings.h>
#include "data_source.h"
#include "bmem_pmgr.h"
#include <stdio.h>

/* The CD driver's module state, all of it, in its .sdata order. */
static s32 sCdDriveInited SDATA = 0; /* InitCdDrive has set the drive's mode */
static s32 sCdAsyncEnabled SDATA = 0; /* nonzero: requests run on the tick's state machine, not blocking */
static s32 sCdSyncQueueMode SDATA = 0; /* nonzero with sCdAsyncEnabled 0: requests queue, then run blocking */
static s32 sCdBusy SDATA = 0;                       /* 0/1 */
static CdFileEntry *sFileTable SDATA = NULL;        /* SetFileTable */
static s32 sFileTableCount SDATA = 0;               /* SetFileTableCount */
static s32 sCdIdle SDATA = 1;                       /* 0/1 */
static s32 sCdOperation SDATA = CD_OPERATION_CLOSE; /* StartCdOperation's op, GetCdOperation's result */
static s32 sCdState SDATA = CD_STATE_IDLE;          /* the state machine's phase */
static CdFileEntry *sCdSeekParam SDATA = NULL; /* the state machines seek to &sCdSeekParam->pos */
static s32 sCdReadSectorCount SDATA = 0;       /* CdRead sector count */
static void *sCdReadBuffer SDATA = NULL;       /* CdRead target buffer */
static CdFileEntry *sCdSavedSeekParam SDATA = NULL; /* LoadFile's saved sCdSeekParam */
static s32 sCdLock SDATA = 0;                       /* LockCd / UnlockCd */
static s32 sCdQueueEnabled SDATA = 0;               /* ServiceCdDriver runs the request queue */
static CdRequestNode *sCdRequestQueue SDATA = NULL; /* list head */
static s32 sCdTickStep SDATA = CD_TICK_NONE;        /* CD_TICK_* */
static s32 sCdCallbackInstalled SDATA = 0; /* StartCdService has run, StopCdServiceIfIdle not since */
static s32 sCdTimeoutCounter SDATA = 0;    /* CD_STATE_SETLOC_WAIT's polls; SetCdState clears it */
static s32 sCdUseVSyncCallback SDATA = 1; /* ServiceCdDriver is the VSyncCallback; 0: the DrawSystem callback */
static char sCdFileVersionSuffix[] SDATA = ";1"; /* the ISO9660 CD file-version suffix */

/* `pos` (self->pos, CdFileEntry::pos) is Sony's CdlLOC; CdControl takes it
 * as the u_char * parameter bytes, hence those casts. */

/* ---- part 1: the lifecycle ---- */

/* Nothing calls New_CdDriver: SetActiveDataSource copies the driver's slots
 * into its clients' tables, so the request methods below run on other
 * objects. The ctor runs InitCdDrive (part 3) once per boot. */

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
    s32 size; /* MATCHING: not shared with status; one local for both is longer */
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

/* The pretend file entry a state-machine seek aims at; defined at the end. */
extern CdFileEntry sCdSeekEntry;

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
            CdIntToPos(CdPosToInt(&self->pos) + sectors, &sCdSeekEntry.pos);
            if (mode == SEEK_SET) {
                if (sCdAsyncEnabled != 0) {
                    /* the state machine seeks to &sCdSeekParam->pos */
                    sCdSeekParam = &sCdSeekEntry;
                    sCdTickStep = CD_TICK_STATE_MACHINE;
                } else {
                    do {
                        CdControl(CdlSetloc, (u_char *)&sCdSeekEntry.pos, 0);
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
            /* MATCHING: a goto, not do-while: a do-while sets up the -1 once, ahead of the loop */
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

void CdDriver__LoadFile(CdDriver *self, char *name) {
    CdFileEntry *entry;
    s32 sectorCount;
    s32 readSize;
    void *buffer;
    s32 status;

    if (sCdAsyncEnabled == 0 && sCdSyncQueueMode == 0) {
        FileResource__LoadFile((FileResource *)self, name);
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
            /* MATCHING: entry->size spelled at both uses; one local reads it once, retail twice */
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
            if (node->unused4 != 0) {
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
 * The module level: the three request methods that are not per-request, the
 * queue's front end (EnqueueCdRequest), the driver mode, the file table, the
 * lock and the service tick. The game reaches it through data_source.c's
 * wrappers while the active data source is DATASOURCE_CD.
 */

extern char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */

void CdDriver__RequestLoadFile(CdDriver *self, char *name) {
    s32 *unassigned; /* never assigned */
    s32 fileIndex;

    LockCd();

    if (name != NULL) {
        if (sCdAsyncEnabled != 0) {
            /* MATCHING: retail's bug: a store through a pointer never set, so to
             * whatever address the caller left behind */
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

        sCdUseVSyncCallback = useVSyncCallback;
        sCdAsyncEnabled = async;
        sCdSyncQueueMode = syncQueueMode;

        return 1;
    }

    return 0;
}

void SetFileTable(CdFileEntry *table) {
    sFileTable = table;
}

void SetFileTableCount(s32 count) {
    sFileTableCount = count;
}

s32 GetFileTableCount(void) {
    return sFileTableCount;
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

        /* MATCHING: a hit jumps past the not-found message; a break, then a test
         * of tries, compiles to a different loop exit. */
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

    if (sCdUseVSyncCallback != 0) {
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

    if (sCdUseVSyncCallback != 0) {
        VSyncCallback((void (*)(void))ServiceCdDriver);
    }

    return 0;
}

void StartCdService(void) {
    LockCd();

    if (sCdCallbackInstalled == 0) {
        if (sCdUseVSyncCallback != 0) {
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
        if (sCdUseVSyncCallback != 0) {
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

    /* MATCHING: retail's store order, not field order; statement order sets it here */
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
 * The read state machines take one step each time ServiceCdDriver runs,
 * after a request method has started an operation with StartCdOperation and
 * set the seek target (sCdSeekParam), the sector count and the buffer.
 * SetCdState moves sCdState; ResetCdStateMachine ends the operation. The
 * queue's nodes are appended by AllocCdRequestNode and consumed from the
 * front by CdDriver__RunRequestQueue. Every function but the three state
 * setters brackets its body with LockCd / UnlockCd.
 */

/* CdSync / CdReadSync mode: return the current status at once (0 waits).
 * CdReadSync then answers -1 for an error, 0 when the read is done, and
 * otherwise the sectors still to come. */
#define CD_SYNC_POLL 1

/* Polls of CdSync that answer CdlNoIntr before the seek is issued again. */
#define CD_WAIT_TIMEOUT 601

/* BMemPMgrFree is data_source.h's, included above. */

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
        node->unused4 = 0;
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
    CdFileEntry *cur = sFileTable;
    s32 i = 0;

    LockCd();
    do {
        if (strstr(cur->name, name) != NULL) {
            UnlockCd();
            return cur;
        }
        i++;
        cur++;
    } while (i < sFileTableCount);
    return NULL;
}

/* The index of the first entry whose name contains `name`, or -1. */
s32 FindCdFileIndex(char *name) {
    CdFileEntry *cur = sFileTable;
    s32 i = 0;

    LockCd();
    while (strstr(cur->name, name) == NULL) {
        i++;
        if (i >= sFileTableCount) {
            return -1;
        }
        cur++;
    }
    UnlockCd();
    return i;
}

void *GetCdFileEntry(s32 index) {
    CdFileEntry *table = sFileTable;
    CdFileEntry *entry;

    LockCd();
    entry = &table[index];
    UnlockCd();
    return entry;
}

void TickCdStateMachine(void) {
    s32 result;
    s32 newState;

    LockCd();
    /* MATCHING: arms that change state break to one SetCdState call and the rest jump
     * to the unlock; a SetCdState call in each arm is not merged back into one. */
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
                    sCdTimeoutCounter++;
                    if (sCdTimeoutCounter < CD_WAIT_TIMEOUT) {
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
    /* MATCHING: arms that change state break to one SetCdState call and the rest jump
     * to the unlock; a SetCdState call in each arm is not merged back into one. */
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
                    sCdTimeoutCounter++;
                    if (sCdTimeoutCounter < CD_WAIT_TIMEOUT) {
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
    sCdTimeoutCounter = 0;
    sCdBusy = 0;
}

void SetCdState(s32 state) {
    sCdState = state;
    sCdTimeoutCounter = 0;
}

/* ---- part 5: the blocking file calls ---- */

/*
 * The blocking file access CdDriver's open, close, seek and read slots call
 * outside the queue modes, and a pair that turns a FileResource into a CD
 * driver object. Like the slots, these run on whichever FileResource object
 * called them and use only its isOpen, pos and size.
 *
 * Nothing in the game calls the install/destroy pair or NoOp2, NoOp3 and
 * NoOp4, stores their address or builds it.
 */
/* FileResource and its table come from include/file_resource.h, through data_source.h. */

extern char sCdFileNotFoundFmt[]; /* "File not found. path = %s\n" */

void FileResource__InstallCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->ctor(self);
    self->methods = (FileResourceMethods *)GetCdDriverMethods();
    self->isOpen = 0;
}

void FileResource__DestroyCdReadDriver(FileResource *self) {
    GetFileResourceMethods()->finalize(self);
}

void NoOp2(void) {}

/* MATCHING: the retry is a label and goto; a while or for loop computes &path once,
 * outside it */
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

/* MATCHING: the seek retry and the CdSync wait are gotos, only the CdReadSync wait a
 * do-while */
s32 ReadCdFile(CdDriver *self, void *buf, u32 size) {
    s32 sectors;
    s32 status;
    char scratch[CD_SECTOR_SIZE]; /* MATCHING: never used; it puts syncResult where retail keeps it */
    u_char syncResult[16];        /* MATCHING: CdSync writes 8 bytes; 16 keeps the frame layout */

    if (self->isOpen != 0) {
    retry:
        sectors = size >> CD_SECTOR_SHIFT;
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
    return sCdUseVSyncCallback;
}

/* CdDriver's method table (include/cd_driver.h): FileResource's slots up to
 * +0x074, the eleven data-source slots filled with the driver's own methods.
 * A (void *) entry is a method declared on another class's type. */
CdDriverMethods gCdDriverMethods = {
    /* +0x000 header */ CDDRIVER_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ CdDriver__CdDriver,
    /* +0x00C finalize */ CdDriver__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ CdDriver__NoOpSlot40,
    /* +0x044 open */ CdDriver__Open,
    /* +0x048 close */ CdDriver__Close,
    /* +0x04C seek */ CdDriver__Seek,
    /* +0x050 slot50 */ CdDriver__NoOpSlot50,
    /* +0x054 read */ CdDriver__Read,
    /* +0x058 loadFile */ CdDriver__LoadFile,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ (void *)FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ CdDriver__RunRequestQueue,
    /* +0x06C requestLoadFile */ CdDriver__RequestLoadFile,
    /* +0x070 stopService */ (void *)CdDriver__StopService,
    /* +0x074 cancelRequests */ CdDriver__CancelRequests,
};

/* The pretend file entry CdDriver__Seek aims the state machine at
 * (sCdSeekParam): only its pos is used, the sector a seek moves to. */
CdFileEntry sCdSeekEntry = {{0}};
