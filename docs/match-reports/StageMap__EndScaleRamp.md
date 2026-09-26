# StageMap__EndScaleRamp — MATCHED (18/18 words)

> Renamed from `StageMap__FlushRateLatch` on 2026-09-26 (tools/rename.py). Address 0x8004d088.

> Renamed from `Class866E8__FlushRateLatch` on 2026-09-26 (tools/rename.py). Address 0x8004d088.

> Renamed from `func_8004D088` on 2026-09-24 (tools/rename.py). Address 0x8004d088.

Not a vtable slot in this unit's own dispatch (`class_3ac78.h` documents
it as its OWN independent view's `StageMapMethods::slot140`, called
from that unit's `StageMap__UnloadAllSlots` — not this unit's concern; here it is
just a plain function with a real body).

## Disassembly

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
move  $s0, $a0
sw    $ra, 0x14($sp)
lw    $v0, 0x1E0($s0)     ; v0 = self->unk1E0
beqz  $v0, .skip
 nop
lui   $a1, %hi(StageMap__ResetCellScale)
addiu $a1, $a1, %lo(StageMap__ResetCellScale)
jal   StageMap__ForEachSlot
 move $a2, $zero
sw    $zero, 0x1E0($s0)    ; self->unk1E0 = 0 (unconditional on this path)
.skip:
...epilogue
```

## Final C

```c
void StageMap__EndScaleRamp(Obj866E8 *self) {
    if (self->unk1E0 != 0) {
        StageMap__ForEachSlot(self, StageMap__ResetCellScale, 0);
        self->unk1E0 = 0;
    }
}
```

`StageMap__ForEachSlot` is still `INCLUDE_ASM` in this unit; forward-declared per
the established "calling into a still-INCLUDE_ASM function is fine"
convention. Its own signature was derived from THIS call site plus
`StageMap__StepScaleRamp`'s (own report): `(Obj866E8 *self, void
(*itemCallback)(Obj866E8*, Unk10ChildObj_3bb8c_b*), void
(*perArrCallback)(Obj866E8*, Elem*))` — both known callers pass 0 for the
third argument, so its true type is inferred from `StageMap__ForEachSlot`'s own
body (still unmatched) rather than confirmed live.

## New struct/global knowledge

- `Obj866E8::unk1E0` (`s32`, +0x1E0) — a gate/countdown value, also used
  by the sibling `StageMap__StepScaleRamp` (own report).
- `extern void StageMap__ForEachSlot(...)` added (still raw asm in this unit).

## Attempts

1 (matched on first attempt).

### Proposed learning

None new.

## Naming

**Tier B.** Not a vtable slot. One-shot sibling of
`StageMap__StepScaleRamp`: if `self->rateCountdown != 0`, resets
every child's rate (`ForEachElem(self, ResetChildRate, 0)`) and clears the
latch to 0 in a single call, no per-tick decrement. "Latch" distinguishes
it from the countdown sibling -- it fires once and clears, rather than
ticking down.

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

This function: `StageMap__FlushRateLatch` -> `StageMap__EndScaleRamp` (`python3 tools/rename.py StageMap__FlushRateLatch StageMap__EndScaleRamp`, tier B): after a ramp, sets every cell's scale to 1/1 x3 and zeroes the count.
