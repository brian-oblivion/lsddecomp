# SceneNode__SetSemiTransRate

> Renamed from `Class6B5CC__SetSemiTransRate` on 2026-09-26 (tools/rename.py). Address 0x8001d3a0.

> Renamed from `func_8001D3A0` on 2026-09-23 (tools/rename.py). Address 0x8001d3a0.

**Unit:** code_d294 · **Size:** 11 words · **Status:** MATCHED (11/11 words)

## What it does

`SceneNode` vtable slot `+0x068`. Sets a 2-bit field at bit 28 of
`self->unk10` to `a1` (passed through unmodified — no boolean coercion,
unlike its `SceneNode__SetDisplay`/`SceneNode__SetLighting` siblings), tail-returning
`GetSetBitField`'s result. See `SceneNode__SetDisplay.md` for the shared
`GetSetBitField` background.

## The C

```c
u32 SceneNode__SetSemiTransRate(SceneNodeObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 0x1C, 2, a1);
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build (part of the five-function bitfield-setter
group; see `SceneNode__SetDisplay.md`).

## Naming

Round 71 (alpha). `func_8001D3A0` -> `SceneNode__SetSemiTransRate`, **tier A**. Table slot +0x068. Writes the 2-bit field at attribute bits 28-29 (GsAZERO..GsATHREE, the semi-transparency rate) and returns the old value. class_3bb8c_s calls the slot setSemiTransRate.

## Round 101 (delta): track 7

Step 3 (locals and parameters): `a1` -> `rate` (the 2-bit GsAZERO..GsATHREE field). Byte-identical.
