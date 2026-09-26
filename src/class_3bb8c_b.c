/*
 * class_3bb8c_b -- the last third of StageMap (include/StageMap.h),
 * sharing include/class_3bb8c.h with class_3bb8c.c.
 *
 *  - FindSlotIndexByNeighbour, FindSlotIndexByChunk: slot lookups.
 *  - The footprint: once every chunk is loaded, RefreshFootprint sets bit
 *    31 of `attribute` (libgs GsDOFF, display off) on the cells of the
 *    current `rects` and every cell chained behind them, rebuilds `rects`,
 *    the up to four cell rectangles around the target
 *    (ComputeFootprintFromRotation and BuildFootprintSlots/
 *    SplitFootprintSlot in a flat grid, SetFootprintFromQuery and
 *    InitFootprintSlot in a vertical one), and clears the bit on the new
 *    ones (SetFootprintCellFlag). IsPointOutOfBounds tests a cell against
 *    `bounds` (SetBounds).
 *  - The scale ramp: StartScaleRamp, StepScaleRamp, EndScaleRamp and their
 *    per-cell callbacks AddScaleStepToCell and ResetCellScale, run over
 *    every cell by ForEachSlot/ForEachSlotCell.
 *  - GetUnk1CC, and GetStageMapMethods.
 */
#include "common.h"
#include <libgte.h>
#include "class_3bb8c.h"
#include "LbdFile.h"
#include "GridCell.h"

s32 StageMap__FindSlotIndexByNeighbour(StageMap *self, s32 key) {
    s32 result;
    s32 i;
    ChunkSlot *e;

    result = 0;
    for (i = 0; i < 7; i++) {
        e = &self->slots[i];
        if (e->loader->elemKey == key) {
            result = i;
            break;
        }
    }
    return result;
}

s32 StageMap__FindSlotIndexByChunk(StageMap *self, s32 key) {
    s32 i;
    ChunkSlot *e;

    for (i = 0; i < 7; i++) {
        e = &self->slots[i];
        if (e->loader->chunkIndex == key && e->loader->headerReady != 0) {
            return i;
        }
    }
    return -1;
}

void StageMap__RefreshFootprint(StageMap *self) {
    s32 idx;

    if (self->chunksLoaded == 0) {
        return;
    }
    idx = self->gridHalfCells * 2;
    StageMap__SetFootprintCellFlag(self, 0);
    if (self->config->isVertical == 0) {
        StageMap__ComputeFootprintFromRotation(self, idx, self->gridCells);
    } else {
        StageMap__SetFootprintFromQuery(self);
    }
    StageMap__SetFootprintCellFlag(self, 1);
}

/* The template ComputeFootprintFromRotation copies before RotMatrix fills
 * its rotation part (bss). */
extern MATRIX D_8008E98C;

void StageMap__ComputeFootprintFromRotation(StageMap *self, s32 arg1, s32 arg2) {
    SceneNodeSub44 *sub;
    Descriptor10Ext buf;
    s32 point0;
    s32 point1;
    u16 angle; /* u16, not s32: the s32 form is byte-identical except
                     * for an 8-byte-smaller frame (round 71) */
    MATRIX mat;
    s32 offset;
    s32 flag;
    s32 half;
    void *rot;

    sub = self->target->coord2->param;
    rot = &sub->rotate;
    self->methods->getTargetDescriptor(self, &buf, 0);
    point0 = buf.base.b2;
    point1 = buf.base.b3;
    angle = sub->rotate.y;
    if (sub->rotate.y < 0) {
        angle += 0x1000;
    }

    mat = D_8008E98C;
    mat.t[0] = 0;
    mat.t[1] = 0;
    mat.t[2] = self->gridSpan;
    RotMatrix(rot, &mat);
    /* (0, 0, gridSpan) rotated in place: t is both the input and the output. */
    ApplyMatrixLV(&mat, (VECTOR *)mat.t, (VECTOR *)mat.t);

    if ((u16)(angle - 0x200) < 0x400 || (u16)(angle - 0xA00) < 0x400) {
        offset = mat.t[2];
        self->footprintWidth = arg2;
        self->footprintHeight = arg1;
        self->footprintCol = (mat.t[0] > 0) ? point0 : point0 - arg1 + 1;
        self->footprintRow = (mat.t[2] > 0) ? point1 - (u16)self->gridHalfCells - 1
                                            : point1 - (u16)self->gridHalfCells + 1;
        flag = 0;
    } else if ((u16)(angle - 0x600) < 0x400 || (u16)(angle - 0x200) >= 0xC00) {
        offset = mat.t[0];
        self->footprintWidth = arg1;
        self->footprintHeight = arg2;
        self->footprintCol = (mat.t[0] > 0) ? point0 - (u16)self->gridHalfCells - 1
                                            : point0 - (u16)self->gridHalfCells + 1;
        self->footprintRow = (mat.t[2] > 0) ? point1 : point1 - arg2 + 1;
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
    StageMap__BuildFootprintSlots(self);
}

/* Matched round 75. Three source-shape levers closed what was filed since
 * round 19 as a whole-function register rotation
 * (docs/match-reports/StageMap__BuildFootprintSlots.md): ONE slot pointer reused for the
 * second slot (no separate slot1), assigned once at the join after the
 * row<0 test (reorg fills the bgez delay slot from it and deletes the
 * redundant copy on the other path -- no barrier, no duplicate); the
 * clipped remainder in its own local `over` rather than `span -= 0x14`;
 * and `count += 1` as a statement in each arm plus at the join. */
void StageMap__BuildFootprintSlots(StageMap *self) {
    s32 flag;
    s32 col;
    s32 width;
    s32 height;
    s32 quadrant;
    s32 row;
    CellRect *slot;
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
    slot = &self->rects.e[0];
    slot->slotIndex = self->methods->findSlotIndexByNeighbour(self, quadrant);
    slot->col = (col >= 0) ? col : 0;
    slot->row = row;
    span = col + width;
    if (span >= 0x15) {
        over = span - 0x14;
        slot->width = width - over;
        count = StageMap__SplitFootprintSlot(self, slot, 0, quadrant, col, row, width, height);
        count += 1;
        slot = &self->rects.e[count];
        slot->slotIndex = self->methods->findSlotIndexByNeighbour(self, quadrant + 1);
        slot->col = 0;
        slot->row = self->rects.e[0].row;
        slot->width = over;
        slot->height = self->rects.e[0].height;
    } else {
        slot->width = width;
        count = StageMap__SplitFootprintSlot(self, slot, 0, quadrant, col, row, width, height);
    }
    count += 1;
    self->rectCount = count;
}

s32 StageMap__SplitFootprintSlot(StageMap *self, CellRect *slot, s32 count, s32 baseIdx, s32 col,
                                 s32 row, s32 width, s32 height) {
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
        slot = &self->rects.e[count];

        if (col < 10) {
            elemArg = baseIdx + 2;
            slot->slotIndex = self->methods->findSlotIndexByNeighbour(self, elemArg);
            slot->col = col + 10;
            /* Stored in BOTH arms: cross-jumping merges the copies, and
             * the join label keeps the col reload below after it. */
            slot->height = span;
        } else {
            elemArg = baseIdx + 3;
            slot->slotIndex = self->methods->findSlotIndexByNeighbour(self, elemArg);
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
            slot = &self->rects.e[count];
            slot->slotIndex = self->methods->findSlotIndexByNeighbour(self, elemArg + 1);
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

void StageMap__SetFootprintFromQuery(StageMap *self) {
    s32 junk;
    Descriptor10Ext buf;

    self->methods->getTargetDescriptor(self, &buf, 0);
    self->rectCount = 0;
    self->rectCount = StageMap__InitFootprintSlot(self, junk, 0, buf.chunkIndex);
    if (IsPointOutOfBounds(self->bounds, &buf.base.b2) != 0) {
        if (buf.chunkIndex + 1 < self->config->rows) {
            self->rectCount =
                StageMap__InitFootprintSlot(self, junk, self->rectCount, buf.chunkIndex + 1);
        }
    }
    if (buf.chunkIndex - 1 >= 0) {
        self->rectCount = StageMap__InitFootprintSlot(self, junk, self->rectCount, buf.chunkIndex - 1);
    }
}

/* The in-range test is written as the NEGATION returning 0, with `return 1`
 * as the else arm: that is jump.c's "if (...) x = a; else x = b;" shape with
 * x = $v0 and b = 1, so the constant is preset into $v0 ahead of the first
 * test and stays live across every check block (keeping the block temps out
 * of $v0, and pushing `bounds` to $a2), and the final `>=` folds back into
 * the store-flag `slt`. See docs/match-reports/IsPointOutOfBounds.md. */
s32 IsPointOutOfBounds(CellBounds *bounds, s8 *point) {
    if (bounds != NULL && point[0] >= bounds->minCol && bounds->maxCol >= point[0] &&
        point[1] >= bounds->minRow && bounds->maxRow >= point[1]) {
        return 0;
    }
    return 1;
}

s32 StageMap__InitFootprintSlot(StageMap *self, s32 unused, s32 key, s32 arg3) {
    CellRect *slot;

    slot = &self->rects.e[key];
    *slot = gFullSlotRect;
    slot->slotIndex = self->methods->findSlotIndexByChunk(self, arg3);
    return key + 1;
}

void StageMap__SetFootprintCellFlag(StageMap *self, s32 setBit) {
    s32 i;
    s32 j;
    s32 k;
    CellRect *slot;
    ChunkSlot *e;
    GridCell **cell;
    GridCell *next;

    slot = self->rects.e;
    for (i = 0; i < self->rectCount; slot++, i++) {
        e = &self->slots[slot->slotIndex];
        if (e->loader->headerReady == 0) {
            continue;
        }
        cell = e->cells + slot->col + slot->row * 20;
        for (j = 0; j < slot->height; j++) {
            for (k = 0; k < slot->width; k++) {
                if (setBit != 0) {
                    (*cell)->attribute &= 0x7FFFFFFF;
                } else {
                    (*cell)->attribute |= 0x80000000;
                }
                next = (*cell)->nextInCell;
                while (next != 0) {
                    if (setBit != 0) {
                        next->attribute &= 0x7FFFFFFF;
                    } else {
                        next->attribute |= 0x80000000;
                    }
                    next = next->nextInCell;
                }
                cell++;
            }
            cell += 20 - slot->width;
        }
    }
}

void *StageMap__GetUnk1CC(StageMap *self) {
    return &self->unk1CC;
}

void StageMap__SetBounds(StageMap *self, CellBounds *arg1) {
    self->bounds = arg1;
}

/* Picks one of the four static Ratio16[3] scale steps by the sign of
 * `rate` and by `flag`, then sets scaleRampTicks to |rate| scaled by the chosen
 * step's y denominator (scaleStep[1].den).
 *
 * Two source shapes here are load-bearing and neither is cosmetic:
 *
 *  - The `goto` ladder, and its asymmetry. Retail emits TWO stores to
 *    scaleStep: the rate>0/flag!=0 path has its own (in a `j`'s delay slot at
 *    0x8004CFDC) and the other three SHARE one (0x8004CFF8). Writing the
 *    field directly on that one path and going through `table` on the other
 *    three is what reproduces that split. Byte-exact since round 9.
 *  - `scale` and `val`. Retail loads the step's y denominator ONCE (`lh $v1,6($v0)`)
 *    before the sign branch and keeps a single `mflo` after the join, with a
 *    `mult` in each arm. Caching the load in `scale` and letting an explicit
 *    if/else assign a local `val` is what defers that `mflo`; the
 *    default-then-overwrite spelling makes cc1 extract it eagerly, and
 *    storing to self->scaleRampTicks directly instead of through `val` perturbs the
 *    table-selection half as well. Both were measured -- round 58 and
 *    docs/match-reports/StageMap__StartScaleRamp.md.
 *
 * `~rate + 1` is retail's own negation (`nor`/`addiu`), not `-rate`. */
void StageMap__StartScaleRamp(StageMap *self, s32 rate, s32 flag) {
    Ratio16 *table;
    s32 val;
    s32 scale;

    if (rate <= 0) {
        goto rate_le;
    }
    table = D_8008699C;
    if (flag == 0) {
        goto store;
    }
    self->scaleStep = D_800869A8;
    goto merge;
rate_le:
    table = D_800869B4;
    if (flag == 0) {
        goto store;
    }
    table = D_800869C0;
store:
    self->scaleStep = table;
merge:
    scale = self->scaleStep[1].den;
    if (rate >= 0) {
        val = scale * rate;
    } else {
        val = scale * (~rate + 1);
    }
    self->scaleRampTicks = val;
}

void StageMap__StepScaleRamp(StageMap *self) {
    if (self->scaleRampTicks > 0) {
        StageMap__ForEachSlot(self, StageMap__AddScaleStepToCell, 0);
        self->scaleRampTicks -= 1;
        if (self->scaleRampTicks == 0) {
            self->scaleRampTicks = -1;
        }
    }
}

void StageMap__EndScaleRamp(StageMap *self) {
    if (self->scaleRampTicks != 0) {
        StageMap__ForEachSlot(self, StageMap__ResetCellScale, 0);
        self->scaleRampTicks = 0;
    }
}

void StageMap__AddScaleStepToCell(StageMap *self, GridCell *item) {
    item->methods->updateScale(item, 0, self->scaleStep);
}

void StageMap__ResetCellScale(StageMap *self, GridCell *item) {
    item->methods->updateScale(item, 1, D_800869CC);
}

void StageMap__ForEachSlot(StageMap *self, StageMapCellFn arg1, ChunkSlotFn arg2) {
    s32 i;
    ChunkSlot *e;

    for (i = 0; i < 7; i++) {
        e = &self->slots[i];
        if (arg2 != 0) {
            arg2(self, e);
        }
        StageMap__ForEachSlotCell(self, arg1, e);
    }
}

void StageMap__ForEachSlotCell(StageMap *self, StageMapCellFn callback, ChunkSlot *item) {
    GridCell **p;
    GridCell **end;

    end = item->cells + (0x668 / 4);
    p = item->cells;
    for (; p < end; p++) {
        callback(self, *p);
    }
}

StageMapMethods *GetStageMapMethods(void) {
    return &gStageMapMethods;
}
