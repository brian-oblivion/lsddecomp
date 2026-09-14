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

void func_80040AE8(Obj6EAC0 *self, s32 a1, Pair32E99C *a2) {
    Pair32E99C buf;
    s32 i, bound;
    Obj6EAC0 **elemp;

    if (self->unkC != 0) {
        return;
    }
    func_80041C3C()->slot4C(self, a1, a2);
    buf = *a2;
    elemp = self->unkB4 + self->unkAC;
    i = self->unkAC;
    bound = i;
    if (i < bound + self->unkAB) {
        do {
            if (self->unkAA != 0 && i == self->unkAA) {
                buf.a += 0x10;
            }
            (*elemp)->methods->slot4C(*elemp, self, &buf);
            buf.a += self->unkB0;
            bound = self->unkAC;
            elemp++;
            i++;
        } while (i < bound + self->unkAB);
    }
}

void func_80040C00(Obj6EAC0 *self) {
    Obj6EAC0 **elemp;
    s32 i, bound;

    if (self->unkC != 0) {
        if (self->unkB4 != NULL) {
            elemp = self->unkB4 + self->unkAC;
            i = self->unkAC;
            bound = i;
            if (i < bound + self->unkAB) {
                do {
                    (*elemp)->methods->slot50(*elemp);
                    elemp++;
                    bound = self->unkAC;
                    i++;
                } while (i < bound + self->unkAB);
            }
        }
        func_80041C3C()->slot50(self);
    }
}

s32 func_80040CD0(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 **elemp = self->unkB4 + self->unkAC;
    s32 i = self->unkAC;
    s32 bound = i;
    if (i < bound + self->unkAB) {
        do {
            Obj6EAC0 *elem = *elemp;
            s32 result;
            elemp++;
            i++;
            result = elem->methods->slot60(elem, a1);
            bound = self->unkAC;
            a2 = result;
        } while (i < bound + self->unkAB);
    }
    return a2;
}

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

/* func_80040FC0 -- MATCHED round 38 (24/24). A permuter search (208
 * iterations, rc=0) closed the last residue: retail materializes the
 * 0x40 comparison constant into its own register BEFORE copying `dst`
 * into `d`, and GCC 2.6.3 only reproduces that emission order when the
 * constant is named by a separate local assigned first. See
 * docs/match-reports/func_80040FC0.md. */
u8 *func_80040FC0(u8 *dst, u8 *src) {
    u8 *d;
    u32 special;
    u32 c;
    u32 v;
    u32 peek;

    if (*src++ != 0) {
        special = 0x40;
        d = dst;
        do {
            d++;
            c = *src;
            dst++;
            if (c < 0x80 && c != special) {
                v = c - 0x1F;
            } else {
                v = c - 0x20;
            }
            src++;
            d[-1] = v;
            peek = *src;
            src++;
        } while (peek != 0);
    }
    *dst = 0;
    return dst;
}

/* func_80041020 -- MATCHED round 38 (31/31). Round 37 got structure and
 * length exact via the two-cursor idiom (`d = dst; dst++; *d = x;`),
 * leaving a pure 3-way register-identity residue. A permuter search
 * (158 iterations, rc=0) closed it: copying the second byte's value
 * into its own local (`trail`) before using it in the comparisons and
 * arithmetic, instead of reusing `c` directly, changes GCC 2.6.3's
 * register allocation to match retail's exactly. See
 * docs/match-reports/func_80041020.md. */
u8 *func_80041020(u8 *dst, u8 *src) {
    u8 *d;
    u32 c;
    u32 v;
    u32 lead;
    u32 trail;

    if (*src != 0) {
        do {
            d = dst;
            dst++;
            c = *src;
            if (c >= 0x30) {
                lead = 0x82;
            } else {
                lead = 0x81;
            }
            *d = lead;
            d = dst;
            dst++;
            c = *src;
            trail = c;
            if (trail < 0x60 && trail != 0x20) {
                v = trail + 0x1F;
            } else {
                v = trail + 0x20;
            }
            src++;
            *d = v;
        } while (*src != 0);
    }
    *dst = 0;
    return dst;
}

/* func_8004109C -- STALL (register identity), best 49/56, correct length.
 * See docs/match-reports/func_8004109C.md: round 35 caught the round-23/27
 * preserved body calling two symbols (func_80013348, func_800411A8) that no
 * longer exist post round-34 SDK-object renaming (they are strlen/itoa now)
 * -- it could never have linked, so the recorded 42/56 was never actually
 * measured. Fixing the names and reordering the two VLA declarations
 * (padded before text) gets two of the four permuted registers exactly
 * right; only fill/text remain swapped relative to retail. Preserved body
 * and its declarations are inlined in that report. */
INCLUDE_ASM("asm/nonmatchings/code_2cc8c_f", func_8004109C);
