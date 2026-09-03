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

extern s32 D_800902E0;
extern void func_80012C20(const char *fmt, s32 arg1); /* asm/psyq_2258.s,
                                                           Psy-Q printf-like;
                                                           declared locally
                                                           with THIS call
                                                           site's own arity
                                                           (fmt + 1 vararg),
                                                           same convention as
                                                           code_8220.h's own
                                                           independent
                                                           extern for it */

void func_8003FC70(s32 mode) {
    if (mode == 1) {
        goto set;
    }
    if (mode < 2) {
        if (mode == 0) {
            goto zero;
        }
        goto err;
    }
    if (mode == 2) {
        goto set;
    }
    if (mode == 3) {
        goto set;
    }
    goto err;
zero:
    D_800902E0 = 0;
    return;
set:
    D_800902E0 = mode;
    return;
err:
    func_80012C20("not supported light mode %d\n", mode);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FCFC);

extern void func_80024B9C(s32 a0);
extern void func_80024BA8(s32 a0);

void func_8003FD4C(s32 a0, s32 a1) {
    func_80024B9C((-(a0 * 5 * 64)) / a1);
    func_80024BA8(0x1400000);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8003FDB0);

void func_8003FE2C(Class6E99CObj *self, void *a1, s32 a2, s32 a3) {
    ClassEAC0Methods *base;
    void *tableEntry;

    base = func_800408BC();
    if (a2 != 0) {
        tableEntry = &D_8006EA90[a2 * 3];
    } else {
        tableEntry = D_8006EAA8;
    }
    base->ctor((ClassEAC0Obj *)self, a1, tableEntry, a3);
    self->methods = func_800404C0();
    self->methods->slot40(self, a2);
}

void func_8003FED8(Class6E99CObj *self, s32 a1) {
    self->unk70 = a1;
    self->unk6C = 0;
    self->unk74 = 0xA;
    self->unk78 = 0;
    self->unk7C = 0;
    self->methods->slot60(self, 0);
    self->methods->slot64(self, 0);
    self->unk98 = 0;
}

void func_8003FF44(Class6E99CObj *self, void *a1, s32 a2) {
    s32 old;

    if (a2 != 2) {
        return;
    }
    old = self->unk80;
    self->unk80 = old - 1;
    if (old > 0) {
        if (self->unk7C == 9) {
            return;
        }
        if (self->unk78 & 4) {
            self->unk64 += (u8)self->unk74;
        }
        if (self->unk78 & 2) {
            self->unk65 += (u8)self->unk74;
        }
        if (self->unk78 & 1) {
            self->unk66 += (u8)self->unk74;
        }
    } else {
        self->methods->slotE0(self, a1);
    }
}

void func_8004001C(Class6E99CObj *self, s32 a1) {
    self->unk74 = a1;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_80040024);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800400B0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_80040154);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800402F0);

void *func_800403F8(Class6E99CObj *self) {
    if (self->unk78 == 0xF) {
        return D_8006EAA8;
    }
    return &D_8006EA90[self->unk78 * 3];
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_8004042C);

void func_80040490(Class6E99CObj *self) {
    s32 t0, t1;

    t0 = self->unk90;
    t1 = self->unk94;
    self->unk50 = t0;
    self->unk54 = t1;
    __asm__("" ::: "memory");
    self->unk60 = self->unk88;
    self->unk62 = self->unk8C;
}

void func_800404B4(Class6E99CObj *self, s32 a1, s32 a2) {
    self->unk98 = a1;
    self->unk9C = a2;
}

Class6E99CMethods *func_800404C0(void) {
    return &D_8006E99C;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_e", func_800404D0);

void func_8004054C(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    func_8001E57C()->ctor(self);
    self->methods = func_800408BC();
    self->methods->slot40(self, a1, a2, a3);
}

void func_800405D0(ClassEAC0Obj *self, SkipShort2 *a1, void *a2, s32 a3) {
    ClassEAC0Methods *methods;

    self->unk44 = a3;
    self->unk48 = 1;
    self->unk4C = 0;
    self->unk58 = 0;
    self->unk5C = 0;
    self->unk5E = 0;
    self->unk60 = a1->x;
    self->unk62 = a1->y;
    methods = self->methods;
    if (a2 == NULL) {
        a2 = D_8008A924;
    }
    methods->slotB8(self, 1, a2);
    self->methods->slotCC(self, 0xD);
}
