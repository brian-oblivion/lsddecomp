# SceneNode__SetLighting

> Renamed from `Class6B5CC__SetLighting` on 2026-09-26 (tools/rename.py). Address 0x8001d3cc.

> Renamed from `func_8001D3CC` on 2026-09-23 (tools/rename.py). Address 0x8001d3cc.

**Unit:** code_d294 · **Size:** 11 words · **Status:** MATCHED (11/11 words)

## What it does

`SceneNode` vtable slot `+0x06C`. Sets bit 6 (a 1-bit field) of
`self->unk10` to `(a1 == 0)`, tail-returning `GetSetBitField`'s result. See
`SceneNode__SetDisplay.md` for the shared `GetSetBitField` background.

## The C

```c
u32 SceneNode__SetLighting(SceneNodeObj *self, s32 a1) {
    return GetSetBitField(&self->unk10, 6, 1, a1 == 0);
}
```

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build (part of the five-function bitfield-setter
group; see `SceneNode__SetDisplay.md`).

## Naming

Round 71 (alpha). `func_8001D3CC` -> `SceneNode__SetLighting`, **tier A**. Table slot +0x06C. Sets attribute bit 6, GsLOFF, to on == 0 (lighting on when on != 0) and returns the old LOFF bit (not inverted, unlike SetDisplay). No caller dispatches this slot by name yet.

## Round 101 (delta): track 7

Step 3 (locals and parameters): `a1` -> `on` (written inverted into GsLOFF, so nonzero means lit). Byte-identical.

Step 5 (comments): Function comment added (GsLOFF inverted on write).
