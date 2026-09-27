/*
 * class_3bb8c -- the middle third of StageMap (include/StageMap.h): chunk
 * loading and the position-to-cell math. class_3bb8c_b.c holds the rest;
 * both units share include/class_3bb8c.h, which holds the class's data
 * tables.
 *
 *  - SetTargetAndLoadChunks, ComputeCellOffsets, ComputeCellWorldOffsets:
 *    a cell descriptor to a world position and the chunk it lies in.
 *  - Enable/Disable, UpdateFootprintTracking: the per-tick tracking of the
 *    target (see the class banner).
 *  - LoadChunksAround, ComputeNeighbourMask, ComputeChunkLoadEntry,
 *    ApplyChunkLoads, CountPendingLoads, OnNotifyTag1: loading the chunks
 *    around a centre chunk into the slots, and finishing each load.
 *  - PopulateSlotCells / ClearSlotCells: linking a loaded chunk's
 *    placements and models into its slot's cells, and clearing them.
 *  - GetTargetDescriptor, ComputeFootprintDescriptor, SplitChunkIndex,
 *    GetLastEventSlotChunk, FindSlotByNeighbour, FindSlotForPosition:
 *    queries. A slot's position is read through SplitCoord2
 *    (include/class_3bb8c.h), a GsCOORDINATE2 view with halfword reads.
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

/* MATCH, round 40 (bravo): permuter-found zero, first-ever search on this
 * function (1838 iterations, rc=0). The lead: hoist the shared `0x400`
 * constant used by BOTH `arg0[0]`/`arg0[2]`'s tail addend into a named
 * local, declared between the `outBuf[1]` and `outBuf[2]` assignment
 * statements -- the exact position retail's own constant-load sits,
 * confirmed by the score dropping straight to 0. Every prior round's
 * attempts targeted the outBuf[0]/outBuf[2] STORE-vs-LOAD scheduling
 * directly and never touched this constant; the permuter found a
 * completely different axis. See docs/match-reports/ComputeCellWorldOffsets.md. */
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
    x = (origin->x - dims->columns * 0x5000) + cell->b0 * 0xA000;
    z = origin->z - rowSpan * 0x5000;
    chunkPos[0] = x;
    if (row & 1) {
        chunkPos[0] = x - 0x5000;
    }
    chunkPos[1] = origin->y;
    halfCell = 0x400;
    chunkPos[2] = z + row * 0xA000;
    outPos[0] = (cell->b2 << 11) + chunkPos[0] + (cell->h4 + halfCell);
    outPos[1] = cell->h6 + chunkPos[1];
    outPos[2] = (cell->b3 << 11) + chunkPos[2] + (cell->h8 + halfCell);
    chunkPos[0] += 0x5000;
    chunkPos[2] = chunkPos[2] + 0x5000;
    return chunkIndex;
}

void StageMap__Enable(StageMap *self) {
    self->enabled = 1;
}

void StageMap__Disable(StageMap *self) {
    self->methods->unloadAllSlots(self);
    self->enabled = 0;
}

/* StageMap__UpdateFootprintTracking -- see docs/match-reports/StageMap__UpdateFootprintTracking.md. */
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
        self->methods->notifyParents(self, 5);
    }

    return specIndex;
}

/* MATCH, round 63 (delta): closed a 137/140 stall that had stood since round
 * 40 across four re-verifications, ten inert structural variants and a
 * 37,155-iteration permuter search -- see docs/match-reports/StageMap__LoadChunksAround.md.
 * The 3-word residue was a genuine pure register-identity difference (funcdiff
 * ins 0 / del 0, no asm-differ markers): retail held the second loop's element
 * pointer in $a2, the build in $v0. The fix was to DELETE a local -- the
 * second loop reuses `e`, the same variable the first loop walks, instead of a
 * separate `e2`. Nothing else in the body changed.
 * That axis is exactly the one a permuter cannot reach: it mutates a body, it
 * does not merge two of its locals into one. Same lever as StageMap__ComputeChunkLoadEntry this
 * round.
 * The `__asm__("")` barrier this body used to carry before `u14 = ...` is gone:
 * with `e` merged it is no longer needed, verified by whole-image rebuild. */
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
    ChunkLoadEntry loads[7];

    if (specs != 0) {
        columns = self->config->columns;
        oddRow = (centreChunk / columns) & 1;
        onGridMask = StageMap__ComputeNeighbourMask(self, centreChunk, oddRow);

        count = 0;
        for (i = 0; i < 7; i++) {
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
                    origin->tx.w = centrePos->x - 0x5000;
                    origin->ty = centrePos->y + offset->y;
                    origin->tz.w = centrePos->z - 0x5000;
                }
                slot->cellParent->coord2->flg = 0;
                StageMap__ComputeChunkLoadEntry(self, &loads[count], columns, oddRow, centreChunk,
                                                onGridMask, specs[i].neighbour);
                count++;
            }
        }

        for (i = 0; i < 7; i++) {
            slot = &self->slots[i];
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
        offGrid = (chunk < columns) ? 3 : 0;
        if (chunk >= columns * (rows - 1)) {
            offGrid |= 0x60;
        }
        if (chunk % columns == 0) {
            offGrid |= oddRow ? 0x25 : 4;
        }
        if ((chunk + 1) % columns != 0) {
            return ~offGrid;
        }
        offGrid |= oddRow ? 0x10 : 0x52;
        return ~offGrid;
    } else {
        offGrid = -1;
        for (i = 0; i < rows; i++) {
            offGrid <<= 1;
        }
        return ~offGrid;
    }
}

/* MATCH, round 63 (delta): closed a 58/63 stall that had stood since round
 * 27 across five re-verifications and ~330,000 permuter iterations -- see
 * docs/match-reports/StageMap__ComputeChunkLoadEntry.md. The 5-word residue really was pure
 * register identity (funcdiff ins 0 / del 0, no asm-differ markers), and the
 * fix was FEWER variables, not more: retail carries the multiply result AND
 * the running sum AND both branch addends in ONE local (`sum`, retail's
 * $v1), with the `val +` hoisted out of every branch into a single
 * `value = val + sum;` after the if/else (retail's $v0). Round 32 tried the
 * opposite -- splitting `fieldVal`/`sum` out of `value` -- and measured it
 * inert; the permuter then searched around that same split for 330k
 * iterations without ever reaching the merged shape.
 * The `do {} while (0);` below is LOAD-BEARING: removing it drifts the
 * image. It was inherited with the near-miss body and is verified here. */
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
    do {
    } while (0);
    result = 1;
    goto storeKey;

nullCase:
    out->file = NULL;

storeKey:
    out->neighbour = neighbour;
    return result;
}

/* MATCH, round 73 (bravo): 105/105. Retail's `+4` walker is a
 * strength-reduced giv of the walked PARAMETER, not a second user
 * pointer: its init (`addiu s3,a1,4`) sits in the loop preheader after
 * the count guard and reads $a1, which is what loop.c emits when the biv
 * is `arr1` itself (initial value = the incoming argument register).
 * `sp` is therefore assigned from `arr1` inside the body and `arr1` is
 * advanced directly; the old `ep = arr1` copy is what swapped s3/s4.
 * See docs/match-reports/StageMap__ApplyChunkLoads.md. */
void StageMap__ApplyChunkLoads(StageMap *self, ChunkLoadEntry *entry, s32 count) {
    s32 i;
    ChunkSlot *slot;
    ChunkLoadEntryTail *tail;

    for (i = 0; i < count; i++) {
        tail = (ChunkLoadEntryTail *)&entry->chunkIndex;
        slot = self->methods->findSlotByNeighbour(self, tail->neighbour);
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, 6, slot, i);
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
    for (i = 0; i < 7; i++) {
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

    if (command != 2) {
        return;
    }
    for (i = 0; i < 7; i++) {
        slot = &self->slots[i];
        if (slot->loader->dataReady != 0) {
            slot->loader->dataReady = 0;
            ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, 7, slot, i);
        }
        pending = self->loadsPending;
        if (pending == 1 && slot->loadPending != 0) {
            if (slot->loader->headerReady != 0) {
                self->methods->populateSlotCells(self, slot);
                slot->loader->headerReady = 2;
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

/* MATCH, round 73 (bravo): 150/150. The 142/150 residue carried since
 * round 40 (`info` in $a1 where retail has $v0) was ONE `info` local
 * assigned on both sides of the slot4 call. Two locals (`info`, `info2`)
 * make each block-local, so local-alloc ties each to its addu result.
 * See docs/match-reports/StageMap__PopulateSlotCells.md. */
/* StageMap__PopulateSlotCells (populateSlotCells, +0x104) -- own local view of the
 * records reached only from here. Kept in this .c, not class_3bb8c.h: none
 * of the 11 sibling units sharing that header touch these. */

typedef struct BE54LoadReq {
    void *buffer; /* +0x000, New_LinkResource's descriptor's buffer (code_33808.c's Src6F240) */
    u8 pad4[0xC];
} BE54LoadReq;

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
    BE54LoadReq src;

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
    src.buffer = (u8 *)header2 + header2->placementsOffset + header2->placementsSize;
    grid->linkResource = New_LinkResource((struct Src6F240 *)&src);
    rec.next = 0;

    i = 0;
    hidden = 0x80000000;
    cellOff = 0;
    overflowOff = 0x640;
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
                overflowOff += 4;
            } else {
                cell = (GridCell **)(cells + cellOff);
            }
            (*cell)->model = (void *)model;
            (*cell)->tmd = (s32)((TmdModel *)(*cell)->model)->object;
            GsLinkObject4((u_long)((TmdModel *)(*cell)->model)->object,
                          (GsDOBJ2 *)&(*cell)->attribute, 0);
            coord = (*cell)->coord2;
            /* Keeps the rec.x/.y/.z stack loads below the load of
             * (*cell)->coord2; without it GCC hoists all three above it. */
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
        cellOff += 4;
        (*cell)->nextInCell = 0;
        i++;
    }
}

void StageMap__ClearSlotCells(StageMap *self, ChunkSlot *slot) {
    GridCell **p;
    GridCell **end;

    if (slot->loader->chunkIndex >= 0) {
        ((LbdFileReleaseHeaderElemFn)slot->loader->methods->releaseHeader)(slot->loader, slot);
        end = slot->cells + 0x19A;
        for (p = slot->cells; p < end; p++) {
            (*p)->attribute |= 0x80000000;
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

/* MATCH, round 63 (delta): closed a six-round stall (72/106 since round 19)
 * with three source-shape corrections, none of them register pinning -- see
 * docs/match-reports/StageMap__ComputeFootprintDescriptor.md.
 *   1. `b2`/`b3` are s32 locals RE-READ from `out->base.b2`/`b3` after the
 *      byte stores. An s8 field shifted directly in the expression compiles
 *      to `lbu` + `sll 0x18` + `sra 0xd`; assigning it to an s32 local first
 *      folds the sign extension into retail's `lb` + `sll 0xb`.
 *   2. The 0x400 sits INSIDE the subtracted group -- `x - (y + (b<<11) +
 *      0x400)`. GCC reassociates that to retail's `addiu a0,a0,-0x400`.
 *      Writing `(x - 0x400) - (...)` instead narrows the constant to HImode
 *      and emits `li 0xfc00` + `addu`.
 *   3. `out->unk24 = e;` is the LAST statement of the block. Every earlier
 *      placement schedules its `sw` too early; only trailing it after the
 *      h8 store reproduces retail's order. */
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
        out->chunkCentre.x = chunkOrigin->tx.w + 0x5000;
        out->chunkCentre.y = chunkOrigin->ty;
        out->chunkCentre.z = chunkOrigin->tz.w + 0x5000;

        origin = (SplitCoord2 *)slot->cellParent->coord2;
        out->relPos.x = pos->x.w - out->chunkCentre.x;
        out->relPos.y = pos->y.w;
        out->relPos.z = pos->z.w - out->chunkCentre.z;

        rel = pos->x.w - origin->tx.w;
        if (rel < 0) {
            rel += 0x7FF;
        }
        out->base.b2 = rel >> 11;

        rel = pos->z.w - origin->tz.w;
        if (rel < 0) {
            rel += 0x7FF;
        }
        out->base.b3 = rel >> 11;

        cellCol = out->base.b2;
        out->base.h4 = pos->x.h - (origin->tx.h + (cellCol << 11) + 0x400);
        out->base.h6 = pos->y.h;
        cellRow = out->base.b3;
        out->base.h8 = pos->z.h - (origin->tz.h + (cellRow << 11) + 0x400);
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

    for (i = 0; i < 7; i++) {
        slot = &self->slots[i];
        if (slot->loader->elemKey == neighbour) {
            return slot;
        }
    }
}

/* MATCH, round 73 (bravo): 70/70. The last word was the operand order
 * of the second bounds `addu`. At expand time a MEM operand of a
 * commutative `+` is placed second whatever the source order, while a
 * named variable keeps its source position; retail's `field + tol` order
 * therefore needs the field in a named local at the add. Assigning `w`
 * INSIDE the upper-bound test keeps the `arg1` load ahead of the field
 * load, as retail schedules it (a `w = ...;` statement before the `if`
 * fixes the add but swaps those two loads). See
 * docs/match-reports/StageMap__FindSlotForPosition.md. */
ChunkSlot *StageMap__FindSlotForPosition(StageMap *self, LongVec3 *pos) {
    s32 i;
    s32 span;
    s32 layerTop;
    ChunkSlot *slot;
    SplitCoord2 *origin;
    s32 edge;

    i = 0;
    span = 0xA000;
    layerTop = 0;
    for (; i < 7; i++, layerTop -= 0x800) {
        slot = self->methods->findSlotByNeighbour(self, i);
        origin = (SplitCoord2 *)slot->cellParent->coord2;
        if (pos->x >= origin->tx.w && pos->x < (edge = origin->tx.w) + span) {
            if (pos->z >= origin->tz.w && pos->z < (edge = origin->tz.w) + span) {
                if (self->config->isVertical == 0) {
                    return slot;
                }
                if (layerTop >= pos->y) {
                    if (layerTop - 0x800 >= pos->y) {
                        continue;
                    }
                    return slot;
                }
            }
        }
    }
    return 0;
}
