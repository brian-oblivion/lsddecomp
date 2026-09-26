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
#include "class_3bb8c.h"
#include "PlacementGrid.h"
#include "LinkResource.h"
#include "LbdFile.h"
#include "GridCell.h"

s32 StageMap__SetTargetAndLoadChunks(StageMap *self, void *arg1, SceneNode *arg2, Descriptor10 *arg3) {
    s32 stackBuf[3];
    s32 ret;

    self->target = arg2;
    self->targetCell.base = *arg3;
    ret = ComputeCellWorldOffsets(arg1, stackBuf, self->config, &self->origin, arg3);
    return self->methods->loadChunksAround(self, ret, (LongVec3 *)stackBuf, sDefaultTargetSpecs);
}

s32 StageMap__ComputeCellOffsets(StageMap *self, void *arg1, void *arg2) {
    s32 outBuf[3];

    return ComputeCellWorldOffsets(arg1, outBuf, self->config, &self->origin, arg2);
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
s32 ComputeCellWorldOffsets(s32 *arg0, s32 *outBuf, StageGridDimensions *arg2, LongVec3 *arg3,
                            Descriptor10 *arg4) {
    s32 idx;
    s32 factor;
    s32 sum;
    s32 v1;
    s32 a0v;
    s32 off;

    if (arg2->isVertical == 0) {
        idx = arg4->b1;
        factor = arg2->rows;
        sum = arg4->b0 + arg2->columns * idx;
    } else {
        factor = 1;
        idx = 0;
        sum = 0;
    }
    v1 = (arg3->x - arg2->columns * 0x5000) + arg4->b0 * 0xA000;
    a0v = arg3->z - factor * 0x5000;
    outBuf[0] = v1;
    if (idx & 1) {
        outBuf[0] = v1 - 0x5000;
    }
    outBuf[1] = arg3->y;
    off = 0x400;
    outBuf[2] = a0v + idx * 0xA000;
    arg0[0] = (arg4->b2 << 11) + outBuf[0] + (arg4->h4 + off);
    arg0[1] = arg4->h6 + outBuf[1];
    arg0[2] = (arg4->b3 << 11) + outBuf[2] + (arg4->h8 + off);
    outBuf[0] += 0x5000;
    outBuf[2] = outBuf[2] + 0x5000;
    return sum;
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
    Descriptor10Ext buf;
    ChunkSlot *e;
    s32 key;
    s32 result;
    u16 oldRaw;

    if (self->methods->getTargetDescriptor(self, &buf, 0) == 0) {
        return 0;
    }

    e = buf.slot;
    key = e->loader->elemKey;
    result = sFootprintResultRemap[key];

    if (self->config->isVertical == 0) {
        self->methods->loadChunksAround(self, buf.chunkIndex, &buf.chunkCentre,
                                        sFootprintResultPtrTable[result]);
    }

    self->methods->refreshFootprint(self);

    oldRaw = *(u16 *)&self->targetCell;
    self->targetCell = buf;

    if ((s16)oldRaw != *(s16 *)&buf) {
        self->methods->notifyParents(self, 5);
    }

    return result;
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
void StageMap__LoadChunksAround(StageMap *self, s32 val, LongVec3 *arg2, ChunkSlotSpec *arg3) {
    s32 divisor;
    s32 flag;
    s32 savedResult;
    s32 count;
    s32 i;
    ChunkSlot *e;
    SplitCoord2 *u14;
    LongVec3 *tbl;
    ChunkLoadEntry stackBuf[7];

    if (arg3 != 0) {
        divisor = self->config->columns;
        flag = (val / divisor) & 1;
        savedResult = StageMap__ComputeNeighbourMask(self, val, flag);

        count = 0;
        for (i = 0; i < 7; i++) {
            e = self->methods->findSlotByNeighbour(self, i);
            e->neighbour = arg3[i].neighbour;
            if (arg3[i].load != 0) {
                tbl = &sNeighbourOffsets[arg3[i].neighbour];
                u14 = (SplitCoord2 *)e->cellParent->coord2;
                if (self->config->isVertical == 0) {
                    u14->unk18.w = arg2->x + tbl->x;
                    u14->unk1C = arg2->y;
                    u14->unk20.w = arg2->z + tbl->z;
                } else {
                    u14->unk18.w = arg2->x - 0x5000;
                    u14->unk1C = arg2->y + tbl->y;
                    u14->unk20.w = arg2->z - 0x5000;
                }
                e->cellParent->coord2->flg = 0;
                StageMap__ComputeChunkLoadEntry(self, &stackBuf[count], divisor, flag, val,
                                                savedResult, arg3[i].neighbour);
                count++;
            }
        }

        for (i = 0; i < 7; i++) {
            e = &self->slots[i];
            e->loader->elemKey = e->neighbour;
        }

        self->methods->applyChunkLoads(self, stackBuf, count);
    }
}

s32 StageMap__ComputeNeighbourMask(StageMap *self, s32 val, s32 flag) {
    StageGridDimensions *u;
    s32 divisor;
    s32 unk4;
    s32 count;
    s32 flags;
    s32 i;

    u = self->config;
    divisor = u->columns;
    unk4 = u->isVertical;
    count = u->rows;
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
s32 StageMap__ComputeChunkLoadEntry(StageMap *self, ChunkLoadEntry *arg1, s32 divisor, s32 flag,
                                    s32 val, s32 savedResult, s32 key) {
    s32 mask = sNeighbourBits[key];
    s32 result;

    if ((savedResult & mask) == 0) {
        result = 0;
        goto nullCase;
    }

    if (self->config->isVertical == 0) {
        const ChunkNeighbourDelta *entry = &sChunkNeighbourDeltas[key];
        s32 value;
        s32 sum;

        if (entry->rowDelta == 0) {
            sum = entry->colDeltaOddRow;
        } else {
            sum = divisor * entry->rowDelta;
            if (flag != 0) {
                sum += entry->colDeltaOddRow;
            } else {
                sum += entry->colDeltaEvenRow;
            }
        }
        value = val + sum;
        *(s32 *)((u8 *)arg1 + 4) = value;
    } else {
        *(s32 *)((u8 *)arg1 + 4) = val + key;
    }

    arg1->file = self->chunkFileFn(self->chunkFileCtx, *(s32 *)((u8 *)arg1 + 4), 0, 0);
    do {
    } while (0);
    result = 1;
    goto storeKey;

nullCase:
    arg1->file = NULL;

storeKey:
    arg1->neighbour = key;
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
void StageMap__ApplyChunkLoads(StageMap *self, ChunkLoadEntry *arr1, s32 count) {
    s32 i;
    ChunkSlot *e;
    ChunkLoadEntryTail *sp;

    for (i = 0; i < count; i++) {
        sp = (ChunkLoadEntryTail *)&arr1->chunkIndex;
        e = self->methods->findSlotByNeighbour(self, sp->neighbour);
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, 6, e, i);
        if (arr1->file != 0) {
            if (e->loader->headerReady != 0) {
                self->methods->clearSlotCells(self, e);
            }
            e->loader->chunkIndex = sp->chunkIndex;
            ((LbdFileLoadHeaderFn)e->loader->methods->processBuffer)(e->loader, arr1->file);
            e->loadPending = 1;
            self->loadsPending = 1;
        } else {
            if (e->loader->headerReady != 0) {
                self->methods->clearSlotCells(self, e);
            }
            if (e->loader->loadState != 0) {
                e->loader->methods->cancelRequests(e->loader);
                e->loadPending = 0;
            }
        }
        arr1++;
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

void StageMap__OnNotifyTag1(StageMap *self, void *arg1, s32 mode) {
    s32 i;
    ChunkSlot *e;
    s32 curMode;

    if (mode != 2) {
        return;
    }
    for (i = 0; i < 7; i++) {
        e = &self->slots[i];
        if (e->loader->dataReady != 0) {
            e->loader->dataReady = 0;
            ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, 7, e, i);
        }
        curMode = self->loadsPending;
        if (curMode == 1 && e->loadPending != 0) {
            if (e->loader->headerReady != 0) {
                self->methods->populateSlotCells(self, e);
                e->loader->headerReady = 2;
                e->loadPending = 0;
                if (--self->pendingLoadCount == 0) {
                    self->pendingLoadCount = 0;
                    self->loadsPending = 0;
                    self->chunksLoaded = curMode;
                }
            } else if (e->loader->loadState == 0) {
                e->loadPending = 0;
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

/* PlacementGrid__ResolveEntry's non-0/non-(-1) return value (what
 * LinkResource__GetModel returns): a TmdModel (include/TmdModel.h), read
 * only for its +0x010, TmdModel's `object`. A view of TmdModel, left for
 * that class (round 89, LinkResource's unification did not retype it). */
typedef struct LinkResEntry {
    u8 pad0[0x10];
    s32 unk10; /* +0x010 */
} LinkResEntry;

typedef struct BE54LoadReq {
    s32 field0;
    u8 pad4[0xC];
} BE54LoadReq;

extern void GsLinkObject4(s32 tmd, void *objp, s32 n);

void StageMap__PopulateSlotCells(StageMap *self, ChunkSlot *entry) {
    LbdFileHeader *info;
    LbdFileHeader *info2;
    LbdFile *hdr;
    PlacementGrid *target;
    LinkResource *res;
    GridCell **slot;
    u8 *base;
    SceneNodeSub14 *gpu;
    SceneNodeSub44 *vec;
    s32 b;
    s32 c;
    s32 d;
    s32 h1;
    s32 idxVal;
    s32 flagBit;
    s32 i;
    s32 off1;
    s32 off2;
    CellPlacement outBuf;
    BE54LoadReq req;

    hdr = entry->loader;
    target = entry->placements;
    info = hdr->buffer;
    target->buffer = (u8 *)info + info->placementsOffset;
    target->bufferSize = 0;
    res = target->linkResource;
    if (res != 0) {
        res->methods->release(res);
    }
    info2 = hdr->buffer;
    req.field0 = (s32)info2 + info2->placementsOffset + info2->placementsSize;
    target->linkResource = New_LinkResource((struct Src6F240 *)&req);
    outBuf.next = 0;

    i = 0;
    flagBit = 0x80000000;
    off1 = 0;
    off2 = 0x640;
    for (;;) {
        idxVal = ((PlacementGridResolveEntryFn)target->methods->processBuffer)(target, &outBuf, i);
        if (idxVal == 0) {
            return;
        }
        if (idxVal == -1) {
            s32 flags10a;
            slot = (GridCell **)((u8 *)entry->cells + off1);
            flags10a = (*slot)->attribute;
            (*slot)->attribute = flags10a | flagBit;
            (*slot)->model = 0;
            (*slot)->tmd = 0;
        } else {
            base = (u8 *)entry->cells;
            if (outBuf.chained != 0) {
                slot = (GridCell **)(base + off2);
                off2 += 4;
            } else {
                slot = (GridCell **)(base + off1);
            }
            (*slot)->model = (void *)idxVal;
            (*slot)->tmd = ((LinkResEntry *)(*slot)->model)->unk10;
            GsLinkObject4(((LinkResEntry *)(*slot)->model)->unk10, &(*slot)->attribute, 0);
            gpu = (*slot)->coord2;
            /* Keeps the outBuf.x/.y/.z stack loads below the load of
             * (*slot)->coord2; without it GCC hoists all three above it. */
            __asm__("");
            b = outBuf.x;
            c = outBuf.y;
            d = outBuf.z;
            gpu->tx = b;
            gpu->ty = c;
            gpu->tz = d;
            vec = (*slot)->coord2->param;
            vec->rotate.x = 0;
            h1 = outBuf.rotY;
            vec->rotate.z = 0;
            vec->rotate.y = h1;
            (*slot)->flags36 = outBuf.cellFlags;
            (*slot)->coord2->flg = 0;
            {
                s32 flags10 = (*slot)->attribute;
                (*slot)->attribute = flags10 | flagBit;
            }
        }
        if (outBuf.next) {
            GridCell **next = (GridCell **)((u8 *)entry->cells + off2);
            (*slot)->nextInCell = *next;
            continue;
        }
        off1 += 4;
        (*slot)->nextInCell = 0;
        i++;
    }
}

void StageMap__ClearSlotCells(StageMap *self, ChunkSlot *entry) {
    GridCell **p;
    GridCell **end;

    if (entry->loader->chunkIndex >= 0) {
        ((LbdFileReleaseHeaderElemFn)entry->loader->methods->releaseHeader)(entry->loader, entry);
        end = entry->cells + 0x19A;
        for (p = entry->cells; p < end; p++) {
            (*p)->attribute |= 0x80000000;
            (*p)->model = 0;
            (*p)->tmd = 0;
        }
    }
}

Descriptor10 *StageMap__GetTargetDescriptor(StageMap *self, Descriptor10Ext *arg1, void **out) {
    void *v1;

    v1 = &self->target->coord2->tx;
    if (out != 0) {
        *out = v1;
    }
    if (arg1 != 0) {
        if (self->methods->computeFootprintDescriptor(self, arg1, v1) != 0) {
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
s32 StageMap__ComputeFootprintDescriptor(StageMap *self, Descriptor10Ext *out, SplitLongVec3 *in) {
    ChunkSlot *e;
    SplitCoord2 *u14a;
    SplitCoord2 *u14b;
    s32 rate;
    s32 t;
    s32 b2;
    s32 b3;

    e = self->methods->findSlotForPosition(self, (LongVec3 *)in);
    if (e != 0) {
        rate = e->loader->chunkIndex;
        out->chunkIndex = rate;
        StageMap__SplitChunkIndex(self, (u8 *)out, rate);

        u14a = (SplitCoord2 *)self->methods->findSlotByNeighbour(self, e->loader->elemKey)->cellParent->coord2;
        out->chunkCentre.x = u14a->unk18.w + 0x5000;
        out->chunkCentre.y = u14a->unk1C;
        out->chunkCentre.z = u14a->unk20.w + 0x5000;

        u14b = (SplitCoord2 *)e->cellParent->coord2;
        out->relPos.x = in->x.w - out->chunkCentre.x;
        out->relPos.y = in->y.w;
        out->relPos.z = in->z.w - out->chunkCentre.z;

        t = in->x.w - u14b->unk18.w;
        if (t < 0) {
            t += 0x7FF;
        }
        out->base.b2 = t >> 11;

        t = in->z.w - u14b->unk20.w;
        if (t < 0) {
            t += 0x7FF;
        }
        out->base.b3 = t >> 11;

        b2 = out->base.b2;
        out->base.h4 = in->x.h - (u14b->unk18.h + (b2 << 11) + 0x400);
        out->base.h6 = in->y.h;
        b3 = out->base.b3;
        out->base.h8 = in->z.h - (u14b->unk20.h + (b3 << 11) + 0x400);
        out->slot = e;

        return 0;
    }
    return 1;
}

void StageMap__SplitChunkIndex(StageMap *self, u8 *out, s32 val) {
    out[0] = val % self->config->columns;
    out[1] = val / self->config->columns;
}

ChunkSlot *StageMap__GetLastEventSlotChunk(StageMap *self, u8 *out) {
    StageMap__SplitChunkIndex(self, out, self->lastEventSlot->loader->chunkIndex);
    return self->lastEventSlot;
}

ChunkSlot *StageMap__FindSlotByNeighbour(StageMap *self, s32 key) {
    s32 i;
    ChunkSlot *e;

    for (i = 0; i < 7; i++) {
        e = &self->slots[i];
        if (e->loader->elemKey == key) {
            return e;
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
ChunkSlot *StageMap__FindSlotForPosition(StageMap *self, LongVec3 *arg1) {
    s32 i;
    s32 tol;
    s32 threshold;
    ChunkSlot *candidate;
    SplitCoord2 *r;
    s32 w;

    i = 0;
    tol = 0xA000;
    threshold = 0;
    for (; i < 7; i++, threshold -= 0x800) {
        candidate = self->methods->findSlotByNeighbour(self, i);
        r = (SplitCoord2 *)candidate->cellParent->coord2;
        if (arg1->x >= r->unk18.w && arg1->x < (w = r->unk18.w) + tol) {
            if (arg1->z >= r->unk20.w && arg1->z < (w = r->unk20.w) + tol) {
                if (self->config->isVertical == 0) {
                    return candidate;
                }
                if (threshold >= arg1->y) {
                    if (threshold - 0x800 >= arg1->y) {
                        continue;
                    }
                    return candidate;
                }
            }
        }
    }
    return 0;
}
