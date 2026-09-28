# StageMap__SetConfig

> Renamed from `Class866E8__SetConfig` on 2026-09-26 (tools/rename.py). Address 0x8004b344.

> Renamed from `func_8004B344` on 2026-09-22 (tools/rename.py). Address 0x8004b344.

**Unit:** dream_day · **Size:** 18 words · **Status:** MATCHED (18/18 words)

## What it does

`StageMap`'s slot +0x0E0 method: calls its own slot +0x040, then stores its
argument into `self->unk68`.

## Derivation

```
lw   $v0, 0x0($s0)
lw   $v0, 0x40($v0)          ; ->slot40
jalr $v0                     ; self->methods->slot40(self), no extra args
 addu $s1, $a1, zero          ; s1 = arg1, saved across the call (delay slot)
sw   $s1, 0x68($s0)          ; self->unk68 = arg1
```

Slot +0x040 is `StageMap__Reset` — **gp_rel-blocked** (see
`docs/research/gp-relative-blocker.md`), still `INCLUDE_ASM`. Its own body
overwrites its incoming `$a1` immediately with a `%gp_rel` global load before
using it, so it doesn't inform whether this slot's declared type takes a
second parameter. Since `StageMap__SetConfig` has no other use of `$a1` besides
saving it to `self->unk68` (no forwarding evidence, unlike `TimedTask__PlaySound`'s
literal-argument pattern — see that report), the call is typed here as
`slot40(self)`, one argument. The delay-slot `move` saving `arg1` into a
temp is ordinary scheduling: the same value is needed after the call
regardless of whether it was also passed into it.

## Proposed learning

None beyond what's already documented for `StageMap` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B344` | `StageMap__SetConfig` | B | Occupant of vtable slot `+0x0E0`. Two statements: dispatch `reset` (`+0x040`, `StageMap__Reset`), then store the argument into `config`. The ORDER is load-bearing and is the whole reason the name is not just "set" -- `StageMap__Reset` itself NULLs `config`, so the reset must run first. Tier B: the pointee is a small parameter block (`dream_day` reads it as `{s16 divisor, s16 count, s32 unk4}` from four functions), but what it configures is not established. |

Slot named this round: `StageMapMethods::slot40` -> `reset`.

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
the per-stage config. Header now `include/stage_map.h`; evidence in its banner.

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
