# StageMap__Disable — MATCHED (16/16 words)

> Renamed from `Class866E8__Disable` on 2026-09-26 (tools/rename.py). Address 0x8004b57c.

> Renamed from `StageMap__func_8004B57C` on 2026-09-24 (tools/rename.py). Address 0x8004b57c.

> Renamed from `func_8004B57C` on 2026-09-24 (tools/rename.py). Address 0x8004b57c.

`Obj866E8`'s vtable slot +0x0F0.

## Disassembly

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
addu  $s0, $a0, $zero        ; s0 = self
sw    $ra, 0x14($sp)
lw    $v0, 0x0($s0)          ; self->methods
nop
lw    $v0, 0xC0($v0)         ; methods->slotC0
jalr  $v0
 nop                          ; self->methods->slotC0(self)
sw    $zero, 0x70($s0)       ; self->unk70 = 0
...
jr $ra
```

## Final C

```c
void StageMap__Disable(Obj866E8 *self) {
    self->methods->slotC0(self);
    self->unk70 = 0;
}
```

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8Methods::slotC0` typed `void (*)(Obj866E8 *self)`, return
  discarded.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B57C` | `StageMap__Disable` | A | Occupant of `gStageMapMethods` +0x0F0. Body: `self->methods->slotC0(self); self->enabled = 0;` -- dispatches a teardown slot, then clears the same field `StageMap__Enable` sets. See `StageMap__Enable.md` for the cross-unit confirmation of the `enabled` field name. |

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

## Track 7 (2026-09-27, round 95, charlie)

Nothing renamed; `enabled` is a boolean and keeps 0.
