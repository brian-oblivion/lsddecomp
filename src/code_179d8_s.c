/*
 * code_179d8_s -- the CD-ROM read driver's per-object methods: slots +0x44..
 * +0x58 and +0x68 of gCdDriverMethods (`tools/classtable.py gCdDriverMethods`), the
 * class whose constructor is code_179d8_o and whose module-level half
 * (request queue, service pump, file table) is code_179d8_q/_r.
 *
 * Open/Close/Seek/Read/LoadFile share one shape. With the driver in plain
 * synchronous mode (gCdAsyncEnabled and gCdSyncQueueMode both 0) each forwards to
 * code_179d8_h.c's blocking OpenCdFile/CloseCdFile/GetCdFileSize/ReadCdFile
 * (LoadFile to the base class's FileResource__LoadFile). Otherwise a call
 * from outside the queue only enqueues a CD_OP_* request; when
 * RunRequestQueue (slot +0x68, ticked by ServiceCdDriver) dispatches that
 * request back through the same slot with inQueueDispatch set, the method
 * starts the operation: on code_179d8_r's state machine if async, or by a
 * CdControl/CdSync/CdRead spin if not. RunRequestQueue then reports the
 * finished request through `flags` (CD_FLAG_*) and frees its node. Slot
 * +0x50 (NoOpSlot50) is empty. RunRequestQueue's two switches own this
 * unit's only jump tables, jtbl_80010810/jtbl_80010828 (carved round 47).
 */
#include "common.h"
#include <libcd.h>
#include "CdDriver.h"

/* `pos` (self->pos, CdFileEntry::pos) is FileResource.h's CdLoc16, which is
 * Sony's CdlLOC by layout; the casts to CdlLOC / u_char * below go away when
 * that header can take <libcd.h> and use CdlLOC itself. */

/* CdDriver, its table and its methods are include/CdDriver.h's (track 4,
 * round 88); the object's fields are all FileResource's, because the driver
 * runs on its clients' objects (CdDriver.h's banner). */


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
    s32 size;
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
                size = ((CdFileEntry *)gCdSeekParam)->size;
                gCdTickStep = CD_TICK_STATE_MACHINE;
                self->isOpen = 1;
                self->size = size;
            } else {
                BuildCdFilePath(path, name);
                do {
                } while (CdSearchFile(&statBuf, path) == 0);
                self->pos = *(CdLoc16 *)&statBuf.pos;
                self->size = statBuf.size;
                do {
                    CdControl(CdlSetloc, (u_char *)&self->pos, 0);
                    do {
                        status = CdSync(0, 0);
                    } while (status == 0);
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
                    /* the state machine seeks to gCdSeekParam + 0x14 (a
                     * CdFileEntry's pos), so point it 0x14 before the loc */
                    gCdSeekParam = (CdFileEntry *)((u8 *)&gCdSeekLoc - offsetof(CdFileEntry, pos));
                    gCdTickStep = CD_TICK_STATE_MACHINE;
                } else {
                    do {
                        CdControl(CdlSetloc, (u_char *)&gCdSeekLoc, 0);
                        do {
                            status = CdSync(0, 0);
                        } while (status == 0);
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

/* FileResource__LoadFile (include/FileResource.h) takes (self, name) and reads
 * both, but CdDriver__LoadFile passes NEITHER -- retail's jal at 0x80027834
 * has a bare nop delay slot and leaves its own incoming $a0/$a1 in place.
 * The call goes through this typedef (a cast of a function address, no
 * code); it was a conflicting local `extern void FileResource__LoadFile(void)`
 * until round 88. */
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
            {
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
                retry:
                    do {
                        CdControl(CdlSetloc, (u_char *)&gCdSeekParam->pos, 0);
                        do {
                            status = CdSync(0, 0);
                        } while (status == 0);
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
        }
    } else {
        EnqueueCdRequest(self, FindCdFileIndex(name), CD_OP_LOAD_FILE, 0, 0);
    }
    UnlockCd();
}

extern void FreeCdRequestNode(CdRequestNode *node);
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
