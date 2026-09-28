# StageMap__UpdateIfEnabled — MATCH

> Renamed from `Class866E8__UpdateIfEnabled` on 2026-09-26 (tools/rename.py). Address 0x8004ab24.

> Renamed from `func_8004AB24` on 2026-09-22 (tools/rename.py). Address 0x8004ab24.

**Unit:** class_39e08 · **Size:** 25 instructions · **Result:** 25/25 words

## What it does

A guard-and-dispatch method: if `self->unk70` is set, calls two of its own
vtable slots (`+0xF4`, `+0x13C`) back to back with just `self`.

Slot resolution used `tools/classtable.py gStageMapMethods` (the whole vtable was
resolved in the previous commit for this unit): `+0xF4` -> `StageMap__UpdateFootprintTracking`,
`+0x13C` -> `StageMap__StepScaleRamp`. Neither is decompiled yet; only the slot
existence/signature (self-only) was needed here.

## Final source

```c
void StageMap__UpdateIfEnabled(StageMap *self)
{
    if (self->unk70) {
        self->methods->slotF4(self);
        self->methods->slot13C(self);
    }
}
```

`self->unk70` (a plain `s32`, offset 0x6C..0x74 previously undifferentiated
padding) is new struct knowledge, added to `include/class_39e08.h`.

## Residue

None — matched on the first attempt. The one thing worth noting: the first
call's `$a0` is never re-set before the `jalr` (the compiler reuses the
still-live entry value, matching `self` being unmodified since function
entry); the SECOND call explicitly reloads `$a0` from `$s0` even though it
holds the identical value, presumably because `$a0` is caller-saved across
the intervening call and the compiler doesn't track its liveness past a
`jalr`. Writing the two calls as plain sequential statements reproduced
this without any special handling.

## Provenance

round 2026-09-02, runner ALPHA, unit class_39e08.

## Naming

Round 67 (track 3, naming pass).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004AB24` | `StageMap__UpdateIfEnabled` | B | Occupant of vtable slot `+0x098`. The gate field is `+0x070`, and `class_3bb8c`'s two matched accessors pin its meaning exactly: `StageMap__Enable` sets it to 1 and `StageMap__Disable` runs `slotC0` and then clears it to 0 -- an enable/disable pair. When enabled this function dispatches `slotF4` then `slot13C`; `slot13C` is `class_3bb8c_b`'s matched `StageMap__StepScaleRamp`, which decrements a per-object countdown and sweeps every element's cells, i.e. periodic work. Tier B: "update" describes what the two dispatched slots do, not a purpose anyone has established. |

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x070` | `enabled` | A | Set to 1 / cleared to 0 by a matched setter pair in `class_3bb8c`, and used as a plain boolean gate here. A pure flag whose mechanics are its purpose. |

`slotF4` and `slot13C` keep their `slotNN` names: their occupants
(`StageMap__UpdateFootprintTracking`, `StageMap__StepScaleRamp`) are still `func_` in `class_3bb8c`, and
the convention is to name a slot after the method it dispatches to.

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
