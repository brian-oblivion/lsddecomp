/*
 * code_322b4 -- GAME code carved from psyq_322b4 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x322B4..0x330F4 (vram 0x80041AB4..0x800428F4). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: methods of D_8006EC74,
 * D_8006ED4C, D_8006EE1C, D_8006EF50, D_8006EFAC, D_8006EB90, D_8006EED8,
 * D_800879C4 and D_800866E8, calling GetClass6B5CCMethods, GetSetBitField and
 * the BasicClass framework. Owns jtbl_80011290 (attached rodata sub-slot
 * 0x1A90).
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "Class6B5CC.h"

/* Local view of a D_8006EF50 (class id 0x5) object: only the three words its
 * +0x048..+0x058 accessors touch. */
typedef struct D_8006EF50Obj {
    u8 pad00[0xC];
    s32 unkC;  /* +0x00C, read by func_8004264C */
    s32 unk10; /* +0x010, set to 1 by func_80042658, cleared by func_80042664, read by func_8004266C */
    s32 unk14; /* +0x014, set to 1 by func_80042678 */
} D_8006EF50Obj;

/* Local view of a D_8006EED8 (class id 0xB03) object: func_800423E4 sets +0x2C. */
typedef struct D_8006EED8Obj {
    u8 pad00[0x2C];
    s32 unk2C;
} D_8006EED8Obj;

INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041AB4);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041B20);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041BAC);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041BDC);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041C28);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041C3C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041C4C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041C9C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041D18);
/* D_8006ED4C slot +0x040 (reset): empty override. */
void func_80041DA4(Class6B5CC *self) {
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041DAC);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041E2C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041E58);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041ED8);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041EE8);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80041F88);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004202C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004208C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042170);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004220C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004223C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042268);
/* D_8006EB90 and D_8006EC74 slot +0x098 (update): empty override. */
void func_80042294(Class6B5CC *self, void *sender, s32 event) {
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004229C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800422BC);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800422CC);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004232C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800423A8);
/* D_8006EED8 slot +0x064. */
void func_800423E4(D_8006EED8Obj *self) {
    self->unk2C = 1;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800423F0);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042400);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042450);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800424A8);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800424E0);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042550);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800425D8);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800425EC);
/* D_8006EF50 slot +0x048. */
s32 func_8004264C(D_8006EF50Obj *self) {
    return self->unkC;
}
/* D_8006EF50 slot +0x04C. */
void func_80042658(D_8006EF50Obj *self) {
    self->unk10 = 1;
}
/* D_8006EF50 slot +0x050. */
void func_80042664(D_8006EF50Obj *self) {
    self->unk10 = 0;
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004266C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042678);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042684);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042694);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800426E4);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042790);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042814);
/* D_8006EFAC slot +0x09C (dispatchLinkCommand): empty override. */
void func_80042820(Class6B5CC *self, void *sender, s32 event) {
}
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_80042828);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_8004283C);
INCLUDE_ASM("asm/nonmatchings/code_322b4", func_800428E4);
