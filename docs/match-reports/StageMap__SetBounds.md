# StageMap__SetBounds — MATCHED (2/2 words)

> Renamed from `Class866E8__SetBounds` on 2026-09-26 (tools/rename.py). Address 0x8004cfb0.

> Renamed from `func_8004CFB0` on 2026-09-24 (tools/rename.py). Address 0x8004cfb0.

Two-instruction leaf: `jr $ra` / `sw $a1, 0x1DC($a0)`. Not a vtable slot,
no caller found anywhere in the executable — a plain setter, presumably
called from as-yet-uncarved ground (`class_3bb8c_c`) or from outside this
365-function block entirely.

## Disassembly

```
jr   $ra
 sw   $a1, 0x1DC($a0)     ; self->unk1DC = arg1
```

## Final C

```c
void StageMap__SetBounds(Obj866E8 *self, CellBounds *arg1) {
    self->unk1DC = arg1;
}
```

## New struct knowledge

- `Obj866E8::unk1DC` (`CellBounds *`, +0x1DC) — typed from
  `IsPointOutOfBounds`'s own body (see that function's report), the only place
  in this unit that reads it back. `IsPointOutOfBounds` itself stalled, but the
  field's TYPE derivation (a 4-field min/max bounding-box struct) does not
  depend on that function's byte-match — it comes from reading the
  occupant's own disassembly, independent of whether the source shape
  that produces it byte-exactly has been found yet.

## Attempts

1 (matched on first attempt).

### Proposed learning

None new.

## Naming

**Tier A**, pure setter -- `self->unk1DC = arg1;`, two instructions.
Renamed together with the field it writes (`unk1DC` -> `bounds`, this
round): the field's role IS established, by its only other reader,
`IsPointOutOfBounds`, which dereferences it as exactly a
`CellBounds` bounding box.

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
