# Class866E8__SetGridSpan

> Renamed from `func_8004B32C` on 2026-09-22 (tools/rename.py). Address 0x8004b32c.

**Unit:** class_3ac78 · **Size:** 6 words · **Status:** MATCHED (6/6 words)

## What it does

`Class866E8`'s slot +0x0DC setter: stores its argument raw into
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

None beyond what's already documented for `Class866E8` in `TimedTask__PlaySound.md`.

## Naming

Round 67 (track 3, naming pass). **This is the function that established the
class's grid geometry**, so the arithmetic is written out here in full and the
other reports point at it.

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B32C` | `Class866E8__SetGridSpan` | B | Occupant of vtable slot `+0x0DC`. Stores its argument raw and derives two halfwords from it by arithmetic shift. Its only caller is `Class866E8__Reset`, which passes `gDefaultGridSpan` = `0x0000A000`. |

The three numbers, and why they are not a guess:

- `gridCells = span >> 11` = `0xA000 >> 11` = **20**.
- `gridHalfCells = span >> 12` = `0xA000 >> 12` = **10**.
- The constructor places grid cells `0x800` apart on both axes, and
  `0xA000 / 0x800` = **20**.
- `class_3bb8c_b`'s `Class866E8__SetFootprintCellFlag` is BYTE-MATCHED and indexes the same cell
  block with a row stride of **20** (`e->unk10 + slot->h4 + slot->h6 * 20`,
  and `cell += 20 - slot->h8` at each row's end).
- `Class866E8__SetFootprintRect` treats `0x13` = 19 as the last valid column
  and row.

Four independent sightings of the same 20. `>> 11` is therefore
"span divided by the cell size", and the shift is a division by `0x800`, not
an arbitrary bit slice. That is what the names record.

| field | name | tier | evidence |
| --- | --- | --- | --- |
| `Class866E8+0x074` | `gridSpan` | B | Stored raw here; `class_3bb8c`'s `Class866E8__ComputeFootprintFromRotation` copies it into a matrix-query template. The world-space extent reading is from the arithmetic above. |
| `Class866E8+0x078` | `gridHalfCells` | B | `span >> 12`, half of `gridCells`. |
| `Class866E8+0x07A` | `gridCells` | B | `span >> 11`; equals the byte-verified row stride. |

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
them; they are offered because `class_3bb8c` reads all three and currently has
only offset names for them.
