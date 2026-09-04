#include "common.h"
#include "code_2cc8c.h"

void func_80040664(Obj6EAC0 *self, s32 a1, void *a2) {
    if (self->unkC == 0) {
        func_8001E57C()->slot4C(self, a1, 0);
        self->methods->slotBC(self, a2);
    }
}

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

void func_80040948(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3) {
    s32 i;
    Obj6EAC0 **cursor;

    ((void (*)(Obj6EAC0 *, s32, s32))func_80041C3C()->slot08)(self, a1, 0x20);
    self->methods = func_80040FB0();
    self->unkA9 = a2;
    self->unkAB = a2;
    self->unkAC = 0;
    self->unkAA = 0;
    cursor = func_80017B34(a2 * 4);
    if (cursor != NULL) {
        self->unkB4 = cursor;
        i = 0;
        if (i < a2) {
            do {
                *cursor = func_80041AB4(a1, 0x20);
                i++;
                cursor++;
            } while (i < a2);
        }
        self->methods->slot40(self, a3);
    }
}

void func_80040A30(Obj6EAC0 *self) {
    func_800183DC(self->unkB4, self->unkA9);
    self->unkB4 = func_80017CFC(self->unkB4);
    func_80041C3C()->slot0C(self);
}

void func_80040A88(Obj6EAC0 *self, s32 a1) {
    self->methods->slotD4(self, 7);
    self->methods->slotCC(self, a1);
}

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040AE8);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040C00);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040CD0);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040D74);

INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_80040E14);

void func_80040EDC(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 *elem = self->unkB4[a2];
    elem->methods->slotC4(elem, a1 & 0xFF);
}

void func_80040F20(void) {
}

void func_80040F28(Obj6EAC0 *self, u8 *a1) {
    Obj6EAC0 **elemp = self->unkB4;
    u8 *p = a1;
    if (p != NULL && *p != 0) {
        do {
            Obj6EAC0 *elem = *elemp;
            elem->methods->slotC4(elem, *p);
            p++;
            elemp++;
        } while (*p != 0);
    }
}

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
