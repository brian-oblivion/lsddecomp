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
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "LbdFile.h"
#include "GridCell.h"

s32 StageMap__FindSlotIndexByNeighbour(StageMap *self, s32 key) {
    s32 index;
    s32 i;
    ChunkSlot *slot;

    index = 0;
    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->elemKey == key) {
            index = i;
            break;
        }
    }
    return index;
}

s32 StageMap__FindSlotIndexByChunk(StageMap *self, s32 chunkIndex) {
    s32 i;
    ChunkSlot *slot;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->chunkIndex == chunkIndex && slot->loader->headerReady != 0) {
            return i;
        }
    }
    return -1;
}

void StageMap__RefreshFootprint(StageMap *self) {
    s32 acrossCells;

    if (self->chunksLoaded == 0) {
        return;
    }
    acrossCells = self->gridHalfCells * 2;
    StageMap__SetFootprintVisible(self, 0);
    if (self->config->isVertical == 0) {
        StageMap__ComputeFootprintFromRotation(self, acrossCells, self->gridCells);
    } else {
        StageMap__SetFootprintFromQuery(self);
    }
    StageMap__SetFootprintVisible(self, 1);
}

void StageMap__ComputeFootprintFromRotation(StageMap *self, s32 acrossCells, s32 aheadCells) {
    SceneNodeSub44 *param;
    Descriptor10Ext desc;
    s32 cellCol;
    s32 cellRow;
    u16 angle; /* u16, not s32: the s32 form is byte-identical except
                     * for an 8-byte-smaller frame (round 71) */
    MATRIX mat;
    s32 lateral;
    s32 shiftCol;
    s32 half;
    void *rot;

    param = self->target->coord2->param;
    rot = &param->rotate;
    self->methods->getTargetDescriptor(self, &desc, 0);
    cellCol = desc.base.b2;
    cellRow = desc.base.b3;
    angle = param->rotate.y;
    if (param->rotate.y < 0) {
        angle += ONE;
    }

    mat = GsIDMATRIX;
    mat.t[0] = 0;
    mat.t[1] = 0;
    mat.t[2] = self->gridSpan;
    RotMatrix(rot, &mat);
    /* (0, 0, gridSpan) rotated in place: t is both the input and the output. */
    ApplyMatrixLV(&mat, (VECTOR *)mat.t, (VECTOR *)mat.t);

    if ((u16)(angle - ANGLE_DEG(45)) < ANGLE_DEG(90) || (u16)(angle - ANGLE_DEG(225)) < ANGLE_DEG(90)) {
        lateral = mat.t[2];
        self->footprintWidth = aheadCells;
        self->footprintHeight = acrossCells;
        self->footprintCol = (mat.t[0] > 0) ? cellCol : cellCol - acrossCells + 1;
        self->footprintRow = (mat.t[2] > 0) ? cellRow - (u16)self->gridHalfCells - 1
                                            : cellRow - (u16)self->gridHalfCells + 1;
        shiftCol = 0;
    } else if ((u16)(angle - ANGLE_DEG(135)) < ANGLE_DEG(90) ||
               (u16)(angle - ANGLE_DEG(45)) >= ANGLE_DEG(270)) {
        lateral = mat.t[0];
        self->footprintWidth = acrossCells;
        self->footprintHeight = aheadCells;
        self->footprintCol = (mat.t[0] > 0) ? cellCol - (u16)self->gridHalfCells - 1
                                            : cellCol - (u16)self->gridHalfCells + 1;
        self->footprintRow = (mat.t[2] > 0) ? cellRow : cellRow - aheadCells + 1;
        shiftCol = 1;
    }

    half = self->gridHalfCells;
    lateral >>= STAGE_CELL_SHIFT;
    if (lateral >= half) {
        lateral = half - 1;
    }
    half = -half;
    if (half >= lateral) {
        lateral = half + 1;
    }
    if (shiftCol) {
        self->footprintCol = (u16)self->footprintCol + lateral;
    } else {
        self->footprintRow = (u16)self->footprintRow + lateral;
    }
    StageMap__BuildFootprintRects(self);
}

/* Matched round 75. Three source-shape levers closed what was filed since
 * round 19 as a whole-function register rotation
 * (docs/match-reports/StageMap__BuildFootprintRects.md): ONE slot pointer reused for the
 * second slot (no separate slot1), assigned once at the join after the
 * row<0 test (reorg fills the bgez delay slot from it and deletes the
 * redundant copy on the other path -- no barrier, no duplicate); the
 * clipped remainder in its own local `over` rather than `span -= 0x14`;
 * and `count += 1` as a statement in each arm plus at the join. */
void StageMap__BuildFootprintRects(StageMap *self) {
    s32 wrappedCol;
    s32 col;
    s32 width;
    s32 height;
    s32 key;
    s32 row;
    CellRect *rect;
    s32 span;
    s32 count;
    s32 over;

    wrappedCol = 0;
    col = self->footprintCol;
    width = self->footprintWidth;
    height = self->footprintHeight;
    key = CHUNK_NEIGHBOUR_CENTRE;
    if (col < 0) {
        col += STAGE_CHUNK_CELLS;
        wrappedCol = 1;
        key = CHUNK_NEIGHBOUR_PREV_COL;
    }
    row = self->footprintRow;
    if (row < 0) {
        row += STAGE_CHUNK_CELLS;
        if (wrappedCol != 0) {
            col -= STAGE_CHUNK_HALF_CELLS;
            key = CHUNK_NEIGHBOUR_PREV_ROW_LO;
        } else if (col < STAGE_CHUNK_HALF_CELLS) {
            col += STAGE_CHUNK_HALF_CELLS;
            key = CHUNK_NEIGHBOUR_PREV_ROW_LO;
        } else {
            col -= STAGE_CHUNK_HALF_CELLS;
            key = CHUNK_NEIGHBOUR_PREV_ROW_HI;
        }
    }
    rect = &self->rects.e[0];
    rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, key);
    rect->col = (col >= 0) ? col : 0;
    rect->row = row;
    span = col + width;
    if (span > STAGE_CHUNK_CELLS) {
        over = span - STAGE_CHUNK_CELLS;
        rect->width = width - over;
        count = StageMap__SplitFootprintRect(self, rect, 0, key, col, row, width, height);
        count += 1;
        rect = &self->rects.e[count];
        rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, key + 1);
        rect->col = 0;
        rect->row = self->rects.e[0].row;
        rect->width = over;
        rect->height = self->rects.e[0].height;
    } else {
        rect->width = width;
        count = StageMap__SplitFootprintRect(self, rect, 0, key, col, row, width, height);
    }
    count += 1;
    self->rectCount = count;
}

s32 StageMap__SplitFootprintRect(StageMap *self, CellRect *rect, s32 count, s32 key, s32 col,
                                 s32 row, s32 width, s32 height) {
    s32 rowsBelow;
    s32 span;
    s32 belowKey;
    s32 widthLeft;

    if (row + height > STAGE_CHUNK_CELLS) {
        /* The rectangle runs past the bottom edge (row 20): clip this rect
         * and open a new one for the part below. */
        rowsBelow = (row + height) - STAGE_CHUNK_CELLS;
        span = rowsBelow;
        rect->height = height - rowsBelow;
        count = count + 1;
        rect = &self->rects.e[count];

        if (col < STAGE_CHUNK_HALF_CELLS) {
            belowKey = key + 2;
            rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, belowKey);
            rect->col = col + STAGE_CHUNK_HALF_CELLS;
            /* Stored in BOTH arms: cross-jumping merges the copies, and
             * the join label keeps the col reload below after it. */
            rect->height = span;
        } else {
            belowKey = key + 3;
            rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, belowKey);
            rect->col = col - STAGE_CHUNK_HALF_CELLS;
            rect->height = span;
        }

        span = rect->col + width;
        rect->row = 0;
        if (span > STAGE_CHUNK_CELLS) {
            /* ...and past the right edge (column 20) too. */
            count = count + 1;
            /* Two statements, not `(width + 20) - span`: fold rewrites
             * that tree as `width - (span - 20)` and CSE then shares
             * `span - 20` with the store below (round 71). */
            widthLeft = width + STAGE_CHUNK_CELLS;
            widthLeft = widthLeft - span;
            rect->width = widthLeft;
            rect = &self->rects.e[count];
            rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, belowKey + 1);
            rect->col = 0;
            rect->row = 0;
            rect->width = span - STAGE_CHUNK_CELLS;
            rect->height = rowsBelow;
        } else {
            rect->width = width;
        }
    } else {
        rect->height = height;
    }
    return count;
}

void StageMap__SetFootprintFromQuery(StageMap *self) {
    s32 junk;
    Descriptor10Ext desc;

    self->methods->getTargetDescriptor(self, &desc, 0);
    self->rectCount = 0;
    self->rectCount = StageMap__InitFootprintRect(self, junk, 0, desc.chunkIndex);
    if (IsPointOutOfBounds(self->bounds, &desc.base.b2) != 0) {
        if (desc.chunkIndex + 1 < self->config->rows) {
            self->rectCount =
                StageMap__InitFootprintRect(self, junk, self->rectCount, desc.chunkIndex + 1);
        }
    }
    if (desc.chunkIndex - 1 >= 0) {
        self->rectCount = StageMap__InitFootprintRect(self, junk, self->rectCount, desc.chunkIndex - 1);
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

s32 StageMap__InitFootprintRect(StageMap *self, s32 unused, s32 index, s32 chunkIndex) {
    CellRect *rect;

    rect = &self->rects.e[index];
    *rect = gFullSlotRect;
    rect->slotIndex = self->methods->findSlotIndexByChunk(self, chunkIndex);
    return index + 1;
}

void StageMap__SetFootprintVisible(StageMap *self, s32 visible) {
    s32 i;
    s32 j;
    s32 k;
    CellRect *rect;
    ChunkSlot *slot;
    GridCell **cell;
    GridCell *next;

    rect = self->rects.e;
    for (i = 0; i < self->rectCount; rect++, i++) {
        slot = &self->slots[rect->slotIndex];
        if (slot->loader->headerReady == 0) {
            continue;
        }
        cell = slot->cells + rect->col + rect->row * STAGE_CHUNK_CELLS;
        for (j = 0; j < rect->height; j++) {
            for (k = 0; k < rect->width; k++) {
                if (visible != 0) {
                    (*cell)->attribute &= ~GsDOFF;
                } else {
                    (*cell)->attribute |= GsDOFF;
                }
                next = (*cell)->nextInCell;
                while (next != NULL) {
                    if (visible != 0) {
                        next->attribute &= ~GsDOFF;
                    } else {
                        next->attribute |= GsDOFF;
                    }
                    next = next->nextInCell;
                }
                cell++;
            }
            cell += STAGE_CHUNK_CELLS - rect->width;
        }
    }
}

void *StageMap__GetUnk1CC(StageMap *self) {
    return &self->unk1CC;
}

void StageMap__SetBounds(StageMap *self, CellBounds *bounds) {
    self->bounds = bounds;
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
void StageMap__StartScaleRamp(StageMap *self, s32 rate, s32 fast) {
    Ratio16 *table;
    s32 val;
    s32 scale;

    if (rate <= 0) {
        goto rate_le;
    }
    table = sScaleStepUpSlow;
    if (fast == 0) {
        goto store;
    }
    self->scaleStep = sScaleStepUpFast;
    goto merge;
rate_le:
    table = sScaleStepDownSlow;
    if (fast == 0) {
        goto store;
    }
    table = sScaleStepDownFast;
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

void StageMap__AddScaleStepToCell(StageMap *self, GridCell *cell) {
    cell->methods->updateScale(cell, 0, self->scaleStep);
}

void StageMap__ResetCellScale(StageMap *self, GridCell *cell) {
    cell->methods->updateScale(cell, 1, sScaleOne);
}

void StageMap__ForEachSlot(StageMap *self, StageMapCellFn cellFn, ChunkSlotFn slotFn) {
    s32 i;
    ChunkSlot *slot;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slotFn != 0) {
            slotFn(self, slot);
        }
        StageMap__ForEachSlotCell(self, cellFn, slot);
    }
}

void StageMap__ForEachSlotCell(StageMap *self, StageMapCellFn cellFn, ChunkSlot *slot) {
    GridCell **cell;
    GridCell **end;

    end = slot->cells + STAGE_SLOT_CELLS;
    cell = slot->cells;
    for (; cell < end; cell++) {
        cellFn(self, *cell);
    }
}

StageMapMethods *GetStageMapMethods(void) {
    return &gStageMapMethods;
}
