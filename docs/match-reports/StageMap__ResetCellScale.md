# StageMap__ResetCellScale — MATCHED (14/14 words)

> Renamed from `StageMap__ResetChildRate` on 2026-09-26 (tools/rename.py). Address 0x8004d108.

> Renamed from `Class866E8__ResetChildRate` on 2026-09-26 (tools/rename.py). Address 0x8004d108.

> Renamed from `func_8004D108` on 2026-09-24 (tools/rename.py). Address 0x8004d108.

Sibling of `StageMap__AddScaleStepToCell` (see that report for how the true call chain —
`StageMap__ForEachSlot` forwards to `StageMap__ForEachSlotCell`, which does the actual
`jalr` — was resolved). Same `Unk10ChildMethods_3bb8c_b::slot48` slot,
different literal arguments.

## Disassembly

```
addiu $sp, $sp, -0x18
move  $a0, $a1              ; a0 = item (original arg0, "self", is discarded entirely)
sw    $ra, 0x10($sp)
lw    $v0, 0x0($a0)         ; v0 = item->methods
lw    $v0, 0x48($v0)        ; v0 = item->methods->slot48
lui   $a2, %hi(sScaleOne)
addiu $a2, $a2, %lo(sScaleOne)
jalr  $v0
 ori  $a1, $zero, 0x1
...epilogue
```

Notable: the original first parameter (`self`) is never referenced after
the top-of-function register shuffle overwrites `a0` with `item` — this
function genuinely ignores its own `self` argument, unlike its sibling
`StageMap__AddScaleStepToCell` which uses it (`self->unk1E4`). Confirmed real, not a
missing-parameter bug, by cross-checking the only caller
(`StageMap__EndScaleRamp`, which passes this function's address to
`StageMap__ForEachSlot` exactly like `StageMap__StepScaleRamp` passes `StageMap__AddScaleStepToCell`'s —
same call shape, same two-parameter signature required by the eventual
`StageMap__ForEachSlotCell` dispatcher).

## Final C

```c
void StageMap__ResetCellScale(Obj866E8 *self, Unk10ChildObj_3bb8c_b *item) {
    item->methods->slot48(item, 1, sScaleOne);
}
```

## New struct/global knowledge

- `extern s32 sScaleOne[3];` — a 3-word (12-byte) data block, address-of
  only. Never dereferenced in this unit, so left untyped beyond its size.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new beyond `StageMap__AddScaleStepToCell`'s (same call-chain-tracing lesson).

## Naming

**Tier B.** Not a vtable slot -- the `StageMap__EndScaleRamp` callback
sibling of `StageMap__AddScaleStepToCell`. Body: `item->methods->slot48(
item, 1, &sScaleOne)`, the constant "off" entry rather than the parent's
own `rateEntry`. Named to read as the inverse of `ApplyRateToChild`.

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

This function: `StageMap__ResetChildRate` -> `StageMap__ResetCellScale` (`python3 tools/rename.py StageMap__ResetChildRate StageMap__ResetCellScale`, tier A): one-line callback: `updateScale(cell, 1 = set, 1/1 x3)`.

## Round 96 (track 7, delta)

Parameter `item` -> `cell`. `D_800869CC` -> `sScaleOne` (tools/rename.py,
first as sScaleOneStep, then sScaleOne; tier A: 1/1, 1/1, 1/1, set with
updateScale's `set` = 1). Its extern moved from include/class_3bb8c.h into
class_3bb8c_b.c, the only reader. SceneNode's SCALE_ONE holds the same
values at another address.
