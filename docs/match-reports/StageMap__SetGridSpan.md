# StageMap__SetGridSpan

> Renamed from `Class866E8__SetGridSpan` on 2026-09-26 (tools/rename.py). Address 0x8004b32c.

> Renamed from `func_8004B32C` on 2026-09-22 (tools/rename.py). Address 0x8004b32c.

**Unit:** class_39e08 · **Size:** 6 words · **Status:** MATCHED (6/6 words)

## What it does

`StageMap`'s slot +0x0DC setter: stores its argument raw into
`self->unk74`, and also splits it into two halfword fields via arithmetic
shifts (`>>11` and `>>12`).

## Derivation

```
sra $v0, $a1, 11      ; v0 = arg1 >> 11
sw  $a1, 0x74($a0)    ; self->unk74 = arg1 (raw)
sra $a1, $a1, 12       ; a1 = arg1 >> 12 (overwrites the incoming register)
sh  $v0, 0x7A($a0)    ; self->unk7A = (s16)(arg1 >> 11)
jr  $ra
 sh $a1, 0x78($a0)     ; self->unk78 = (s16)(arg1 >> 12)
```

`sra` (arithmetic, sign-preserving) on both shifts, so `arg1` is signed
(`s32`); the two halfword fields are `s16`. Written as three straight-line C
statements (`unk74 = arg1; unk7A = arg1 >> 11; unk78 = arg1 >> 12;`); the
compiler's own scheduler reordered/interleaved them into the retail
instruction order without needing the C to mirror it.

## Proposed learning

None beyond what's already documented for `StageMap` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass). **This is the function that established the
class's grid geometry**, so the arithmetic is written out here in full and the
other reports point at it.

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B32C` | `StageMap__SetGridSpan` | B | Occupant of vtable slot `+0x0DC`. Stores its argument raw and derives two halfwords from it by arithmetic shift. Its only caller is `StageMap__Reset`, which passes `gDefaultGridSpan` = `0x0000A000`. |

The three numbers, and why they are not a guess:

- `gridCells = span >> 11` = `0xA000 >> 11` = **20**.
- `gridHalfCells = span >> 12` = `0xA000 >> 12` = **10**.
- The constructor places grid cells `0x800` apart on both axes, and
  `0xA000 / 0x800` = **20**.
- `class_3bb8c_b`'s `StageMap__SetFootprintVisible` is BYTE-MATCHED and indexes the same cell
  block with a row stride of **20** (`e->unk10 + slot->h4 + slot->h6 * 20`,
  and `cell += 20 - slot->h8` at each row's end).
- `StageMap__SetFootprintRect` treats `0x13` = 19 as the last valid column
  and row.

Four independent sightings of the same 20. `>> 11` is therefore
"span divided by the cell size", and the shift is a division by `0x800`, not
an arbitrary bit slice. That is what the names record.

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `StageMap+0x074` | `gridSpan` | B | Stored raw here; `class_39e08`'s `StageMap__ComputeFootprintFromRotation` copies it into a matrix-query template. The world-space extent reading is from the arithmetic above. |
| `StageMap+0x078` | `gridHalfCells` | B | `span >> 12`, half of `gridCells`. |
| `StageMap+0x07A` | `gridCells` | B | `span >> 11`; equals the byte-verified row stride. |

## Proposed field names

Cross-unit, for the head to apply by type scope. These are the SAME three
offsets on `Obj866E8` in `include/class_3bb8c.h` (a different unit's header,
so not mine to edit):

| struct | field | proposed | tier | evidence |
| --- | --- | --- | --- | --- |
| `Obj866E8` | `unk74` | `gridSpan` | B | as above |
| `Obj866E8` | `unk78` | `gridHalfCells` | B | as above |
| `Obj866E8` | `unk7A` | `gridCells` | B | as above |

Also posted to the round broadcast. Nothing in THIS unit's build depends on
them; they are offered because `class_39e08` reads all three and currently has
only offset names for them.

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

## Track 7 (2026-09-27, round 98, charlie)

`span >> 11` / `>> 12` -> `STAGE_CELL_SHIFT` / `STAGE_CELL_SHIFT + 1` (include/StageMap.h): the span in cells, and half that. Zero bytes.
