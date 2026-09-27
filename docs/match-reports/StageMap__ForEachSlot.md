# StageMap__ForEachSlot — MATCHED (36/36 words)

> Renamed from `StageMap__ForEachElem` on 2026-09-26 (tools/rename.py). Address 0x8004d140.

> Renamed from `Class866E8__ForEachElem` on 2026-09-26 (tools/rename.py). Address 0x8004d140.

> Renamed from `func_8004D140` on 2026-09-24 (tools/rename.py). Address 0x8004d140.

Iterates `self->arr[0..6]`, invoking an optional per-element callback
(`arg2`, called `(self, &arr[i])` when non-NULL) and then always forwarding
`(self, arg1, &arr[i])` to `StageMap__ForEachSlotCell`. Already had a prototype and a
two-hop derivation in `include/class_3bb8c.h` from a previous round
(established from `StageMap__StepScaleRamp`/`StageMap__EndScaleRamp`'s call sites); this round
supplied the body.

Matches the "explicit intermediate element pointer in an array loop" idiom
from `docs/DECOMPILATION_LEARNINGS.md`: `e = &self->arr[i];` rather than
repeated `self->arr[i]` field access.

## Final C

```c
void StageMap__ForEachSlot(Obj866E8 *self, void (*arg1)(Obj866E8 *self, EntryChildObj *item), void (*arg2)(Obj866E8 *self, Elem *item)) {
    s32 i;
    Elem *e;

    for (i = 0; i < 7; i++) {
        e = &self->arr[i];
        if (arg2 != 0) {
            arg2(self, e);
        }
        StageMap__ForEachSlotCell(self, arg1, e);
    }
}
```

## Attempts

1 (matched on first attempt — the header's existing derivation and the
established "intermediate element pointer" idiom were enough to reproduce
the loop exactly, including the `0xEC`-based offset accumulator retail
uses instead of re-deriving `&arr[i]` from scratch each iteration).

### Proposed learning

None new — confirms the existing "intermediate element pointer" idiom
generalizes to a loop whose body is entirely calls (no field reads other
than the pointer itself).

## Naming

**Tier A.** Not a vtable slot -- a generic iteration helper: loops
`self->arr[0..6]`, optionally invoking a per-`Elem` callback (`arg2`),
then always forwarding to `StageMap__ForEachSlotCell` for each
element. Mechanics-is-purpose: it is exactly what its name says, a
for-each over the object's `Elem` array, used by both the rate/countdown
callers this round and (per the header's existing prototype) elsewhere.

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
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
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

This function: `StageMap__ForEachElem` -> `StageMap__ForEachSlot` (`python3 tools/rename.py StageMap__ForEachElem StageMap__ForEachSlot`, tier A): an iterator over the seven slots (slot callback, then the cell callback over its cells).

## Round 96 (track 7, delta)

Parameters `arg1`/`arg2` -> `cellFn`/`slotFn` (the prototype's `elemFn`
follows), `e` -> `slot`; loop bound `ARRAY_COUNT(self->slots)`.
