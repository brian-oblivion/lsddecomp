# StageMap__SetAcceptedTags

> Renamed from `Class866E8__SetAcceptedTags` on 2026-09-26 (tools/rename.py). Address 0x8004add0.

> Renamed from `func_8004ADD0` on 2026-09-22 (tools/rename.py). Address 0x8004add0.

**Unit:** class_39e08 · **Size:** 2 words · **Status:** MATCHED (2/2 words)

## What it does

`StageMap`'s slot +0x0CC setter: stores its argument into `self->unkE8`.

## Derivation

```
jr $ra
 sw $a1, 0xE8($a0)
```

A one-field `s32` setter, leaf, tail instruction in the delay slot of `jr`.

## Proposed learning

None beyond what's already documented for `StageMap` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004ADD0` | `StageMap__SetAcceptedTags` | A | Occupant of vtable slot `+0x0CC`; a one-word setter into `+0x0E8`. The field's only reader is `StageMap__ForwardAcceptedCommand` in this same unit, which walks it as a NUL-terminated array of vtable header words and uses a match to decide whether to act on a sender. The name is that reader's contract, stated once. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x0E8` | `acceptedTags` | A | Sole reader establishes it exactly: a NUL-terminated list of class header words that gates sender acceptance. |

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
