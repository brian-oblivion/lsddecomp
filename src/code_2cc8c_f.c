/*
 * code_2cc8c_f -- the `Obj6EAC0` class (base table `D_8006EAC0`, override
 * table `D_8006EB90`; see include/code_2cc8c.h for the full derivation)
 * plus three unrelated free functions that happen to live in this file
 * (`DecodeFullWidthSjis`/`EncodeFullWidthSjis`/`FormatFullWidthNumber`,
 * confirmed by their OWN callers elsewhere to take a plain buffer, not an
 * `Obj6EAC0 *`, despite matching this file's dominant `self`-typed style).
 *
 * Named round 54 (runner alpha, track 3); every non-`func_` symbol below
 * is new this round. Working hypothesis (tier B, this unit's own evidence
 * only -- see include/code_2cc8c.h's own `Obj6EAC0` comment for the full
 * case): a small on-screen text/digit display. A leaf instance
 * (`hasChildren`==0) is one character glyph (`SetChar`); a container
 * instance holds a `children` array laid out along one axis, `posX`/
 * `posY` as a running cursor advanced by `childPitch` per child, with one
 * extra gap inserted at `gapIndex` (plausibly a decimal point). `SetText`
 * walks a string dispatching one child per byte, and this file's own
 * `FormatFullWidthNumber` builds exactly the padded, Shift-JIS-encoded
 * digit string `SetText` would consume.
 *
 * All 20 non-trivial functions in this unit are MATCHED; zero live
 * INCLUDE_ASM, zero NON_MATCHING bodies. `Obj6EAC0__NoOpSetter`/
 * `Obj6EAC0__NoOpSlotD0` are splat-generated `jr $ra; nop` occupants.
 */
#include "common.h"
#include "code_2cc8c.h"

void Obj6EAC0__Layout(Obj6EAC0 *self, s32 a1, void *a2) {
    if (self->hasChildren == 0) {
        GetClass6B5CCMethods()->attachToParent((Class6B5CC *)self, (Class6B5CC *)a1, 0);
        self->methods->slotBC(self, a2);
    }
}

s32 func_800406E4(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->flags, 0x1F, 1, a1 == 0) == 0;
}

s32 func_80040714(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->flags, 0x1E, 1, a1 != 0);
}

s32 func_80040740(Obj6EAC0 *self, s32 a1) {
    return GetSetBitField(&self->flags, 0x1C, 2, a1);
}

void Obj6EAC0__ApplyColor(Obj6EAC0 *self, u8 *dst, u8 *src, s32 overwrite);

void Obj6EAC0__SetColor(Obj6EAC0 *self, s32 overwrite, u8 *src) {
    Obj6EAC0__ApplyColor(self, self->color, src, overwrite);
}

typedef struct { s8 r, g, b; } RGB80040790;

void Obj6EAC0__ApplyColor(Obj6EAC0 *self, u8 *dst, u8 *src, s32 overwrite) {
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

void Obj6EAC0__SetPosition(Obj6EAC0 *self, Pair32E99C *a1) {
    if (self->hasChildren != 0) {
        *(Pair32E99C *)&self->posX = *a1;
    }
}

void func_80040824(Obj6EAC0 *self, s32 *a1) {
    if (self->hasChildren != 0) {
        self->unk60 = ((u16 *)a1)[0];
        self->unk62 = ((u16 *)&a1[1])[0];
    }
}

void Obj6EAC0__SetChar(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3)
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

s32 Obj6EAC0__SetMask(Obj6EAC0 *self, s32 a1) {
    return self->mask = (1 << a1) - 1;
}

Obj6EAC0Methods *Obj6EAC0__GetBaseMethods(void) {
    return &D_8006EAC0;
}

Obj6EAC0Methods *Obj6EAC0__GetDerivedMethods(void);

Unk64Elem *New_Obj6EAC0(void *ctx, s32 len, char *name) {
    Obj6EAC0 *self = BMemPMgrAlloc(0xB8);
    if (self != NULL) {
        Obj6EAC0__GetDerivedMethods()->slot08(self, (s32)ctx, len, (s32)name);
        return (Unk64Elem *)self;
    }
    return NULL;
}

void Obj6EAC0__Construct(Obj6EAC0 *self, s32 a1, s32 a2, s32 a3) {
    s32 i;
    Obj6EAC0 **cursor;

    ((void (*)(Obj6EAC0 *, s32, s32))Get_vtable_D8006EC74()->slot08)(self, a1, 0x20);
    self->methods = Obj6EAC0__GetDerivedMethods();
    self->totalChildCount = a2;
    self->childCount = a2;
    self->childStart = 0;
    self->gapIndex = 0;
    cursor = BMemPMgrAlloc(a2 * 4);
    if (cursor != NULL) {
        self->children = cursor;
        i = 0;
        if (i < a2) {
            do {
                *cursor = New_D8006EC74(a1, 0x20);
                i++;
                cursor++;
            } while (i < a2);
        }
        self->methods->slot40(self, a3);
    }
}

void Obj6EAC0__Destruct(Obj6EAC0 *self) {
    ReleaseBasicClassArray(self->children, self->totalChildCount);
    self->children = BMemPMgrFree(self->children);
    Get_vtable_D8006EC74()->slot0C(self);
}

void Obj6EAC0__FinishConstruct(Obj6EAC0 *self, s32 a1) {
    self->methods->slotD4(self, 7);
    self->methods->slotCC(self, a1);
}

void Obj6EAC0__LayoutChildrenWithGap(Obj6EAC0 *self, s32 a1, Pair32E99C *a2) {
    Pair32E99C buf;
    s32 i, bound;
    Obj6EAC0 **elemp;

    if (self->hasChildren != 0) {
        return;
    }
    Get_vtable_D8006EC74()->slot4C(self, a1, a2);
    buf = *a2;
    elemp = self->children + self->childStart;
    i = self->childStart;
    bound = i;
    if (i < bound + self->childCount) {
        do {
            if (self->gapIndex != 0 && i == self->gapIndex) {
                buf.a += 0x10;
            }
            (*elemp)->methods->slot4C(*elemp, self, &buf);
            buf.a += self->childPitch;
            bound = self->childStart;
            elemp++;
            i++;
        } while (i < bound + self->childCount);
    }
}

void func_80040C00(Obj6EAC0 *self) {
    Obj6EAC0 **elemp;
    s32 i, bound;

    if (self->hasChildren != 0) {
        if (self->children != NULL) {
            elemp = self->children + self->childStart;
            i = self->childStart;
            bound = i;
            if (i < bound + self->childCount) {
                do {
                    (*elemp)->methods->slot50(*elemp);
                    elemp++;
                    bound = self->childStart;
                    i++;
                } while (i < bound + self->childCount);
            }
        }
        Get_vtable_D8006EC74()->slot50(self);
    }
}

s32 Obj6EAC0__QueryChildren(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 **elemp = self->children + self->childStart;
    s32 i = self->childStart;
    s32 bound = i;
    if (i < bound + self->childCount) {
        do {
            Obj6EAC0 *elem = *elemp;
            s32 result;
            elemp++;
            i++;
            result = elem->methods->slot60(elem, a1);
            bound = self->childStart;
            a2 = result;
        } while (i < bound + self->childCount);
    }
    return a2;
}

void Obj6EAC0__PropagateColor(Obj6EAC0 *self, s32 a1) {
    Obj6EAC0 **elemp = self->children + self->childStart;
    s32 i = self->childStart;
    s32 bound = i;
    s32 count = i + self->childCount;
    if (i < count) {
        do {
            Obj6EAC0 *elem = *elemp;
            s32 ab;
            elemp++;
            ab = self->childCount;
            elem->methods->slotB8(elem, a1);
            i++;
            bound = self->childStart;
        } while (i < (bound + self->childCount));
    }
}

void Obj6EAC0__LayoutChildren(Obj6EAC0 *self, Pair32E99C *a1) {
    if (self->hasChildren != 0) {
        Pair32E99C buf;
        s32 i;
        s32 bound;
        Obj6EAC0 **elemp;

        Get_vtable_D8006EC74()->slotBC(self, a1);
        buf = *a1;
        i = 0;
        elemp = self->children;
        if (i < self->totalChildCount) {
            do {
                (*elemp)->methods->slotBC(*elemp, &buf);
                buf.a += self->childPitch;
                bound = self->totalChildCount;
                elemp++;
                i++;
            } while (i < bound);
        }
    }
}

void Obj6EAC0__SetChildChar(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 *elem = self->children[a2];
    elem->methods->slotC4(elem, a1 & 0xFF);
}

void Obj6EAC0__NoOpSetter(void) {
}

void Obj6EAC0__SetText(Obj6EAC0 *self, u8 *a1) {
    Obj6EAC0 **elemp = self->children;
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

void Obj6EAC0__NoOpSlotD0(void) {
}

void Obj6EAC0__SetChildPitch(Obj6EAC0 *self, s32 a1) {
    self->childPitch = a1;
}

Obj6EAC0Methods *Obj6EAC0__GetDerivedMethods(void) {
    return &D_8006EB90;
}

/* DecodeFullWidthSjis -- MATCHED round 38 (24/24). A permuter search (208
 * iterations, rc=0) closed the last residue: retail materializes the
 * 0x40 comparison constant into its own register BEFORE copying `dst`
 * into `d`, and GCC 2.6.3 only reproduces that emission order when the
 * constant is named by a separate local assigned first. See
 * docs/match-reports/DecodeFullWidthSjis.md. */
u8 *DecodeFullWidthSjis(u8 *dst, u8 *src) {
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

/* EncodeFullWidthSjis -- MATCHED round 38 (31/31). Round 37 got structure and
 * length exact via the two-cursor idiom (`d = dst; dst++; *d = x;`),
 * leaving a pure 3-way register-identity residue. A permuter search
 * (158 iterations, rc=0) closed it: copying the second byte's value
 * into its own local (`trail`) before using it in the comparisons and
 * arithmetic, instead of reusing `c` directly, changes GCC 2.6.3's
 * register allocation to match retail's exactly. See
 * docs/match-reports/EncodeFullWidthSjis.md. */
u8 *EncodeFullWidthSjis(u8 *dst, u8 *src) {
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

/* FormatFullWidthNumber -- MATCHED round 38 (56/56). Round 35 got structure and
 * length exact (padded/text VLAs, strlen/itoa naming fixed post-SDK-object
 * renaming) leaving a 4-value register-identity residue (fill/text
 * swapped relative to retail). A permuter search (733 iterations, rc=0)
 * closed it: declaration order text/fill/padded (not round 35's
 * padded/text) PLUS splitting `fill = width - strlen(...)` into two
 * statements (`fill = strlen(...); fill = width - fill;`) together
 * reproduce retail's exact register assignment. Also fixed a stale
 * prototype: the preserved body's forward declaration of EncodeFullWidthSjis
 * as `(Obj6EAC0 *, char *)` predates that function's own round-38 match
 * as `u8 *EncodeFullWidthSjis(u8 *, u8 *)` -- calling it now needs `self` cast
 * to `u8 *`. See docs/match-reports/FormatFullWidthNumber.md. */
extern char *strcpy(char *dst, char *src);
extern void *memset(unsigned char *dst, unsigned char c, int n);
extern int strlen(char *s);
extern char *itoa(int n);

void FormatFullWidthNumber(Obj6EAC0 *self, s32 a1, s32 width, s32 unpadded) {
    char text[width + 1];
    s32 fill;
    char padded[width + 1];

    fill = strlen(strcpy(text, itoa(a1)));
    fill = width - fill;
    if (unpadded == 0) {
        memset((unsigned char *)padded, '0', width);
        strcpy(&padded[fill], text);
    }
    EncodeFullWidthSjis((u8 *)self, (u8 *)(unpadded != 0 ? text : padded));
}
