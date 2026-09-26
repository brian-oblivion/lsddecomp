/*
 * code_179d8_e -- the SPU/VAB sound-streaming backend (`gActiveDataSource ==
 * 0x23`, confirmed round 52 against alpha's own naming of that global in
 * code_171e0.c) and its small mood/context-tagged sound-cue queue.
 *
 * functions 120..148 of the original 274-function code_179d8 monolith,
 * 0x1CC08..0x1D508 (vram 0x8002C408..0x8002CD08).  Carved round 17
 * (2026-09-04) out of what the yaml called `code_179d8_mid_b`; the remainder
 * behind it is now `code_179d8_mid_c`.
 *
 * Owns NO switch jump table -- zero `jtbl_` references anywhere in the slice
 * -- so no rodata sub-slot is attached to this unit.
 *
 * CLASS FRAMEWORK (established round 17, sharpened round 43 and round 52).
 * `tools/classtable.py --scan` hits two real class tables in this unit's own
 * globals: `gVabDriverMethods` (29 slots, header word 0x00000023) and
 * `gVabStreamObjMethods` (39 slots, header word 0x00000A03).  They share the
 * same BasicClass tail (slots +0x10..+0x38) -- two classes off the same
 * base, not one subclassing the other.
 *
 * `gVabDriverMethods`'s header word (0x23) is not a coincidence: it is
 * exactly the value `code_171e0.c`'s `gActiveDataSource` compares against to
 * select this backend (the other value, 0x13, selects the CD-ROM read
 * driver, `gVabDriverMethods`'s sibling `D_8006D4E8` in `code_179d8_q.c`).
 * `code_171e0.c`'s own `GetActiveDataSourceMethods` returns `GetVabDriverMethods()` (this
 * unit) exactly when `gActiveDataSource == 0x23`, and `VabStreamObj`'s own
 * constructor/close (below) chain their base-class calls through that
 * accessor's return.  ROUND 87 CORRECTION (track 4): that does not make
 * gVabDriverMethods VabStreamObj's base.  Both are Class6D430 subclasses
 * (ids 0x23 and 0xA03, parent 0x3); VabStreamObj chains to whichever driver
 * is active, as every data source does.  VabDriver is now declared once, in
 * `include/VabDriver.h`: its ctor/dtor (`code_179d8_d.c`) and the eleven
 * interface slots it overrides, six of them defined in this unit
 * (`VabDriver__Read`/`LoadFile`/`RunRequestQueue`/`RequestLoadFile`/
 * `StopService`/`CancelRequests`), are all empty no-ops.
 *
 * `gVabStreamObjMethods` is the real work: `VabStreamObj__VabStreamObj` is
 * its constructor, `VabStreamObj__Close` its close, `VabStreamObj__Update`
 * its per-frame poll (header/body transfer state machine), and
 * `VabStreamObj__LoadVagAttrs` its post-load VAB attribute-table fetch --
 * confirmed round 43 as a PS1 SPU/VAB sound-streaming object.
 * `VabStreamObj__OnBodyReady`'s and `FlushSoundCueSet`'s self objects load
 * `*self` (offset 0, the methods pointer) and dereference table slots at
 * exactly the offsets `classtable.py gVabStreamObjMethods` prints for
 * `VabStreamObj__LoadVagAttrs` (+0x7C) and `VabStreamObj__StopVoice`
 * (+0x84) -- direct confirmation this is genuine `self->methods->slotN(self,
 * ...)` dispatch, not driver code.  `gVabStreamObjMethods`'s whole vtable
 * was read straight out of `asm/data/5E140.data.s` while matching this
 * cluster -- see `docs/match-reports/VabStreamObj__VabStreamObj.md` for the
 * full slot table, including two slots (+0x58, +0x6C) that are null in
 * retail's own data.
 *
 * `InitSoundCueSet`/`FlushSoundCueSet` are unrelated free functions (NOT
 * `gVabStreamObjMethods` vtable slots -- checked, absent from its slot list)
 * operating on a small 3-slot `SoundCueSet` that `Entity`/`DreamSys`/
 * `class_3bb8c_n` embed and tag with their own context (round 52: Entity.c
 * passes `this->moodIndex + 1` as the tag).  `FlushSoundCueSet` dispatches
 * each populated slot's stored index through the ACTIVE stream object's own
 * `VabStreamObj__StopVoice` slot, so the queue is a backend-agnostic front
 * door onto whichever data source `gActiveDataSource` currently selects, not
 * something owned by `VabStreamObj` itself.
 *
 * `func_8002C478` and `GetVabDriverMode`/`SetVabDriverMode` are this
 * backend's own implementations of the same generic driver-mode interface
 * `code_171e0.c` dispatches on `gActiveDataSource` -- confirmed round 52 by
 * that unit's own substitution (`func_80026FAC`/`func_80026F34`/
 * `func_80026FE8` call `GetCdDriverMode`/`SetCdDriverMode`/`GetCdUseVSyncCallback`
 * when `gActiveDataSource == 0x13`, else these).  `func_8002C478` itself
 * stays unnamed: its only paired counterpart, `GetCdUseVSyncCallback`, is still
 * unnamed too, so there's nothing to name it AS a stand-in for.
 */
#include "common.h"
#include "VabDriver.h"

/* ------------------------------------------------------------------------
 * gVabStreamObjMethods's table (gVabDriverMethods is include/VabDriver.h's
 * since round 87).  Only the
 * slots and fields THIS unit's functions actually touch are named;
 * everything else is opaque padding, per this project's local-reading
 * convention (see code_179d8_d.c's Table6D940/Obj6D940 for the same idiom
 * applied to a neighbouring class).  Kept LOCAL to this file -- no shared
 * header, so a sibling slice's independent reading of the same tables
 * cannot collide with this one on merge.
 * ------------------------------------------------------------------------ */
typedef struct VabStreamObj VabStreamObj;

/* gVabStreamObjMethods's own methods table -- the class VabStreamObj below
 * dispatches through.  Only the slots this unit's own functions call or are
 * assigned to are named.  round 43 added slot58/slot5C/slot6C/slot9C once
 * VabStreamObj__VabStreamObj, VabStreamObj__Update and
 * VabStreamObj__LoadVagAttrs (all round-17 gp_rel stalls, resolved round 42)
 * were actually worked. */
typedef struct VabStreamObjMethods {
    u8 pad000[0x008];
    void (*slot08)(void *self, char *arg1); /* VabStreamObj__VabStreamObj -- this unit's own new_class_da34 dispatch; arg1 is a base filename, not a plain s32 */
    s32 (*slot0C)(VabStreamObj *self);      /* VabStreamObj__Close, confirmed against gVabStreamObjMethods's own rodata (+0x0C) */
    u8 pad010[0x058 - 0x010];
    /* +0x058 and +0x06C are BOTH null in retail's own gVabStreamObjMethods
     * (confirmed against asm/data/5E140.data.s) -- VabStreamObj__Update and
     * VabStreamObj__VabStreamObj each dispatch through one of them anyway,
     * on a path that is apparently never actually taken for an object built
     * with this exact base table.  That is retail's own behaviour, not a
     * derivation error: the dispatch still has to compile, whatever sits at
     * the address at runtime. */
    void (*slot58)(void *self, char *path); /* VabStreamObj__Update's own dispatch -- begins the VAB body transfer once the ".VB" path is built; null in retail */
    void (*slot5C)(void *self);             /* Class6D430__FreeBuffer (uncarved, cross-unit) -- VabStreamObj__LoadVagAttrs's own dispatch, called before it re-fetches the VAB header */
    u8 pad060[0x06C - 0x060];
    void (*slot6C)(void *self, char *path); /* VabStreamObj__VabStreamObj's own dispatch -- begins the VAB header transfer for the ".VH" path; null in retail */
    u8 pad070[0x078 - 0x070];
    s32 (*slot78)(VabStreamObj *self, s32 arg1); /* VabStreamObj__OnBodyReady, and VabStreamObj__Update's own body-complete notify */
    s32 (*slot7C)(VabStreamObj *self);           /* VabStreamObj__LoadVagAttrs */
    u8 pad080[0x084 - 0x080];
    s32 (*slot84)(VabStreamObj *self, s32 arg1); /* VabStreamObj__StopVoice, arg1 is s16-truncated by the callee */
    u8 pad088[0x09C - 0x088];
    void (*slot9C)(void *self, s32 arg1);   /* VabStreamObj__SetPitchOffset -- VabStreamObj__VabStreamObj's own dispatch, called with arg1 == 0 right after construction */
} VabStreamObjMethods;
extern VabStreamObjMethods gVabStreamObjMethods;

/* This unit's own reduced, local view of Sony's `VagAtr` (include/psyq/
 * LIBSND.H, 32 bytes) -- one entry inside a VabStreamObj::progVagTable
 * sub-array, stride 0x20.  `center`/`shift` land exactly on VagAtr's own
 * same-named bytes (+0x04/+0x05), confirmed round 52 against LIBSND.H;
 * everything else stays opaque padding per the project's independent-
 * local-view convention rather than pulling in the real header (see
 * code_179d8_k.c for the same choice).  Only the two bytes
 * VabStreamObj__PlayTone itself reads are named. */
typedef struct VagAtrView {
    u8 pad0[0x4];
    u8 center; /* +0x04, VagAtr::center -- the VAG's own center note */
    u8 shift;  /* +0x05, VagAtr::shift -- center note fine tune */
    u8 pad6[0x20 - 0x6];
} VagAtrView;

/* This unit's own reduced view of Sony's `VabHdr` (include/psyq/LIBSND.H,
 * 32 bytes) -- only the two fields VabStreamObj__LoadVagAttrs itself reads
 * are named: `ts` (program count) and `vs` (vag count), at the real
 * struct's own offsets +0x12/+0x14. */
typedef struct VabHdrView {
    u8 pad0[0x12];
    u16 ts;  /* +0x12, program count */
    u16 vs;  /* +0x14, vag count */
    u8 pad16[0x20 - 0x16];
} VabHdrView;

/* self for the gVabStreamObjMethods-dispatched methods in this unit.  Only
 * fields this unit's own functions touch are named -- see the unit header
 * comment for how the class was established. */
struct VabStreamObj {
    VabStreamObjMethods *methods; /* +0x000 */
    u8 pad004[0x010 - 0x004];
    u8 *streamBuffer;              /* +0x010, streaming file buffer -- passed to SsVabOpenHead/SsVabTransBody */
    u8 pad014[0x024 - 0x014];
    u32 flags;                      /* +0x024, flag word; bit 0x200 gates the header/body transfer steps */
    u8 pad028[0x02A - 0x028];
    u16 loadState;                    /* +0x02A, load state: 0 idle, 1 header pending, 6 body pending -- unsigned (retail loads it lhu) */
    VabHdrView vabHdr;                  /* +0x02C, filled by SsUtGetVabHdr; 32 bytes, ends exactly at +0x04C */
    VagAtrView *vagAttrPool;              /* +0x04C, VagAtr pool, vabHdr.vs entries */
    VagAtrView **progVagTable;             /* +0x050, array of vabHdr.ts pointers into vagAttrPool */
    s16 vabId;                               /* +0x054 */
    s16 muted;                                /* +0x056, boolean-ish flag */
    u16 attrsReady;                             /* +0x058, unsigned (retail loads it lhu in VabStreamObj__LoadVagAttrs) */
    u16 bodyTransferPending;                      /* +0x05A */
    void *baseFilename;                             /* +0x05C, malloc'd copy of the base filename */
    s32 pitchOffset;                                  /* +0x060, set by VabStreamObj__SetPitchOffset; added to a VagAtrView::center in VabStreamObj__PlayTone */
};

/* Cross-unit calls into the still-uncarved code_179d8_tail monolith --
 * declared LOCAL to this unit, per-call-site typed, since none of them have
 * an established prototype anywhere yet. */
extern void *BMemPMgrAlloc(s32 size);
/* Sony libsnd, prototypes copied from LIBSND.H (plan revision 15; round 74
 * found the SsUtKeyOn and SsUtAllKeyOff lines disagreeing with it). */
extern s16 SsUtKeyOn(s16 vabId, s16 prog, s16 tone, s16 note, s16 fine, s16 voll, s16 volr);
extern s16 SsUtAutoVol(s16 vc, s16 start_vol, s16 end_vol, s16 delta_time);
extern s16 SsUtKeyOffV(s16 voice);
extern void SsUtAllKeyOff(s16 mode);
/* Sony's `SsVabTransCompleted` (`libsnd/vs_vtc`) and `SsSetMute`
 * (`libsnd/scsmute`), linked from the SDK objects since round 34.  The two
 * signatures are this call site's own reading and disagree with the sibling
 * reading in code_179d8_i.c about the return types -- that is the project's
 * independent-local-view convention, and it is exactly why a Psy-Q prototype
 * must never go into a header these units share. */
extern void SsVabTransCompleted(s32 arg0);
extern s32 SsSetMute(s32 arg0);

/* InitSoundCueSet's own "set" (its `arg1`) -- a small slot-table object,
 * unrelated to VabStreamObj (this function is NOT a gVabStreamObjMethods
 * vtable slot; its only callers pass a plain heap/stack struct pointer).
 * Only the fields this unit's own functions touch are named. */
typedef struct SoundCueSlot {
    s32 index; /* -1 = free/sentinel after init, else an index FlushSoundCueSet forwards to VabStreamObj__StopVoice */
    u8 pad4[0x14 - 0x4];
} SoundCueSlot;

typedef struct SoundCueSet {
    s32 tag;    /* +0x00, guard (0 = uninitialized) AND, once initialized, the caller's own tag (round 52: Entity.c passes this->moodIndex + 1) */
    s32 unk4;   /* +0x04, zeroed by InitSoundCueSet, never read by this unit's own functions */
    void *owner; /* +0x08, the caller's own object pointer (Entity, DreamSys, etc -- opaque here) */
    s32 callback; /* +0x0C, a function-pointer-shaped value from the caller (round 52: DreamSys.c passes a vtable slot, class_3bb8c_n.c indexes a table of them) -- stored, never called by this unit's own functions */
    u8 pad10[0x14 - 0x10];
    s32 unk14;  /* +0x14, set to the constant 10 by InitSoundCueSet; no further evidence of its role in this unit */
    SoundCueSlot slots[3]; /* +0x18 */
} SoundCueSet;

/* Forward declaration: GetVabStreamObjMethods is defined later in this file
 * (ROM order), but New_VabStreamObj (earlier in ROM order) calls it. */
VabStreamObjMethods *GetVabStreamObjMethods(void);

s32 VabDriver__Read(void) {
    return 0;
}

void VabDriver__LoadFile(void) {
}

void VabDriver__RunRequestQueue(void) {
}

void VabDriver__RequestLoadFile(void) {
}

void VabDriver__StopService(void) {
}

void VabDriver__CancelRequests(void) {
}

VabDriverMethods *GetVabDriverMethods(void) {
    return &gVabDriverMethods;
}

extern s32 gVabDriverMode;
extern s32 gVabDriverModeArg;

s32 GetVabDriverMode(s32 *arg0) {
    if (arg0 != NULL) {
        *arg0 = gVabDriverModeArg;
    }
    return gVabDriverMode;
}

s32 SetVabDriverMode(s32 a, s32 b)
{
	gVabDriverMode = a;
	gVabDriverModeArg = b;
	return 1;
}

s32 func_8002C478(void) {
    return 0;
}

void *New_VabStreamObj(s32 arg0) {
    void *self;

    self = BMemPMgrAlloc(0x64);
    if (self != NULL) {
        GetVabStreamObjMethods()->slot08(self, arg0);
        return self;
    }
    return NULL;
}

/* Base-class table reached via the uncarved accessor `GetActiveDataSourceMethods()` --
 * not this unit's function to define.  Only the two slots this unit's own
 * functions dispatch through are typed, per the same convention as
 * code_179d8_d.c's own independent reading of the same physical table.
 * `GetActiveDataSourceMethods` (code_171e0.c) returns EITHER gVabDriverMethods (when
 * `gActiveDataSource == 0x23`) or the CD-driver class's own methods
 * otherwise -- see the unit header comment. */
typedef struct DriverBaseMethods {
    u8 pad000[0x008];
    void (*slot08)(void *self);  /* VabStreamObj__VabStreamObj's own base-chain call */
    /* VabStreamObj__Close's own base-chain call -- its own return is likewise a
     * bare tail call with nothing after it, so per CLAUDE.md's rule this
     * defaults to s32 absent positive void evidence. */
    s32 (*slot0C)(void *self);
} DriverBaseMethods;
extern DriverBaseMethods *GetActiveDataSourceMethods(void);

/* Sony's own VAB streaming calls (include/psyq/LIBSND.H), declared locally
 * per this project's convention of not sharing Psy-Q prototypes across
 * units (see the SsVabTransCompleted/SsSetMute comment above). */
extern void SsVabClose(s16 vabId);
extern s16 SsVabOpenHead(u8 *addr, s16 arg1);
extern s16 SsVabTransBody(u8 *addr, s16 vabId);
extern s16 SsUtGetVabHdr(s16 vabId, void *out);
extern s16 SsUtGetProgAtr(s16 vabId, s16 prog, void *out);
extern s16 SsUtGetVagAtr(s16 vabId, s16 prog, s16 tone, void *out);
extern void SsSetMVol(s16 a0, s16 a1);
extern void SsSetTableSize(char *a0, s16 a1, s16 a2);

/* Psy-Q LIBSND.H: extern void SsInit (void); -- track 2 identification,
 * round 78 (was func_80032368, declared s32; the one call discards it). */
extern void SsInit(void);
/* Uncarved code_179d8_tail helpers this cluster calls. */
extern char *GetSsSizeTableBuf(void);
extern s32 IsWBgmActive(void);
extern void SsEnd(void);
extern void SsQuit(void);
extern void SsSetTickMode(s32 a0);
extern void SsStart(void);
extern void *BMemPMgrFree(void *ptr);
extern char *BuildFileName(char *dest, char *arg1, char *arg2, char *arg3);
extern s32 strlen(char *s);
extern char *strcpy(char *dest, char *src);

/* ".VH"/".VB" -- already emitted by splat in .sdata, referenced not
 * retyped (a literal here would duplicate the bytes and shift the image). */
extern const char gVabHeaderSuffix[];
extern const char gVabBodySuffix[];

extern s32 gVabSizeTableInited;
extern s32 gVabStreamInited;
extern s32 gVabVolumeInited;
extern s32 gOpenVabCount;
extern s32 D_8008A8CC;
extern void *gPendingVabBuffer;

void VabStreamObj__VabStreamObj(VabStreamObj *self, char *arg1) {
    void *buf;
    char path[0x20];

    GetActiveDataSourceMethods()->slot08(self);
    self->methods = GetVabStreamObjMethods();
    self->vagAttrPool = NULL;
    self->progVagTable = NULL;
    self->vabId = 0;
    self->muted = 0;
    self->methods->slot9C(self, 0);
    self->attrsReady = 0;
    self->bodyTransferPending = 0;
    self->baseFilename = NULL;
    if (gVabSizeTableInited == 0) {
        SsInit();
        gVabSizeTableInited = 1;
        SsSetTableSize(GetSsSizeTableBuf(), 2, 1);
    }
    if (gVabStreamInited == 0) {
        D_8008A8CC = 0x3C;
        SsSetTickMode(1);
        gVabStreamInited = 1;
    }
    gOpenVabCount++;
    if (arg1 != NULL) {
        buf = BMemPMgrAlloc(strlen(arg1) + 1);
        if (buf != NULL) {
            self->baseFilename = buf;
            strcpy(buf, arg1);
            BuildFileName(path, buf, NULL, gVabHeaderSuffix);
            self->loadState = 1;
            self->methods->slot6C(self, path);
        }
    }
}

s32 VabStreamObj__Close(VabStreamObj *self) {
    SsVabClose(self->vabId);
    if (--gOpenVabCount < 0) {
        gOpenVabCount = 0;
    }
    if (gOpenVabCount == 0 && IsWBgmActive() == 0) {
        gVabSizeTableInited = 0;
        gVabVolumeInited = 0;
        gVabStreamInited = 0;
        SsEnd();
        SsQuit();
    }
    BMemPMgrFree(self->vagAttrPool);
    BMemPMgrFree(self->progVagTable);
    BMemPMgrFree(self->baseFilename);
    return GetActiveDataSourceMethods()->slot0C(self);
}

void VabStreamObj__Update(VabStreamObj *self) {
    char path[0x20];

    switch (self->loadState) {
    case 0:
        break;
    case 1:
        if (self->flags & 0x200) {
            self->vabId = SsVabOpenHead(self->streamBuffer, -1);
            BuildFileName(path, self->baseFilename, NULL, gVabBodySuffix);
            gPendingVabBuffer = self->streamBuffer;
            self->loadState = 6;
            self->streamBuffer = NULL;
            self->methods->slot58(self, path);
            if (self->baseFilename != NULL) {
                BMemPMgrFree(self->baseFilename);
                self->baseFilename = NULL;
            }
        }
        break;
    case 6:
        if (self->flags & 0x200) {
            self->vabId = SsVabTransBody(self->streamBuffer, self->vabId);
            if (self->vabId != -1) {
                self->bodyTransferPending = 1;
                self->methods->slot78(self, 1);
            }
        }
        break;
    default:
        break;
    }
}

s32 VabStreamObj__OnBodyReady(VabStreamObj *self, s32 arg1) {
    s32 result;

    result = 0;
    if (self->bodyTransferPending != 0) {
        if (arg1 != 0) {
            SsVabTransCompleted(1);
            self->bodyTransferPending = 0;
            self->attrsReady = 1;
            self->methods->slot7C(self);
            result = 1;
        }
    }
    return result;
}

/* This unit's own reduced view of Sony's `ProgAtr` (include/psyq/LIBSND.H,
 * 16 bytes) -- only the one field VabStreamObj__LoadVagAttrs itself reads is
 * named, per the same local-struct convention used for VabHdrView above (and
 * matching code_179d8_k.c's own reduced `ProgAtr` reading). */
typedef struct ProgAtrView {
    u8 tones; /* +0x00, program's tone count, written by SsUtGetProgAtr */
    u8 pad1[0x10 - 0x1];
} ProgAtrView;

/* Class6D430__FreeBuffer -- uncarved, cross-unit; reached only through
 * gVabStreamObjMethods's own +0x5C slot (declared above as `slot5C`), never
 * called directly by name here. */

void VabStreamObj__LoadVagAttrs(VabStreamObj *self)
{
    ProgAtrView prog;
    VagAtrView *pool;
    s32 i;
    s32 j;
    s16 result;

    if (self->attrsReady == 0) {
        return;
    }
    self->methods->slot5C(self);
    self->streamBuffer = gPendingVabBuffer;
    result = SsUtGetVabHdr(self->vabId, &self->vabHdr);
    if (result == -1) {
        return;
    }
    self->vagAttrPool = BMemPMgrAlloc(self->vabHdr.vs << 5);
    if (self->vagAttrPool == NULL) {
        return;
    }
    self->progVagTable = BMemPMgrAlloc(self->vabHdr.ts << 2);
    if (self->progVagTable == NULL) {
        return;
    }
    pool = self->vagAttrPool;
    for (i = 0; i < self->vabHdr.ts; i++) {
        self->progVagTable[i] = pool;
        result = SsUtGetProgAtr(self->vabId, i, &prog);
        if (result == -1) {
            return;
        }
        for (j = 0; j < prog.tones; j++) {
            result = SsUtGetVagAtr(self->vabId, i, j, pool);
            if (result == -1) {
                return;
            }
            pool++;
        }
    }
    if (gVabVolumeInited == 0) {
        SsStart();
        SsSetMVol(0x78, 0x78);
        gVabVolumeInited = 1;
    }
}

/* Resolve a packed program/tone index, look up its VagAtr and key it on.
 * `prog` is loaded BEFORE `lo` is computed on purpose: GCC 2.6.3's combine
 * folds `index - (index >> 4) * 16` into `index & 0xF` whenever `hi`'s
 * FIRST use is the multiply (flow.c links a set only to its next use), and
 * retail kept the unfolded sll+subu (docs/match-reports/VabStreamObj__PlayTone.md). */
s32 VabStreamObj__PlayTone(VabStreamObj *self, s32 index, s32 arg2, s32 arg3) {
    s32 hi;
    s32 lo;
    VagAtrView *entry;
    VagAtrView *prog;
    s16 result;

    if (index >= 0) {
        hi = index >> 4;
        prog = self->progVagTable[hi];
        lo = index - hi * 16;
        entry = &prog[lo];
        result = SsUtKeyOn(self->vabId, (s16)hi, (s16)lo, (s16)(entry->center + self->pitchOffset),
                                entry->shift, (s16)arg2, (s16)arg2);
        if (result >= 0) {
            SsUtAutoVol(result, (s16)arg2, (s16)arg3, 2);
            return result;
        }
    }
    return -1;
}

/* The PS1 SPU's own hardware voice count -- the boundary VabStreamObj__StopVoice
 * checks `index` against. */
#define SPU_VOICE_COUNT 0x18

s32 VabStreamObj__StopVoice(VabStreamObj *self, s32 index) {
    if (index < SPU_VOICE_COUNT) {
        SsUtKeyOffV(index);
    } else {
        SsUtAllKeyOff(0);
    }
    return -1;
}

s32 VabStreamObj__Mute(VabStreamObj *self) {
    s32 flag;

    flag = self->muted;
    if (flag == 0) {
        SsSetMute(1);
        flag = 1;
        self->muted = flag;
    }
    return flag;
}

s32 VabStreamObj__Unmute(VabStreamObj *self) {
    s32 flag;

    flag = self->muted;
    if (flag != 0) {
        flag = SsSetMute(0);
        self->muted = 0;
    }
    return flag;
}

void VabStreamObj__func_2cbdc(void) {
}

void VabStreamObj__func_2cbe4(void) {
}

void VabStreamObj__func_2cbec(void) {
}

void VabStreamObj__SetPitchOffset(VabStreamObj *self, s32 arg1) {
    self->pitchOffset = arg1 * 12 - 0x18;
}

VabStreamObjMethods *GetVabStreamObjMethods(void) {
    return &gVabStreamObjMethods;
}

extern s32 gOpenVabCount;

s32 GetOpenVabCount(void) {
    return gOpenVabCount;
}

extern s32 D_8008A8CC;

s32 func_8002CC28(void) {
    return D_8008A8CC;
}

s32 InitSoundCueSet(void *unused, SoundCueSet *set, s32 tag, void *owner, s32 callback) {
    SoundCueSlot *slot;
    s32 count;
    s32 sentinel;

    if (set->tag != 0) {
        return 0;
    }
    slot = set->slots;
    sentinel = -1;
    count = 2;
    set->tag = tag;
    set->owner = owner;
    set->callback = callback;
    do {
        slot->index = sentinel;
        count--;
        slot++;
    } while (count >= 0);
    set->unk4 = 0;
    set->unk14 = 10;
    return 1;
}

void FlushSoundCueSet(VabStreamObj *self, SoundCueSet *set) {
    s32 i;
    SoundCueSlot *slot;

    slot = set->slots;
    for (i = 0; i < 3; i++) {
        if (slot->index >= 0) {
            slot->index = self->methods->slot84(self, slot->index);
        }
        slot++;
    }
    set->tag = 0;
}
