/*
 * class_3bb8c_b -- StageMap's drawn window and scale ramp: the last of the
 * class's methods (include/StageMap.h; the others are in class_39e08.c and
 * class_39e08.c).
 *
 *  - FindSlotIndexByNeighbour, FindSlotIndexByChunk: which of the seven
 *    slots holds a neighbour key, or a loaded chunk.
 *  - The footprint, the window of cells that is drawn. Once every chunk is
 *    loaded, RefreshFootprint hides the cells of the current `rects`, and
 *    every cell chained behind them, rebuilds `rects` and shows the cells of
 *    the new ones (SetFootprintVisible: GsDOFF in each cell's `attribute`).
 *    `rects` is up to four CellRects, one per slot the window overlaps. In
 *    a flat grid the window lies ahead of the target along the axis nearest
 *    its facing and is shifted sideways toward where it looks
 *    (ComputeFootprintFromRotation); BuildFootprintRects and
 *    SplitFootprintRect clip it at the chunk's right and bottom edges into
 *    the neighbouring slots. In a vertical grid it is whole chunks: the
 *    target's, the previous one, and the next one when the target's cell
 *    lies outside `bounds` (SetFootprintFromQuery, InitFootprintRect,
 *    IsPointOutOfBounds; SetBounds).
 *  - The scale ramp: StartScaleRamp picks a step and a tick count,
 *    StepScaleRamp adds the step to every cell's scale once a tick, and
 *    EndScaleRamp sets every cell back to 1/1 (AddScaleStepToCell and
 *    ResetCellScale, run on every cell of every slot by ForEachSlot and
 *    ForEachSlotCell).
 *  - GetUnk1CC, and GetStageMapMethods.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "LbdFile.h"
#include "GridCell.h"

/* The four scale steps (Ratio16[3], x/y/z) StartScaleRamp picks for
 * `scaleStep`: y +1/64 and +1/4 for a positive rate (fast 0, nonzero),
 * -1/64 and -1/4 otherwise; x and z 0/1. */
extern Ratio16 sScaleStepUpSlow[3];
extern Ratio16 sScaleStepUpFast[3];
extern Ratio16 sScaleStepDownSlow[3];
extern Ratio16 sScaleStepDownFast[3];

/* 1/1, 1/1, 1/1: the scale ResetCellScale sets on every cell. */
extern Ratio16 sScaleOne[3];

/* The index of the slot whose chunk is at neighbour key `key`; 0 when
 * none is. */
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

/* The index of the slot holding chunk chunkIndex with its header read; -1
 * when none does. */
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

/* The flat grid's window, from the target's cell and its y rotation: a
 * window aheadCells deep along whichever of x and z the target faces
 * (within 45 degrees), starting at the target's cell and running the way it
 * faces, and acrossCells wide, centred on the target and shifted by the
 * off-axis part of a gridSpan-long facing vector, in cells, kept inside half
 * the grid. The window goes to BuildFootprintRects as footprintCol/Row and
 * footprintWidth/Height. MATCHING: the (u16) casts are retail's lhu. */
void StageMap__ComputeFootprintFromRotation(StageMap *self, s32 acrossCells, s32 aheadCells) {
    GsCOORD2PARAM *param;
    Descriptor10Ext desc;
    s32 cellCol;
    s32 cellRow;
    u16 angle; /* MATCHING: u16; s32 makes the frame 8 bytes smaller */
    MATRIX mat;
    s32 lateral;
    s32 shiftCol;
    s32 half;
    void *rot;

    param = self->target->coord2->param;
    rot = &param->rotate; /* MATCHING: here, before the call, so it lives across it */
    self->methods->getTargetDescriptor(self, &desc, 0);
    cellCol = desc.base.b2;
    cellRow = desc.base.b3;
    angle = param->rotate.vy;
    if (param->rotate.vy < 0) {
        angle += ONE;
    }

    /* The facing vector: (0, 0, gridSpan) turned by the target's rotation,
     * in place (t is both ApplyMatrixLV's input and its output). */
    mat = GsIDMATRIX;
    mat.t[0] = 0;
    mat.t[1] = 0;
    mat.t[2] = self->gridSpan;
    RotMatrix(rot, &mat);
    ApplyMatrixLV(&mat, (VECTOR *)mat.t, (VECTOR *)mat.t);

    /* Facing +-x, then facing +-z. MATCHING: the second test is retail's; the
     * two cover every angle. */
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

/* Splits the window (footprintCol/Row, footprintWidth/Height) into
 * `rects`: a window that starts left of the centre chunk or above it starts
 * in that neighbour's slot, with its column and row moved into that chunk
 * (rows above are staggered by half a chunk); a part past the right edge
 * goes to the slot of key + 1, and SplitFootprintRect splits off the part
 * past the bottom edge. MATCHING: one `rect` pointer reused for the second
 * rectangle, `over` its own local, and `count += 1` in each arm and again at
 * the join. */
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
        /* The rectangle runs past the chunk's bottom edge: clip this rect
         * and open a new one, in the slot below, for the rest. */
        rowsBelow = (row + height) - STAGE_CHUNK_CELLS;
        span = rowsBelow;
        rect->height = height - rowsBelow;
        count = count + 1;
        rect = &self->rects.e[count];

        if (col < STAGE_CHUNK_HALF_CELLS) {
            belowKey = key + 2;
            rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, belowKey);
            rect->col = col + STAGE_CHUNK_HALF_CELLS;
            /* MATCHING: stored in both arms (cross-jumping merges them, and
             * the join keeps the col reload below after it). */
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
            /* ...and past the right edge too. */
            count = count + 1;
            /* MATCHING: two statements; one expression shares span - 20
             * with the store below. */
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
    s32 junk; /* MATCHING: never set; InitFootprintRect ignores the argument */
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

/* 1 when there are no bounds or the point (cell column, row) lies outside
 * them, else 0. MATCHING: the in-range test returns 0 and `return 1`
 * follows it; the other order allocates differently. */
s32 IsPointOutOfBounds(CellBounds *bounds, s8 *point) {
    if (bounds != NULL && point[0] >= bounds->minCol && bounds->maxCol >= point[0] &&
        point[1] >= bounds->minRow && bounds->maxRow >= point[1]) {
        return 0;
    }
    return 1;
}

/* rects[index] becomes the whole of the slot holding chunkIndex; returns the
 * next index. */
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

/* Picks the scale step by the sign of `rate` and by `fast`, and runs the
 * ramp for |rate| times the step's y denominator ticks (scaleStep[1].den).
 * MATCHING: the goto ladder (retail stores scaleStep on the fast positive
 * path and once for the other three), `scale` loaded once before the sign
 * test, and `val` set in an if/else; `~rate + 1` is retail's negation. */
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
