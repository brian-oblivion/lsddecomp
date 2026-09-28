# StageMap__SplitChunkIndex

> Renamed from `StageMap__ComputeDivisorSplit` on 2026-09-26 (tools/rename.py). Address 0x8004c368.

> Renamed from `Class866E8__ComputeDivisorSplit` on 2026-09-26 (tools/rename.py). Address 0x8004c368.

> Renamed from `func_8004C368` on 2026-09-24 (tools/rename.py). Address 0x8004c368.

**Unit:** DayTaskStageMap · **Size:** 34 words · **Status:** MATCHED (first attempt).

## Result

```c
void StageMap__SplitChunkIndex(Obj866E8 *self, u8 *out, s32 val) {
    out[0] = val % self->unk68->divisor;
    out[1] = val / self->unk68->divisor;
}
```

## Derivation

Retail's body is two near-identical `div`/trap-check sequences (the maspsx
`--expand-div` expansion for a runtime, non-constant divisor: zero-check
`break 7`, `INT_MIN/-1` overflow check `break 6`), one reading `mfhi`
(remainder) and one reading `mflo` (quotient), each reloading
`self->unk68->divisor` fresh from memory. Writing plain `%` and `/` lets the
compiler emit the expansion itself; no manual reconstruction of the trap
sequence was needed.

The double reload of `self->unk68->divisor` (once per statement, not cached
into a local) matches retail exactly -- two separate C statements, two
separate field reads, no explicit sharing.

This is also the function that pinned down `Unk68Struct`'s first field
(`divisor`, a signed halfword at `+0x000`) -- corroborated independently by
`StageMap__ComputeNeighbourMask` and `ComputeCellWorldOffsets`/`StageMap__ComputeCellOffsets`'s call site later in the
same round.

### Proposed learning

None new -- confirms the existing "div by a non-constant expands to a
several-instruction sequence with a zero/overflow check; write plain `%`/`/`"
guidance in CLAUDE.md verbatim, including the double-reload-not-cached shape
for a value read via two independent statements with no intervening
assignment to a local.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C368` | `StageMap__SplitChunkIndex` | A | Takes `self` first (a method). Pure two-line computation: `out[0] = val % self->unk68->divisor; out[1] = val / self->unk68->divisor;` -- a mod/div split against the object's own divisor, no other side effect. Mechanics-only name, tier A by the "getter/clamp" clause (a pure, unconditional computation whose mechanics fully describe it). |

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

This function: `StageMap__ComputeDivisorSplit` -> `StageMap__SplitChunkIndex` (`python3 tools/rename.py StageMap__ComputeDivisorSplit StageMap__SplitChunkIndex`, tier A): `out[0] = index % columns`, `out[1] = index / columns`.

## Track 7 (2026-09-27, round 95, charlie)

Parameter `val` -> `chunkIndex`. Proposed (head): the `u8 *out` parameter and the +0x114 slot become `Descriptor10 *` with `out->b0`/`out->b1`; not done here because the prototype is in include/StageMap.h and ObjM (ObjMStyleActor.c) calls through the slot.
