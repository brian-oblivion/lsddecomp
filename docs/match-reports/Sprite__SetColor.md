# Sprite__SetColor -- MATCHED (8/8 words), round 82

> Renamed from `func_8004229C` on 2026-09-25 (tools/rename.py). Address 0x8004229c.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/graphics/sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x0B8 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and gVariantSpriteMethods (the sprite classes) (`tools/classtable.py`).
- **What:** Copies three bytes from the argument to +0x78..+0x7A (the embedded GsSPRITE r,g,b per `dream_scene.c`). Retail: `lb,lb,lb` then `sb,sb,sb`, then `jr` with an UNFILLED delay slot.
- **Result:** byte-exact; 8/8 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK).
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Lever

Three builds. (1) three `self->r = rgb[0];` statements over `s8` fields: `lbu`/`sb` interleaved with load-delay nops, longer than retail (image shifted). (2) `s8` locals loaded first, then stored: right length pattern but `lbu` not `lb`, `$a1` reused instead of `$a2`, last `sb` in the delay slot (2/8). (3) whole-struct assignment of a 3-byte all-`s8` struct (`typedef struct {s8 r,g,b;} Rgb_322b4; self->rgb = *rgb;`): byte-exact, whole image green.

## Source

```c
/* Slot +0x0B8 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and gVariantSpriteMethods (the
 * sprite classes): copy three bytes into the embedded GsSPRITE's r,g,b. */
void Sprite__SetColor(Sprite *self, ColorRgb *rgb) {
    self->sprite.rgb = *rgb;
}
```

### Proposed learning

A leaf that loads three signed bytes into three registers with `lb` and only then stores them with `sb`, ending `jr $ra; nop` with the delay slot UNFILLED, is a whole-struct assignment of a 3-byte all-`s8` struct (alignment 1: cc1 moves it by pieces, loads first). Separate field assignments give `lbu` interleaved with stores; `s8` locals give `lbu` and a filled delay slot. Sibling of the alignment-2 `lwl/lwr` whole-struct idiom already in DECOMPILATION_LEARNINGS.

## Track 4 (2026-09-25, round 82, alpha)

Renamed from `func_8004229C`: +0x0B8 is Sprite's first own slot (the SceneNode table ends at +0x0B4), named `setColor` for this occupant, which writes GsSPRITE r,g,b. gTextRowMethods overrides it with TextRow__SetColor; StyleEffect__SpawnSprites passes it its colour triples. `Rgb_322b4` became `ColorRgb` (same all-s8 3-byte struct; the lever above still holds). And the class is unified as `Sprite` in `include/sprite.h` (the base sprite class, id 0x44, table `gSpriteMethods`, formerly `D_8006EE1C`); the Source block above is the unified spelling, byte-identical (whole image green, 0 new `-Wall` warnings).

## History: track 12 (round 106, charlie), comments moved out of the source

The API documentation pass moved these comments' process text here,
verbatim; the source keeps a one-line `MATCHING:` note or the API doc.

From `include/draw_system.h`:

```c
/* An r, g, b colour as the game's objects keep and pass it: a sprite's,
 * a box's, the background's, the ambient and flat lights', a viewport's
 * clear and far colours. Three bytes, not Sony's four-byte CVECTOR.
 * MATCHING: byte members give it size 3 and alignment 1, so a whole-struct
 * copy, which is how its users copy it, is three lb then three sb. */
typedef struct ColorRgb {
    u8 r, g, b;
} ColorRgb;
```

From `include/sprite.h`:

```c
/* libgs GsSPRITE, 0x24 bytes, field for field, with r,g,b as one ColorRgb.
 * Kept local rather than Sony's GsSPRITE: Sprite__SetColor copies the colour
 * as one ColorRgb (the whole-struct copy is what matches), and
 * GetSetBitField takes `attribute` as a u32 *, where Sony's is unsigned long;
 * Sony's type would cost a cast at each of those four sites to save the two
 * in Viewport__DrawNode. */
```
