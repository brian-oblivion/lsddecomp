# Class6B5CC__SetLighting

> Renamed from `func_8001D3CC` on 2026-09-23 (tools/rename.py). Address 0x8001d3cc.

**Unit:** code_d294 · **Size:** 11 words · **Status:** MATCHED (11/11 words)

## What it does

`Class6B5CC` vtable slot `+0x06C`. Sets bit 6 (a 1-bit field) of
`self->unk10` to `(a1 == 0)`, tail-returning `GetSetBitField`'s result. See
`Class6B5CC__SetDisplay.md` for the shared `GetSetBitField` background.

## The C

```c
u32 Class6B5CC__SetLighting(Class6B5CCObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 6, 1, a1 == 0);
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build (part of the five-function bitfield-setter
group; see `Class6B5CC__SetDisplay.md`).
