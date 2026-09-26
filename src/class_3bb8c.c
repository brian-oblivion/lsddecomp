/* First slice of the 365-function class_3bb8c block -- 20 functions,
 * 0x3BB8C..0x3CD88, all matched C, occupants of `gClass866E8Methods` +0x0E4..+0x11C
 * (`tools/classtable.py 0x800866E8`). The remainder is `class_3bb8c_b` and
 * is still a monolithic asm segment.
 *
 * This slice is Class866E8's FOOTPRINT/RATE engine: the position-to-grid-cell
 * math and the per-element resource/GPU work that `class_3ac78`'s own unit
 * header (src/class_3ac78.c) describes as living in "class_3bb8c*" --
 * Class866E8__ComputeFootprintDescriptor converts a world position
 * (QueryPos866E8) into a grid-cell descriptor (Descriptor10, byte row/column
 * plus sub-cell halfword offsets); Class866E8__UpdateFootprintTracking runs
 * every enabled tick (paired with class_3ac78's Class866E8__AdvanceRateCountdown)
 * to refresh that descriptor and notify on change; Class866E8__BuildRateEntries
 * / Class866E8__ComputeRateFlags / Class866E8__ComputeRateEntry /
 * Class866E8__ApplyRateEntries build and apply a per-element rate table from
 * a TargetSpec866E8 key/flag array (sDefaultTargetSpecs); and
 * Class866E8__LoadElementResources / Class866E8__ResetElementCells own an
 * element's resource-load and GPU-link cell array (the same 0x668-byte grid
 * class_3ac78 calls out) and its teardown. Class866E8__Enable/Disable set
 * the `enabled` flag class_3ac78 gates all of this on (cross-confirmed
 * there independently, see docs/match-reports/Class866E8__Enable.md).
 *
 * The class is declared once, in include/Class866E8.h (track 4, round 89);
 * the element's origin is read through Unk14Obj (include/class_3bb8c.h), a
 * GsCOORDINATE2 view with halfword reads.
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
#include "PlacementGrid.h"
#include "LinkResource.h"
#include "LbdFile.h"
#include "GridCell.h"

s32 Class866E8__SetTargetAndBuildRates(Class866E8 *self, void *arg1, SceneNode *arg2, Descriptor10 *arg3) {
    s32 stackBuf[3];
    s32 ret;

    self->target = arg2;
    self->targetCell.base = *arg3;
    ret = ComputeCellWorldOffsets(arg1, stackBuf, self->config, &self->origin, arg3);
    return self->methods->buildRateEntries(self, ret, (Unk54Struct *)stackBuf, sDefaultTargetSpecs);
}

s32 Class866E8__ComputeCellOffsets(Class866E8 *self, void *arg1, void *arg2) {
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
s32 ComputeCellWorldOffsets(s32 *arg0, s32 *outBuf, Unk68Struct *arg2, Unk54Struct *arg3,
                            Descriptor10 *arg4) {
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

void Class866E8__Enable(Class866E8 *self) {
    self->enabled = 1;
}

void Class866E8__Disable(Class866E8 *self) {
    self->methods->resetAllElements(self);
    self->enabled = 0;
}

/* Class866E8__UpdateFootprintTracking -- see docs/match-reports/Class866E8__UpdateFootprintTracking.md. */
s32 Class866E8__UpdateFootprintTracking(Class866E8 *self) {
    Descriptor10Ext buf;
    Class866E8Elem *e;
    s32 key;
    s32 result;
    u16 oldRaw;

    if (self->methods->getTargetDescriptor(self, &buf, 0) == 0) {
        return 0;
    }

    e = buf.unk24;
    key = e->loader->elemKey;
    result = sFootprintResultRemap[key];

    if (self->config->unk4 == 0) {
        self->methods->buildRateEntries(self, buf.unk28, (Unk54Struct *)&buf.unkC,
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
 * 37,155-iteration permuter search -- see docs/match-reports/Class866E8__BuildRateEntries.md.
 * The 3-word residue was a genuine pure register-identity difference (funcdiff
 * ins 0 / del 0, no asm-differ markers): retail held the second loop's element
 * pointer in $a2, the build in $v0. The fix was to DELETE a local -- the
 * second loop reuses `e`, the same variable the first loop walks, instead of a
 * separate `e2`. Nothing else in the body changed.
 * That axis is exactly the one a permuter cannot reach: it mutates a body, it
 * does not merge two of its locals into one. Same lever as Class866E8__ComputeRateEntry this
 * round.
 * The `__asm__("")` barrier this body used to carry before `u14 = ...` is gone:
 * with `e` merged it is no longer needed, verified by whole-image rebuild. */
void Class866E8__BuildRateEntries(Class866E8 *self, s32 val, Unk54Struct *arg2, TargetSpec866E8 *arg3) {
    s32 divisor;
    s32 flag;
    s32 savedResult;
    s32 count;
    s32 i;
    Class866E8Elem *e;
    Unk14Obj *u14;
    Unk54Struct *tbl;
    SetupEntry866E8 stackBuf[7];

    if (arg3 != 0) {
        divisor = self->config->divisor;
        flag = (val / divisor) & 1;
        savedResult = Class866E8__ComputeRateFlags(self, val, flag);

        count = 0;
        for (i = 0; i < 7; i++) {
            e = self->methods->findElemByUnk32(self, i);
            e->key = arg3[i].key;
            if (arg3[i].flag != 0) {
                tbl = &sRateOffsetTable[arg3[i].key];
                u14 = (Unk14Obj *)e->cellParent->coord2;
                if (self->config->unk4 == 0) {
                    u14->unk18.w = arg2->unk0 + tbl->unk0;
                    u14->unk1C = arg2->unk4;
                    u14->unk20.w = arg2->unk8 + tbl->unk8;
                } else {
                    u14->unk18.w = arg2->unk0 - 0x5000;
                    u14->unk1C = arg2->unk4 + tbl->unk4;
                    u14->unk20.w = arg2->unk8 - 0x5000;
                }
                e->cellParent->coord2->flg = 0;
                Class866E8__ComputeRateEntry(self, &stackBuf[count], divisor, flag, val,
                                             savedResult, arg3[i].key);
                count++;
            }
        }

        for (i = 0; i < 7; i++) {
            e = &self->elems[i];
            e->loader->elemKey = e->key;
        }

        self->methods->applyRateEntries(self, stackBuf, count);
    }
}

s32 Class866E8__ComputeRateFlags(Class866E8 *self, s32 val, s32 flag) {
    Unk68Struct *u;
    s32 divisor;
    s32 unk4;
    s32 count;
    s32 flags;
    s32 i;

    u = self->config;
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

/* MATCH, round 63 (delta): closed a 58/63 stall that had stood since round
 * 27 across five re-verifications and ~330,000 permuter iterations -- see
 * docs/match-reports/Class866E8__ComputeRateEntry.md. The 5-word residue really was pure
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
s32 Class866E8__ComputeRateEntry(Class866E8 *self, SetupEntry866E8 *arg1, s32 divisor, s32 flag,
                                 s32 val, s32 savedResult, s32 key) {
    s32 mask = sRateKeyMask[key];
    s32 result;

    if ((savedResult & mask) == 0) {
        result = 0;
        goto nullCase;
    }

    if (self->config->unk4 == 0) {
        const Unk54Struct *entry = &sRateEntryTable[key];
        s32 value;
        s32 sum;

        if (entry->unk0 == 0) {
            sum = entry->unk4;
        } else {
            sum = divisor * entry->unk0;
            if (flag != 0) {
                sum += entry->unk4;
            } else {
                sum += entry->unk8;
            }
        }
        value = val + sum;
        *(s32 *)((u8 *)arg1 + 4) = value;
    } else {
        *(s32 *)((u8 *)arg1 + 4) = val + key;
    }

    arg1->ptr0 = self->valueFn(self->valueFnCtx, *(s32 *)((u8 *)arg1 + 4), 0, 0);
    do {
    } while (0);
    result = 1;
    goto storeKey;

nullCase:
    arg1->ptr0 = NULL;

storeKey:
    arg1->id = key;
    return result;
}

/* MATCH, round 73 (bravo): 105/105. Retail's `+4` walker is a
 * strength-reduced giv of the walked PARAMETER, not a second user
 * pointer: its init (`addiu s3,a1,4`) sits in the loop preheader after
 * the count guard and reads $a1, which is what loop.c emits when the biv
 * is `arr1` itself (initial value = the incoming argument register).
 * `sp` is therefore assigned from `arr1` inside the body and `arr1` is
 * advanced directly; the old `ep = arr1` copy is what swapped s3/s4.
 * See docs/match-reports/Class866E8__ApplyRateEntries.md. */
void Class866E8__ApplyRateEntries(Class866E8 *self, SetupEntry866E8 *arr1, s32 count) {
    s32 i;
    Class866E8Elem *e;
    SetupSub866E8 *sp;

    for (i = 0; i < count; i++) {
        sp = (SetupSub866E8 *)&arr1->rate;
        e = self->methods->findElemByUnk32(self, sp->id);
        ((Class866E8OnElementEventFn)self->methods->notifyWithHull)(self, 6, e, i);
        if (arr1->ptr0 != 0) {
            if (e->loader->headerReady != 0) {
                self->methods->resetElementCells(self, e);
            }
            e->loader->chunkIndex = sp->rate;
            ((LbdFileLoadHeaderFn)e->loader->methods->processBuffer)(e->loader, arr1->ptr0);
            e->flag = 1;
            self->unk1B0 = 1;
        } else {
            if (e->loader->headerReady != 0) {
                self->methods->resetElementCells(self, e);
            }
            if (e->loader->loadState != 0) {
                e->loader->methods->cancelRequests(e->loader);
                e->flag = 0;
            }
        }
        arr1++;
    }
    self->unk1B4 = Class866E8__CountFlaggedElements(self);
}

s32 Class866E8__CountFlaggedElements(Class866E8 *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < 7; i++) {
        if (self->elems[i].flag != 0) {
            count++;
        }
    }
    return count;
}

void Class866E8__OnNotifyTag1(Class866E8 *self, void *arg1, s32 mode) {
    s32 i;
    Class866E8Elem *e;
    s32 curMode;

    if (mode != 2) {
        return;
    }
    for (i = 0; i < 7; i++) {
        e = &self->elems[i];
        if (e->loader->dataReady != 0) {
            e->loader->dataReady = 0;
            ((Class866E8OnElementEventFn)self->methods->notifyWithHull)(self, 7, e, i);
        }
        curMode = self->unk1B0;
        if (curMode == 1 && e->flag != 0) {
            if (e->loader->headerReady != 0) {
                self->methods->loadElementResources(self, e);
                e->loader->headerReady = 2;
                e->flag = 0;
                if (--self->unk1B4 == 0) {
                    self->unk1B4 = 0;
                    self->unk1B0 = 0;
                    self->unk1B8 = curMode;
                }
            } else if (e->loader->loadState == 0) {
                e->flag = 0;
            }
        }
    }
}

/* MATCH, round 73 (bravo): 150/150. The 142/150 residue carried since
 * round 40 (`info` in $a1 where retail has $v0) was ONE `info` local
 * assigned on both sides of the slot4 call. Two locals (`info`, `info2`)
 * make each block-local, so local-alloc ties each to its addu result.
 * See docs/match-reports/Class866E8__LoadElementResources.md. */
/* Class866E8__LoadElementResources (loadElementResources, +0x104) -- own local view of the
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

void Class866E8__LoadElementResources(Class866E8 *self, Class866E8Elem *entry) {
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
            (*slot)->flags36 = outBuf.unk2E;
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

void Class866E8__ResetElementCells(Class866E8 *self, Class866E8Elem *entry) {
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

Descriptor10 *Class866E8__GetTargetDescriptor(Class866E8 *self, Descriptor10Ext *arg1, void **out) {
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
 * docs/match-reports/Class866E8__ComputeFootprintDescriptor.md.
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
s32 Class866E8__ComputeFootprintDescriptor(Class866E8 *self, Descriptor10Ext *out, QueryPos866E8 *in) {
    Class866E8Elem *e;
    Unk14Obj *u14a;
    Unk14Obj *u14b;
    s32 rate;
    s32 t;
    s32 b2;
    s32 b3;

    e = self->methods->findElementForPosition(self, (Unk54Struct *)in);
    if (e != 0) {
        rate = e->loader->chunkIndex;
        out->unk28 = rate;
        Class866E8__ComputeDivisorSplit(self, (u8 *)out, rate);

        u14a = (Unk14Obj *)self->methods->findElemByUnk32(self, e->loader->elemKey)->cellParent->coord2;
        out->unkC = u14a->unk18.w + 0x5000;
        out->unk10 = u14a->unk1C;
        out->unk14 = u14a->unk20.w + 0x5000;

        u14b = (Unk14Obj *)e->cellParent->coord2;
        out->unk18 = in->unk0.w - out->unkC;
        out->unk1C = in->unk4.w;
        out->unk20 = in->unk8.w - out->unk14;

        t = in->unk0.w - u14b->unk18.w;
        if (t < 0) {
            t += 0x7FF;
        }
        out->base.b2 = t >> 11;

        t = in->unk8.w - u14b->unk20.w;
        if (t < 0) {
            t += 0x7FF;
        }
        out->base.b3 = t >> 11;

        b2 = out->base.b2;
        out->base.h4 = in->unk0.h - (u14b->unk18.h + (b2 << 11) + 0x400);
        out->base.h6 = in->unk4.h;
        b3 = out->base.b3;
        out->base.h8 = in->unk8.h - (u14b->unk20.h + (b3 << 11) + 0x400);
        out->unk24 = e;

        return 0;
    }
    return 1;
}

void Class866E8__ComputeDivisorSplit(Class866E8 *self, u8 *out, s32 val) {
    out[0] = val % self->config->divisor;
    out[1] = val / self->config->divisor;
}

Class866E8Elem *Class866E8__GetLastTargetRateSplit(Class866E8 *self, u8 *out) {
    Class866E8__ComputeDivisorSplit(self, out, self->lastEventElem->loader->chunkIndex);
    return self->lastEventElem;
}

Class866E8Elem *Class866E8__FindElemByUnk32(Class866E8 *self, s32 key) {
    s32 i;
    Class866E8Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->elems[i];
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
 * docs/match-reports/Class866E8__FindElementForPosition.md. */
Class866E8Elem *Class866E8__FindElementForPosition(Class866E8 *self, Unk54Struct *arg1) {
    s32 i;
    s32 tol;
    s32 threshold;
    Class866E8Elem *candidate;
    Unk14Obj *r;
    s32 w;

    i = 0;
    tol = 0xA000;
    threshold = 0;
    for (; i < 7; i++, threshold -= 0x800) {
        candidate = self->methods->findElemByUnk32(self, i);
        r = (Unk14Obj *)candidate->cellParent->coord2;
        if (arg1->unk0 >= r->unk18.w && arg1->unk0 < (w = r->unk18.w) + tol) {
            if (arg1->unk8 >= r->unk20.w && arg1->unk8 < (w = r->unk20.w) + tol) {
                if (self->config->unk4 == 0) {
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
