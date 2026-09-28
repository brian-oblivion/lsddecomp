# StageMap__GetLastEventSlotChunk

> Renamed from `StageMap__GetLastTargetRateSplit` on 2026-09-26 (tools/rename.py). Address 0x8004c3f0.

> Renamed from `Class866E8__GetLastTargetRateSplit` on 2026-09-26 (tools/rename.py). Address 0x8004c3f0.

> Renamed from `func_8004C3F0` on 2026-09-24 (tools/rename.py). Address 0x8004c3f0.

**Unit:** class_39e08 · **Size:** 17 words · **Status:** MATCHED (first attempt).

## Result

```c
Unk1BCObj *StageMap__GetLastEventSlotChunk(Obj866E8 *self, u8 *out) {
    StageMap__SplitChunkIndex(self, out, self->unk1BC->unk4->unk30);
    return self->unk1BC;
}
```

## Derivation

Straight-line body, no branches. `self->unk1BC` is loaded once for the call's
third argument (chased through `->unk4->unk30`, a signed halfword) and RELOADED
after the call for the return value -- ordinary conservative-aliasing reload
after a call to a non-`INCLUDE_ASM` function, not anything that needed a
barrier or an explicit local.

`self->unk1BC->unk4` turned out to be the same `ElemTarget` type already
established from `StageMap__ClearSlotCells`/`StageMap__FindSlotByNeighbour` (its own `+0x030` field lines
up with `ElemTarget::unk30`, itself first seen in `StageMap__ClearSlotCells`'s
`entry->unk4->unk30` read). New struct: `Unk1BCObj { u8 pad[4]; ElemTarget
*unk4; }` for `self->unk1BC`'s pointee.

Return type is a straight pass-through of `self->unk1BC` -- no evidence either
way on what it's "really" used for by callers (none in this codebase yet), so
it's typed as its own pointee struct rather than `void *`.

### Proposed learning

None beyond what's already documented -- this one was a clean, quick match
that confirmed cross-referencing `ElemTarget`/`Elem` field offsets across this
unit's own functions is a reliable way to spot repeated struct fields (here,
`+0x030` on the target object, seen independently from two different call
chains: `self->arr[i].unk4->unk30` in `StageMap__ClearSlotCells` and
`self->unk1BC->unk4->unk30` here).

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C3F0` | `StageMap__GetLastEventSlotChunk` | B | `func_8004C368(self, out, self->unk1BC->unk4->unk30); return self->unk1BC;` -- splits `self->unk1BC`'s own target rate (`unk4->unk30`, the same `ElemTarget::unk30` field `StageMap__FindSlotIndexByChunk` and others already read as a rate) via `StageMap__SplitChunkIndex`, and returns `self->unk1BC` itself. "GetLastTarget..." reflects `unk1BC`'s established role (set by `StageMap__FindSlotForPosition`'s callers elsewhere, read only here) as a stashed "most recently resolved" target pointer; "...RateSplit" names the mod/div computation performed on it. Tier B: the field's exact update site is outside this unit's matched functions, so "last" is inferred from usage pattern, not directly observed here. |

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

This function: `StageMap__GetLastTargetRateSplit` -> `StageMap__GetLastEventSlotChunk` (`python3 tools/rename.py StageMap__GetLastTargetRateSplit StageMap__GetLastEventSlotChunk`, tier A): a getter: writes `lastEventSlot`'s chunk column/row and returns that slot.

## Track 7 (2026-09-27, round 95, charlie)

Nothing renamed (`out` is its role).
