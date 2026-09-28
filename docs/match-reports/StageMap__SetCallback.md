# StageMap__SetCallback

> Renamed from `Class866E8__SetCallback` on 2026-09-26 (tools/rename.py). Address 0x8004adc4.

> Renamed from `func_8004ADC4` on 2026-09-22 (tools/rename.py). Address 0x8004adc4.

**Unit:** DayTaskStageMap · **Size:** 3 words · **Status:** MATCHED (3/3 words)

## What it does

`StageMap`'s slot +0x0C8 setter: stores its two arguments into
`self->unk60` and `self->unk64`.

## Derivation

```
sw $a1, 0x60($a0)
jr $ra
 sw $a2, 0x64($a0)
```

A two-field setter, both `s32` (plain `sw`, no shift/sign-extend). Field
offsets and the `StageMap` type come from `include/DayTaskStageMap.h`
(established this round; see `TimedTask__PlaySound.md` for how the class was
identified via `tools/classtable.py gStageMapMethods`).

## Proposed learning

None beyond what's already documented for `StageMap` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004ADC4` | `StageMap__SetCallback` | A | Occupant of vtable slot `+0x0C8`; a two-word setter whose stores land at `+0x060` and `+0x064`. `DayTaskStageMap`'s MATCHED `StageMap__ComputeChunkLoadEntry` invokes exactly that pair as `unk60(unk64, value, 0, 0)` and stores the result -- so the first word is a function pointer and the second its context. A pure setter whose mechanics are its purpose. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x060` | `valueFn` | A | Called as a function pointer by the matched sibling above. |
| `StageMap+0x064` | `valueFnCtx` | A | Passed as that call's first argument and never dereferenced anywhere. |

The two fields keep their `s32` declared type here (this unit only stores raw
words into them); `class_3bb8c.h` already carries the function-pointer typing.
Unifying the two declarations is track-4 work.

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
