#include "common.h"
#include "code_2cc8c.h"

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040664);

s32 func_800406E4(Obj6EAC0 *self, s32 a1) {
    return func_8001EDAC(&self->unk58, 0x1F, 1, a1 == 0) == 0;
}

s32 func_80040714(Obj6EAC0 *self, s32 a1) {
    return func_8001EDAC(&self->unk58, 0x1E, 1, a1 != 0);
}

s32 func_80040740(Obj6EAC0 *self, s32 a1) {
    return func_8001EDAC(&self->unk58, 0x1C, 2, a1);
}

void func_80040790(Obj6EAC0 *self, u8 *dst, u8 *src, s32 overwrite);

void func_8004076C(Obj6EAC0 *self, s32 overwrite, u8 *src) {
    func_80040790(self, self->unk64, src, overwrite);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040790);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_800407F8);

void func_80040824(Obj6EAC0 *self, s32 *a1) {
    if (self->unkC != 0) {
        self->unk60 = ((u16 *)a1)[0];
        self->unk62 = ((u16 *)&a1[1])[0];
    }
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040854);

void func_800408A0(Obj6EAC0 *self, s32 a1) {
    self->unk44 = a1;
}

s32 func_800408A8(Obj6EAC0 *self, s32 a1) {
    return self->unk68 = (1 << a1) - 1;
}

Obj6EAC0Methods *func_800408BC(void) {
    return &D_8006EAC0;
}

Obj6EAC0Methods *func_80040FB0(void);

Unk64Elem *func_800408CC(void *ctx, s32 len, char *name) {
    Obj6EAC0 *self = func_80017B34(0xB8);
    if (self != NULL) {
        func_80040FB0()->slot08(self, (s32)ctx, len, (s32)name);
        return (Unk64Elem *)self;
    }
    return NULL;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040948);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040A30);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040A88);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040AE8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040C00);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040CD0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040D74);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040E14);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040EDC);

void func_80040F20(void) {
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040F28);

void func_80040FA0(void) {
}

void func_80040FA8(Obj6EAC0 *self, s32 a1) {
    self->unkB0 = a1;
}

Obj6EAC0Methods *func_80040FB0(void) {
    return &D_8006EB90;
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040FC0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80041020);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_8004109C);
