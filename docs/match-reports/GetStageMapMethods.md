# GetStageMapMethods — MATCHED (4/4 words)

> Renamed from `GetClass866E8Methods` on 2026-09-26 (tools/rename.py). Address 0x8004d244.

> Renamed from `func_8004D244` on 2026-09-24 (tools/rename.py). Address 0x8004d244.

Get-vtable helper for the class whose method table is `gStageMapMethods`
(`Obj866E8` in this unit's header, `StageMap` in `DayTaskStageMap.h`'s
independent view of the SAME table). This is the SAME real function
`DayTaskStageMap.h` already documents an `extern` prototype for (`"Get-vtable
helper for StageMap. Still raw asm... lives in DayTaskStageMap"`) — it now
has a real body, contributed by this unit.

## Disassembly

```
lui   $v0, %hi(gStageMapMethods)
addiu $v0, $v0, %lo(gStageMapMethods)
jr    $ra
 nop
```

No dereference — this just materializes `&gStageMapMethods`. Confirmed as a
zero-argument call from its only external caller, `DayTaskStageMap.c`'s
`StageMap__StageMap` (the `New_StageMap` constructor): the `jal` there has a
`nop` in its own delay slot (no argument setup) and the very next
instruction stores `$v0` straight into `self->methods` (offset 0), i.e.
`self->methods = GetStageMapMethods();`.

## Final C

```c
Obj866E8Methods *GetStageMapMethods(void) {
    return &gStageMapMethods;
}
```

## New struct/global knowledge

- `extern Obj866E8Methods gStageMapMethods;` added to `include/class_3bb8c.h`
  (this unit's own independent view of the table; `DayTaskStageMap.h` keeps
  its own separate `StageMapMethods` view of the identical memory, per
  the project's established multiple-independent-views convention).

## Attempts

1 (matched on first attempt).

### Proposed learning

None new — confirms the already-established "extern Methods D_xxx; return
&D_xxx;" getter idiom used throughout the project (`pad.c`,
`entity.c`, `DayTaskStageMap.c`, `class_3ac78.c`, `task.c`,
`TodActor.c`).

## Naming

**Tier A.** `lui`/`addiu` of `&gStageMapMethods`, no dereference -- the class's
get-vtable helper. Matches this project's established
`GetClass<addr>Methods` convention for these helpers exactly (e.g.
`GetTimedTaskMethods`, `GetItemListMethods`, `GetNodeGuardedViewportMethods`),
which DayTaskStageMap.h's own extern for this SAME real function already
anticipated under this exact name pattern (previously documented there as
an unnamed extern for "the get-vtable helper... lives in DayTaskStageMap").

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
(include/stage_grid.h), `Unk54Struct` is `LongVec3` (include/scene_node.h),
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

## Round 96 (track 7, delta)

Nothing to change.
