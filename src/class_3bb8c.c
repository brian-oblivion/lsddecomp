/* First slice of the 365-function class_3bb8c block -- 20 functions,
 * 0x3BB8C..0x3CD88. The remainder is `class_3bb8c_b` and is still a
 * monolithic asm segment.
 *
 * Carve notes for whoever takes the NEXT slice: this block holds all 13 of
 * the game's PSX BIOS trampolines (`jr $t2` with the vector in $t2 and the
 * call number in $t1) and 38 switch jump tables. None of either landed in
 * THIS slice -- verified, not assumed -- which is why it needs no attached
 * rodata slot and no `hasm` segment. The next slice will hit both, and both
 * have to be dispositioned at carve time rather than discovered by a runner
 * that has already spent its attempt budget. See Gate 2 in
 * docs/PARALLEL-RUNS.md.
 */
#include "common.h"
#include "class_3bb8c.h"

s32 func_8004B38C(Obj866E8 *self, void *arg1, Unk6CObj *arg2, Descriptor10 *arg3) {
    s32 stackBuf[3];
    s32 ret;

    self->unk6C = arg2;
    self->unkBC = *arg3;
    ret = func_8004B44C(arg1, stackBuf, self->unk68, &self->unk54, arg3);
    return self->methods->slotF8(self, ret, stackBuf, &D_80086904);
}

s32 func_8004B418(Obj866E8 *self, void *arg1, void *arg2) {
    s32 outBuf[3];

    return func_8004B44C(arg1, outBuf, self->unk68, &self->unk54, arg2);
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B44C);

void func_8004B570(Obj866E8 *self) {
    self->unk70 = 1;
}

void func_8004B57C(Obj866E8 *self) {
    self->methods->slotC0(self);
    self->unk70 = 0;
}

/* func_8004B5BC -- see docs/match-reports/func_8004B5BC.md. */
s32 func_8004B5BC(Obj866E8 *self) {
    Descriptor10Ext buf;
    Elem *e;
    s32 key;
    s32 result;
    u16 oldRaw;

    if (self->methods->slot10C(self, &buf, 0) == 0) {
        return 0;
    }

    e = buf.unk24;
    key = e->unk4->unk32;
    result = D_800868FC[key];

    if (self->unk68->unk4 == 0) {
        self->methods->slotF8(self, buf.unk28, &buf.unkC, D_80086974[result]);
    }

    self->methods->slot128(self);

    oldRaw = *(u16 *)((u8 *)self + 0xBC);
    *(Descriptor10Ext *)((u8 *)self + 0xBC) = buf;

    if ((s16)oldRaw != *(s16 *)&buf) {
        self->methods->slot30(self, 5);
    }

    return result;
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004B700);

s32 func_8004B930(Obj866E8 *self, s32 val, s32 flag) {
    Unk68Struct *u;
    s32 divisor;
    s32 unk4;
    s32 count;
    s32 flags;
    s32 i;

    u = self->unk68;
    divisor = u->divisor;
    unk4 = u->unk4;
    count = u->count;
    if (unk4 == 0) {
        flags = (val < divisor) ? 3 : 0;
        if (val >= divisor * (count - 1)) {
            flags |= 0x60;
        }
        if (val % divisor == 0) {
            flags |= flag ? 0x25 : 4;
        }
        if ((val + 1) % divisor != 0) {
            return ~flags;
        }
        flags |= flag ? 0x10 : 0x52;
        return ~flags;
    } else {
        flags = -1;
        for (i = 0; i < count; i++) {
            flags <<= 1;
        }
        return ~flags;
    }
}

#if 0
/* STALL snapshot -- see docs/match-reports/func_8004BA40.md. Length CORRECT
 * (63/63 words), raw word-match 58/63. Round 27 (delta) tried three more
 * structural variants (named `s32 e4 = entry->unk4;` local, operand-order
 * swap `entry->unk4 + val`, and dropping the shared `value` local entirely
 * in favor of storing straight into `*(s32*)(arg1+4)` per branch) -- first
 * two: no change (still 58/63, identical residue). Third: MUCH worse
 * (31/63, address drift) -- removing the shared `value` local changes
 * register pressure across the whole if/else in the wrong direction.
 * Confirms this is genuine register identity, not a source-shape issue. */
s32 func_8004BA40(Obj866E8 *self, SetupEntry866E8 *arg1, s32 divisor, s32 flag, s32 val, s32 savedResult, s32 key)
{
    s32 mask = D_8008688C[key];
    s32 result;

    if ((savedResult & mask) == 0) {
        result = 0;
        goto nullCase;
    }

    if (self->unk68->unk4 == 0) {
        const Unk54Struct *entry = &D_800868A8[key];
        s32 value;

        if (entry->unk0 == 0) {
            value = val + entry->unk4;
        } else {
            s32 lo = divisor * entry->unk0;

            if (flag != 0) {
                value = val + (lo + entry->unk4);
            } else {
                value = val + (lo + entry->unk8);
            }
        }
        *(s32 *)((u8 *)arg1 + 4) = value;
    } else {
        *(s32 *)((u8 *)arg1 + 4) = val + key;
    }

    arg1->ptr0 = self->unk60(self->unk64, *(s32 *)((u8 *)arg1 + 4), 0, 0);
    do {} while (0);
    result = 1;
    goto storeKey;

nullCase:
    arg1->ptr0 = NULL;

storeKey:
    arg1->id = key;
    return result;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BA40);

#if 0
/* STALL snapshot -- see docs/match-reports/func_8004BB3C.md. 90/105 words
 * with CORRECT total length: every instruction in the loop body and the
 * epilogue matches retail one-for-one, including BOTH `addiu sN,sN,0xc`
 * walker increments. The residue is a whole-function $s3<->$s4 identity
 * swap plus the prologue scheduling that follows from it -- a
 * register-identity stall, which project rule 6 forbids fixing with a
 * register pin. The two-differently-BASED-walker shape below is what
 * recovered the previously-missing second increment; do not go back to a
 * single indexed base.
 *
 * ROUND 19 (bravo): re-verified per the head's mid-round drift-check
 * broadcast. Rebuilt exactly as below: 90/105, ZERO drift, compiled
 * length 0x1A4 (105 words) matching retail's own `.s` header exactly.
 * The claim in this report ("correct total length, no insertions or
 * deletions") is CONFIRMED accurate, unlike two other reports' claims
 * the same broadcast flagged as wrong. Not re-attempted further this
 * round -- the residue matches this round's independently-confirmed
 * "declaration order is inert" finding for this exact class.
 */
void func_8004BB3C(Obj866E8 *self, SetupEntry866E8 *arr1, s32 count) {
    SetupEntry866E8 *ep = arr1;
    s32 i;
    Elem *e;
    SetupSub866E8 *sp = (SetupSub866E8 *)((u8 *)arr1 + 4);

    for (i = 0; i < count; i++) {
        e = self->methods->slot118(self, sp->id);
        self->methods->slot88(self, 6, e, i);
        if (ep->ptr0 != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            e->unk4->unk30 = sp->rate;
            e->unk4->methods->slot78(e->unk4, ep->ptr0);
            e->flag = 1;
            self->unk1B0 = 1;
        } else {
            if (e->unk4->unk2C != 0) {
                self->methods->slot108(self, e);
            }
            if (e->unk4->unk2A != 0) {
                e->unk4->methods->slot74(e->unk4);
                e->flag = 0;
            }
        }
        ep++;
        sp++;
    }
    self->unk1B4 = func_8004BCE0(self);
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BB3C);

s32 func_8004BCE0(Obj866E8 *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 7; i++) {
        if (self->arr[i].flag != 0) {
            count++;
        }
    }
    return count;
}

void func_8004BD14(Obj866E8 *self, void *arg1, s32 mode) {
    s32 i;
    Elem *e;
    s32 curMode;

    if (mode != 2) {
        return;
    }
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk2E != 0) {
            e->unk4->unk2E = 0;
            self->methods->slot88(self, 7, e, i);
        }
        curMode = self->unk1B0;
        if (curMode == 1 && e->flag != 0) {
            if (e->unk4->unk2C != 0) {
                self->methods->slot104(self, e);
                e->unk4->unk2C = 2;
                e->flag = 0;
                if (--self->unk1B4 == 0) {
                    self->unk1B4 = 0;
                    self->unk1B0 = 0;
                    self->unk1B8 = curMode;
                }
            } else if (e->unk4->unk2A == 0) {
                e->flag = 0;
            }
        }
    }
}

/* STALL snapshot -- see docs/match-reports/func_8004BE54.md. 130/150 words
 * at CORRECT total length (no address drift): every instruction present,
 * none missing or extra. The residue is two well-documented classes only --
 * plain register identity (info/hdr's ResInfo866E8* lives in v0 in retail,
 * a1 here; a handful of others) and commutative-op operand-order
 * canonicalization (`or`/`addu` reversed regardless of C source order,
 * same phenomenon independently confirmed in func_8004C470's report).
 * Preserved here per convention -- not live C. */
#if 0
/* func_8004BE54 (Obj866E8Methods::slot104) -- own local view of several
 * classes reached only from here. Kept in this .c, not class_3bb8c.h: none
 * of the 11 sibling units sharing that header touch these. */

/* ElemTarget::field10's pointee -- a small size/offset header block. */
struct ResInfo866E8 {
    u8 pad0[0x4];
    s32 unk4;   /* +0x004 */
    s32 unk8;   /* +0x008 */
};

/* target->unk2C's pointee -- only its self-only teardown slot is reached. */
typedef struct LinkResourceMethods LinkResourceMethods;
typedef struct LinkResource {
    LinkResourceMethods *methods;   /* +0x000 */
} LinkResource;
struct LinkResourceMethods {
    u8 pad0[0x4];
    void (*slot4)(LinkResource *self);  /* +0x004 */
};

/* slot78's own non-0/non-(-1) return value -- a resolved link-target
 * record, read only for its `unk10` (tmd base address). */
typedef struct LinkResEntry {
    u8 pad0[0x10];
    s32 unk10;   /* +0x010 */
} LinkResEntry;

/* Elem::unk8's pointee -- coordinates the per-frame GPU link/load loop. */
typedef struct LinkTarget866E8Methods LinkTarget866E8Methods;
struct LinkTarget866E8 {  /* forward-typedef'd in class_3bb8c.h */
    LinkTarget866E8Methods *methods;  /* +0x000 */
    u8 pad004[0x010 - 0x004];
    s32 unk10;                          /* +0x010 */
    s32 unk14;                            /* +0x014 */
    u8 pad018[0x02C - 0x018];
    LinkResource *unk2C;                    /* +0x02C */
};
struct LinkTarget866E8Methods {
    u8 pad000[0x78];
    s32 (*slot78)(LinkTarget866E8 *self, void *outBuf, s32 arg2); /* +0x078 */
};

/* EntryChildObj::unk14's pointee. */
typedef struct EntryGpuVec {
    u8 pad0[0x10];
    s16 unk10;   /* +0x010 */
    s16 unk12;   /* +0x012 */
    s16 unk14;   /* +0x014 */
} EntryGpuVec;
struct EntryGpu {
    s32 unk0;                    /* +0x000 */
    u8 pad4[0x18 - 0x4];
    s32 unk18;                    /* +0x018 */
    s32 unk1C;                      /* +0x01C */
    s32 unk20;                        /* +0x020 */
    u8 pad24[0x44 - 0x24];
    EntryGpuVec *unk44;                  /* +0x044 */
};

/* slot78's own stack-allocated outBuf, 0x38 bytes -- fields established
 * purely from this function's own reads of it. */
typedef struct BE54OutBuf {
    u8 pad0[0xC];
    s32 b;          /* +0x00C */
    s32 c;            /* +0x010 */
    s32 d;              /* +0x014 */
    u8 pad18[0x1A - 0x18];
    u16 h1;               /* +0x01A */
    u8 pad1C[0x2E - 0x1C];
    u16 h2;                 /* +0x02E */
    s32 flag2;                /* +0x030 */
    s32 found;                  /* +0x034 */
    u8 pad38[0x8];               /* trailing bytes never read/written by this function */
} BE54OutBuf;

typedef struct BE54LoadReq {
    s32 field0;
    u8 pad4[0xC];
} BE54LoadReq;

extern LinkResource *func_80043840(BE54LoadReq *req);
extern void GsLinkObject4(s32 tmd, void *objp, s32 n);

void func_8004BE54(Obj866E8 *self, Elem *entry) {
    ResInfo866E8 *info;
    ElemTarget *hdr;
    LinkTarget866E8 *target;
    LinkResource *res;
    EntryChildObj **slot;
    u8 *base;
    EntryGpu *gpu;
    EntryGpuVec *vec;
    s32 b;
    s32 c;
    s32 d;
    s32 h1;
    s32 idxVal;
    s32 flagBit;
    s32 i;
    s32 off1;
    s32 off2;
    BE54OutBuf outBuf;
    BE54LoadReq req;

    hdr = entry->unk4;
    target = entry->unk8;
    info = hdr->field10;
    target->unk10 = (s32)info + info->unk4;
    target->unk14 = 0;
    res = target->unk2C;
    if (res != 0) {
        res->methods->slot4(res);
    }
    info = hdr->field10;
    req.field0 = (s32)info + info->unk4 + info->unk8;
    target->unk2C = func_80043840(&req);
    outBuf.found = 0;

    i = 0;
    flagBit = 0x80000000;
    off1 = 0;
    off2 = 0x640;
    for (;;) {
        idxVal = target->methods->slot78(target, &outBuf, i);
        if (idxVal == 0) {
            return;
        }
        if (idxVal == -1) {
            slot = (EntryChildObj **)((u8 *)entry->unk10 + off1);
            (*slot)->unk10 |= flagBit;
            (*slot)->unk20 = 0;
            (*slot)->unk18 = 0;
        } else {
            base = (u8 *)entry->unk10;
            if (outBuf.flag2 != 0) {
                slot = (EntryChildObj **)(base + off2);
                off2 += 4;
            } else {
                slot = (EntryChildObj **)(base + off1);
            }
            (*slot)->unk20 = idxVal;
            (*slot)->unk18 = ((LinkResEntry *)(*slot)->unk20)->unk10;
            GsLinkObject4(((LinkResEntry *)(*slot)->unk20)->unk10, (u8 *)(*slot) + 0x10, 0);
            gpu = (*slot)->unk14;
            __asm__("");
            b = outBuf.b;
            c = outBuf.c;
            d = outBuf.d;
            gpu->unk18 = b;
            gpu->unk1C = c;
            gpu->unk20 = d;
            vec = (*slot)->unk14->unk44;
            vec->unk10 = 0;
            h1 = outBuf.h1;
            vec->unk14 = 0;
            vec->unk12 = h1;
            (*slot)->unk36 = outBuf.h2;
            gpu = (*slot)->unk14;
            gpu->unk0 = 0;
            (*slot)->unk10 |= flagBit;
        }
        if (outBuf.found) {
            (*slot)->unk38 = *(EntryChildObj **)((u8 *)entry->unk10 + off2);
            continue;
        }
        off1 += 4;
        (*slot)->unk38 = 0;
        i++;
    }
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004BE54);

void func_8004C0AC(Obj866E8 *self, Elem *entry) {
    EntryChildObj **p;
    EntryChildObj **end;

    if (entry->unk4->unk30 >= 0) {
        entry->unk4->methods->slot7C(entry->unk4, entry);
        end = entry->unk10 + 0x19A;
        for (p = entry->unk10; p < end; p++) {
            (*p)->unk10 |= 0x80000000;
            (*p)->unk20 = 0;
            (*p)->unk18 = 0;
        }
    }
}

Descriptor10 *func_8004C158(Obj866E8 *self, Descriptor10Ext *arg1, void **out) {
    void *v1;

    v1 = (u8 *)self->unk6C->unk14 + 0x18;
    if (out != 0) {
        *out = v1;
    }
    if (arg1 != 0) {
        if (self->methods->slot110(self, arg1, v1) != 0) {
            return 0;
        }
    }
    return &self->unkBC;
}

#if 0
/* STALLED at 72/106 words -- see docs/match-reports/func_8004C1C0.md for
 * the full analysis (two independent residue classes: an $a1-vs-$a3
 * register-identity choice for u14b, and a store-then-reread narrow-field
 * codegen sensitivity confirmed with an isolated toolchain reproducer).
 * ROUND 27 (delta): re-verified 72/106, no drift; two more attempts on
 * class 2 (dropping the b2/b3 locals and re-reading `out->base.b2/b3`
 * directly at the h4/h8 use sites) -- both REGRESSED (drift, extra saved
 * register in the prologue), reverted. Not re-attempted further this
 * round. Preserved here per convention. */
s32 func_8004C1C0(Obj866E8 *self, Descriptor10Ext *out, QueryPos866E8 *in) {
    Elem *e;
    Unk14Obj *u14a;
    Unk14Obj *u14b;
    s32 rate;
    s32 t;
    s8 b2;
    s8 b3;

    e = self->methods->slot11C(self, in);
    if (e != 0) {
        rate = e->unk4->unk30;
        out->unk28 = rate;
        func_8004C368(self, (u8 *)out, rate);

        u14a = self->methods->slot118(self, e->unk4->unk32)->unkC->unk14;
        out->unkC = u14a->unk18.w + 0x5000;
        out->unk10 = u14a->unk1C;
        out->unk14 = u14a->unk20.w + 0x5000;

        u14b = e->unkC->unk14;
        out->unk18 = in->unk0.w - out->unkC;
        out->unk1C = in->unk4.w;
        out->unk20 = in->unk8.w - out->unk14;

        t = in->unk0.w - u14b->unk18.w;
        if (t < 0) {
            t += 0x7FF;
        }
        b2 = t >> 11;
        out->base.b2 = b2;

        t = in->unk8.w - u14b->unk20.w;
        if (t < 0) {
            t += 0x7FF;
        }
        b3 = t >> 11;
        out->base.b3 = b3;

        out->base.h4 = (in->unk0.h - 0x400) - (u14b->unk18.h + (b2 << 11));
        out->base.h6 = in->unk4.h;
        out->unk24 = e;
        out->base.h8 = (in->unk8.h - 0x400) - (u14b->unk20.h + (b3 << 11));

        return 0;
    }
    return 1;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C1C0);

void func_8004C368(Obj866E8 *self, u8 *out, s32 val) {
    out[0] = val % self->unk68->divisor;
    out[1] = val / self->unk68->divisor;
}

Unk1BCObj *func_8004C3F0(Obj866E8 *self, u8 *out) {
    func_8004C368(self, out, self->unk1BC->unk4->unk30);
    return self->unk1BC;
}

Elem *func_8004C434(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            return e;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C470);
