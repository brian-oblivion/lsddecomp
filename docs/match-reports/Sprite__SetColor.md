# Sprite__SetColor -- MATCHED (8/8 words), round 82

> Renamed from `func_8004229C` on 2026-09-25 (tools/rename.py). Address 0x8004229c.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x0B8 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and gVariantSpriteMethods (the sprite classes) (`tools/classtable.py`).
- **What:** Copies three bytes from the argument to +0x78..+0x7A (the embedded GsSPRITE r,g,b per `class_3bb8c_k.c`). Retail: `lb,lb,lb` then `sb,sb,sb`, then `jr` with an UNFILLED delay slot.
- **Result:** byte-exact; 8/8 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK).
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Lever

Three builds. (1) three `self->r = rgb[0];` statements over `s8` fields: `lbu`/`sb` interleaved with load-delay nops, longer than retail (image shifted). (2) `s8` locals loaded first, then stored: right length pattern but `lbu` not `lb`, `$a1` reused instead of `$a2`, last `sb` in the delay slot (2/8). (3) whole-struct assignment of a 3-byte all-`s8` struct (`typedef struct {s8 r,g,b;} Rgb_322b4; self->rgb = *rgb;`): byte-exact, whole image green.

## Source

```c
/* Slot +0x0B8 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and gVariantSpriteMethods (the
 * sprite classes): copy three bytes into the embedded GsSPRITE's r,g,b. */
void Sprite__SetColor(Sprite *self, SpriteRgb *rgb) {
    self->sprite.rgb = *rgb;
}
```

### Proposed learning

A leaf that loads three signed bytes into three registers with `lb` and only then stores them with `sb`, ending `jr $ra; nop` with the delay slot UNFILLED, is a whole-struct assignment of a 3-byte all-`s8` struct (alignment 1: cc1 moves it by pieces, loads first). Separate field assignments give `lbu` interleaved with stores; `s8` locals give `lbu` and a filled delay slot. Sibling of the alignment-2 `lwl/lwr` whole-struct idiom already in DECOMPILATION_LEARNINGS.

## Track 4 (2026-09-25, round 82, alpha)

Renamed from `func_8004229C`: +0x0B8 is Sprite's first own slot (the SceneNode table ends at +0x0B4), named `setColor` for this occupant, which writes GsSPRITE r,g,b. gTextRowMethods overrides it with TextRow__SetColor; StyleEffect__SpawnSprites passes it its colour triples. `Rgb_322b4` became `SpriteRgb` (same all-s8 3-byte struct; the lever above still holds). And the class is unified as `Sprite` in `include/Sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).
