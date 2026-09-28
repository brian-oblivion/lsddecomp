# StageMap__FindSlotIndexByNeighbour — MATCHED (18/18 words)

> Renamed from `StageMap__FindElemIndexByUnk32` on 2026-09-26 (tools/rename.py). Address 0x8004c588.

> Renamed from `Class866E8__FindElemIndexByUnk32` on 2026-09-26 (tools/rename.py). Address 0x8004c588.

> Renamed from `func_8004C588` on 2026-09-24 (tools/rename.py). Address 0x8004c588.

Not a vtable slot by call-site inspection needed — it IS one, per
`tools/classtable.py gStageMapMethods` (`+0x120`), though nothing in this unit
dispatches through that slot itself. A plain linear-search helper: walks
`self->arr` looking for the element whose `unk4->unk32` matches a key,
returning its index (or 0 if never found — the loop's initial `result`
default, never overwritten).

## Residue and how it closed (1 residue, 2 attempts)

First attempt wrote the natural indexed loop directly:

```c
if (self->arr[i].unk4->unk32 == key) { ... }
```

This compiled to a MOVING-POINTER loop (`self` itself incremented by
`0x1C` per iteration, reading a fixed `+0xF0` offset from the current
pointer) — GCC 2.6.3 reusing the `self` register as its own induction
variable, since (in this shape) nothing needs `self` again after the
loop. Retail instead keeps a genuinely SEPARATE running byte-offset
register alongside the loop index, never touching `self` itself. Adding
an explicit intermediate pointer variable, exactly mirroring the sibling
function `StageMap__FindSlotByNeighbour`'s already-matched shape, closed it:

```c
Elem *e = &self->arr[i];
if (e->unk4->unk32 == key) { ... }
```

Second attempt (with `e`) matched immediately.

## Final C

```c
s32 StageMap__FindSlotIndexByNeighbour(Obj866E8 *self, s32 key) {
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
```

## New struct knowledge

None new (reuses `Elem`/`ElemTarget::unk32`, both already established by
`DayTaskStageMap.c`'s `StageMap__FindSlotByNeighbour`/`StageMap__CountPendingLoads`).

## Attempts

2 (see residue above).

### Proposed learning

**An explicit intermediate element pointer (`Elem *e = &self->arr[i];`)
is not just style — it changes whether GCC 2.6.3 reuses the array's BASE
pointer register as its own induction variable.** Without it, when the
base pointer (here, `self`) is dead after the loop, GCC freely repurposes
it into a moving pointer, dropping any separately-tracked byte offset.
Retail's own code keeps a genuinely separate running-offset register in
this function — the same shape `StageMap__FindSlotByNeighbour` (already matched) uses.
This generalizes the existing "let GCC hoist its own loop invariants"
guidance in `docs/DECOMPILATION_LEARNINGS.md`: that entry showed
*keeping* an explicit local matters for a hand-hoisted `base`; here the
opposite-looking fix (adding an explicit per-iteration element pointer)
serves the same underlying purpose — controlling which value GCC treats
as the loop's own induction variable. Confirmed again on the very next
function, `StageMap__FindSlotIndexByChunk` (own report), so this is now a two-instance
pattern for this project, not a one-off.

## Naming

**Tier A.** Vtable slot +0x120 of `gStageMapMethods` (`tools/classtable.py`),
class prefix `StageMap` confirmed against that same table's other
already-named slots (e.g. `StageMap__ApplyToSenderFootprint` at +0x12C).
Pure linear-search leaf: the body IS the evidence -- walk `self->arr`,
compare `e->unk4->unk32` to `key`, return the matching index (default 0).
Named by the field it searches on (`unk32`, still unrenamed -- it is a
cross-unit `ElemTarget` field, also read by `StageMap__FindSlotByNeighbour` in
DayTaskStageMap.c, so this unit does not own it) rather than by a guessed
purpose, per this project's "name what the code does" rule.

## Proposed field names

**Head, round 76: NOT APPLIED.** The set is internally inconsistent: `unk30` and `unk32` cannot both be `key`, and `include/class_3bb8c.h`'s view records `unk30` as a raw rate that StageMap__ComputeFootprintDescriptor sign-extends, so equality-compared-here is not enough for `key`. `unk2C -> enabled` rests on "nonzero enables" alone. Re-propose from the base class's naming pass (DayTaskStageMap.c), where all readers are in one unit.

`ElemTarget::unk32` (also read by DayTaskStageMap.c's `StageMap__FindSlotByNeighbour`, so
cross-unit -- not renamed here). Proposed: `key` -- it is exactly what
both `FindElemIndexByUnk32` and its caller-side context treat it as, a
value compared for equality to select an element. Evidence: this
function's entire body is `e->unk4->unk32 == key`.

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

This function: `StageMap__FindElemIndexByUnk32` -> `StageMap__FindSlotIndexByNeighbour` (`python3 tools/rename.py StageMap__FindElemIndexByUnk32 StageMap__FindSlotIndexByNeighbour`, tier B): the same test, returning the index (0 when none).

## Round 96 (track 7, delta)

Locals: `result` -> `index`, `e` -> `slot` (a ChunkSlot). The loop bound is
`ARRAY_COUNT(self->slots)` (7). New comment: returns 0, not -1, when no slot
has the key. Zero bytes.

The unit banner was rewritten (what the file holds); the old one, verbatim:

```c
/*
 * DayTaskStageMap -- the last third of StageMap (include/StageMap.h),
 * sharing include/class_3bb8c.h with DayTaskStageMap.c.
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
```
