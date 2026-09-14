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

/* MATCH, round 40 (bravo): permuter-found zero, first-ever search on this
 * function (1838 iterations, rc=0). The lead: hoist the shared `0x400`
 * constant used by BOTH `arg0[0]`/`arg0[2]`'s tail addend into a named
 * local, declared between the `outBuf[1]` and `outBuf[2]` assignment
 * statements -- the exact position retail's own constant-load sits,
 * confirmed by the score dropping straight to 0. Every prior round's
 * attempts targeted the outBuf[0]/outBuf[2] STORE-vs-LOAD scheduling
 * directly and never touched this constant; the permuter found a
 * completely different axis. See docs/match-reports/func_8004B44C.md. */
s32 func_8004B44C(s32 *arg0, s32 *outBuf, Unk68Struct *arg2, Unk54Struct *arg3, Descriptor10 *arg4) {
    s32 idx;
    s32 factor;
    s32 sum;
    s32 v1;
    s32 a0v;
    s32 off;

    if (arg2->unk4 == 0) {
        idx = arg4->b1;
        factor = arg2->count;
        sum = arg4->b0 + arg2->divisor * idx;
    } else {
        factor = 1;
        idx = 0;
        sum = 0;
    }
    v1 = (arg3->unk0 - arg2->divisor * 0x5000) + arg4->b0 * 0xA000;
    a0v = arg3->unk8 - factor * 0x5000;
    outBuf[0] = v1;
    if (idx & 1) {
        outBuf[0] = v1 - 0x5000;
    }
    outBuf[1] = arg3->unk4;
    off = 0x400;
    outBuf[2] = a0v + idx * 0xA000;
    arg0[0] = (arg4->b2 << 11) + outBuf[0] + (arg4->h4 + off);
    arg0[1] = arg4->h6 + outBuf[1];
    arg0[2] = (arg4->b3 << 11) + outBuf[2] + (arg4->h8 + off);
    outBuf[0] += 0x5000;
    outBuf[2] = outBuf[2] + 0x5000;
    return sum;
}

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

/* STALL snapshot -- see docs/match-reports/func_8004B700.md. 137/140 words,
 * correct length, no drift (round 40 permuter-found improvement, up from
 * 125/140). ROUND 40 (bravo): first-ever permuter search on this function
 * (37155 iterations, no rc captured -- the wrapping shell was torn down
 * before the trailing echo ran, same trap documented for func_8004BB3C in
 * round 17). Best candidate dropped the permuter score 75 -> 15 and never
 * improved further across the remaining ~36,900 iterations. The lead:
 * replace the second `u14 = e->unkC->unk14; u14->unk0 = 0;` reload with a
 * direct `e->unkC->unk14->unk0 = 0;` (no named-local reassignment) --
 * closed 12 of the 15 remaining words. Remaining 3-word residue (second
 * loop's row pointer, retail `$a2` vs built `$v0`) is the SAME pure
 * register-identity class this report already documented and is untouched
 * by this fix -- confirmed via asm-differ, no instruction-shape difference
 * anywhere in that loop, just the one register substitution.
 *
 * ROUND 27 (delta): re-verified per the head's callee-saved-registers
 * broadcast -- compiled prologue saves the IDENTICAL set to retail (s0-s7,
 * fp, ra, same stack slots), so the "extra callee-saved parameter" lever
 * does NOT apply here. Verdict (pure register identity) CONFIRMED, not
 * just plausible.
 *
 * ROUND 32 (bravo2): re-verified, 125/140, no drift, identical residue
 * (tbl/u14/second-loop-row-pointer register swaps, no instruction shape
 * differences). Not re-attempted -- three prior rounds' worth of
 * confirmation (register-identity verdict, callee-saved order match) left
 * nothing untried within this round's budget worth spending on. */
#if 0
void func_8004B700(Obj866E8 *self, s32 val, Unk54Struct *arg2, TargetSpec866E8 *arg3) {
    s32 divisor;
    s32 flag;
    s32 savedResult;
    s32 count;
    s32 i;
    Elem *e;
    Elem *e2;
    Unk14Obj *u14;
    Unk54Struct *tbl;
    SetupEntry866E8 stackBuf[7];

    if (arg3 != 0) {
        divisor = self->unk68->divisor;
        flag = (val / divisor) & 1;
        savedResult = func_8004B930(self, val, flag);

        count = 0;
        for (i = 0; i < 7; i++) {
            e = self->methods->slot118(self, i);
            e->unk2 = arg3[i].key;
            if (arg3[i].flag != 0) {
                tbl = &D_80086838[arg3[i].key];
                __asm__("");
                u14 = e->unkC->unk14;
                if (self->unk68->unk4 == 0) {
                    u14->unk18.w = arg2->unk0 + tbl->unk0;
                    u14->unk1C = arg2->unk4;
                    u14->unk20.w = arg2->unk8 + tbl->unk8;
                } else {
                    u14->unk18.w = arg2->unk0 - 0x5000;
                    u14->unk1C = arg2->unk4 + tbl->unk4;
                    u14->unk20.w = arg2->unk8 - 0x5000;
                }
                e->unkC->unk14->unk0 = 0;
                func_8004BA40(self, &stackBuf[count], divisor, flag, val, savedResult, arg3[i].key);
                count++;
            }
        }

        for (i = 0; i < 7; i++) {
            e2 = &self->arr[i];
            e2->unk4->unk32 = e2->unk2;
        }

        self->methods->slotFC(self, stackBuf, count);
    }
}
#endif

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
/* STALL, round 32 (bravo2): re-verified 58/63, no drift, identical
 * residue. See docs/match-reports/func_8004BA40.md for the full history.
 * This round: separating `fieldVal`/`sum` temps from `value` (to mirror
 * retail's v1-running-sum/v0-final-result split) -- IDENTICAL 58/63, no
 * change. A fresh 144,370-iteration permuter run (independent RNG, own
 * scaffold `--debug`-verified to score the same residue as the real
 * build) also never beat the base score of 25. Genuine negative, not
 * inconclusive; not attempted further this round. */
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
            s32 fieldVal = entry->unk4;
            value = val + fieldVal;
        } else {
            s32 lo = divisor * entry->unk0;
            s32 sum;

            if (flag != 0) {
                sum = lo + entry->unk4;
            } else {
                sum = lo + entry->unk8;
            }
            value = val + sum;
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
 *
 * ROUND 27 (delta): re-verified per the head's callee-saved-registers
 * broadcast -- compiled prologue saves the IDENTICAL set to retail (s0-s6,
 * ra, no fp, same stack slots for every register), only the ORDER of the
 * `sw` instructions and which C variable maps to which physical register
 * differ. The "extra callee-saved parameter" lever does NOT apply.
 * Verdict (pure register identity) CONFIRMED, not just plausible.
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

/* STALL snapshot -- see docs/match-reports/func_8004BE54.md. 142/150 words
 * at CORRECT total length (no address drift), up from 132/150 this round.
 * Preserved here per convention -- not live C.
 *
 * ROUND 40 (bravo): first-ever permuter search on this function (34293
 * iterations, no rc captured -- same "wrapping shell torn down before the
 * trailing echo" trap as func_8004B700 this same round). Best candidate
 * dropped the permuter score 135 -> 65 across four improving steps
 * (135->85->75->70->65) and never reached 0. Two changes, both applied:
 * (1) the `gpu = (*slot)->unk14; gpu->unk0 = 0;` reload replaced with a
 * direct `(*slot)->unk14->unk0 = 0;` (the exact same lever that closed
 * func_8004B700's own residue this round); (2) the `found`-path tail's
 * pointer cast hoisted into a named local (`EntryChildObj **next = ...;
 * (*slot)->unk38 = *next;`) instead of one combined expression. Together
 * these closed 10 of the 18 remaining words. Everything from
 * `0x8004BEE0` through the epilogue now matches retail byte-for-byte
 * (confirmed via asm-differ) -- the entire remaining 8-word residue is
 * the ALREADY-DOCUMENTED `info`/`hdr` register-identity chain at the very
 * top of the function (`0x8004BE84`-`0x8004BEDC`), untouched by this
 * round's fix and unchanged from prior rounds' description.
 *
 * ROUND 32 (bravo2): re-verified, 130/150, no drift, identical residue.
 * ROUND 39 (charlie): 130/150 -> 132/150. Both `(*slot)->unk10 |= flagBit;`
 * sites closed by hoisting the reloaded field into its own named local
 * BEFORE the `|=` (`s32 t = (*slot)->unk10; (*slot)->unk10 = t | flagBit;`)
 * -- operand-order-alone was already confirmed inert (round 32); the hoist
 * is what moves it, same combinatorial shape as func_8004C470's fix. */
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
            s32 flags10a;
            slot = (EntryChildObj **)((u8 *)entry->unk10 + off1);
            flags10a = (*slot)->unk10;
            (*slot)->unk10 = flags10a | flagBit;
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
            (*slot)->unk14->unk0 = 0;
            {
                s32 flags10 = (*slot)->unk10;
                (*slot)->unk10 = flags10 | flagBit;
            }
        }
        if (outBuf.found) {
            EntryChildObj **next = (EntryChildObj **)((u8 *)entry->unk10 + off2);
            (*slot)->unk38 = *next;
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

/* STALLED at 72/106 words -- see docs/match-reports/func_8004C1C0.md for
 * the full analysis (two independent residue classes: an $a1-vs-$a3
 * register-identity choice for u14b, and a store-then-reread narrow-field
 * codegen sensitivity confirmed with an isolated toolchain reproducer).
 * ROUND 27 (delta): re-verified 72/106, no drift; two more attempts on
 * class 2 (dropping the b2/b3 locals and re-reading `out->base.b2/b3`
 * directly at the h4/h8 use sites) -- both REGRESSED (drift, extra saved
 * register in the prologue), reverted. Not re-attempted further this
 * round. Preserved here per convention.
 *
 * ROUND 32 (bravo2): re-verified, 72/106, no drift, identical residue.
 * Set up an isolated permuter scaffold to probe the store-then-reread
 * class-2 residue -- its own `--debug` base score showed 9 INSERTIONS and
 * 9 DELETIONS versus the real in-context build's zero, i.e. the isolated
 * scaffold compiles to a structurally DIFFERENT function than the real
 * build (the same scaffold-context-mismatch trap documented for
 * func_8004BB3C in round 17). Not run further -- a search against a
 * scaffold provably scoring a different residue would not transfer.
 * Scaffold deleted; not attempted further this round. */
#if 0
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

#if 0
/* STALL, round 32 (bravo2): 68/70, CORRECT length (0x118). Up from 63/70 --
 * see docs/match-reports/func_8004C470.md for the full history this builds
 * on. Fix this round: moved `threshold -= 0x800` into the for-loop's own
 * increment-expression (alongside `i++`) instead of a trailing body
 * statement, so that `continue` on the deep-threshold path applies it too.
 * This closed "Residue 2" (the increment/decrement scheduling residue)
 * completely. Remaining residue is ONLY "Residue 1": a commutative `addu`
 * whose register-operand order the report already confirmed (twice, both
 * operand-textual-orders tried) is immune to source reordering -- a
 * project-wide confirmed class, not re-attempted this round.
 * ROUND 38 (alpha + head): 68/70 -> 69/70. The "immune to source reordering"
 * claim above is WRONG as stated -- it was tested by flipping operand order
 * alone, which is indeed inert (measured again: 68/70). What moves it is
 * HOISTING the field into a local AND writing `w + tol`: both together, and
 * only on the FIRST comparison. Hoisting the second as well REGRESSES to
 * 68/70 (67/70 if its operand order is left as `tol + h`). One word remains,
 * at vram 0x8004C500 -- the mirror `addu` in the second comparison.
 * See docs/match-reports/func_8004C470.md. */
Elem *func_8004C470(Obj866E8 *self, Unk54Struct *arg1) {
    s32 i;
    s32 tol;
    s32 threshold;
    Elem *candidate;
    Unk14Obj *r;
    s32 w;

    i = 0;
    tol = 0xA000;
    threshold = 0;
    for (; i < 7; i++, threshold -= 0x800) {
        candidate = self->methods->slot118(self, i);
        r = candidate->unkC->unk14;
        w = r->unk18.w;
        if (arg1->unk0 >= w && arg1->unk0 < w + tol) {
            if (arg1->unk8 >= r->unk20.w && arg1->unk8 < tol + r->unk20.w) {
                if (self->unk68->unk4 == 0) {
                    return candidate;
                }
                if (threshold >= arg1->unk4) {
                    if (threshold - 0x800 >= arg1->unk4) {
                        continue;
                    }
                    return candidate;
                }
            }
        }
    }
    return 0;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c", func_8004C470);
