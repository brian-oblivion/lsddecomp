# Class6B5CC__SetSemiTransRate

> Renamed from `func_8001D3A0` on 2026-09-23 (tools/rename.py). Address 0x8001d3a0.

**Unit:** code_d294 · **Size:** 11 words · **Status:** MATCHED (11/11 words)

## What it does

`Class6B5CC` vtable slot `+0x068`. Sets a 2-bit field at bit 28 of
`self->unk10` to `a1` (passed through unmodified — no boolean coercion,
unlike its `Class6B5CC__SetDisplay`/`Class6B5CC__SetLighting` siblings), tail-returning
`GetSetBitField`'s result. See `Class6B5CC__SetDisplay.md` for the shared
`GetSetBitField` background.

## The C

```c
u32 Class6B5CC__SetSemiTransRate(Class6B5CCObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 0x1C, 2, a1);
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build (part of the five-function bitfield-setter
group; see `Class6B5CC__SetDisplay.md`).
