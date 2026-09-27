/*
 * CdDriver's request methods: open, close, seek, read and loadFile (slots
 * +0x044..+0x058 of gCdDriverMethods, with the empty slot +0x050) and
 * runRequestQueue (+0x068), which feeds the queued requests back to them.
 * The class, its table and the queue's types are include/CdDriver.h's; the
 * constructor is in code_179d8_o.c, the queue front end and the service tick
 * in code_179d8_q.c, the state machines in code_179d8_r.c, the blocking file
 * calls in code_179d8_h.c.
 *
 * `self` is never a CdDriver of its own: SetActiveDataSource copies these
 * slots into every FileResource client's table, so `self` is the TimImage,
 * TodSet, ... that called its own `open`/`read`, and every field used here
 * is FileResource's.
 *
 * Every method but runRequestQueue has one shape, chosen by the driver mode
 * (SetCdDriverMode):
 *   - gCdAsyncEnabled and gCdSyncQueueMode both 0: forward to the blocking
 *     OpenCdFile / CloseCdFile / GetCdFileSize / ReadCdFile, or for loadFile
 *     to FileResource__LoadFile, and return.
 *   - otherwise, called from outside the queue (inQueueDispatch 0): append a
 *     CD_OP_* request for `self` with EnqueueCdRequest.
 *   - called back by runRequestQueue (inQueueDispatch 1), and the drive not
 *     busy: StartCdOperation, then either hand the work to the state machine
 *     through gCdSeekParam / gCdReadSectorCount / gCdReadBuffer and
 *     gCdTickStep (async mode), or do it on the spot as a CdControl(CdlSetloc)
 *     / CdSync / CdRead / CdReadSync spin and ResetCdStateMachine (queued
 *     synchronous mode).
 *
 * runRequestQueue looks only at the head node: a node not yet started is
 * dispatched to its owner's slot; a started one, once the drive is idle, is
 * reported to its owner as CD_FLAG_* bits in `flags` (then setFlag) and
 * freed, and the service stops when the queue empties.
 */
#include "common.h"
#include <libcd.h>
#include "CdDriver.h"

/* `pos` (self->pos, CdFileEntry::pos) is FileResource.h's CdLoc16, which is
 * Sony's CdlLOC by layout; the casts to CdlLOC / u_char * below go away when
 * that header can take <libcd.h> and use CdlLOC itself. */


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

void CdDriver__Open(CdDriver *self, char *name, s32 param0, s32 param1) {
    char path[CD_PATH_SIZE];
    CdlFILE statBuf;
    CdFileEntry *entry;
    s32 size; /* MATCHING: not shared with status: one local adds a move in the CdSync loop */
    s32 status;

    if (gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        OpenCdFile(self, name);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && self->isOpen == 0) {
            StartCdOperation(CD_OPERATION_OPEN, CD_STATE_SETLOC);
            if (gCdAsyncEnabled != 0) {
                entry = FindCdFileEntry(name);
                gCdSeekParam = entry;
                if (entry == NULL) {
                    return;
                }
                self->pos = entry->pos;
                /* MATCHING: re-read through gCdSeekParam, here, not entry->size later */
                size = gCdSeekParam->size;
                gCdTickStep = CD_TICK_STATE_MACHINE;
                self->isOpen = 1;
                self->size = size;
            } else {
                BuildCdFilePath(path, name);
                do {
                } while (CdSearchFile(&statBuf, path) == NULL);
                self->pos = *(CdLoc16 *)&statBuf.pos;
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
    if (gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        CloseCdFile(self);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0) {
            StartCdOperation(CD_OPERATION_CLOSE, CD_STATE_IDLE);
            self->isOpen = 0;
            ResetCdStateMachine();
        }
    } else {
        EnqueueCdRequest(self, 0, CD_OP_CLOSE, 0, 0);
    }
    UnlockCd();
}

extern CdlLOC gCdSeekLoc;

extern s32 GetCdFileSize(CdDriver *self);

s32 CdDriver__Seek(CdDriver *self, u32 offset, s32 mode) {
    s32 status;
    u32 sectors;

    if (gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        return GetCdFileSize(self);
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && self->isOpen != 0) {
            StartCdOperation(CD_OPERATION_SEEK, CD_STATE_SETLOC);
            sectors = offset >> CD_SECTOR_SHIFT;
            if ((offset & (CD_SECTOR_SIZE - 1)) != 0) {
                sectors = sectors + 1;
            }
            CdIntToPos(CdPosToInt((CdlLOC *)&self->pos) + sectors, &gCdSeekLoc);
            if (mode == 0) {
                if (gCdAsyncEnabled != 0) {
                    /* the state machine seeks to &gCdSeekParam->pos: aim it
                     * at a pretend entry whose pos is gCdSeekLoc */
                    gCdSeekParam = (CdFileEntry *)((u8 *)&gCdSeekLoc - offsetof(CdFileEntry, pos));
                    gCdTickStep = CD_TICK_STATE_MACHINE;
                } else {
                    do {
                        CdControl(CdlSetloc, (u_char *)&gCdSeekLoc, 0);
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

    if (gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        ReadCdFile(self, buf, size);
        return 0;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && self->isOpen != 0) {
            StartCdOperation(CD_OPERATION_READ, CD_STATE_READ);
            if (gCdAsyncEnabled != 0) {
                gCdReadSectorCount = size >> CD_SECTOR_SHIFT;
                gCdReadBuffer = buf;
                gCdTickStep = CD_TICK_STATE_MACHINE;
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

    if (gCdAsyncEnabled == 0 && gCdSyncQueueMode == 0) {
        ((LoadFileNoArgsFn)FileResource__LoadFile)();
        self->flags |= CD_FLAG_LOAD_FILE_DONE;
        self->methods->setFlag(self);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && (self->buffer == NULL || self->freeGuard != 0)) {
            StartCdOperation(CD_OPERATION_LOAD_FILE, CD_STATE_SETLOC);
            gCdSavedSeekParam = gCdSeekParam;
            entry = FindCdFileEntry(name);
            gCdSeekParam = entry;
            if (entry == NULL) {
                return;
            }
            /* MATCHING: entry->size spelled at both uses; one local merges the two loads */
            sectorCount = entry->size >> CD_SECTOR_SHIFT;
            gCdReadSectorCount = sectorCount;
            if ((entry->size & (CD_SECTOR_SIZE - 1)) != 0) {
                gCdReadSectorCount = sectorCount + 1;
            }
            readSize = gCdReadSectorCount << CD_SECTOR_SHIFT;
            if (self->buffer == NULL) {
                buffer = BMemPMgrAlloc(readSize);
                if (buffer == NULL) {
                    self->methods->close(self);
                    return;
                }
                gCdReadBuffer = buffer;
                self->buffer = buffer;
            } else {
                gCdReadBuffer = self->buffer;
            }
            if (gCdAsyncEnabled != 0) {
                self->bufferSize = readSize;
                gCdTickStep = CD_TICK_LOAD_FILE;
            } else {
            /* MATCHING: a goto, not do-while, as in CdDriver__Read */
            retry:
                do {
                    CdControl(CdlSetloc, (u_char *)&gCdSeekParam->pos, 0);
                    do {
                        status = CdSync(0, 0);
                    } while (status == CdlNoIntr);
                } while (status == CdlDiskError);
                CdRead(gCdReadSectorCount, self->buffer, CdlModeSpeed);
                do {
                    status = CdReadSync(0, 0);
                } while (status > 0);
                if (status == -1) {
                    goto retry;
                }
                self->bufferSize = readSize;
                gCdRequestQueue->active = 1;
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
    node = gCdRequestQueue;
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
        } else if (gCdIdle != 0) {
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
            self->methods->setFlag(self);
            FreeCdRequestNode(node);
            if (gCdRequestQueue == NULL) {
                self->methods->stopService(self);
            }
        }
    }
    UnlockCd();
}
