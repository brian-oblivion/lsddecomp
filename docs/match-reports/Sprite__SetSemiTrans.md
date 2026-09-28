# Sprite__SetSemiTrans -- MATCHED (11/11 words), round 82

> Renamed from `func_8004223C` on 2026-09-25 (tools/rename.py). Address 0x8004223c.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/graphics/sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x064 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods, gTextRowMethods and gVariantSpriteMethods (`tools/classtable.py`).
- **What:** `GetSetBitField(&self->sprite.attribute, 0x1E, 1, a1 != 0)` over the GsSPRITE attribute at +0x064; same shape as `BoxFill__SetSemiTrans` in `screen_widgets.c` and the `scene_node.c` +0x10 family. `GetSetBitField` prototype copied locally from `include/scene_node.h`.
- **Result:** byte-exact; 11/11 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* Sprite classes slot +0x064: attribute bit 30. */
s32 Sprite__SetSemiTrans(Sprite *self, s32 a1) {
    return GetSetBitField(&self->sprite.attribute, 0x1E, 1, a1 != 0);
}
```

## Track 4 (2026-09-25, round 82, alpha)

Renamed from `func_8004223C` for its slot (+0x064 `setSemiTrans`): attribute bit 30 is GsALON, the same bit SceneNode__SetSemiTrans sets on the GsDOBJ2. And the class is unified as `Sprite` in `include/sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).

## Track 7 (round 99, charlie)

Bit position `0x1E` -> `SPRITE_ATTR_ALON_SHIFT` (30); `a1` -> `on`. Byte-exact.
