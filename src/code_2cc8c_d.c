#include "common.h"
#include "code_2cc8c.h"

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003E8B8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003E968);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EA0C);

void func_8003EA24(Unk18Obj *self, s32 a1) {
    self->unk3C = a1;
}

/* Only writes unk44 the first time (guarded by the unk70 latch). */
void func_8003EA2C(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk44 = a1;
    }
}

/* Same guard as func_8003EA2C, writes unk48 instead. */
void func_8003EA48(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk48 = a1;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EA64);

void func_8003EA6C(void) {
}

void func_8003EA74(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EA7C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EA84);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EAA4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EAC4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EACC);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EB84);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EBC4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EBF8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EC2C);

void func_8003ECC0(void) {
}

void func_8003ECC8(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003ECD0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EDF4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EE40);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EE88);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003EEC0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F04C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F1A8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F230);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F23C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F244);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F24C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F25C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_d", func_8003F28C);
