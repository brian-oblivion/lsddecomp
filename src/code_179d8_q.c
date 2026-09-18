/*
 * code_179d8_q -- the CD-ROM read driver.
 *
 * This unit is the module-level half of the class whose method table is
 * D_8006D4E8 (header word 0x13; the object itself and its read/seek/close
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
 *     D_8008A894 list (code_179d8_r owns the list itself) and starts the
 *     service;
 *   - the service pump: ServiceCdDriver, installed as a VSyncCallback (or as
 *     a callback on the singleton func_80020C5C returns), which ticks
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

/* --- local views of the D_8006D4E8 class ---------------------------------
 * This unit defines three of that class's own method slots (+0x06C, +0x070,
 * +0x074) and sees its objects through three per-call-site views that differ
 * only in which fields they type. They are the same struct; merging them is
 * track-4 work rather than a rename, so they stay split. The `_<addr>`
 * suffix names the function each view was read from -- the convention this
 * unit already used.
 * ------------------------------------------------------------------------ */

/* The class's method table down to +0x058: the one slot
 * Class6D4E8__RequestLoadFile dispatches. `tools/classtable.py D_8006D4E8`
 * resolves that slot to func_80027800 (code_179d8_s), which loads a named
 * file off the disc, so the slot is named for the method it dispatches to.
 * The sibling class D_8006D430 (include/code_171e0.h's
 * Class6D430Methods) leaves the identical offset unnamed -- this
 * stays an independent local view, per the project's multiple-local-views
 * convention, rather than an edit to that shared header. */
typedef struct Methods6D4E8_C80 Methods6D4E8_C80;
struct Methods6D4E8_C80 {
    u8 pad00[0x58];
    /* +0x58 */ void (*loadFile)(void *self, char *name);
};

typedef struct Obj6D4E8_C80 Obj6D4E8_C80;
struct Obj6D4E8_C80 {
    /* +0x00 */ Methods6D4E8_C80 *methods;
    u8 pad04[0x22 - 0x04];
    /* +0x22 */ u16 pendingRequests; /* ++ per queued request, -- per cancel */
    /* +0x24 */ s32 flags;           /* OR-ed bit set; no bit is read here */
};

/* +0x04 of whatever object a still-uninitialized local $s2 points at on this
 * path -- see the Class6D4E8__RequestLoadFile report for why that local is
 * never assigned. Only the one field this store touches is typed, and the
 * object's identity is unknowable from here, so the name stays a
 * placeholder. */
typedef struct UnkC80 UnkC80;
struct UnkC80 {
    u8 pad00[0x04];
    /* +0x04 */ s32 unk04;
};

/* The request op codes are a small enumeration shared with code_179d8_s,
 * which enqueues 2 (open by name), 3 (close), 4 (seek) and 5 (read) from the
 * class's other slots. Only the one this unit itself uses is named. */
#define CD_OP_LOAD_FILE 7

struct Obj6D4E8_282AC;
extern void EnqueueCdRequest(struct Obj6D4E8_282AC *owner, s32 fileIndex,
                             s32 op, s32 param0, s32 param1);
extern s32 FindCdFileIndex(char *name); /* code_179d8_r: name -> table index */
extern s32 gCdAsyncEnabled;

void Class6D4E8__RequestLoadFile(Obj6D4E8_C80 *self, char *name)
{
    UnkC80 *s2;
    s32 idx;

    LockCd();

    if (name != NULL) {
        if (gCdAsyncEnabled != 0) {
            s2->unk04 = 1;
            idx = FindCdFileIndex(name);
            EnqueueCdRequest((struct Obj6D4E8_282AC *)self, idx,
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

void Class6D4E8__StopCdService(void)
{
    LockCd();
    StopCdServiceIfIdle();
    UnlockCd();
}

/* The queued-request node AllocCdRequestNode (code_179d8_r) allocates and
 * FreeCdRequestNode (code_179d8_r) unlinks and frees -- only the fields this
 * call site itself reads are typed here. `active` is the flag StartCdOperation
 * (code_179d8_r) sets on the head node when it starts an operation on it;
 * AllocCdRequestNode clears it at allocation. The list head is D_8008A894. */
typedef struct CdRequest_D70 CdRequest_D70;
struct CdRequest_D70 {
    /* +0x00 */ s32 active;
    u8 pad04[0x0C - 0x04];
    /* +0x0C */ s32 owner;
    u8 pad10[0x20 - 0x10];
    /* +0x20 */ CdRequest_D70 *next;
};

typedef struct Obj6D4E8_D70 Obj6D4E8_D70;
struct Obj6D4E8_D70 {
    u8 pad00[0x22];
    /* +0x22 */ u16 pendingRequests;
    /* +0x24 */ s32 flags;
};

extern s32 D_8008A894;
extern s32 gCdIdle;
extern s32 D_8008A888;
extern s32 D_8008A87C;
extern void CdFlush(void);
extern void ResetCdStateMachine(void); /* code_179d8_r: reset the state machine */
extern void FreeCdRequestNode(CdRequest_D70 *req); /* code_179d8_r: unlink+free */

void Class6D4E8__CancelRequests(Obj6D4E8_D70 *self)
{
    CdRequest_D70 *entry;
    CdRequest_D70 *node;
    CdRequest_D70 *next;
    s32 saved;

    LockCd();

    entry = (CdRequest_D70 *)D_8008A894;

    if (entry != NULL && self->pendingRequests != 0) {
        self->flags = 0;

        if (entry->owner == (s32)self && entry->active != 0 && gCdIdle == 0) {
            CdFlush();
            ResetCdStateMachine();
            saved = D_8008A888;
            D_8008A888 = 0;
            D_8008A87C = saved;
        }

        for (node = (CdRequest_D70 *)D_8008A894; node != NULL; node = next) {
            next = node->next;
            if (node->owner == (s32)self) {
                FreeCdRequestNode(node);
                self->pendingRequests--;
            }
        }
    }

    UnlockCd();
}

/* D_8006D4E8's own method table, 29 slots per tools/classtable.py (header
 * word 0x13 at +0x000, DestroyChained at +0x004/own-slot, func_80027228 at
 * +0x008/ctor, func_80027274 at +0x00C/dtor, the 13 inherited BasicClass
 * slots at +0x010..+0x038, then own slots at +0x040..+0x074 -- this unit
 * defines Class6D4E8__RequestLoadFile (+0x06C), Class6D4E8__StopCdService
 * (+0x070) and Class6D4E8__CancelRequests (+0x074)). This function is this
 * class's "get my own method table" accessor, the same convention
 * GetClass6D3C8Methods uses for D_8006D3C8 and GetClass6D430Methods uses for D_8006D430
 * (see include/code_171e0.h) -- just an address-of, not gp_rel, since
 * D_8006D4E8 lives in .data, not .sdata. */
extern s32 D_8006D4E8[];

s32 *GetClass6D4E8Methods(void)
{
    return D_8006D4E8;
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

extern s32 gCdBusy;

s32 IsCdBusy(void)
{
    return gCdBusy;
}

extern s32 gCdIdle;

s32 IsCdIdle(void)
{
    return gCdIdle;
}

extern s32 gCdOperation;

s32 GetCdOperation(void)
{
    return gCdOperation;
}

extern s32 gCdState;

s32 GetCdState(void)
{
    return gCdState;
}

/* D_8008A860 keeps its placeholder name: it is written only by
 * SetCdDriverMode's second argument and read back only here and in
 * code_179d8_s, where every read is `gCdAsyncEnabled == 0 && D_8008A860 == 0`
 * -- i.e. "neither mode is on, take the plain synchronous path". Nothing
 * establishes what the second mode IS, so nothing here names it. */
extern s32 D_8008A860;
extern s32 gCdAsyncEnabled;

s32 GetCdDriverMode(s32 *outMode2)
{
    if (outMode2 != NULL) {
        *outMode2 = D_8008A860;
    }
    return gCdAsyncEnabled;
}

extern s32 func_80020C5C(void); /* returns D_8008A83C, a singleton object */
extern s32 ServiceCdDriver(void);
extern s32 gCdBusy;
extern s32 gCdAsyncEnabled;
extern s32 D_8008A860;
extern s32 gCdUseVSyncCallback;

/* The singleton func_80020C5C returns; only the slot this call site
 * dispatches (+0x84 of its method table) is typed here. That slot is handed
 * either ServiceCdDriver or 0, so it installs and clears a callback -- named
 * for what this one call site does with it, which is all the evidence
 * there is. */
typedef struct ObjF18Methods ObjF18Methods;
struct ObjF18Methods {
    u8 pad00[0x84];
    void (*setCallback)(void *self, void *cb);
};

typedef struct ObjF18 ObjF18;
struct ObjF18 {
    ObjF18Methods *methods;
};

s32 SetCdDriverMode(s32 async, s32 mode2, s32 useVSyncCallback)
{
    ObjF18 *obj;

    if (gCdBusy == 0) {
        if (useVSyncCallback == 0) {
            obj = (ObjF18 *)func_80020C5C();

            if (gCdAsyncEnabled == 0) {
                if (async != 0) {
                    obj->methods->setCallback(obj, (void *)ServiceCdDriver);
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

extern s32 gFileTable;

void SetFileTable(s32 table)
{
    gFileTable = table;
}

extern s32 gFileTableCount;

void SetFileTableCount(s32 count)
{
    gFileTableCount = count;
}

extern s32 gFileTableCount;

s32 GetFileTableCount(void)
{
    return gFileTableCount;
}

/* A disc position in the shape Psy-Q's CdlLOC has (minute/second/sector/
 * track), but declared as two s16 rather than four u8: the game's own struct
 * is 2-aligned, which is why a whole-struct assignment of it compiles to
 * lwl/lwr + swl/swr instead of a plain lw/sw (the idiom CLAUDE.md and
 * code_179d8_h.c document). The two halves are never read apart here, so
 * they keep placeholder names. Kept as this unit's own local view, the same
 * shape as code_179d8_h.c's Pair16_179D8H under a different name. */
typedef struct CdLoc16 CdLoc16;
struct CdLoc16 {
    s16 unk0;
    s16 unk2;
};

/* CdSearchFile's output buffer, which is Sony's CdlFILE: pos, size, name[16]
 * = 0x18 bytes (include/psyq/LIBCD.H). The 0x18 was derived here
 * independently, from the span between this local's stack slot (sp+0x50) and
 * the next saved register (sp+0x68), and it is the same figure
 * code_179d8_h.c's func_80028920 derived for the same Sony function. Only
 * the two fields this call site copies out are typed. */
typedef struct CdFileInfo CdFileInfo;
struct CdFileInfo {
    CdLoc16 pos;
    u32 size;
    u8 pad8[0x18 - 0x8];
};

/* One element of the file table: 0x1C bytes of {name, disc position, size}.
 * `name` is passed by its own address (offset 0) to func_800289CC, which
 * builds the full path from it; `pos` and `size` are then filled in from a
 * CdSearchFile lookup on that path, so an entry is a name resolved once and
 * reused as a seek target. gFileTable and gFileTableCount (this unit's
 * SetFileTable / SetFileTableCount) are the array's base and length --
 * FindCdFileIndex (code_179d8_r) walks the identical 0x1C stride over
 * gFileTable doing strstr() against `name`, confirming the layout
 * independently. */
typedef struct CdFileEntry CdFileEntry;
struct CdFileEntry {
    /* +0x00 */ char name[0x14];
    /* +0x14 */ CdLoc16 pos;
    /* +0x18 */ u32 size;
};

extern const char sFileNotFoundMsg[]; /* "File not found. file = %s\n" */
extern s32 CdSearchFile(CdFileInfo *fileInfo, char *path); /* libcd/iso9660.o */
extern void printf(const char *fmt, void *arg1);
extern char *func_800289CC(char *dest, char *suffix); /* code_179d8_r */
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
        func_800289CC(path, entries->name);

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
extern s32 gCdUseVSyncCallback;
extern s32 D_8008A898;
extern void TickCdStateMachine(void); /* code_179d8_r: state-machine step 1 */
extern void TickCdLoadFileStateMachine(void); /* code_179d8_r: state-machine step 2 */
extern s32 gCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

/* The class's method table down to +0x068 (see GetClass6D4E8Methods's
 * class-map comment above); only the one slot this call site dispatches is
 * typed, following the pad-to-offset convention include/code_171e0.h uses
 * for D_8006D430's own table. tools/classtable.py resolves +0x068 to
 * func_80027A24 (code_179d8_s), which walks the D_8008A894 request list,
 * dispatches each request through its owner's own slots and frees it with
 * FreeCdRequestNode -- so the slot is named for what that method does. */
typedef struct Methods6D4E8_80EC Methods6D4E8_80EC;
struct Methods6D4E8_80EC {
    u8 pad00[0x68];
    void (*runRequestQueue)(void);
};

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

    if (D_8008A898 == 1) {
        TickCdStateMachine();
    } else if (D_8008A898 == 2) {
        TickCdLoadFileStateMachine();
    }

    if (gCdQueueEnabled != 0) {
        ((Methods6D4E8_80EC *)GetClass6D4E8Methods())->runRequestQueue();
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

extern s32 D_8008A898;
extern s32 gCdCallbackInstalled;
extern s32 gCdUseVSyncCallback;
extern s32 gCdQueueEnabled;
extern void VSyncCallback(void (*cb)(void));

void StopCdServiceIfIdle(void)
{
    LockCd();

    if (D_8008A898 == 0 && gCdCallbackInstalled != 0) {
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

/* The same 0x24-byte queue node CdRequest_D70 above is a view of, from the
 * writing side: AllocCdRequestNode (code_179d8_r) allocates one and links it onto
 * D_8008A894, and only the fields this call site writes are typed here
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

typedef struct Obj6D4E8_282AC Obj6D4E8_282AC;
struct Obj6D4E8_282AC {
    u8 pad00[0x22];
    /* +0x22 */ u16 pendingRequests;
    /* +0x24 */ s32 flags;
};

/* The store order below is retail's own (+0x08, +0x14, +0x0C, +0x10, +0x18),
 * not ascending offset -- see the match report: this compiler keeps
 * statement order for these, so the statements are in retail's order. */
void EnqueueCdRequest(Obj6D4E8_282AC *owner, s32 fileIndex, s32 op,
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
