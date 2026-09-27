/*
 * class_3bb8c -- the middle of StageMap's methods (include/StageMap.h, whose
 * banner describes the class): placing a cell descriptor in the world,
 * loading the seven chunk slots around a centre chunk, linking a loaded
 * chunk into its slot's cells, and the queries that turn a position back
 * into a slot and cell. class_3ac78.c holds the methods before these and
 * class_3bb8c_b.c those after; the class's data tables are declared in
 * include/class_3bb8c.h.
 *
 *  - SetTargetAndLoadChunks, ComputeCellOffsets, ComputeCellWorldOffsets:
 *    a cell descriptor to a world position and the chunk it lies in.
 *  - Enable/Disable, UpdateFootprintTracking: the per-tick tracking of the
 *    target.
 *  - LoadChunksAround, ComputeNeighbourMask, ComputeChunkLoadEntry,
 *    ApplyChunkLoads, CountPendingLoads: moving the slots around a centre
 *    chunk and starting (or cancelling) each slot's LbdFile load.
 *  - OnNotifyTag1: on the DrawSystem's per-VSync notification, finishing
 *    the loads that have completed.
 *  - PopulateSlotCells / ClearSlotCells: linking a loaded chunk's
 *    placements and models into its slot's cells, and clearing them.
 *  - GetTargetDescriptor, ComputeFootprintDescriptor, SplitChunkIndex,
 *    GetLastEventSlotChunk, FindSlotByNeighbour, FindSlotForPosition:
 *    queries. A slot's position is its cellParent's GsCOORDINATE2, read
 *    through SplitCoord2 (include/class_3bb8c.h).
 *
 * Positions are in world units: a cell is STAGE_CELL_SIZE square, a chunk
 * STAGE_CHUNK_SIZE (include/StageMap.h).
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "PlacementGrid.h"
#include "LinkResource.h"
#include "LbdFile.h"
#include "GridCell.h"

/* The height of one layer of a vertical grid: FindSlotForPosition gives
 * neighbour key i the y range (-(i + 1) * height, -i * height]. */
#define VERTICAL_LAYER_HEIGHT 2048

s32 StageMap__SetTargetAndLoadChunks(StageMap *self, void *outPos, SceneNode *target, Descriptor10 *cell) {
    s32 chunkCentre[3];
    s32 chunkIndex;

    self->target = target;
    self->targetCell.base = *cell;
    chunkIndex = ComputeCellWorldOffsets(outPos, chunkCentre, self->config, &self->origin, cell);
    return self->methods->loadChunksAround(self, chunkIndex, (LongVec3 *)chunkCentre, sDefaultTargetSpecs);
}

s32 StageMap__ComputeCellOffsets(StageMap *self, void *outPos, void *cell) {
    s32 chunkCentre[3];

    return ComputeCellWorldOffsets(outPos, chunkCentre, self->config, &self->origin, cell);
}

/* A cell descriptor to world positions: writes the chunk's centre to
 * chunkPos and the cell point (cell column/row plus the offset inside the
 * cell, measured from the cell's centre) to outPos, and returns the chunk's
 * index (0 in a vertical grid). The grid is centred on `origin`; odd rows sit
 * half a chunk to -x. */
s32 ComputeCellWorldOffsets(s32 *outPos, s32 *chunkPos, StageGridDimensions *dims, LongVec3 *origin,
                            Descriptor10 *cell) {
    s32 row;
    s32 rowSpan;
    s32 chunkIndex;
    s32 x;
    s32 z;
    s32 halfCell;

    if (dims->isVertical == 0) {
        row = cell->b1;
        rowSpan = dims->rows;
        chunkIndex = cell->b0 + dims->columns * row;
    } else {
        rowSpan = 1;
        row = 0;
        chunkIndex = 0;
    }
    x = (origin->x - dims->columns * (STAGE_CHUNK_SIZE / 2)) + cell->b0 * STAGE_CHUNK_SIZE;
    z = origin->z - rowSpan * (STAGE_CHUNK_SIZE / 2);
    chunkPos[0] = x;
    if (row & 1) {
        chunkPos[0] = x - STAGE_CHUNK_SIZE / 2;
    }
    chunkPos[1] = origin->y;
    halfCell = STAGE_CELL_SIZE / 2; /* MATCHING: a local, set here, places retail's constant load */
    chunkPos[2] = z + row * STAGE_CHUNK_SIZE;
    outPos[0] = (cell->b2 << STAGE_CELL_SHIFT) + chunkPos[0] + (cell->h4 + halfCell);
    outPos[1] = cell->h6 + chunkPos[1];
    outPos[2] = (cell->b3 << STAGE_CELL_SHIFT) + chunkPos[2] + (cell->h8 + halfCell);
    chunkPos[0] += STAGE_CHUNK_SIZE / 2;
    chunkPos[2] = chunkPos[2] + STAGE_CHUNK_SIZE / 2;
    return chunkIndex;
}

void StageMap__Enable(StageMap *self) {
    self->enabled = 1;
}

void StageMap__Disable(StageMap *self) {
    self->methods->unloadAllSlots(self);
    self->enabled = 0;
}

/* The per-tick tracking: re-reads the target's descriptor; in a flat grid,
 * reloads the slots the target's slot selects (sFootprintResultRemap) around
 * its chunk; moves the drawn window; notifies the parents when the chunk
 * changed. Returns the selected spec's index (0 for the centre slot, which
 * changes nothing). */
s32 StageMap__UpdateFootprintTracking(StageMap *self) {
    Descriptor10Ext desc;
    ChunkSlot *slot;
    s32 neighbour;
    s32 specIndex;
    u16 oldChunk;

    if (self->methods->getTargetDescriptor(self, &desc, 0) == 0) {
        return 0;
    }

    slot = desc.slot;
    neighbour = slot->loader->elemKey;
    specIndex = sFootprintResultRemap[neighbour];

    if (self->config->isVertical == 0) {
        self->methods->loadChunksAround(self, desc.chunkIndex, &desc.chunkCentre,
                                        sFootprintResultPtrTable[specIndex]);
    }

    self->methods->refreshFootprint(self);

    oldChunk = *(u16 *)&self->targetCell;
    self->targetCell = desc;

    if ((s16)oldChunk != *(s16 *)&desc) {
        self->methods->notifyParents(self, STAGEMAP_EVENT_CHUNK_CHANGED);
    }

    return specIndex;
}

/* Moves each slot `specs` marks for loading to the centre position plus
 * its neighbour key's offset (a vertical grid: to its layer), builds a
 * ChunkLoadEntry for it, gives every slot its new key, then starts the
 * loads (applyChunkLoads). */
void StageMap__LoadChunksAround(StageMap *self, s32 centreChunk, LongVec3 *centrePos,
                                ChunkSlotSpec *specs) {
    s32 columns;
    s32 oddRow;
    s32 onGridMask;
    s32 count;
    s32 i;
    ChunkSlot *slot;
    SplitCoord2 *origin;
    LongVec3 *offset;
    ChunkLoadEntry loads[CHUNK_NEIGHBOUR_COUNT];

    if (specs != 0) {
        columns = self->config->columns;
        oddRow = (centreChunk / columns) & 1;
        onGridMask = StageMap__ComputeNeighbourMask(self, centreChunk, oddRow);

        count = 0;
        for (i = 0; i < CHUNK_NEIGHBOUR_COUNT; i++) {
            slot = self->methods->findSlotByNeighbour(self, i);
            slot->neighbour = specs[i].neighbour;
            if (specs[i].load != 0) {
                offset = &sNeighbourOffsets[specs[i].neighbour];
                origin = (SplitCoord2 *)slot->cellParent->coord2;
                if (self->config->isVertical == 0) {
                    origin->tx.w = centrePos->x + offset->x;
                    origin->ty = centrePos->y;
                    origin->tz.w = centrePos->z + offset->z;
                } else {
                    origin->tx.w = centrePos->x - (STAGE_CHUNK_SIZE / 2);
                    origin->ty = centrePos->y + offset->y;
                    origin->tz.w = centrePos->z - (STAGE_CHUNK_SIZE / 2);
                }
                slot->cellParent->coord2->flg = 0;
                StageMap__ComputeChunkLoadEntry(self, &loads[count], columns, oddRow, centreChunk,
                                                onGridMask, specs[i].neighbour);
                count++;
            }
        }

        for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
            slot = &self->slots[i]; /* MATCHING: the first loop's `slot`; a second local swaps a register */
            slot->loader->elemKey = slot->neighbour;
        }

        self->methods->applyChunkLoads(self, loads, count);
    }
}

s32 StageMap__ComputeNeighbourMask(StageMap *self, s32 chunk, s32 oddRow) {
    StageGridDimensions *dims;
    s32 columns;
    s32 isVertical;
    s32 rows;
    s32 offGrid;
    s32 i;

    dims = self->config;
    columns = dims->columns;
    isVertical = dims->isVertical;
    rows = dims->rows;
    if (isVertical == 0) {
        offGrid = (chunk < columns) ? CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_LO) |
                                          CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_HI)
                                    : 0;
        if (chunk >= columns * (rows - 1)) {
            offGrid |= CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_LO) |
                       CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_HI);
        }
        if (chunk % columns == 0) {
            offGrid |= oddRow ? CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_LO) |
                                    CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_COL) |
                                    CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_LO)
                              : CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_COL);
        }
        if ((chunk + 1) % columns != 0) {
            return ~offGrid;
        }
        offGrid |= oddRow ? CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_COL)
                          : CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_HI) |
                                CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_COL) |
                                CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_HI);
        return ~offGrid;
    } else {
        offGrid = -1;
        for (i = 0; i < rows; i++) {
            offGrid <<= 1;
        }
        return ~offGrid;
    }
}

/* Fills `out` for the slot taking neighbour key `neighbour` of centreChunk:
 * the neighbour's chunk index and the chunk's file record from the
 * callback, or a NULL file when that neighbour lies off the grid. Returns 1
 * for a file, 0 for none. chunkIndex is written as a whole word (see
 * ChunkLoadEntry). MATCHING: one `step` local carries every addend. */
s32 StageMap__ComputeChunkLoadEntry(StageMap *self, ChunkLoadEntry *out, s32 columns, s32 oddRow,
                                    s32 centreChunk, s32 onGridMask, s32 neighbour) {
    s32 bit = sNeighbourBits[neighbour];
    s32 result;

    if ((onGridMask & bit) == 0) {
        result = 0;
        goto nullCase;
    }

    if (self->config->isVertical == 0) {
        const ChunkNeighbourDelta *delta = &sChunkNeighbourDeltas[neighbour];
        s32 chunk;
        s32 step;

        if (delta->rowDelta == 0) {
            step = delta->colDeltaOddRow;
        } else {
            step = columns * delta->rowDelta;
            if (oddRow != 0) {
                step += delta->colDeltaOddRow;
            } else {
                step += delta->colDeltaEvenRow;
            }
        }
        chunk = centreChunk + step;
        *(s32 *)((u8 *)out + 4) = chunk;
    } else {
        *(s32 *)((u8 *)out + 4) = centreChunk + neighbour;
    }

    out->file = self->chunkFileFn(self->chunkFileCtx, *(s32 *)((u8 *)out + 4), 0, 0);
    do { /* MATCHING: removing it drifts the image */
    } while (0);
    result = 1;
    goto storeKey;

nullCase:
    out->file = NULL;

storeKey:
    out->neighbour = neighbour;
    return result;
}

/* Starts each entry's load in the slot holding its neighbour key (after
 * clearing the cells of a chunk already linked there), or cancels the slot's
 * load for a NULL file; then counts the slots left pending.
 * MATCHING: `tail` is taken from `entry` inside the loop and `entry` itself
 * advances; a copy of the parameter swaps two saved registers. */
void StageMap__ApplyChunkLoads(StageMap *self, ChunkLoadEntry *entry, s32 count) {
    s32 i;
    ChunkSlot *slot;
    ChunkLoadEntryTail *tail;

    for (i = 0; i < count; i++) {
        tail = (ChunkLoadEntryTail *)&entry->chunkIndex;
        slot = self->methods->findSlotByNeighbour(self, tail->neighbour);
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);
        if (entry->file != 0) {
            if (slot->loader->headerReady != 0) {
                self->methods->clearSlotCells(self, slot);
            }
            slot->loader->chunkIndex = tail->chunkIndex;
            ((LbdFileLoadHeaderFn)slot->loader->methods->processBuffer)(slot->loader, entry->file);
            slot->loadPending = 1;
            self->loadsPending = 1;
        } else {
            if (slot->loader->headerReady != 0) {
                self->methods->clearSlotCells(self, slot);
            }
            if (slot->loader->loadState != 0) {
                slot->loader->methods->cancelRequests(slot->loader);
                slot->loadPending = 0;
            }
        }
        entry++;
    }
    self->pendingLoadCount = StageMap__CountPendingLoads(self);
}

s32 StageMap__CountPendingLoads(StageMap *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        if (self->slots[i].loadPending != 0) {
            count++;
        }
    }
    return count;
}

void StageMap__OnNotifyTag1(StageMap *self, void *sender, s32 command) {
    s32 i;
    ChunkSlot *slot;
    s32 pending;

    if (command != DRAWSYSTEM_EVENT_VSYNC) {
        return;
    }
    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->dataReady != 0) {
            slot->loader->dataReady = 0;
            ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(
                self, STAGEMAP_EVENT_SLOT_DATA_READY, slot, i);
        }
        pending = self->loadsPending;
        if (pending == 1 && slot->loadPending != 0) {
            if (slot->loader->headerReady != 0) {
                self->methods->populateSlotCells(self, slot);
                slot->loader->headerReady = LBDFILE_HEADER_CONSUMED;
                slot->loadPending = 0;
                if (--self->pendingLoadCount == 0) {
                    self->pendingLoadCount = 0;
                    self->loadsPending = 0;
                    self->chunksLoaded = pending;
                }
            } else if (slot->loader->loadState == 0) {
                slot->loadPending = 0;
            }
        }
    }
}

/* Links a loaded chunk into its slot: points the slot's PlacementGrid at the
 * header's placement records, replaces its LinkResource with one over the
 * header's model block, then resolves record after record until the grid
 * returns 0. A record with no model (-1) hides its lattice cell; one with a
 * model links it into the next lattice cell (a chained record: the next
 * overflow cell), sets the cell's position, y rotation and flags, and hides
 * it until the drawn window shows it. While a record has `next` set, the
 * cell's nextInCell is the overflow cell the following record takes.
 * MATCHING: `header` and `header2` are two locals (one changes the
 * allocation), and the cells are walked by byte offset (an index changes the
 * code). */
void StageMap__PopulateSlotCells(StageMap *self, ChunkSlot *slot) {
    LbdFileHeader *header;
    LbdFileHeader *header2;
    LbdFile *loader;
    PlacementGrid *grid;
    LinkResource *oldResource;
    GridCell **cell;
    u8 *cells;
    SceneNodeSub14 *coord;
    SceneNodeSub44 *param;
    s32 x;
    s32 y;
    s32 z;
    s32 rotY;
    s32 model;
    s32 hidden;
    s32 i;
    s32 cellOff;
    s32 overflowOff;
    CellPlacement rec;
    ResourceSourceRequest req;

    loader = slot->loader;
    grid = slot->placements;
    header = loader->buffer;
    grid->buffer = (u8 *)header + header->placementsOffset;
    grid->bufferSize = 0;
    oldResource = grid->linkResource;
    if (oldResource != 0) {
        oldResource->methods->release(oldResource);
    }
    header2 = loader->buffer;
    req.src.buffer = (u8 *)header2 + header2->placementsOffset + header2->placementsSize;
    grid->linkResource = New_LinkResource(&req.src);
    rec.next = 0;

    i = 0;
    hidden = GsDOFF;
    cellOff = 0;
    overflowOff = STAGE_SLOT_LATTICE_CELLS * sizeof(GridCell *);
    for (;;) {
        model = ((PlacementGridResolveEntryFn)grid->methods->processBuffer)(grid, &rec, i);
        if (model == 0) {
            return;
        }
        if (model == -1) {
            s32 attr;
            cell = (GridCell **)((u8 *)slot->cells + cellOff);
            attr = (*cell)->attribute;
            (*cell)->attribute = attr | hidden;
            (*cell)->model = 0;
            (*cell)->tmd = 0;
        } else {
            cells = (u8 *)slot->cells;
            if (rec.chained != 0) {
                cell = (GridCell **)(cells + overflowOff);
                overflowOff += sizeof(GridCell *);
            } else {
                cell = (GridCell **)(cells + cellOff);
            }
            (*cell)->model = (void *)model;
            (*cell)->tmd = (s32)((TmdModel *)(*cell)->model)->object;
            GsLinkObject4((u_long)((TmdModel *)(*cell)->model)->object,
                          (GsDOBJ2 *)&(*cell)->attribute, 0);
            coord = (*cell)->coord2;
            /* MATCHING: keeps the rec.x/.y/.z loads below the coord2 load */
            __asm__("");
            x = rec.x;
            y = rec.y;
            z = rec.z;
            coord->tx = x;
            coord->ty = y;
            coord->tz = z;
            param = (*cell)->coord2->param;
            param->rotate.x = 0;
            rotY = rec.rotY;
            param->rotate.z = 0;
            param->rotate.y = rotY;
            (*cell)->flags36 = rec.cellFlags;
            (*cell)->coord2->flg = 0;
            {
                s32 attr = (*cell)->attribute;
                (*cell)->attribute = attr | hidden;
            }
        }
        if (rec.next) {
            GridCell **overflow = (GridCell **)((u8 *)slot->cells + overflowOff);
            (*cell)->nextInCell = *overflow;
            continue;
        }
        cellOff += sizeof(GridCell *);
        (*cell)->nextInCell = 0;
        i++;
    }
}

void StageMap__ClearSlotCells(StageMap *self, ChunkSlot *slot) {
    GridCell **p;
    GridCell **end;

    if (slot->loader->chunkIndex >= 0) {
        ((LbdFileReleaseHeaderElemFn)slot->loader->methods->releaseHeader)(slot->loader, slot);
        end = slot->cells + STAGE_SLOT_CELLS;
        for (p = slot->cells; p < end; p++) {
            (*p)->attribute |= GsDOFF;
            (*p)->model = 0;
            (*p)->tmd = 0;
        }
    }
}

Descriptor10 *StageMap__GetTargetDescriptor(StageMap *self, Descriptor10Ext *desc, void **outPos) {
    void *pos;

    pos = &self->target->coord2->tx;
    if (outPos != 0) {
        *outPos = pos;
    }
    if (desc != 0) {
        if (self->methods->computeFootprintDescriptor(self, desc, pos) != 0) {
            return 0;
        }
    }
    return &self->targetCell.base;
}

/* The descriptor of world position `pos`: the slot holding it, that slot's
 * chunk index and column/row, the chunk's centre, the position relative to
 * it, and the cell column/row and the offset from the cell's centre. Returns
 * 0, or 1 when no slot holds the position.
 * MATCHING: cellCol/cellRow re-read the stored bytes, the half cell sits
 * inside the subtracted group, and `out->slot` is stored last. */
s32 StageMap__ComputeFootprintDescriptor(StageMap *self, Descriptor10Ext *out, SplitLongVec3 *pos) {
    ChunkSlot *slot;
    SplitCoord2 *chunkOrigin;
    SplitCoord2 *origin;
    s32 chunkIndex;
    s32 rel;
    s32 cellCol;
    s32 cellRow;

    slot = self->methods->findSlotForPosition(self, (LongVec3 *)pos);
    if (slot != 0) {
        chunkIndex = slot->loader->chunkIndex;
        out->chunkIndex = chunkIndex;
        StageMap__SplitChunkIndex(self, (u8 *)out, chunkIndex);

        chunkOrigin = (SplitCoord2 *)self->methods->findSlotByNeighbour(self, slot->loader->elemKey)
                          ->cellParent->coord2;
        out->chunkCentre.x = chunkOrigin->tx.w + STAGE_CHUNK_SIZE / 2;
        out->chunkCentre.y = chunkOrigin->ty;
        out->chunkCentre.z = chunkOrigin->tz.w + STAGE_CHUNK_SIZE / 2;

        origin = (SplitCoord2 *)slot->cellParent->coord2;
        out->relPos.x = pos->x.w - out->chunkCentre.x;
        out->relPos.y = pos->y.w;
        out->relPos.z = pos->z.w - out->chunkCentre.z;

        rel = pos->x.w - origin->tx.w;
        if (rel < 0) {
            rel += STAGE_CELL_SIZE - 1;
        }
        out->base.b2 = rel >> STAGE_CELL_SHIFT;

        rel = pos->z.w - origin->tz.w;
        if (rel < 0) {
            rel += STAGE_CELL_SIZE - 1;
        }
        out->base.b3 = rel >> STAGE_CELL_SHIFT;

        cellCol = out->base.b2;
        out->base.h4 = pos->x.h - (origin->tx.h + (cellCol << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2);
        out->base.h6 = pos->y.h;
        cellRow = out->base.b3;
        out->base.h8 = pos->z.h - (origin->tz.h + (cellRow << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2);
        out->slot = slot;

        return 0;
    }
    return 1;
}

void StageMap__SplitChunkIndex(StageMap *self, u8 *out, s32 chunkIndex) {
    out[0] = chunkIndex % self->config->columns;
    out[1] = chunkIndex / self->config->columns;
}

ChunkSlot *StageMap__GetLastEventSlotChunk(StageMap *self, u8 *out) {
    StageMap__SplitChunkIndex(self, out, self->lastEventSlot->loader->chunkIndex);
    return self->lastEventSlot;
}

ChunkSlot *StageMap__FindSlotByNeighbour(StageMap *self, s32 neighbour) {
    s32 i;
    ChunkSlot *slot;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->elemKey == neighbour) {
            return slot;
        }
    }
}

/* The slot whose chunk holds `pos` in x and z (in a vertical grid, also
 * the layer holding y), or NULL.
 * MATCHING: `edge` is assigned inside each upper-bound test. */
ChunkSlot *StageMap__FindSlotForPosition(StageMap *self, LongVec3 *pos) {
    s32 i;
    s32 span;
    s32 layerTop;
    ChunkSlot *slot;
    SplitCoord2 *origin;
    s32 edge;

    i = 0;
    span = STAGE_CHUNK_SIZE;
    layerTop = 0;
    for (; i < CHUNK_NEIGHBOUR_COUNT; i++, layerTop -= VERTICAL_LAYER_HEIGHT) {
        slot = self->methods->findSlotByNeighbour(self, i);
        origin = (SplitCoord2 *)slot->cellParent->coord2;
        if (pos->x >= origin->tx.w && pos->x < (edge = origin->tx.w) + span) {
            if (pos->z >= origin->tz.w && pos->z < (edge = origin->tz.w) + span) {
                if (self->config->isVertical == 0) {
                    return slot;
                }
                if (layerTop >= pos->y) {
                    if (layerTop - VERTICAL_LAYER_HEIGHT >= pos->y) {
                        continue;
                    }
                    return slot;
                }
            }
        }
    }
    return 0;
}
