/*
 * code_10ee0 -- GAME code carved from the head of psyq_10ee0 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x10EE0..0x11474 (vram
 * 0x800206E0..0x80020C74). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the 19 methods of D_8006C070 (new_class_6c078
 * allocates it) and a getter/setter pair for the game gp variable D_8008A83C.
 * libgpu/sys starts right after, at ResetGraph (now psyq_11474).
 *
 * Round 81 (bravo) matched the ten small methods/accessors; the rest are
 * still fresh track-1 ground.
 */
#include "common.h"
#include "BasicClass.h"

/*
 * Local view of the D_8006C070 object (a BasicClass subclass). Only the
 * fields this unit's matched methods touch are named; the draw singleton
 * other units reach through func_80020C5C() is (per its callers) an object
 * of this class.
 */
typedef struct Class6C070 Class6C070;
typedef struct Class6C070Methods Class6C070Methods;

struct Class6C070Methods {
    BASICCLASS_SLOTS(Class6C070, (Class6C070 *self));
};

struct Class6C070 {
    BASICCLASS_FIELDS(Class6C070Methods);
    /* +0x00C */ u8 pad0C[0x10 - 0x0C];
    /* +0x010 */ s32 unk10;           /* cleared by 8002089C; 80020B4C stores unk20 only while 0 */
    /* +0x014 */ u8 pad14[0x20 - 0x14];
    /* +0x020 */ s32 unk20;           /* 80020B4C sets, 80020B68 gets */
    /* +0x024 */ u8 pad24[0x2C - 0x24];
    /* +0x02C */ s32 unk2C;           /* 80020C3C sets */
    /* +0x030 */ s32 unk30;           /* 80020C44 sets */
};

extern Class6C070Methods D_8006C070;  /* the class's method table */
extern Class6C070 *D_8008A83C;        /* sdata: the singleton func_80020C5C returns */

extern void GsSwapDispBuff(void);     /* LIBGS.H */

INCLUDE_ASM("asm/nonmatchings/code_10ee0", new_class_6c078);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020730);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020784);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800207DC);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_8002085C);
void func_8002089C(Class6C070 *self) {
    if (self->unk10 != 0) {
        self->unk10 = 0;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800208B8);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800208D8);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800208F8);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020970);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800209A0);
s32 func_80020A1C(Class6C070 *self) {
    return 0;
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020A24);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020A74);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020AF4);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020B4C);
s32 func_80020B68(Class6C070 *self) {
    return self->unk20;
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020B74);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020C08);
void func_80020C3C(Class6C070 *self, s32 value) {
    self->unk2C = value;
}
void func_80020C44(Class6C070 *self, s32 value) {
    self->unk30 = value;
}
Class6C070Methods *func_80020C4C(void) {
    return &D_8006C070;
}
Class6C070 *func_80020C5C(void) {
    return D_8008A83C;
}
void func_80020C68(Class6C070 *obj) {
    D_8008A83C = obj;
}
