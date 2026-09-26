/*
 * code_179d8_q -- the CD-ROM read driver.
 *
 * This unit is the module-level half of the class whose method table is
 * gCdDriverMethods (header word 0x13; the object itself and its read/seek/close
 * methods are code_179d8_s, its constructor code_179d8_o). It owns four
 * things, all of them singleton state in .sdata:
 *
 *   - the file table: an array of 0x1C-byte CdFileEntry records (name, disc
 *     position, size) at gFileTable/gFileTableCount, whose positions
 *     ResolveFileEntries fills in with CdSearchFile;
 *   - the driver mode: gCdAsyncEnabled and gCdUseVSyncCallback, set through
 *     SetCdDriverMode, which decide whether a request is queued and serviced
 *     in the background or performed by a blocking CdSync spin;
 *   - the request queue's front door: EnqueueCdRequest appends a node to the
 *     gCdRequestQueue list (code_179d8_r owns the list itself) and starts the
 *     service;
 *   - the service pump: ServiceCdDriver, installed as a VSyncCallback (or as
 *     the DrawSystem singleton's callback, its setCallback slot +0x084), which ticks
 *     code_179d8_r's CD state machine and drains the request queue, plus
 *     LockCd/UnlockCd, the latch that keeps that tick out of a half-updated
 *     queue.
 *
 * code_171e0.c reaches all of this through wrappers gated on
 * `gActiveDataSource == 0x13`, this class's header word; the other value that gate
 * takes, 0x23, selects the SPU/VAB streamer in code_179d8_e.c. So the two
 * are interchangeable data sources behind one small dispatch layer.
 */
#include "common.h"
#include "CdDriver.h"
#include "DrawSystem.h"

/* CdDriver, its table and its methods are include/CdDriver.h's (track 4,
 * round 88): the per-call-site views this unit declared (Obj6D4E8_C80,
 * Obj6D4E8_D70, Obj6D4E8_282AC, and the table views Methods6D4E8_C80 /
 * Methods6D4E8_80EC) were that one class. */

/* +0x04 of whatever object a still-uninitialized local $s2 points at on this
 * path -- see the CdDriver__RequestLoadFile report for why that local is
 * never assigned. Only the one field this store touches is typed, and the
 * object's identity is unknowable from here, so the name stays a
 * placeholder. */
typedef struct UnkC80 UnkC80;
struct UnkC80 {
    u8 pad00[0x04];
    /* +0x04 */ s32 unk04;
};

extern void EnqueueCdRequest(CdDriver *owner, s32 fileIndex,
                             s32 op, s32 param0, s32 param1);
extern s32 FindCdFileIndex(char *name); /* code_179d8_r: name -> table index */

void CdDriver__RequestLoadFile(CdDriver *self, char *name)
{
    UnkC80 *s2;
    s32 idx;

    LockCd();

    if (name != NULL) {
        if (gCdAsyncEnabled != 0) {
            s2->unk04 = 1;
            idx = FindCdFileIndex(name);
            EnqueueCdRequest(self, idx,
                             CD_OP_LOAD_FILE, 0, 0);
        } else {
            self->methods->loadFile(self, name);

            if (self->pendingRequests == 0) {
                self->flags |= 4;
            }
        }
    }

    UnlockCd();
}

extern void LockCd(void);
extern void UnlockCd(void);
extern void StopCdServiceIfIdle(void);

void CdDriver__StopService(void)
{
    LockCd();
    StopCdServiceIfIdle();
    UnlockCd();
}

extern void CdFlush(void);
extern void ResetCdStateMachine(void); /* code_179d8_r: reset the state machine */
extern void FreeCdRequestNode(CdRequestNode *req); /* code_179d8_r: unlink+free */

void CdDriver__CancelRequests(CdDriver *self)
{
    CdRequestNode *entry;
    CdRequestNode *node;
    CdRequestNode *next;
    CdFileEntry *saved;

    LockCd();

    entry = gCdRequestQueue;

    if (entry != NULL && self->pendingRequests != 0) {
        self->flags = 0;

        if (entry->owner == self && entry->active != 0 && gCdIdle == 0) {
            CdFlush();
            ResetCdStateMachine();
            saved = gCdSavedSeekParam;
            gCdSavedSeekParam = NULL;
            gCdSeekParam = saved;
        }

        for (node = gCdRequestQueue; node != NULL; node = next) {
            next = node->next;
            if (node->owner == self) {
                FreeCdRequestNode(node);
                self->pendingRequests--;
            }
        }
    }

    UnlockCd();
}

/* The class's own table getter (include/CdDriver.h) -- an address-of, not
 * gp_rel: gCdDriverMethods lives in .data, not .sdata. */
CdDriverMethods *GetCdDriverMethods(void)
{
    return &gCdDriverMethods;
}

/* libcd/sys entry points (lib/libcd/sys.o, linked since round 34) --
 * per-call-site typed for this unit, per the code_179d8_h.c convention.
 * The two constants are Psy-Q's own (include/psyq/LIBCD.H: CdlSetmode 0x0E,
 * CdlModeSpeed 0x80 = double speed); they are spelled locally rather than by
 * including LIBCD.H, because this unit's libcd declarations are deliberately
 * per-call-site and Sony's prototypes would conflict with them. */
extern s32 CdSetDebug(s32 level);
extern s32 CdControlB(u_char com, void *param, void *result);

#define CD_CMD_SETMODE 0x0E
#define CD_MODE_DOUBLE_SPEED 0x80

extern s32 sCdDriveInited;

void InitCdDrive(void)
{
    u8 mode;

    if (sCdDriveInited != 0) {
        return;
    }

    CdSetDebug(0);
    mode = CD_MODE_DOUBLE_SPEED;
    while (CdControlB(CD_CMD_SETMODE, &mode, 0) == 0) {
    }
    sCdDriveInited = 1;
}


s32 IsCdBusy(void)
{
    return gCdBusy;
}


s32 IsCdIdle(void)
{
    return gCdIdle;
}


s32 GetCdOperation(void)
{
    return gCdOperation;
}


s32 GetCdState(void)
{
    return gCdState;
}

/* D_8008A860 keeps its placeholder name: it is written only by
 * SetCdDriverMode's second argument and read back only here and in
 * code_179d8_s, where every read is `gCdAsyncEnabled == 0 && D_8008A860 == 0`
 * -- i.e. "neither mode is on, take the plain synchronous path". Nothing
 * establishes what the second mode IS, so nothing here names it. */

s32 GetCdDriverMode(s32 *outMode2)
{
    if (outMode2 != NULL) {
        *outMode2 = D_8008A860;
    }
    return gCdAsyncEnabled;
}

extern s32 ServiceCdDriver(void);

s32 SetCdDriverMode(s32 async, s32 mode2, s32 useVSyncCallback)
{
    DrawSystem *obj;

    if (gCdBusy == 0) {
        if (useVSyncCallback == 0) {
            obj = GetDrawSystem();

            if (gCdAsyncEnabled == 0) {
                if (async != 0) {
                    obj->methods->setCallback(obj, (void (*)(void))ServiceCdDriver);
                }
            } else {
                if (async == 0) {
                    obj->methods->setCallback(obj, 0);
                }
            }
        }

        gCdUseVSyncCallback = useVSyncCallback;
        gCdAsyncEnabled = async;
        D_8008A860 = mode2;

        return 1;
    }

    return 0;
}


void SetFileTable(CdFileEntry *table)
{
    gFileTable = table;
}


void SetFileTableCount(s32 count)
{
    gFileTableCount = count;
}


s32 GetFileTableCount(void)
{
    return gFileTableCount;
}

/* CdSearchFile's output buffer, which is Sony's CdlFILE: pos, size, name[16]
 * = 0x18 bytes (include/psyq/LIBCD.H). The 0x18 was derived here
 * independently, from the span between this local's stack slot (sp+0x50) and
 * the next saved register (sp+0x68), and it is the same figure
 * code_179d8_h.c's OpenCdFile derived for the same Sony function. Only
 * the two fields this call site copies out are typed. */
typedef struct CdFileInfo CdFileInfo;
struct CdFileInfo {
    CdLoc16 pos;
    u32 size;
    u8 pad8[0x18 - 0x8];
};

extern const char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */
extern s32 CdSearchFile(CdFileInfo *fileInfo, char *path); /* libcd/iso9660.o */
extern void printf(const char *fmt, void *arg1);
extern char *BuildCdFilePath(char *dest, char *suffix); /* code_179d8_r */
extern void InitCdDrive(void);

#define CD_SEARCH_RETRIES 0x65

s32 ResolveFileEntries(CdFileEntry *entries, s32 count)
{
    CdFileEntry *end;
    char path[0x40];
    CdFileInfo info;
    s32 tries;

    end = entries + count;

    InitCdDrive();

    for (; entries < end; entries++) {
        BuildCdFilePath(path, entries->name);

        for (tries = 0; tries < CD_SEARCH_RETRIES; tries++) {
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
 * below: the service tick returns immediately while gCdLock is set, so every
 * public entry point in this unit and in code_179d8_r/_s brackets its body
 * with LockCd()/UnlockCd() to keep the VSync-driven tick out of a
 * half-updated queue. Not a mutex -- nothing spins or blocks on it. */
extern s32 gCdLock;

void LockCd(void)
{
    gCdLock = 1;
}

extern s32 gCdLock;

void UnlockCd(void)
{
    gCdLock = 0;
}

extern s32 GetBMemPMgrBusy(void); /* code_8220_b */
extern void TickCdStateMachine(void); /* code_179d8_r: state-machine step 1 */
extern void TickCdLoadFileStateMachine(void); /* code_179d8_r: state-machine step 2 */
extern s32 gCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

s32 ServiceCdDriver(void)
{
    if (gCdLock != 0) {
        return 0;
    }

    if (GetBMemPMgrBusy() != 0) {
        return 0;
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback(0);
    }

    if (gCdTickStep == 1) {
        TickCdStateMachine();
    } else if (gCdTickStep == 2) {
        TickCdLoadFileStateMachine();
    }

    if (gCdQueueEnabled != 0) {
        GetCdDriverMethods()->runRequestQueue();
    }

    if (gCdUseVSyncCallback != 0) {
        VSyncCallback((void (*)(void))ServiceCdDriver);
    }

    return 0;
}

extern s32 gCdCallbackInstalled;

void StartCdService(void)
{
    LockCd();

    if (gCdCallbackInstalled == 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback((void (*)(void))ServiceCdDriver);
        }
        gCdCallbackInstalled = 1;
    }

    gCdQueueEnabled = 1;
    UnlockCd();
}

extern s32 gCdCallbackInstalled;
extern s32 gCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

void StopCdServiceIfIdle(void)
{
    LockCd();

    if (gCdTickStep == 0 && gCdCallbackInstalled != 0) {
        if (gCdUseVSyncCallback != 0) {
            VSyncCallback(0);
        }
        gCdCallbackInstalled = 0;
        gCdQueueEnabled = 0;
    }

    UnlockCd();
}

extern s32 gCdQueueEnabled;

void DisableCdQueue(void)
{
    LockCd();
    gCdQueueEnabled = 0;
    UnlockCd();
}

/* The same 0x24-byte queue node CdRequestNode above is a view of, from the
 * writing side: AllocCdRequestNode (code_179d8_r) allocates one and links it onto
 * gCdRequestQueue, and only the fields this call site writes are typed here
 * (padded to their offsets, per this unit's convention). `op` takes the
 * CD_OP_* values, `fileIndex` is FindCdFileIndex's index into gFileTable (0
 * when the op does not name a file), and param0/param1 are the two per-op
 * arguments code_179d8_s passes through: a byte count and a flag for op 4, a
 * buffer and a size for op 5. */
typedef struct CdRequest_282AC CdRequest_282AC;
struct CdRequest_282AC {
    u8 pad00[0x08];
    /* +0x08 */ s32 op;
    /* +0x0C */ s32 owner;
    /* +0x10 */ s32 fileIndex;
    /* +0x14 */ s32 param0;
    /* +0x18 */ s32 param1;
};
extern CdRequest_282AC *AllocCdRequestNode(void); /* code_179d8_r: alloc + link */

/* The store order below is retail's own (+0x08, +0x14, +0x0C, +0x10, +0x18),
 * not ascending offset -- see the match report: this compiler keeps
 * statement order for these, so the statements are in retail's order. */
void EnqueueCdRequest(CdDriver *owner, s32 fileIndex, s32 op,
                      s32 param0, s32 param1)
{
    CdRequest_282AC *entry = AllocCdRequestNode();

    entry->op = op;
    entry->param0 = param0;
    entry->owner = (s32)owner;
    entry->fileIndex = fileIndex;
    entry->param1 = param1;

    owner->pendingRequests++;
    owner->flags = 0;
    StartCdService();
}
