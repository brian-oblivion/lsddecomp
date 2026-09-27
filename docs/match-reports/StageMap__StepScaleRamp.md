# StageMap__StepScaleRamp — MATCHED (24/24 words)

> Renamed from `StageMap__AdvanceRateCountdown` on 2026-09-26 (tools/rename.py). Address 0x8004d028.

> Renamed from `Class866E8__AdvanceRateCountdown` on 2026-09-26 (tools/rename.py). Address 0x8004d028.

> Renamed from `func_8004D028` on 2026-09-24 (tools/rename.py). Address 0x8004d028.

Sibling of `StageMap__EndScaleRamp` (own report) — same shape, one instruction
longer because the gate here is a genuine countdown rather than a
one-shot latch.

## Disassembly

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
move  $s0, $a0
sw    $ra, 0x14($sp)
lw    $v0, 0x1E0($s0)        ; v0 = self->unk1E0
blez  $v0, .skip
 nop
lui   $a1, %hi(StageMap__AddScaleStepToCell)
addiu $a1, $a1, %lo(StageMap__AddScaleStepToCell)
jal   StageMap__ForEachSlot
 move $a2, $zero
lw    $v0, 0x1E0($s0)         ; reload
addiu $v0, $v0, -1
bnez  $v0, .skip
 sw   $v0, 0x1E0($s0)          ; delay slot: self->unk1E0 = v0 (ALWAYS runs)
addiu $v0, $zero, -1
sw    $v0, 0x1E0($s0)           ; only reached when the decrement hit exactly 0
.skip:
...epilogue
```

The classic "default value (decrement), then conditionally overwritten by
an `if` with no `else`" idiom already documented in
`docs/DECOMPILATION_LEARNINGS.md`: the decrement's delay slot stores the
new value unconditionally, and the branch-not-taken path (decrement hit
0) overwrites it again with `-1`.

## Final C

```c
void StageMap__StepScaleRamp(Obj866E8 *self) {
    if (self->unk1E0 > 0) {
        StageMap__ForEachSlot(self, StageMap__AddScaleStepToCell, 0);
        self->unk1E0 -= 1;
        if (self->unk1E0 == 0) {
            self->unk1E0 = -1;
        }
    }
}
```

`StageMap__AddScaleStepToCell` is defined later in this file (ROM order), so it needs a
forward declaration here — same pattern already used for
`StageMap__ResetCellScale` in `StageMap__EndScaleRamp`.

## New struct knowledge

None new (reuses `Obj866E8::unk1E0`, established by `StageMap__EndScaleRamp`).

## Attempts

1 (matched on first attempt — same "default, conditionally overwritten"
idiom already proven elsewhere in this project made the shape
recognizable immediately).

### Proposed learning

None new — confirms the existing idiom, does not extend it.

## Naming

**Tier B.** Not a vtable slot. While `self->rateCountdown > 0`: applies
the configured rate entry to every child via
`ForEachElem(self, ApplyRateToChild, 0)`, then decrements the countdown,
clamping to -1 once it reaches 0 (a "done" sentinel, distinct from the
0 the sibling `StageMap__EndScaleRamp` uses for "off"). Named for the
mechanics: it is the per-tick advance of the rate/countdown pair
established by `StageMap__StartScaleRamp`.

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

This function: `StageMap__AdvanceRateCountdown` -> `StageMap__StepScaleRamp` (`python3 tools/rename.py StageMap__AdvanceRateCountdown StageMap__StepScaleRamp`, tier B): while `scaleRampTicks` > 0, adds the step to every cell's scale (updateScale, add) and counts down; -1 when done.

## Round 96 (track 7, delta)

Nothing to change.
