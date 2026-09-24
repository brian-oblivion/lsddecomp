/* Second slice of the 365-function class_3bb8c block, 0x3CD88..0x3DA54 --
 * same class (Obj866E8, D_800866E8) as class_3bb8c.c's first slice, split
 * only for parallel runners, so this unit reuses that unit's header
 * (same convention as Entity.c/Entity_b.c). */
#include "common.h"
#include "class_3bb8c.h"

s32 Class866E8__FindElemIndexByUnk32(Obj866E8 *self, s32 key) {
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

s32 Class866E8__FindElemIndexByUnk30(Obj866E8 *self, s32 key) {
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
 * order, after Class866E8__RefreshFootprint), but Class866E8__RefreshFootprint calls them before their
 * own definitions appear. Signatures are typed from the registers loaded
 * at each call site, per this unit's established convention for calling a
 * same-unit function whose body is still INCLUDE_ASM. */
extern void Class866E8__SetFootprintCellFlag(Obj866E8 *self, s32 arg1);
extern void Class866E8__SetFootprintFromQuery(Obj866E8 *self);
extern void Class866E8__ComputeFootprintFromRotation(Obj866E8 *self, s32 arg1, s32 arg2);

void Class866E8__RefreshFootprint(Obj866E8 *self) {
    s32 idx;

    if (self->unk1B8 == 0) {
        return;
    }
    idx = self->gridHalfCells * 2;
    Class866E8__SetFootprintCellFlag(self, 0);
    if (self->unk68->unk4 == 0) {
        Class866E8__ComputeFootprintFromRotation(self, idx, self->gridCells);
    } else {
        Class866E8__SetFootprintFromQuery(self);
    }
    Class866E8__SetFootprintCellFlag(self, 1);
}

/* Forward declaration: defined later in this file (in ROM order, after
 * Class866E8__SplitFootprintSlot), but tail-called here before its own definition appears. */
extern void Class866E8__BuildFootprintSlots(Obj866E8 *self);

void Class866E8__ComputeFootprintFromRotation(Obj866E8 *self, s32 arg1, s32 arg2) {
    Unk6C14SubObj *sub;
    CC74QueryBuf buf;
    s32 point0;
    s32 point1;
    u16 angle;      /* u16, not s32: the s32 form is byte-identical except
                     * for an 8-byte-smaller frame (round 71) */
    QueryTemplate866E8 mat;
    s32 offset;
    s32 flag;
    s32 half;
    void *rot;

    sub = self->unk6C->unk14->unk44;
    rot = (u8 *)sub + 0x10;
    self->methods->slot10C(self, &buf, 0);
    point0 = buf.point[0];
    point1 = buf.point[1];
    angle = sub->unk12;
    if ((s16)sub->unk12 < 0) {
        angle += 0x1000;
    }

    mat = D_8008E98C;
    mat.unk14 = 0;
    mat.unk18 = 0;
    mat.unk1C = self->gridSpan;
    RotMatrix(rot, &mat);
    ApplyMatrixLV(&mat, &mat.unk14, &mat.unk14);

    if ((u16)(angle - 0x200) < 0x400 || (u16)(angle - 0xA00) < 0x400) {
        offset = mat.unk1C;
        self->unk80 = arg2;
        self->unk84 = arg1;
        self->unk7C = (mat.unk14 > 0) ? point0 : point0 - arg1 + 1;
        self->unk7E = (mat.unk1C > 0) ? point1 - (u16)self->gridHalfCells - 1
                                      : point1 - (u16)self->gridHalfCells + 1;
        flag = 0;
    } else if ((u16)(angle - 0x600) < 0x400 || (u16)(angle - 0x200) >= 0xC00) {
        offset = mat.unk14;
        self->unk80 = arg1;
        self->unk84 = arg2;
        self->unk7C = (mat.unk14 > 0) ? point0 - (u16)self->gridHalfCells - 1
                                      : point0 - (u16)self->gridHalfCells + 1;
        self->unk7E = (mat.unk1C > 0) ? point1 : point1 - arg2 + 1;
        flag = 1;
    }

    half = self->gridHalfCells;
    offset >>= 11;
    if (offset >= half) {
        offset = half - 1;
    }
    half = -half;
    if (half >= offset) {
        offset = half + 1;
    }
    if (flag) {
        self->unk7C = (u16)self->unk7C + offset;
    } else {
        self->unk7E = (u16)self->unk7E + offset;
    }
    Class866E8__BuildFootprintSlots(self);
}

/* Forward declaration: defined next in this file (ROM order), called here
 * before its own definition appears. */
extern s32 Class866E8__SplitFootprintSlot(Obj866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 col, s32 row, s32 width, s32 height);

/* Matched round 75. Three source-shape levers closed what was filed since
 * round 19 as a whole-function register rotation
 * (docs/match-reports/Class866E8__BuildFootprintSlots.md): ONE slot pointer reused for the
 * second slot (no separate slot1), assigned once at the join after the
 * h6<0 test (reorg fills the bgez delay slot from it and deletes the
 * redundant copy on the other path -- no barrier, no duplicate); the
 * clipped remainder in its own local `over` rather than `span -= 0x14`;
 * and `count += 1` as a statement in each arm plus at the join. */
void Class866E8__BuildFootprintSlots(Obj866E8 *self) {
    s32 flag;
    s32 h4;
    s32 width;
    s32 height;
    s32 quadrant;
    s32 h6;
    GridSlot866E8 *slot;
    s32 span;
    s32 count;
    s32 over;

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
        h6 += 0x14;
        if (flag != 0) {
            h4 -= 0xA;
            quadrant = 0;
        } else if (h4 < 0xA) {
            h4 += 0xA;
            quadrant = 0;
        } else {
            h4 -= 0xA;
            quadrant = 1;
        }
    }
    slot = &self->slots8C[0];
    slot->elemIdx = self->methods->slot120(self, quadrant);
    slot->h4 = (h4 >= 0) ? h4 : 0;
    slot->h6 = h6;
    span = h4 + width;
    if (span >= 0x15) {
        over = span - 0x14;
        slot->h8 = width - over;
        count = Class866E8__SplitFootprintSlot(self, slot, 0, quadrant, h4, h6, width, height);
        count += 1;
        slot = &self->slots8C[count];
        slot->elemIdx = self->methods->slot120(self, quadrant + 1);
        slot->h4 = 0;
        slot->h6 = self->slots8C[0].h6;
        slot->h8 = over;
        slot->hA = self->slots8C[0].hA;
    } else {
        slot->h8 = width;
        count = Class866E8__SplitFootprintSlot(self, slot, 0, quadrant, h4, h6, width, height);
    }
    count += 1;
    self->unk88 = count;
}

s32 Class866E8__SplitFootprintSlot(Obj866E8 *self, GridSlot866E8 *slot, s32 count, s32 baseIdx, s32 col, s32 row, s32 width, s32 height) {
    s32 overflow;
    s32 span;
    s32 elemArg;
    s32 widthLeft;

    if (row + height >= 21) {
        /* The rectangle runs past the bottom edge (row 20): clip this slot
         * and open a new one for the part below. */
        overflow = (row + height) - 20;
        span = overflow;
        slot->hA = height - overflow;
        count = count + 1;
        slot = &self->slots8C[count];

        if (col < 10) {
            elemArg = baseIdx + 2;
            slot->elemIdx = self->methods->slot120(self, elemArg);
            slot->h4 = col + 10;
            /* Stored in BOTH arms: cross-jumping merges the copies, and
             * the join label keeps the h4 reload below after it. */
            slot->hA = span;
        } else {
            elemArg = baseIdx + 3;
            slot->elemIdx = self->methods->slot120(self, elemArg);
            slot->h4 = col - 10;
            slot->hA = span;
        }

        span = slot->h4 + width;
        slot->h6 = 0;
        if (span >= 21) {
            /* ...and past the right edge (column 20) too. */
            count = count + 1;
            /* Two statements, not `(width + 20) - span`: fold rewrites
             * that tree as `width - (span - 20)` and CSE then shares
             * `span - 20` with the store below (round 71). */
            widthLeft = width + 20;
            widthLeft = widthLeft - span;
            slot->h8 = widthLeft;
            slot = &self->slots8C[count];
            slot->elemIdx = self->methods->slot120(self, elemArg + 1);
            slot->h4 = 0;
            slot->h6 = 0;
            slot->h8 = span - 20;
            slot->hA = overflow;
        } else {
            slot->h8 = width;
        }
    } else {
        slot->hA = height;
    }
    return count;
}

/* Forward declaration: defined later in this file (in ROM order, after
 * Class866E8__SetFootprintFromQuery), and EXCLUDED from this round's targets (documented
 * STALL, see docs/match-reports/IsPointOutOfBounds.md) -- calling into it
 * while it is still INCLUDE_ASM is fine, per this unit's established
 * convention. Signature per that report. */
extern s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point);

/* Forward declaration: defined later in this file (in ROM order, after
 * IsPointOutOfBounds), but Class866E8__SetFootprintFromQuery calls it before its own definition
 * appears. */
extern s32 Class866E8__InitFootprintSlot(Obj866E8 *self, s32 unused, s32 key, s32 arg3);

void Class866E8__SetFootprintFromQuery(Obj866E8 *self) {
    s32 junk;
    CC74QueryBuf buf;

    self->methods->slot10C(self, &buf, 0);
    self->unk88 = 0;
    self->unk88 = Class866E8__InitFootprintSlot(self, junk, 0, buf.count);
    if (IsPointOutOfBounds(self->unk1DC, buf.point) != 0) {
        if (buf.count + 1 < self->unk68->count) {
            self->unk88 = Class866E8__InitFootprintSlot(self, junk, self->unk88, buf.count + 1);
        }
    }
    if (buf.count - 1 >= 0) {
        self->unk88 = Class866E8__InitFootprintSlot(self, junk, self->unk88, buf.count - 1);
    }
}

#ifdef NON_MATCHING
/* NON_MATCHING: 2/27 words, length exact, zero drift (re-measured round 71;
 * round 71 traced every diff to one root: retail's block-local temps avoid
 * $v0; 376k more permuter iterations, no zero). Residue: register
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
 * (round 58) (docs/match-reports/IsPointOutOfBounds.md). Hand-derived. */
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point) {
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
INCLUDE_ASM("asm/nonmatchings/class_3bb8c_b", IsPointOutOfBounds);
#endif

s32 Class866E8__InitFootprintSlot(Obj866E8 *self, s32 unused, s32 key, s32 arg3) {
    Unk54Struct *slot;

    slot = (Unk54Struct *) ((u8 *) self + 0x8C + key * sizeof(Unk54Struct));
    *slot = D_80086990;
    slot->unk0 = self->methods->slot124(self, arg3);
    return key + 1;
}

void Class866E8__SetFootprintCellFlag(Obj866E8 *self, s32 setBit) {
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

void *Class866E8__GetUnk1CC(Obj866E8 *self) {
    return &self->unk1CC;
}

void Class866E8__SetBounds(Obj866E8 *self, Bounds866E8_3bb8c_b *arg1) {
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
 *    docs/match-reports/Class866E8__ConfigureRateEntry.md.
 *
 * `~rate + 1` is retail's own negation (`nor`/`addiu`), not `-rate`. */
void Class866E8__ConfigureRateEntry(Obj866E8 *self, s32 rate, s32 flag) {
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

/* Forward declaration: defined later in this file (after Class866E8__AdvanceRateCountdown in
 * ROM-address order), but passed to Class866E8__ForEachElem as a function-pointer
 * argument before its own definition appears. */
void Class866E8__ApplyRateToChild(Obj866E8 *self, EntryChildObj *item);

void Class866E8__AdvanceRateCountdown(Obj866E8 *self) {
    if (self->unk1E0 > 0) {
        Class866E8__ForEachElem(self, Class866E8__ApplyRateToChild, 0);
        self->unk1E0 -= 1;
        if (self->unk1E0 == 0) {
            self->unk1E0 = -1;
        }
    }
}

/* Forward declaration: defined later in this file (after Class866E8__FlushRateLatch in
 * ROM-address order), but passed to Class866E8__ForEachElem as a function-pointer
 * argument before its own definition appears. */
void Class866E8__ResetChildRate(Obj866E8 *self, EntryChildObj *item);

void Class866E8__FlushRateLatch(Obj866E8 *self) {
    if (self->unk1E0 != 0) {
        Class866E8__ForEachElem(self, Class866E8__ResetChildRate, 0);
        self->unk1E0 = 0;
    }
}

void Class866E8__ApplyRateToChild(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 0, self->unk1E4);
}

void Class866E8__ResetChildRate(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 1, D_800869CC);
}

void Class866E8__ForEachElem(Obj866E8 *self, void (*arg1)(Obj866E8 *self, EntryChildObj *item), void (*arg2)(Obj866E8 *self, Elem *item)) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (arg2 != 0) {
            arg2(self, e);
        }
        Class866E8__ForEachEntryChild(self, arg1, e);
    }
}

void Class866E8__ForEachEntryChild(Obj866E8 *self, void (*callback)(Obj866E8 *self, EntryChildObj *item), Elem *item) {
    EntryChildObj **p;
    EntryChildObj **end;

    end = item->unk10 + (0x668 / 4);
    p = item->unk10;
    for (; p < end; p++) {
        callback(self, *p);
    }
}

Obj866E8Methods *GetClass866E8Methods(void) {
    return &D_800866E8;
}
