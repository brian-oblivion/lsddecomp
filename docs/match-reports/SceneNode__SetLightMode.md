# SceneNode__SetLightMode

> Renamed from `Class6B5CC__SetLightMode` on 2026-09-26 (tools/rename.py). Address 0x8001d3f8.

> Renamed from `func_8001D3F8` on 2026-09-23 (tools/rename.py). Address 0x8001d3f8.

**Unit:** code_d294 · **Size:** 11 words · **Status:** MATCHED (11/11 words)

## What it does

`SceneNode` vtable slot `+0x070`. Sets a 3-bit field at bit 3 of
`self->unk10` to `a1` (passed through unmodified), tail-returning
`GetSetBitField`'s result. See `SceneNode__SetDisplay.md` for the shared
`GetSetBitField` background.

## The C

```c
u32 SceneNode__SetLightMode(SceneNodeObj *self, u32 a1) {
    return GetSetBitField(&self->unk10, 3, 3, a1);
}
```

Together with its four siblings, this establishes `self->unk10` as a
packed flags/small-fields register with (at least) five non-overlapping
bit ranges: `[3,6)` (3 bits, this function), bit 6 (1 bit,
`SceneNode__SetLighting`), `[28,30)` (2 bits, `SceneNode__SetSemiTransRate`), bit 30 (1 bit,
`SceneNode__SetSemiTrans`), bit 31 (1 bit, `SceneNode__SetDisplay`).

## Provenance

round 11 (2026-09-03), runner charlie, unit code_d294 (fresh carve, first attempt).
Matched on the first build (part of the five-function bitfield-setter
group; see `SceneNode__SetDisplay.md`).

## Naming

Round 71 (alpha). `func_8001D3F8` -> `SceneNode__SetLightMode`, **tier B**. Table slot +0x070. Writes the 3-bit field at attribute bits 3-5, which LIBGS.H defines bit by bit as GsFOG, GsMATE and GsLLMOD, and returns the old value. Tier B: the field is the light-mode group by bit position, but no caller in src/ shows which values the game writes.
