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

None beyond what's already documented for `Class866E8` in `Class86668__SetChildFlag8.md`.
