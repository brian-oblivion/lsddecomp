/*
 * code_179d8_s -- the CD-ROM read driver's per-object methods: slots +0x44..
 * +0x58 and +0x68 of D_8006D4E8 (`tools/classtable.py D_8006D4E8`), the
 * class whose constructor is code_179d8_o and whose module-level half
 * (request queue, service pump, file table) is code_179d8_q/_r.
 *
 * Open/Close/Seek/Read/LoadFile share one shape. With the driver in plain
 * synchronous mode (gCdAsyncEnabled and D_8008A860 both 0) each forwards to
 * code_179d8_h.c's blocking OpenCdFile/CloseCdFile/GetCdFileSize/ReadCdFile
 * (LoadFile to the base class's Class6D430__AllocBuffer). Otherwise a call
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

/* A disc position in the shape of Psy-Q's CdlLOC, declared as two s16 so the
 * struct is 2-aligned and a whole-struct copy compiles to lwl/lwr + swl/swr
 * (the idiom CLAUDE.md documents). The halves are never read apart here.
 * Same shape and name as code_179d8_q.c's own local CdLoc16. */
typedef struct CdLoc16 {
    s16 unk0;
    s16 unk2;
} CdLoc16;

typedef struct Class6D4E8 Class6D4E8;

/* D_8006D4E8, this class's own method table, down to the last slot this
 * unit dispatches. Each slot is named for the method `tools/classtable.py
 * D_8006D4E8` resolves it to. */
typedef struct Class6D4E8Methods {
    u8 pad0[0x44];
    /* +0x44 */ s32 (*open)(Class6D4E8 *self, void *name, s32 arg2, s32 arg3); /* Class6D4E8__Open */
    /* +0x48 */ s32 (*close)(Class6D4E8 *self);                    /* Class6D4E8__Close */
    /* +0x4C */ s32 (*seek)(Class6D4E8 *self, s32 offset, s32 mode); /* Class6D4E8__Seek */
    u8 pad50[0x54 - 0x50];                                         /* Class6D4E8__NoOpSlot50 */
    /* +0x54 */ s32 (*read)(Class6D4E8 *self, s32 buf, s32 size);  /* Class6D4E8__Read */
    /* +0x58 */ s32 (*loadFile)(Class6D4E8 *self, void *name);     /* Class6D4E8__LoadFile */
    u8 pad5C[0x64 - 0x5C];
    /* +0x64 */ s32 (*setFlag)(Class6D4E8 *self);                  /* Class6D430__SetFlag */
    u8 pad68[0x70 - 0x68];
    /* +0x70 */ s32 (*stopCdService)(Class6D4E8 *self);            /* Class6D4E8__StopCdService */
} Class6D4E8Methods;

/* An instance of the D_8006D4E8 class, this unit's own local view. The same
 * object is ObjA34_179D8H to code_179d8_h.c's synchronous OpenCdFile/
 * CloseCdFile/GetCdFileSize/ReadCdFile, Obj6D4E8 to its constructor
 * (code_179d8_o.c), three Obj6D4E8_* views in code_179d8_q.c, and -- the
 * class derives from D_8006D430, whose ctor it chains -- Class6D430 in
 * include/code_171e0.h. Field names agree with those views where they
 * name the field. */
struct Class6D4E8 {
    /* +0x00 */ Class6D4E8Methods *methods;
    u8 pad4[0xC - 0x4];
    /* +0x0C */ s32 isOpen;          /* 1 after Open resolves the file, 0 after Close */
    /* +0x10 */ void *buffer;        /* LoadFile's destination: BMemPMgrAlloc'd if NULL */
    /* +0x14 */ u32 bufferSize;      /* LoadFile's sector-rounded read size */
    /* +0x18 */ CdLoc16 pos;         /* the open file's disc position */
    /* +0x1C */ u32 size;            /* the open file's byte size */
    /* +0x20 */ u16 freeGuard;       /* nonzero: LoadFile reuses an existing buffer */
    /* +0x22 */ u16 pendingRequests; /* ++ per EnqueueCdRequest, -- per completion */
    /* +0x24 */ s32 flags;           /* CD_FLAG_* completion bits, see below */
    /* +0x28 */ u16 inQueueDispatch; /* 1 only while RunRequestQueue calls a slot */
};

/* Request op codes, carried in a queue node's `op` (EnqueueCdRequest's third
 * argument) and dispatched back by Class6D4E8__RunRequestQueue to the slot of
 * the same name. CD_OP_LOAD_FILE has the same spelling in code_179d8_q.c. */
#define CD_OP_OPEN      2
#define CD_OP_CLOSE     3
#define CD_OP_SEEK      4
#define CD_OP_READ      5
#define CD_OP_LOAD_FILE 7

/* Bits Class6D4E8__RunRequestQueue ORs into `flags` when a request completes.
 * Bit 0 (1) is left a literal: it is also Class6D430__SetFlag's bit, and the
 * queue node field that sets it here (`unk4`) has no established meaning. */
#define CD_FLAG_DONE           0x002 /* some request completed */
#define CD_FLAG_NONE_PENDING   0x004 /* ... and pendingRequests reached 0 */
#define CD_FLAG_OPEN_DONE      0x010
#define CD_FLAG_CLOSE_DONE     0x020
#define CD_FLAG_SEEK_DONE      0x040
#define CD_FLAG_READ_DONE      0x080
#define CD_FLAG_LOAD_FILE_DONE 0x200

/* gCdState values StartCdOperation's second argument sets
 * (code_179d8_r.c): 1 issues a CdlSetloc seek, 7 issues a CdRead. */
#define CD_STATE_IDLE   0
#define CD_STATE_SETLOC 1
#define CD_STATE_READ   7

/* gCdTickStep values: which of code_179d8_r.c's two state machines
 * ServiceCdDriver ticks. */
#define CD_TICK_STATE_MACHINE 1 /* TickCdStateMachine */
#define CD_TICK_LOAD_FILE     2 /* TickCdLoadFileStateMachine */

/* Psy-Q libcd values, spelled locally as code_179d8_q.c/_r.c do: CdlSetloc
 * (command 2), CdlModeSpeed (0x80, double speed) and CdSync's CdlDiskError
 * (5). */
#define CD_CMD_SETLOC        2
#define CD_MODE_DOUBLE_SPEED 0x80
#define CD_SYNC_DISK_ERROR   5

extern s32 gCdAsyncEnabled;
extern s32 D_8008A860;
extern s32 gCdBusy;

extern void CloseCdFile(Class6D4E8 *self);
extern void LockCd(void);
/* StartCdOperation(op, state): op is the value GetCdOperation later reports
 * (0 Close, 1 Open, 2 Seek, 3 Read, 4 LoadFile, each used by exactly one
 * method below and left literal), state is the first CD_STATE_*. */
extern void StartCdOperation(s32 op, s32 state);
extern void ResetCdStateMachine(void);
extern void EnqueueCdRequest(Class6D4E8 *arg0, s32 arg1, s32 arg2, s32 arg3,
                           s32 arg4);
extern void UnlockCd(void);

/* One 0x1C-byte record of the file table at gFileTable, which
 * FindCdFileEntry/GetCdFileEntry (code_179d8_r.c) return: the same layout
 * and name as code_179d8_q.c's own local CdFileEntry. */
typedef struct CdFileEntry {
    /* +0x00 */ char name[0x14];
    /* +0x14 */ CdLoc16 pos;
    /* +0x18 */ u32 size;
} CdFileEntry;

extern void *FindCdFileEntry(char *arg0);
extern s32 FindCdFileIndex(char *arg0);
extern void *gCdSeekParam;
extern s32 gCdTickStep;

extern void OpenCdFile(Class6D4E8 *self, char *suffix);
extern char *BuildCdFilePath(char *dest, char *suffix);
extern s32 CdSearchFile(void *statBuf, char *path);
extern void CdControl(s32 arg0, void *buf, s32 arg2);
extern s32 CdSync(s32 mode, void *result);

/* CdSearchFile's output buffer (Sony's CdlFILE, 0x18 bytes); the same name
 * as code_179d8_q.c's own local view. Only the two fields copied out are
 * typed. */
typedef struct CdFileInfo {
    /* +0x00 */ CdLoc16 pos;
    /* +0x04 */ u32 size;
    u8 pad8[0x18 - 8];
} CdFileInfo;

void Class6D4E8__Open(Class6D4E8 *self, char *name, s32 arg2, s32 arg3) {
    char path[0x40];
    CdFileInfo statBuf;
    CdFileEntry *rec;
    s32 size;
    s32 v0;

    if (gCdAsyncEnabled == 0 && D_8008A860 == 0) {
        OpenCdFile(self, name);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && self->isOpen == 0) {
            StartCdOperation(1, CD_STATE_SETLOC);
            if (gCdAsyncEnabled != 0) {
                rec = FindCdFileEntry(name);
                gCdSeekParam = rec;
                if (rec == NULL) {
                    return;
                }
                self->pos = rec->pos;
                size = ((CdFileEntry *)gCdSeekParam)->size;
                gCdTickStep = CD_TICK_STATE_MACHINE;
                self->isOpen = 1;
                self->size = size;
            } else {
                BuildCdFilePath(path, name);
                do {
                } while (CdSearchFile(&statBuf, path) == 0);
                self->pos = statBuf.pos;
                self->size = statBuf.size;
                do {
                    CdControl(CD_CMD_SETLOC, &self->pos, 0);
                    do {
                        v0 = CdSync(0, 0);
                    } while (v0 == 0);
                } while (v0 == CD_SYNC_DISK_ERROR);
                self->isOpen = 1;
                ResetCdStateMachine();
            }
        }
    } else {
        EnqueueCdRequest(self, FindCdFileIndex(name), CD_OP_OPEN, arg2, arg3);
    }
    UnlockCd();
}

void Class6D4E8__Close(Class6D4E8 *self) {
    if (gCdAsyncEnabled == 0 && D_8008A860 == 0) {
        CloseCdFile(self);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0) {
            StartCdOperation(0, CD_STATE_IDLE);
            self->isOpen = 0;
            ResetCdStateMachine();
        }
    } else {
        EnqueueCdRequest(self, 0, CD_OP_CLOSE, 0, 0);
    }
    UnlockCd();
}

extern u8 gCdSeekLoc[8];
extern void *gCdSeekParam;
extern s32 gCdTickStep;

extern s32 GetCdFileSize(Class6D4E8 *self);
extern s32 CdPosToInt(void *pos);
extern void CdIntToPos(s32 i, void *pos);
extern void CdControl(s32 arg0, void *buf, s32 arg2);
extern s32 CdSync(s32 mode, void *result);

s32 Class6D4E8__Seek(Class6D4E8 *self, u32 offset, s32 mode) {
    s32 v0;
    u32 sectors;

    if (gCdAsyncEnabled == 0 && D_8008A860 == 0) {
        return GetCdFileSize(self);
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && self->isOpen != 0) {
            StartCdOperation(2, CD_STATE_SETLOC);
            sectors = offset >> 11;
            if ((offset & 0x7FF) != 0) {
                sectors = sectors + 1;
            }
            v0 = CdPosToInt(&self->pos);
            CdIntToPos(v0 + sectors, gCdSeekLoc);
            if (mode == 0) {
                if (gCdAsyncEnabled != 0) {
                    /* the state machine seeks to gCdSeekParam + 0x14 (a
                     * CdFileEntry's pos), so point it 0x14 before the loc */
                    gCdSeekParam = gCdSeekLoc - 0x14;
                    gCdTickStep = CD_TICK_STATE_MACHINE;
                } else {
                    do {
                        CdControl(CD_CMD_SETLOC, gCdSeekLoc, 0);
                        do {
                            v0 = CdSync(0, 0);
                        } while (v0 == 0);
                    } while (v0 == CD_SYNC_DISK_ERROR);
                    ResetCdStateMachine();
                }
            } else {
                ResetCdStateMachine();
                UnlockCd();
                if ((self->size & 0x7FF) != 0) {
                    return ((self->size >> 11) + 1) << 11;
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

void Class6D4E8__NoOpSlot50(void) {
}

extern s32 gCdReadSectorCount; /* CdRead sector count */
extern void *gCdReadBuffer; /* CdRead target buffer */
extern s32 gCdTickStep;

extern void ReadCdFile(Class6D4E8 *self, void *arg1, s32 arg2);
extern s32 CdRead(s32 sectors, void *buf, s32 mode);
extern s32 CdReadSync(s32 mode, s32 result);
extern void ResetCdStateMachine(void);

s32 Class6D4E8__Read(Class6D4E8 *self, void *buf, u32 size) {
    s32 v1;

    if (gCdAsyncEnabled == 0 && D_8008A860 == 0) {
        ReadCdFile(self, buf, size);
        return 0;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && self->isOpen != 0) {
            StartCdOperation(3, CD_STATE_READ);
            if (gCdAsyncEnabled != 0) {
                gCdReadSectorCount = size >> 11;
                gCdReadBuffer = buf;
                gCdTickStep = CD_TICK_STATE_MACHINE;
            } else {
            retry:
                CdRead(size >> 11, buf, CD_MODE_DOUBLE_SPEED);
                do {
                    v1 = CdReadSync(0, 0);
                } while (v1 > 0);
                if (v1 == -1) {
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

extern void Class6D430__AllocBuffer(void); /* arity-ok: the definition takes (Class6D430 *this, s32 arg1) and reads both, but Class6D4E8__LoadFile passes NEITHER -- retail's jal at 0x80027834 has a bare nop delay slot and leaves its own incoming $a0/$a1 in place */
extern void *gCdSavedSeekParam;

/* A gCdRequestQueue node, 0x24 bytes: this unit's own local view of
 * code_179d8_r.c's CdRequestNode, down to the last field read here (the
 * list links sit past it). Field names are the ones code_179d8_q.c's
 * EnqueueCdRequest writes and code_179d8_r.c's StartCdOperation sets. */
typedef struct CdRequestNode {
    /* +0x00 */ s32 active;         /* set by StartCdOperation when the op starts */
    /* +0x04 */ s32 unk4;           /* zeroed at allocation; nonzero ORs flags bit 0 */
    /* +0x08 */ s32 op;             /* CD_OP_* */
    /* +0x0C */ Class6D4E8 *owner;  /* the requesting object */
    /* +0x10 */ s32 fileIndex;      /* FindCdFileIndex's index, 0 if none */
    /* +0x14 */ s32 param0;
    /* +0x18 */ s32 param1;
} CdRequestNode;

extern CdRequestNode *gCdRequestQueue;
extern void *BMemPMgrAlloc(s32 size);

void Class6D4E8__LoadFile(Class6D4E8 *self, char *name) {
    CdFileEntry *rec;
    s32 sectorCount;
    s32 readSize;
    void *ret;
    s32 v1;

    if (gCdAsyncEnabled == 0 && D_8008A860 == 0) {
        Class6D430__AllocBuffer();
        self->flags |= CD_FLAG_LOAD_FILE_DONE;
        self->methods->setFlag(self);
        return;
    }
    LockCd();
    if (self->inQueueDispatch != 0) {
        if (gCdBusy == 0 && (self->buffer == NULL || self->freeGuard != 0)) {
            StartCdOperation(4, CD_STATE_SETLOC);
            gCdSavedSeekParam = gCdSeekParam;
            rec = FindCdFileEntry(name);
            gCdSeekParam = rec;
            if (rec == NULL) {
                return;
            }
            {
                sectorCount = rec->size >> 11;
                gCdReadSectorCount = sectorCount;
                if ((rec->size & 0x7FF) != 0) {
                    gCdReadSectorCount = sectorCount + 1;
                }
                readSize = gCdReadSectorCount << 11;
                if (self->buffer == NULL) {
                    ret = BMemPMgrAlloc(readSize);
                    if (ret == NULL) {
                        self->methods->close(self);
                        return;
                    }
                    gCdReadBuffer = ret;
                    self->buffer = ret;
                } else {
                    gCdReadBuffer = self->buffer;
                }
                if (gCdAsyncEnabled != 0) {
                    self->bufferSize = readSize;
                    gCdTickStep = CD_TICK_LOAD_FILE;
                } else {
                retry:
                    do {
                        CdControl(CD_CMD_SETLOC, (u8 *)gCdSeekParam + 0x14, 0);
                        do {
                            v1 = CdSync(0, 0);
                        } while (v1 == 0);
                    } while (v1 == CD_SYNC_DISK_ERROR);
                    CdRead(gCdReadSectorCount, self->buffer, CD_MODE_DOUBLE_SPEED);
                    do {
                        v1 = CdReadSync(0, 0);
                    } while (v1 > 0);
                    if (v1 == -1) {
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

extern s32 gCdIdle; /* "idle"/"ready" flag, 0/1 */
extern void FreeCdRequestNode(CdRequestNode *node);
extern void *GetCdFileEntry(s32 index);

void Class6D4E8__RunRequestQueue(void) {
    CdRequestNode *node;
    Class6D4E8 *self;
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
                self->methods->open(self, GetCdFileEntry(node->fileIndex),
                                  node->param0, node->param1);
                break;
            case CD_OP_CLOSE:
                self->methods->close(self);
                break;
            case CD_OP_SEEK:
                self->methods->seek(self, node->param0, node->param1);
                break;
            case CD_OP_READ:
                self->methods->read(self, node->param0, node->param1);
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
                self->methods->stopCdService(self);
            }
        }
    }
    UnlockCd();
}
