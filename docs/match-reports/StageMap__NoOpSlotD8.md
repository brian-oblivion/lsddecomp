# StageMap__NoOpSlotD8

> Renamed from `func_8004B324` on 2026-09-27 (tools/rename.py). Address 0x8004b324.

**Unit:** dream_day &middot; **Size:** 2 words &middot; **Status:** MATCHED
(`void StageMap__NoOpSlotD8(void) {}`)

## What it does

Nothing. `StageMap`'s vtable slot `+0x0D8` is an empty body -- `jr $ra; nop`
-- and splat generated the match itself. This report exists only because
round 67's naming pass touched every function in the unit and the symbol was
deliberately left alone.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap__NoOpSlotD8` | `StageMap__NoOpSlotD8` (KEPT) | C | An empty vtable stub. Nothing establishes its purpose, and nothing establishes its signature either: `void (void)` is what the empty body PERMITS, not what a caller was seen to pass, and no decompiled function dispatches slot `+0x0D8`. This project already has a precedent for exactly this case -- `code_8220.h` keeps `BasicClassMethods::slot34` unnamed for the same reason, and `SceneNode` keeps `SceneNode__NoOpSlot5C`/`SceneNode__OnPadEvent`/`SceneNode__Update`/`SceneNode__NoOpSlotB0` unnamed as "vtable no-op stubs". A tier-A name here would be pure invention, and FINISHING-PLAN.md is explicit that a wrong tier-A name is worse than a placeholder. |

What IS known, written down so the next reader does not re-derive it: the slot
is `+0x0D8`, it sits between `StageMap__GetCurrentCellKey` (`+0x0D4`) and
`StageMap__SetGridSpan` (`+0x0DC`), both of which are accessors for the
current-cell record and the grid geometry, so the neighbourhood is
accessor-shaped. That is a hint, not evidence, and it is not enough for a name.

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

## Track 7 (2026-09-27, round 98, charlie)

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B324` | `StageMap__NoOpSlotD8` | A | The body is empty (`jr $ra; nop`), so its mechanics are its whole purpose: a leaf whose mechanics ARE its purpose is tier A by the naming rules. The form follows the project's other empty slots (`SceneNode__NoOpSlot5C`, `CdStream__NoOpSlot5C`, `MoviePlayer__NoOpSlot5C`, `NodeGuardedViewport__NoOpSlotB8`). This supersedes round 67's KEEP above, which predates the `NoOpSlotNN` precedent; round 67's reasoning (no caller, no known signature) still holds and the name claims neither. The slot keeps its name `slotD8`, as `slot5C` does for the others. |
