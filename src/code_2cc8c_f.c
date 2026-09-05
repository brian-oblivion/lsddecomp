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

typedef struct { s8 r, g, b; } RGB80040790;

void func_80040790(Obj6EAC0 *self, u8 *dst, u8 *src, s32 overwrite) {
    u8 *d;
    d = dst;
    if (overwrite) {
        *(RGB80040790 *)d = *(RGB80040790 *)src;
    } else {
        d[0] += src[0];
        d[1] += src[1];
        d[2] += src[2];
    }
}

void func_800407F8(Obj6EAC0 *self, Pair32E99C *a1) {
    if (self->unkC != 0) {
        *(Pair32E99C *)&self->unk50 = *a1;
    }
}

void func_80040824(Obj6EAC0 *self, s32 *a1) {
    if (self->unkC != 0) {
        self->unk60 = ((u16 *)a1)[0];
        self->unk62 = ((u16 *)&a1[1])[0];
    }
}

void func_80040854(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3)
{
    void (*fn)();
    Obj6EAC0 *q;

    q = self;
    fn = q->methods->slot4C;
    /* The do/while(0) wrapper is a no-op scoping device, load-bearing for
     * delay-slot scheduling only -- see the match report. Without it GCC
     * swaps the prologue's $ra/$s1 callee-save STORE ORDER. */
    do {
        fn(q, a1, a2, a3);
        q->unk48 = 0;
        q->unk4C = a3;
    } while (0);
}

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

void func_80040D74(Obj6EAC0 *self, s32 a1) {
    Obj6EAC0 **elemp = self->unkB4 + self->unkAC;
    s32 i = self->unkAC;
    s32 bound = i;
    s32 count = i + self->unkAB;
    if (i < count) {
        do {
            Obj6EAC0 *elem = *elemp;
            s32 ab;
            elemp++;
            ab = self->unkAB;
            elem->methods->slotB8(elem, a1);
            i++;
            bound = self->unkAC;
        } while (i < (bound + self->unkAB));
    }
}

void func_80040E14(Obj6EAC0 *self, Pair32E99C *a1) {
    if (self->unkC != 0) {
        Pair32E99C buf;
        s32 i;
        s32 bound;
        Obj6EAC0 **elemp;

        func_80041C3C()->slotBC(self, a1);
        buf = *a1;
        i = 0;
        elemp = self->unkB4;
        if (i < self->unkA9) {
            do {
                (*elemp)->methods->slotBC(*elemp, &buf);
                buf.a += self->unkB0;
                bound = self->unkA9;
                elemp++;
                i++;
            } while (i < bound);
        }
    }
}

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
