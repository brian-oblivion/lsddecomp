# StageMap__GetCurrentCellKey

> Renamed from `Class866E8__GetCurrentCellKey` on 2026-09-26 (tools/rename.py). Address 0x8004b31c.

> Renamed from `func_8004B31C` on 2026-09-22 (tools/rename.py). Address 0x8004b31c.

**Unit:** DayTaskStageMap · **Size:** 2 words · **Status:** MATCHED (2/2 words)

## What it does

`StageMap`'s slot +0x0D4 accessor: returns the address of an embedded
field/sub-array at `self`+0x1C0. Address-of only — nothing reads through the
returned pointer here, so its pointee's real element type is unconfirmed.

## Derivation

```
jr    $ra
 addiu $v0, $a0, 0x1C0
```

A leaf computing `&self->unk1C0` and returning it. Sized the trailing
`unk1C0` field as `u8 unk1C0[0x1E8 - 0x1C0]` (0x28 bytes) in
`include/DayTaskStageMap.h` so `StageMap`'s total size comes out to exactly
0x1E8 — the same constant `New_StageMap`'s allocator call uses — without
asserting anything about the field's internal structure.

## Proposed learning

None beyond what's already documented for `StageMap` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B31C` | `StageMap__GetCurrentCellKey` | B | Occupant of vtable slot `+0x0D4`; returns `&self->curCellTag`. That address is the start of the 4-byte record `StageMap__DispatchToRectCells` rewrites immediately before every cell visit: a tag halfword copied from `cellTag`, then the current column and row as bytes. So the accessor hands out "which cell is being visited right now". Tier B -- the record's contents are established, what a consumer does with the pointer is not (no decompiled function reads through it). |

This supersedes the earlier reading in this report, which sized `+0x1C0` as
one opaque 0x28-byte block because nothing then reached inside it.
`StageMap__DispatchToRectCells` does, and its three writes are what named
the fields.

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
