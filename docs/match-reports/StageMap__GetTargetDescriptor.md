# StageMap__GetTargetDescriptor

> Renamed from `Class866E8__GetTargetDescriptor` on 2026-09-26 (tools/rename.py). Address 0x8004c158.

> Renamed from `func_8004C158` on 2026-09-24 (tools/rename.py). Address 0x8004c158.

**Unit:** DayTaskStageMap · **Size:** 26 words · **Status:** MATCHED (first attempt).

## Result

```c
Descriptor10 *StageMap__GetTargetDescriptor(Obj866E8 *self, s32 arg1, void **out) {
    void *v1;

    v1 = (u8 *)self->unk6C->unk14 + 0x18;
    if (out != 0) {
        *out = v1;
    }
    if (arg1 != 0) {
        if (self->methods->slot110(self, arg1, v1) != 0) {
            return 0;
        }
    }
    return &self->unkBC;
}
```

## Derivation

Two independent gates, both falling through to the same `return
&self->unkBC;` (the `unkBC` `Descriptor10` established by `StageMap__SetTargetAndLoadChunks`
this same round):

- `arg1 == 0`: skip the `slot110` dispatch entirely, go straight to the
  fallback return.
- `arg1 != 0`: call `self->methods->slot110(self, arg1, v1)`; only a
  **nonzero** result short-circuits to `return 0;` (in the delay slot of the
  call's `bnez`, unconditionally overwritten to 0, which is what makes this
  read backwards at a glance -- the call's actual return value is discarded,
  a `bnez`+`li v0,0` pair encodes "return 0 iff call succeeded").

`self->unk6C` turned out to be a pointer (`Unk6CObj`), not the raw `s32` an
earlier guess might suggest -- `StageMap__SetTargetAndLoadChunks` (matched later in the same
round) only ever stores its own `arg2` there raw, never dereferencing it, so
nothing in that function alone would have caught the mistake; this function's
own `+0x014` dereference is what pins the type down. `unk6C->unk14` is itself
only ever used for address-of-plus-offset arithmetic (`+0x018`), never
dereferenced further here, so it stays `void *`.

### Proposed learning

**A `bnez`-then-unconditional-`li`-zero pair in a delay slot inverts the
obvious reading of a call's result.** The call's own return value is
discarded; the pair is testing "was it nonzero" purely to decide whether the
*caller's own* return value should become 0. Skimming the raw instructions
suggests "save the call result", but it's "gate the caller's own return on
the call result's zero-ness". Traced correctly here by working out, for each
of the two fall-through targets (`L8004C1A8` vs `L8004C1AC`), what value `$v0`
actually holds *at the target*, not at the branch -- the same discipline
broadcast #3 (delay-slot-belongs-to-the-target) states more generally.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C158` | `StageMap__GetTargetDescriptor` | B | Occupant of `gStageMapMethods` +0x10C. Resolves `self->unk6C`'s own position substruct, optionally hands it to `slot110` (`StageMap__ComputeFootprintDescriptor`) to fill a caller-supplied `Descriptor10Ext`, and always returns `&self->unkBC` -- this object's own current footprint descriptor. "Get...Descriptor" names the return value's role; "Target" reflects `self->unk6C`'s established role as the stored position source (`StageMap__SetTargetAndLoadChunks` sets it). |

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/scene_node.h),
`EntryDesc866E8` is `Ratio16[3]` (include/scene_node.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

## Track 7 (2026-09-27, round 95, charlie)

Parameters and locals, tier A: `arg1` -> `desc`, `out` -> `outPos` (the slot declaration's name), `v1` -> `pos` (the target's coord2 translation).

## Round 97 (alpha): Sony's GsCOORDINATE2

SceneNodeSub14 is deleted: SceneNode.coord2 is Sony's GsCOORDINATE2 (flg; MATRIX coord, whose t is the offset from the parent; MATRIX workm, whose t is the world position; param, super, sub -- 0x50 bytes, offset for offset). Accessors here follow the compiler's list: tx/ty/tz -> coord.t[0]/[1]/[2], unk38 -> workm.t; a local that holds coord.t or workm.t is `long *` (MATRIX.t is long[3]; s32 is int); any cast to GsCOORDINATE2 * is gone. Byte-identical.
