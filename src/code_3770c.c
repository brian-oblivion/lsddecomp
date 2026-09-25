/*
 * code_3770c -- GAME code carved from psyq_3770c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x3770C..0x38110 (vram 0x80046F0C..0x80047910). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the 18 methods of
 * D_800817E0.
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "BasicClass.h"

/* Local view of the D_800817E0 class: a CD streaming object (StSetRing /
 * StFreeRing / CdSync over libcd). Only the fields this unit's matched
 * methods touch are named. */
typedef struct CdStreamObj CdStreamObj;
typedef struct CdStreamObjMethods CdStreamObjMethods;

struct CdStreamObjMethods {
    BASICCLASS_SLOTS(CdStreamObj, (CdStreamObj *self));
};

struct CdStreamObj {
    BASICCLASS_FIELDS(CdStreamObjMethods);
    /* +0x00C */ u8 pad0C[0x24 - 0x0C];
    /* +0x024 */ u8 cdResult[8];      /* CdSync result buffer (func_800478D0) */
    /* +0x02C */ s32 unk2C;           /* non-zero: ring already set up */
    /* +0x030 */ u8 pad30[0x50 - 0x30];
    /* +0x050 */ u32 *ring;           /* StSetRing's ring_addr (func_800470C8) */
};

extern CdStreamObjMethods D_800817E0;  /* the class's method table */

/* LIBCD.H */
extern void StSetRing(u32 *ring_addr, u32 ring_size);
extern void StClearRing(void);
extern void StUnSetRing(void);
extern u32 StFreeRing(u32 *base);
extern int CdSync(int mode, u8 *result);

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

INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80046F0C);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80046F88);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047074);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800470C8);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047114);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047240);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_8004728C);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800472EC);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047388);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800473E4);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800474C8);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047574);
void func_800475C8(CdStreamObj *self) {
}
void func_800475D0(CdStreamObj *self) {
}
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800475D8);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047638);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047694);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800477B0);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047810);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047870);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047890);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800478B0);
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_800478D0);
void func_800478F8(CdStreamObj *self) {
}
INCLUDE_ASM("asm/nonmatchings/code_3770c", func_80047900);
