# func_8001D3CC

**Unit:** code_d294 · **Size:** 11 words · **Status:** MATCHED (11/11 words)

## What it does

`Class6B5CC` vtable slot `+0x06C`. Sets bit 6 (a 1-bit field) of
`self->unk10` to `(a1 == 0)`, tail-returning `func_8001EDAC`'s result. See
`func_8001D344.md` for the shared `func_8001EDAC` background.

## The C

```c
u32 func_8001D3CC(Class6B5CCObj *self, s32 a1) {
    return func_8001EDAC(&self->unk10, 6, 1, a1 == 0);
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build (part of the five-function bitfield-setter
group; see `func_8001D344.md`).
