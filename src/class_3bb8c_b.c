/* Second slice of the 365-function class_3bb8c block, 0x3CD88..0x3DA54 --
 * same class (Obj866E8, D_800866E8) as class_3bb8c.c's first slice, split
 * only for parallel runners, so this unit reuses that unit's header
 * (same convention as Entity.c/Entity_b.c). */
#include "common.h"
#include "class_3bb8c.h"

s32 func_8004C588(Obj866E8 *self, s32 key) {
    s32 result;
    s32 i;
    Elem *e;

    result = 0;
    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk32 == key) {
            result = i;
            break;
        }
    }
    return result;
}

s32 func_8004C5D0(Obj866E8 *self, s32 key) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (e->unk4->unk30 == key && e->unk4->unk2C != 0) {
            return i;
        }
    }
    return -1;
}

/* Forward declarations: all three are defined later in this file (in ROM
 * order, after func_8004C620), but func_8004C620 calls them before their
 * own definitions appear. Signatures are typed from the registers loaded
 * at each call site, per this unit's established convention for calling a
 * same-unit function whose body is still INCLUDE_ASM. */
extern void func_8004CE24(Obj866E8 *self, s32 arg1);
extern void func_8004CC74(Obj866E8 *self);
extern void func_8004C6A8(Obj866E8 *self, s32 arg1, s32 arg2);

void func_8004C620(Obj866E8 *self) {
    s32 idx;

    if (self->unk1B8 == 0) {
        return;
    }
    idx = self->unk78 * 2;
    func_8004CE24(self, 0);
    if (self->unk68->unk4 == 0) {
        func_8004C6A8(self, idx, self->unk7A);
    } else {
        func_8004CC74(self);
    }
    func_8004CE24(self, 1);
}

/* Forward declaration: defined later in this file (in ROM order, after
 * func_8004CAF0), but tail-called here before its own definition appears. */
extern void func_8004C93C(Obj866E8 *self);

/* STALLED at 85/165 words (round 34) -- see docs/match-reports/func_8004C6A8.md.
 * Every branch TARGET in the raw-range dispatch now agrees with retail
 * (previously 60/165 with a different, wrong-polarity CFG shape); compiled
 * length is still 3 words (12 bytes) short of retail's 165. The residue is a
 * register-class choice: retail promotes a value that is never live across a
 * CALL (the RotMatrix pointer argument, `(u8 *)sub + 0x10`) into its own
 * callee-saved register anyway, which saturates all nine $s/$fp slots and
 * forces `flag` to spill to the stack (an extra sw/lw pair retail has and
 * this body does not); nothing tried reproduces that promotion. Restored
 * here per project convention. */
#if 0
void func_8004C6A8(Obj866E8 *self, s32 arg1, s32 arg2) {
    Unk6C14SubObj *sub;
    CC74QueryBuf buf;
    s32 point0;
    s32 point1;
    s32 raw;
    QueryTemplate866E8 desc;
    s32 v1;
    s32 v2;
    s32 v3;
    s32 s3;
    s32 v;
    s32 flag;

    sub = self->unk6C->unk14->unk44;
    self->methods->slot10C(self, &buf, 0);
    point0 = buf.point[0];
    point1 = buf.point[1];
    raw = sub->unk12;
    if ((s16)sub->unk12 < 0) {
        raw += 0x1000;
    }

    desc = D_8008E98C;
    desc.unk14 = 0;
    desc.unk18 = 0;
    desc.unk1C = self->unk74;
    RotMatrix((u8 *)sub + 0x10, &desc);
    ApplyMatrixLV(&desc, &desc.unk14, &desc.unk14);

    v1 = raw - 0x200;
    if ((u16)v1 < 0x400) {
        goto block1;
    }
    v2 = raw - 0xA00;
    if ((u16)v2 >= 0x400) {
        goto continue_dispatch;
    }

block1:
    s3 = desc.unk1C;
    self->unk80 = arg2;
    self->unk84 = arg1;
    if (desc.unk14 > 0) {
        v = point0;
    } else {
        v = point0 - arg1 + 1;
    }
    self->unk7C = (s16)v;
    if (desc.unk1C > 0) {
        v = point1 - (u16)self->unk78 - 1;
    } else {
        v = point1 - (u16)self->unk78 + 1;
    }
    self->unk7E = (s16)v;
    flag = 0;
    goto shared;

continue_dispatch:
    v3 = raw - 0x600;
    if ((u16)v3 < 0x400) {
        goto block2;
    }
    if ((u16)v1 < 0xC00) {
        goto shared;
    }

block2:
    s3 = desc.unk14;
    self->unk80 = arg1;
    self->unk84 = arg2;
    if (desc.unk14 > 0) {
        v = point0 - (u16)self->unk78 - 1;
    } else {
        v = point0 - (u16)self->unk78 + 1;
    }
    self->unk7C = (s16)v;
    if (desc.unk1C > 0) {
        v = point1;
    } else {
        v = point1 - arg2 + 1;
    }
    self->unk7E = (s16)v;
    flag = 1;

shared:
    v = self->unk78;
    s3 >>= 11;
    if (s3 >= v) {
        s3 = v - 1;
    }
    v = -v;
    if (v >= s3) {
        s3 = v + 1;
    }
    if (flag) {
        self->unk7C = (u16)self->unk7C + s3;
    } else {
        self->unk7E = (u16)self->unk7E + s3;
    }
    func_8004C93C(self);
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C6A8);

#ifdef NON_MATCHING
/* NON_MATCHING: 45/109 words, length exact, zero drift. Residue: register
 * identity, a clean rotation across the whole function
 * (self/h4/h6/slot0/span all permuted, matching set of registers), which
 * CLAUDE.md HARD RULE 6 marks a STALL by definition, not a judgement call
 * (docs/match-reports/func_8004C93C.md). Two real CFG/scheduling fixes
 * closed most of the gap (a hoisted "flag=0" default matching the
 * project's established "default value in the guarding branch's delay
 * slot" idiom; a deliberately-duplicated `slot0 = &self->slots8C[0];` on
 * both arms of the h6<0 test, held apart with a bare `__asm__("")`
 * scheduling barrier to stop the compiler tail-merging the two identical
 * stores back into one). Hand-derived. */
void func_8004C93C(Obj866E8 *self) {
    s32 flag;
    s32 h4;
    s32 width;
    s32 height;
    s32 quadrant;
    s32 h6;
    GridSlot866E8 *slot0;
    s32 span;
    s32 count;
    GridSlot866E8 *slot1;

    flag = 0;
    h4 = self->unk7C;
    width = self->unk80;
    height = self->unk84;
    quadrant = 3;
    if (h4 < 0) {
        h4 += 0x14;
        flag = 1;
        quadrant = 2;
    }
    h6 = self->unk7E;
    if (h6 < 0) {
        if (flag == 0) {
            h6 += 0x14;
            if (h4 < 0xA) {
                h4 += 0xA;
                quadrant = 0;
            } else {
                h4 -= 0xA;
                quadrant = 1;
            }
        } else {
            h4 -= 0xA;
            quadrant = 0;
        }
        slot0 = &self->slots8C[0];
    } else {
        __asm__("");
        slot0 = &self->slots8C[0];
    }
    slot0->elemIdx = self->methods->slot120(self, quadrant);
    slot0->h4 = (h4 >= 0) ? h4 : 0;
    slot0->h6 = h6;
    span = h4 + width;
    if (span >= 0x15) {
        span -= 0x14;
        slot0->h8 = width - span;
        count = func_8004CAF0(self, slot0, 0, quadrant, h4, h6, width, height);
        count += 1;
        slot1 = &self->slots8C[count];
        slot1->elemIdx = self->methods->slot120(self, quadrant + 1);
        slot1->h4 = 0;
        slot1->h6 = self->slots8C[0].h6;
        slot1->h8 = span;
        slot1->hA = self->slots8C[0].hA;
    } else {
        slot0->h8 = width;
        count = func_8004CAF0(self, slot0, 0, quadrant, h4, h6, width, height);
    }
    count += 1;
    self->unk88 = count;
}
#else
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004C93C);
#endif

/* STALLED at 62/97 words (round 58, up from 55/97) -- see
 * docs/match-reports/func_8004CAF0.md. Frame size, callee-saved register SET
 * and CFG shape match retail exactly. Round 58 retired the "permuter scaffold
 * is untrustworthy" blocker that rounds 19 and 33 had recorded (it was a unit
 * error: funcdiff reports no insertion/deletion counts at all, so the "0 ins /
 * 0 del" those rounds weighed against the permuter was never measured), ran
 * this function's first search, and closed two of its three structural diffs
 * with the pair of levers in the body below -- the do-while(0) around the
 * first half AND the named h8Val temp, which only work JOINTLY (either alone
 * changes the function's length). What is left is one arithmetic
 * reassociation plus the 3-register rotation (self/slot/temp among
 * $s0/$s1/$s3) that is very likely DOWNSTREAM of it. Preserved here per
 * project convention rather than only in the report. */
#if 0
s32 func_8004CAF0(Obj866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8) {
    s32 hSpan;
    s32 hSpan2;
    s32 h4;
    s32 nextArg;
    s32 h8Val;

    if (p6 + p8 >= 21) {
        do {
            hSpan = (p6 + p8) - 20;
            hSpan2 = hSpan;
            slot->hA = p8 - hSpan;
            count = count + 1;
            slot = &self->slots8C[count];

            if (p5 < 10) {
                nextArg = baseIdx + 2;
                slot->elemIdx = self->methods->slot120(self, nextArg);
                __asm__("");
                h4 = p5 + 10;
            } else {
                nextArg = baseIdx + 3;
                slot->elemIdx = self->methods->slot120(self, nextArg);
                __asm__("");
                h4 = p5 - 10;
            }
            slot->h4 = h4;
            slot->hA = hSpan2;
        } while (0);

        hSpan2 = slot->h4 + p7;
        slot->h6 = 0;
        if (hSpan2 >= 21) {
            count = count + 1;
            h8Val = (p7 + 20) - hSpan2;
            slot->h8 = h8Val;
            slot = &self->slots8C[count];
            slot->elemIdx = self->methods->slot120(self, nextArg + 1);
            slot->h4 = 0;
            slot->h6 = 0;
            slot->h8 = hSpan2 - 20;
            slot->hA = hSpan;
            return count;
        }
        slot->h8 = p7;
        return count;
    }
    slot->hA = p8;
    return count;
}
#endif

INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CAF0);

/* Forward declaration: defined later in this file (in ROM order, after
 * func_8004CC74), and EXCLUDED from this round's targets (documented
 * STALL, see docs/match-reports/func_8004CD38.md) -- calling into it
 * while it is still INCLUDE_ASM is fine, per this unit's established
 * convention. Signature per that report. */
extern s32 func_8004CD38(Bounds866E8_3bb8c_b *bounds, s8 *point);

/* Forward declaration: defined later in this file (in ROM order, after
 * func_8004CD38), but func_8004CC74 calls it before its own definition
 * appears. */
extern s32 func_8004CDA4(Obj866E8 *self, s32 unused, s32 key, s32 arg3);

void func_8004CC74(Obj866E8 *self) {
    s32 junk;
    CC74QueryBuf buf;

    self->methods->slot10C(self, &buf, 0);
    self->unk88 = 0;
    self->unk88 = func_8004CDA4(self, junk, 0, buf.count);
    if (func_8004CD38(self->unk1DC, buf.point) != 0) {
        if (buf.count + 1 < self->unk68->count) {
            self->unk88 = func_8004CDA4(self, junk, self->unk88, buf.count + 1);
        }
    }
    if (buf.count - 1 >= 0) {
        self->unk88 = func_8004CDA4(self, junk, self->unk88, buf.count - 1);
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: 2/27 words, length exact, zero drift. Residue: register
 * identity (retail copies `bounds` into $a2 with an unconditional `move
 * a2,a0` before the null check, dedicates $v0 to the constant 1 for the
 * whole body, and re-extracts point[0] from the saved-but-unshifted $a3 in
 * a delay slot; every hand-derived C shape tried instead keeps everything
 * in $a0/$v0 and re-materializes `li v0,1` at three of the four exits).
 * Eleven independent source restatements (if-chains, `||`, `goto`, a
 * consistent-alias local, narrow `s8` locals, a nested ternary, and a
 * `result`/`do..while(0)` variable) all converge on this same shape or
 * worse; permuter search (round 18, 71363 iterations) also failed to
 * reach zero against the identical, since-confirmed-faithful scaffold
 * (round 58) (docs/match-reports/func_8004CD38.md). Hand-derived. */
s32 func_8004CD38(Bounds866E8_3bb8c_b *bounds, s8 *point) {
    if (bounds == NULL) {
        return 1;
    }
    if (point[0] < bounds->unk0) {
        return 1;
    }
    if (bounds->unk4 < point[0]) {
        return 1;
    }
    if (point[1] < bounds->unk2) {
        return 1;
    }
    return bounds->unk8 < point[1];
}
#else
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", func_8004CD38);
#endif

s32 func_8004CDA4(Obj866E8 *self, s32 unused, s32 key, s32 arg3) {
    Unk54Struct *slot;

    slot = (Unk54Struct *) ((u8 *) self + 0x8C + key * sizeof(Unk54Struct));
    *slot = D_80086990;
    slot->unk0 = self->methods->slot124(self, arg3);
    return key + 1;
}

void func_8004CE24(Obj866E8 *self, s32 setBit) {
    s32 i;
    s32 j;
    s32 k;
    GridSlot866E8 *slot;
    Elem *e;
    EntryChildObj **cell;
    EntryChildObj *next;

    slot = self->slots8C;
    for (i = 0; i < self->unk88; slot++, i++) {
        e = &self->arr[slot->elemIdx];
        if (e->unk4->unk2C == 0) {
            continue;
        }
        cell = e->unk10 + slot->h4 + slot->h6 * 20;
        for (j = 0; j < slot->hA; j++) {
            for (k = 0; k < slot->h8; k++) {
                if (setBit != 0) {
                    (*cell)->unk10 &= 0x7FFFFFFF;
                } else {
                    (*cell)->unk10 |= 0x80000000;
                }
                next = (*cell)->unk38;
                while (next != 0) {
                    if (setBit != 0) {
                        next->unk10 &= 0x7FFFFFFF;
                    } else {
                        next->unk10 |= 0x80000000;
                    }
                    next = next->unk38;
                }
                cell++;
            }
            cell += 20 - slot->h8;
        }
    }
}

void *func_8004CFA8(Obj866E8 *self) {
    return &self->unk1CC;
}

void func_8004CFB0(Obj866E8 *self, Bounds866E8_3bb8c_b *arg1) {
    self->unk1DC = arg1;
}

/* Picks one of the four static 0xC-byte EntryDesc866E8 entries by the sign of
 * `rate` and by `flag`, then sets unk1E0 to |rate| scaled by the chosen
 * entry's unk6.
 *
 * Two source shapes here are load-bearing and neither is cosmetic:
 *
 *  - The `goto` ladder, and its asymmetry. Retail emits TWO stores to
 *    unk1E4: the rate>0/flag!=0 path has its own (in a `j`'s delay slot at
 *    0x8004CFDC) and the other three SHARE one (0x8004CFF8). Writing the
 *    field directly on that one path and going through `table` on the other
 *    three is what reproduces that split. Byte-exact since round 9.
 *  - `scale` and `val`. Retail loads the entry's unk6 ONCE (`lh $v1,6($v0)`)
 *    before the sign branch and keeps a single `mflo` after the join, with a
 *    `mult` in each arm. Caching the load in `scale` and letting an explicit
 *    if/else assign a local `val` is what defers that `mflo`; the
 *    default-then-overwrite spelling makes cc1 extract it eagerly, and
 *    storing to self->unk1E0 directly instead of through `val` perturbs the
 *    table-selection half as well. Both were measured -- round 58 and
 *    docs/match-reports/func_8004CFB8.md.
 *
 * `~rate + 1` is retail's own negation (`nor`/`addiu`), not `-rate`. */
void func_8004CFB8(Obj866E8 *self, s32 rate, s32 flag) {
    EntryDesc866E8 *table;
    s32 val;
    s32 scale;

    if (rate <= 0) {
        goto rate_le;
    }
    table = &D_8008699C;
    if (flag == 0) {
        goto store;
    }
    self->unk1E4 = &D_800869A8;
    goto merge;
rate_le:
    table = &D_800869B4;
    if (flag == 0) {
        goto store;
    }
    table = &D_800869C0;
store:
    self->unk1E4 = table;
merge:
    scale = self->unk1E4->unk6;
    if (rate >= 0) {
        val = scale * rate;
    } else {
        val = scale * (~rate + 1);
    }
    self->unk1E0 = val;
}

/* Forward declaration: defined later in this file (after func_8004D028 in
 * ROM-address order), but passed to func_8004D140 as a function-pointer
 * argument before its own definition appears. */
void func_8004D0D0(Obj866E8 *self, EntryChildObj *item);

void func_8004D028(Obj866E8 *self) {
    if (self->unk1E0 > 0) {
        func_8004D140(self, func_8004D0D0, 0);
        self->unk1E0 -= 1;
        if (self->unk1E0 == 0) {
            self->unk1E0 = -1;
        }
    }
}

/* Forward declaration: defined later in this file (after func_8004D088 in
 * ROM-address order), but passed to func_8004D140 as a function-pointer
 * argument before its own definition appears. */
void func_8004D108(Obj866E8 *self, EntryChildObj *item);

void func_8004D088(Obj866E8 *self) {
    if (self->unk1E0 != 0) {
        func_8004D140(self, func_8004D108, 0);
        self->unk1E0 = 0;
    }
}

void func_8004D0D0(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 0, self->unk1E4);
}

void func_8004D108(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 1, D_800869CC);
}

void func_8004D140(Obj866E8 *self, void (*arg1)(Obj866E8 *self, EntryChildObj *item), void (*arg2)(Obj866E8 *self, Elem *item)) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (arg2 != 0) {
            arg2(self, e);
        }
        func_8004D1D0(self, arg1, e);
    }
}

void func_8004D1D0(Obj866E8 *self, void (*callback)(Obj866E8 *self, EntryChildObj *item), Elem *item) {
    EntryChildObj **p;
    EntryChildObj **end;

    end = item->unk10 + (0x668 / 4);
    p = item->unk10;
    for (; p < end; p++) {
        callback(self, *p);
    }
}

Obj866E8Methods *func_8004D244(void) {
    return &D_800866E8;
}
