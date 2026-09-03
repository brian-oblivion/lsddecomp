#include "common.h"
#include "code_2cc8c.h"

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003F2AC);

extern s32 func_8003F764(s32 *arr);
extern s32 func_8003F82C(s32 a0);

void func_8003F674(s32 *src, s32 *dst) {
    s32 maxAbs;
    s32 shift;

    maxAbs = func_8003F764(src);
    shift = func_8003F82C(maxAbs);
    if (shift >= 0x10) {
        shift -= 0xF;
        dst[0] = src[0] >> shift;
        dst[1] = src[1] >> shift;
        dst[2] = src[2] >> shift;
        dst[3] = src[3] >> shift;
        dst[4] = src[4] >> shift;
        dst[5] = src[5] >> shift;
    } else {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        dst[4] = src[4];
        dst[5] = src[5];
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003F764);

s32 func_8003F82C(s32 a0) {
    s32 count;

    count = 0;
    if (a0 <= 0) {
        return count;
    }
    do {
        a0 >>= 1;
        count++;
    } while (a0 > 0);
    return count;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003F848);

extern void *D_800902E4;

void func_8003FB0C(void *a0) {
    D_800902E4 = a0;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FB1C);

extern void *D_8008E794;

void func_8003FBE4(void *a0) {
    D_8008E794 = a0;
}

extern void func_80021678(void *arg0); /* asm/psyq_GsLinkObject4.s, Psy-Q
                                           library, not game code */

void func_8003FBF4(Class6E99CObj *self) {
    func_80021678(self->unk10);
}

extern void func_80021580(s32 stride, s32 mask); /* asm/psyq_GsLinkObject4.s,
                                                     Psy-Q library, not game
                                                     code */

void func_8003FC18(s32 a0, s32 a1, TexPageDesc *desc) {
    desc->width = a0 & 0xFFFF;
    desc->height = a1 & 0xFFFF;
    desc->size = (4 << desc->shift) + desc->stride - 4;
    func_80021580(desc->stride, 1 << desc->shift);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FC70);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FCFC);

extern void func_80024B9C(s32 a0);
extern void func_80024BA8(s32 a0);

void func_8003FD4C(s32 a0, s32 a1) {
    func_80024B9C((-(a0 * 5 * 64)) / a1);
    func_80024BA8(0x1400000);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FDB0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FE2C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FED8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FF44);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8004001C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_80040024);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800400B0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_80040154);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800402F0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800403F8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8004042C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_80040490);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800404B4);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800404C0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800404D0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8004054C);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800405D0);
