/* Second slice of the 365-function class_3bb8c block, 0x3CD88..0x3DA54 --
 * same class (Obj866E8, D_800866E8, "Class866E8" in class_3ac78's own
 * independent view of the same vtable) as class_3bb8c.c's first slice,
 * split only for parallel runners, so this unit reuses that unit's header
 * (same convention as Entity.c/Entity_b.c).
 *
 * Functionally this slice is the class's SPATIAL GRID / FOOTPRINT
 * subsystem: a 7-element array of Elem entries (self->arr), each mapped
 * onto up to four GridSlot866E8 rectangles (self->gridSlots), and a
 * per-cell "reserved" bit (bit 31 of EntryChildObj::unk10) that
 * RefreshFootprint clears, recomputes (via either
 * ComputeFootprintFromRotation or SetFootprintFromQuery, gated on
 * self->unk68->unk4) and sets again through SetFootprintCellFlag. A
 * second, unrelated mechanism lives at the tail of the unit: a rate/
 * countdown pair (self->rateCountdown/self->rateEntry) that
 * AdvanceRateCountdown/FlushRateLatch apply to every EntryChildObj under
 * self->arr via the generic ForEachElem/ForEachEntryChild iterators.
 *
 * "Footprint" is not this unit's own coinage: class_3ac78's independent
 * view of this same vtable already named the analogous mechanism there
 * (Class866E8__ApplyToSenderFootprint, SetFootprintRect,
 * SetFootprintFromCell) before this unit's naming pass, and this unit's
 * names were chosen to agree with that vocabulary. */
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
        self->footprintWidth = arg2;
        self->footprintHeight = arg1;
        self->footprintCol = (mat.unk14 > 0) ? point0 : point0 - arg1 + 1;
        self->footprintRow = (mat.unk1C > 0) ? point1 - (u16)self->gridHalfCells - 1
                                      : point1 - (u16)self->gridHalfCells + 1;
        flag = 0;
    } else if ((u16)(angle - 0x600) < 0x400 || (u16)(angle - 0x200) >= 0xC00) {
        offset = mat.unk14;
        self->footprintWidth = arg1;
        self->footprintHeight = arg2;
        self->footprintCol = (mat.unk14 > 0) ? point0 - (u16)self->gridHalfCells - 1
                                      : point0 - (u16)self->gridHalfCells + 1;
        self->footprintRow = (mat.unk1C > 0) ? point1 : point1 - arg2 + 1;
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
        self->footprintCol = (u16)self->footprintCol + offset;
    } else {
        self->footprintRow = (u16)self->footprintRow + offset;
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
 * row<0 test (reorg fills the bgez delay slot from it and deletes the
 * redundant copy on the other path -- no barrier, no duplicate); the
 * clipped remainder in its own local `over` rather than `span -= 0x14`;
 * and `count += 1` as a statement in each arm plus at the join. */
void Class866E8__BuildFootprintSlots(Obj866E8 *self) {
    s32 flag;
    s32 col;
    s32 width;
    s32 height;
    s32 quadrant;
    s32 row;
    GridSlot866E8 *slot;
    s32 span;
    s32 count;
    s32 over;

    flag = 0;
    col = self->footprintCol;
    width = self->footprintWidth;
    height = self->footprintHeight;
    quadrant = 3;
    if (col < 0) {
        col += 0x14;
        flag = 1;
        quadrant = 2;
    }
    row = self->footprintRow;
    if (row < 0) {
        row += 0x14;
        if (flag != 0) {
            col -= 0xA;
            quadrant = 0;
        } else if (col < 0xA) {
            col += 0xA;
            quadrant = 0;
        } else {
            col -= 0xA;
            quadrant = 1;
        }
    }
    slot = &self->gridSlots[0];
    slot->elemIdx = self->methods->slot120(self, quadrant);
    slot->col = (col >= 0) ? col : 0;
    slot->row = row;
    span = col + width;
    if (span >= 0x15) {
        over = span - 0x14;
        slot->width = width - over;
        count = Class866E8__SplitFootprintSlot(self, slot, 0, quadrant, col, row, width, height);
        count += 1;
        slot = &self->gridSlots[count];
        slot->elemIdx = self->methods->slot120(self, quadrant + 1);
        slot->col = 0;
        slot->row = self->gridSlots[0].row;
        slot->width = over;
        slot->height = self->gridSlots[0].height;
    } else {
        slot->width = width;
        count = Class866E8__SplitFootprintSlot(self, slot, 0, quadrant, col, row, width, height);
    }
    count += 1;
    self->gridSlotCount = count;
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
        slot->height = height - overflow;
        count = count + 1;
        slot = &self->gridSlots[count];

        if (col < 10) {
            elemArg = baseIdx + 2;
            slot->elemIdx = self->methods->slot120(self, elemArg);
            slot->col = col + 10;
            /* Stored in BOTH arms: cross-jumping merges the copies, and
             * the join label keeps the col reload below after it. */
            slot->height = span;
        } else {
            elemArg = baseIdx + 3;
            slot->elemIdx = self->methods->slot120(self, elemArg);
            slot->col = col - 10;
            slot->height = span;
        }

        span = slot->col + width;
        slot->row = 0;
        if (span >= 21) {
            /* ...and past the right edge (column 20) too. */
            count = count + 1;
            /* Two statements, not `(width + 20) - span`: fold rewrites
             * that tree as `width - (span - 20)` and CSE then shares
             * `span - 20` with the store below (round 71). */
            widthLeft = width + 20;
            widthLeft = widthLeft - span;
            slot->width = widthLeft;
            slot = &self->gridSlots[count];
            slot->elemIdx = self->methods->slot120(self, elemArg + 1);
            slot->col = 0;
            slot->row = 0;
            slot->width = span - 20;
            slot->height = overflow;
        } else {
            slot->width = width;
        }
    } else {
        slot->height = height;
    }
    return count;
}

/* Forward declaration: defined later in this file (in ROM order, after
 * Class866E8__SetFootprintFromQuery). */
extern s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point);

/* Forward declaration: defined later in this file (in ROM order, after
 * IsPointOutOfBounds), but Class866E8__SetFootprintFromQuery calls it before its own definition
 * appears. */
extern s32 Class866E8__InitFootprintSlot(Obj866E8 *self, s32 unused, s32 key, s32 arg3);

void Class866E8__SetFootprintFromQuery(Obj866E8 *self) {
    s32 junk;
    CC74QueryBuf buf;

    self->methods->slot10C(self, &buf, 0);
    self->gridSlotCount = 0;
    self->gridSlotCount = Class866E8__InitFootprintSlot(self, junk, 0, buf.count);
    if (IsPointOutOfBounds(self->bounds, buf.point) != 0) {
        if (buf.count + 1 < self->unk68->count) {
            self->gridSlotCount = Class866E8__InitFootprintSlot(self, junk, self->gridSlotCount, buf.count + 1);
        }
    }
    if (buf.count - 1 >= 0) {
        self->gridSlotCount = Class866E8__InitFootprintSlot(self, junk, self->gridSlotCount, buf.count - 1);
    }
}

/* The in-range test is written as the NEGATION returning 0, with `return 1`
 * as the else arm: that is jump.c's "if (...) x = a; else x = b;" shape with
 * x = $v0 and b = 1, so the constant is preset into $v0 ahead of the first
 * test and stays live across every check block (keeping the block temps out
 * of $v0, and pushing `bounds` to $a2), and the final `>=` folds back into
 * the store-flag `slt`. See docs/match-reports/IsPointOutOfBounds.md. */
s32 IsPointOutOfBounds(Bounds866E8_3bb8c_b *bounds, s8 *point) {
    if (bounds != NULL && point[0] >= bounds->minX && bounds->maxX >= point[0]
        && point[1] >= bounds->minY && bounds->maxY >= point[1]) {
        return 0;
    }
    return 1;
}

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

    slot = self->gridSlots;
    for (i = 0; i < self->gridSlotCount; slot++, i++) {
        e = &self->arr[slot->elemIdx];
        if (e->unk4->unk2C == 0) {
            continue;
        }
        cell = e->unk10 + slot->col + slot->row * 20;
        for (j = 0; j < slot->height; j++) {
            for (k = 0; k < slot->width; k++) {
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
            cell += 20 - slot->width;
        }
    }
}

void *Class866E8__GetUnk1CC(Obj866E8 *self) {
    return &self->unk1CC;
}

void Class866E8__SetBounds(Obj866E8 *self, Bounds866E8_3bb8c_b *arg1) {
    self->bounds = arg1;
}

/* Picks one of the four static 0xC-byte EntryDesc866E8 entries by the sign of
 * `rate` and by `flag`, then sets rateCountdown to |rate| scaled by the chosen
 * entry's scale field.
 *
 * Two source shapes here are load-bearing and neither is cosmetic:
 *
 *  - The `goto` ladder, and its asymmetry. Retail emits TWO stores to
 *    rateEntry: the rate>0/flag!=0 path has its own (in a `j`'s delay slot at
 *    0x8004CFDC) and the other three SHARE one (0x8004CFF8). Writing the
 *    field directly on that one path and going through `table` on the other
 *    three is what reproduces that split. Byte-exact since round 9.
 *  - `scale` and `val`. Retail loads the entry's scale field ONCE (`lh $v1,6($v0)`)
 *    before the sign branch and keeps a single `mflo` after the join, with a
 *    `mult` in each arm. Caching the load in `scale` and letting an explicit
 *    if/else assign a local `val` is what defers that `mflo`; the
 *    default-then-overwrite spelling makes cc1 extract it eagerly, and
 *    storing to self->rateCountdown directly instead of through `val` perturbs the
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
    self->rateEntry = &D_800869A8;
    goto merge;
rate_le:
    table = &D_800869B4;
    if (flag == 0) {
        goto store;
    }
    table = &D_800869C0;
store:
    self->rateEntry = table;
merge:
    scale = self->rateEntry->scale;
    if (rate >= 0) {
        val = scale * rate;
    } else {
        val = scale * (~rate + 1);
    }
    self->rateCountdown = val;
}

/* Forward declaration: defined later in this file (after Class866E8__AdvanceRateCountdown in
 * ROM-address order), but passed to Class866E8__ForEachElem as a function-pointer
 * argument before its own definition appears. */
void Class866E8__ApplyRateToChild(Obj866E8 *self, EntryChildObj *item);

void Class866E8__AdvanceRateCountdown(Obj866E8 *self) {
    if (self->rateCountdown > 0) {
        Class866E8__ForEachElem(self, Class866E8__ApplyRateToChild, 0);
        self->rateCountdown -= 1;
        if (self->rateCountdown == 0) {
            self->rateCountdown = -1;
        }
    }
}

/* Forward declaration: defined later in this file (after Class866E8__FlushRateLatch in
 * ROM-address order), but passed to Class866E8__ForEachElem as a function-pointer
 * argument before its own definition appears. */
void Class866E8__ResetChildRate(Obj866E8 *self, EntryChildObj *item);

void Class866E8__FlushRateLatch(Obj866E8 *self) {
    if (self->rateCountdown != 0) {
        Class866E8__ForEachElem(self, Class866E8__ResetChildRate, 0);
        self->rateCountdown = 0;
    }
}

void Class866E8__ApplyRateToChild(Obj866E8 *self, EntryChildObj *item) {
    item->methods->slot48(item, 0, self->rateEntry);
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
