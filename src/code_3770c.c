/*
 * code_3770c -- GAME code carved from psyq_3770c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x3770C..0x38110 (vram 0x80046F0C..0x80047910). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the 18 methods of
 * D_800817E0.
 *
 * Round 81 matched the ten smallest methods (empty slots, the table getter,
 * the libcd/libspu wrappers); the rest are still INCLUDE_ASM.
 */
#include "common.h"
#include "BasicClass.h"

/* Local view of the D_800817E0 class: a CD streaming object (StSetRing /
 * StFreeRing / CdSync over libcd). Only the fields this unit's matched
 * methods touch are named. */
typedef struct CdStreamObj CdStreamObj;
typedef struct CdStreamObjMethods CdStreamObjMethods;

struct CdStreamObjMethods {
    BASICCLASS_SLOTS(CdStreamObj, (CdStreamObj *self, s32 arg1, s32 arg2, s32 arg3));
    /* +0x040 */ void (*setRing)(CdStreamObj *self, u32 *ring, u32 size); /* func_800470C8 */
    /* +0x044 */ void *slot44;                                            /* func_80047114 */
    /* +0x048 */ void (*slot48)(CdStreamObj *self);                       /* func_8004728C */
    /* +0x04C */ void (*seek)(CdStreamObj *self, u8 *loc);              /* func_800472EC */
    /* +0x050 */ void *slot50;                                            /* func_800473E4 */
    /* +0x054 */ void (*slot54)(CdStreamObj *self);                       /* func_800474C8 */
    /* +0x058 */ void (*slot58)(CdStreamObj *self);                       /* func_80047574 */
    /* +0x05C */ void (*slot5C)(CdStreamObj *self);                       /* func_800475C8, empty */
    /* +0x060 */ void (*slot60)(CdStreamObj *self);                       /* func_800475D0, empty */
    /* +0x064 */ void (*mute)(CdStreamObj *self);                         /* func_800475D8 */
    /* +0x068 */ void (*demute)(CdStreamObj *self);                       /* func_80047638 */
    /* +0x06C */ void *slot6C;                                            /* func_80047694 */
    /* +0x070 */ u32 (*freeRing)(CdStreamObj *self, u32 *base);           /* func_80047870 */
    /* +0x074 */ void (*unsetRing)(CdStreamObj *self);                    /* func_80047890 */
    /* +0x078 */ void (*clearRing)(CdStreamObj *self);                    /* func_800478B0 */
    /* +0x07C */ void (*slot7C)(CdStreamObj *self);                       /* func_800478F8, empty */
};

struct CdStreamObj {
    BASICCLASS_FIELDS(CdStreamObjMethods);
    /* +0x00C */ u8 loc[0x24 - 0x0C];  /* passed to seek (func_80047574) */
    /* +0x024 */ u8 cdResult[8];      /* CdSync result buffer (func_800478D0) */
    /* +0x02C */ s32 unk2C;           /* state: 0 idle, 1 seeking, 2, 4 */
    /* +0x030 */ s32 muted;           /* func_800475D8 / func_80047638 */
    /* +0x034 */ s32 unk34;           /* < 4 selects read mode 0x1C0, else 0x140 */
    /* +0x038 */ u8 pad38[0x40 - 0x38];
    /* +0x040 */ s32 unk40;
    /* +0x044 */ void *cbArg;
    /* +0x048 */ void (*cb48)(void *arg);
    /* +0x04C */ void (*cb4C)(void *arg);
    /* +0x050 */ u32 *ring;           /* StSetRing's ring_addr (func_800470C8) */
    /* +0x054 */ void (*cb54)(void *arg);
    /* +0x058 */ s32 unk58;
};

extern CdStreamObjMethods D_800817E0;  /* the class's method table */
CdStreamObjMethods *func_80047900(void);
void func_80047388(u8 status, u8 *result);

/* LIBCD.H */
extern void StSetRing(u32 *ring_addr, u32 ring_size);
extern void StClearRing(void);
extern void StUnSetRing(void);
extern u32 StFreeRing(u32 *base);
extern int CdSync(int mode, u8 *result);
extern void *CdSyncCallback(void (*func)(u8 status, u8 *result));
extern int CdControl(u8 com, u8 *param, u8 *result);
extern int CdControlF(u8 com, u8 *param);
extern int CdRead2(u32 mode);
extern void StSetStream(u32 mode, u32 start_frame, u32 end_frame, void (*func1)(), void (*func2)());

extern void *BMemPMgrAlloc(s32 size);
extern CdStreamObj *D_8008A950;  /* the active stream object */

/* LIBSPU.H */
typedef struct {
    s16 left;
    s16 right;
} SpuVolume;

typedef struct {
    SpuVolume volume;
    s32 reverb;
    s32 mix;
} SpuExtAttr;

typedef struct {
    u32 mask;
    SpuVolume mvol;
    SpuVolume mvolmode;
    SpuVolume mvolx;
    SpuExtAttr cd;
    SpuExtAttr ext;
} SpuCommonAttr;

extern void SpuSetCommonAttr(SpuCommonAttr *attr);

CdStreamObj *func_80046F0C(s32 arg1, s32 arg2, s32 arg3) {
    CdStreamObj *obj = BMemPMgrAlloc(0x5C);

    if (obj != NULL) {
        func_80047900()->ctor(obj, arg1, arg2, arg3);
        return obj;
    }
    return NULL;
}
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80046F88);
void func_80047074(CdStreamObj *self) {
    self->methods->slot48(self);
    Get_vtable_BasicClass()->finalize((BasicClass *)self);
}
void func_800470C8(CdStreamObj *self, u32 *ring, u32 size) {
    if (self->unk2C == 0) {
        StSetRing(ring, size >> 11);
        self->ring = ring;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047114);
s32 func_80047240(CdStreamObj *self) {
    SpuCommonAttr attr;

    attr.mask = 0x2C3;
    attr.mvol.left = 0x3FFF;
    attr.mvol.right = 0x3FFF;
    attr.cd.volume.left = 0x7FFF;
    attr.cd.volume.right = 0x7FFF;
    attr.cd.mix = 1;
    SpuSetCommonAttr(&attr);
    return 1;
}
void func_8004728C(CdStreamObj *self) {
    CdStreamObj *cur;

    if (self->unk2C != 0) {
        cur = D_8008A950;
        if (cur == self) {
            cur->methods->slot54(cur);
            cur->unk2C = 0;
            D_8008A950 = NULL;
        }
    }
}
void func_800472EC(CdStreamObj *self, u8 *loc) {
    if (self->unk2C != 2 && D_8008A950 == self) {
        if (self->cb54 != NULL) {
            CdSyncCallback(func_80047388);
            CdControlF(0x15, loc);
        } else {
            while (CdControl(0x15, loc, 0) == 0) {
            }
        }
        self->unk2C = 1;
    }
}
void func_80047388(u8 status, u8 *result) {
    if (D_8008A950 != NULL && status == 2) {
        CdSyncCallback(NULL);
        if (D_8008A950->cb54 != NULL) {
            D_8008A950->cb54(D_8008A950->cbArg);
        }
    }
}
void func_800473E4(CdStreamObj *self, u32 startFrame, s32 arg2) {
    u32 mode;

    if (self->unk2C == 1 && D_8008A950 == self) {
        mode = 0x140;
        if (self->unk34 < 4) {
            mode = 0x1C0;
        }
        if (arg2 != 0) {
            self->unk40 = arg2;
        }
        self->unk58 = 0;
        StSetStream(0, startFrame, -1, 0, 0);
        self->methods->mute(self);
        while (CdControl(2, self->loc, 0) == 0 || CdRead2(mode) == 0) {
        }
        self->methods->demute(self);
        self->unk2C = 2;
    }
}
void func_800474C8(CdStreamObj *self) {
    if (self->unk2C == 2 && D_8008A950 == self) {
        self->methods->mute(self);
        self->methods->clearRing(self);
        self->methods->unsetRing(self);
        while (CdControl(9, 0, 0) == 0) {
        }
        self->unk2C = 4;
    }
}
void func_80047574(CdStreamObj *self) {
    CdStreamObj *cur;

    if (self->unk2C == 4) {
        cur = D_8008A950;
        if (cur == self) {
            cur->unk2C = 0;
            cur->methods->seek(cur, cur->loc);
        }
    }
}
void func_800475C8(CdStreamObj *self) {
}
void func_800475D0(CdStreamObj *self) {
}
void func_800475D8(CdStreamObj *self) {
    if (self->muted == 0 && D_8008A950 == self) {
        while (CdControl(0xB, 0, 0) == 0) {
        }
        self->muted = 1;
    }
}
void func_80047638(CdStreamObj *self) {
    if (self->muted != 0 && D_8008A950 == self) {
        while (CdControl(0xC, 0, 0) == 0) {
        }
        self->muted = 0;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047694);
void func_800477B0(CdStreamObj *self, u32 *base) {
    if (self->cb48 != NULL) {
        self->cb48(self->cbArg);
        self->methods->freeRing(self, base);
    }
}
void func_80047810(CdStreamObj *self) {
    if (self->cb4C != NULL) {
        self->cb48(self->cbArg);
        self->methods->slot48(self);
    }
}
u32 func_80047870(CdStreamObj *self, u32 *base) {
    return StFreeRing(base);
}
void func_80047890(CdStreamObj *self) {
    StUnSetRing();
}
void func_800478B0(CdStreamObj *self) {
    StClearRing();
}
int func_800478D0(CdStreamObj *self, int mode) {
    return CdSync(mode, self->cdResult);
}
void func_800478F8(CdStreamObj *self) {
}
CdStreamObjMethods *func_80047900(void) {
    return &D_800817E0;
}
