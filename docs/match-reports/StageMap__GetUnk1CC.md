# StageMap__GetUnk1CC — MATCHED (2/2 words)

> Renamed from `Class866E8__GetUnk1CC` on 2026-09-26 (tools/rename.py). Address 0x8004cfa8.

> Renamed from `func_8004CFA8` on 2026-09-24 (tools/rename.py). Address 0x8004cfa8.

Two-instruction leaf: `jr $ra` / `addiu $v0, $a0, 0x1CC`. Not a vtable slot
(not in `gStageMapMethods`, per `tools/classtable.py`), and no caller found
anywhere in the executable (only reference is its own `.s` file) — an
unused or not-yet-carved accessor.

## Disassembly

```
jr   $ra
 addiu $v0, $a0, 0x1CC     ; return &self->unk1CC (address-of only, no load)
```

## Final C

```c
void *StageMap__GetUnk1CC(Obj866E8 *self) {
    return &self->unk1CC;
}
```

## New struct knowledge (`include/class_3bb8c.h`)

- `Obj866E8::unk1CC` (`s32`, +0x1CC) — real type unknown, only its address
  is ever taken here. No caller exists in this executable to cross-check
  against, so the field type is a placeholder (matches the project's
  convention for address-of-only fields, e.g. `StageMap::unk1C0` in
  `include/class_3ac78.h`).

## Attempts

1 (matched on first attempt — pure address arithmetic, no ambiguity).

### Proposed learning

None new.

## Naming

**Tier A**, pure getter -- `return &self->unk1CC;`, two instructions, no
caller found anywhere in the executable. `unk1CC` itself is left
unrenamed: `StageMap__GetUnk1CC` only ever takes its address, never
reads through it, so its real type (and therefore any real name) is not
established from this unit -- class_3ac78's own independent view of the
same offset (`StageMap__Reset`: "set to -1") does not clarify it
either. Following the "GetSetUnk10Field0"-style precedent for a field
whose meaning is unknown but whose offset is fixed.

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
