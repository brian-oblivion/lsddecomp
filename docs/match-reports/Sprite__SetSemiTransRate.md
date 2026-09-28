# Sprite__SetSemiTransRate -- MATCHED (11/11 words), round 82

> Renamed from `func_80042268` on 2026-09-25 (tools/rename.py). Address 0x80042268.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/graphics/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x068 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods, gTextRowMethods and gVariantSpriteMethods (`tools/classtable.py`).
- **What:** `GetSetBitField(&self->sprite.attribute, 0x1C, 2, a1)` (bits 28..29 of the GsSPRITE attribute).
- **Result:** byte-exact; 11/11 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* Sprite classes slot +0x068: attribute bits 28..29. */
s32 Sprite__SetSemiTransRate(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1C, 2, a1);
}
```

## Track 4 (2026-09-25, round 82, alpha)

Renamed from `func_80042268` for its slot (+0x068 `setSemiTransRate`): attribute bits 28-29 are the libgs semitransparency rate, as in SceneNode__SetSemiTransRate. And the class is unified as `Sprite` in `include/Sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).

## Track 7 (round 99, charlie)

Bit position `0x1C` -> `SPRITE_ATTR_RATE_SHIFT` (28); `a1` -> `rate`. Byte-exact.
