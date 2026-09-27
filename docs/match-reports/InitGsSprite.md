# InitGsSprite -- MATCHED (57/57 words), round 82

Round 82, runner alpha (fifth slot on Sprite). Unit `src/Sprite.c`. Fresh ground, no prior body attempt.

- **Where:** not in any method table (called by Sprite__Reset) (`tools/classtable.py`).
- **What:** Fills the GsSPRITE: attribute = (pmode & 3) << 24 (colour mode), x = y = 0, w/h from the cell, mx/my = w/2, h/2 read back from the sprite, tpage = GetTPage(pmode & 3, abr, px, py), u/v the cell origin's low bytes, cx/cy from the image (the CLUT position), r = g = b = 0x80, rotate 0, scalex = scaley = 0x1000.
- **Result:** byte-exact; 57/57 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK).
- **Types:** `struct GsIMAGE` is completed in the unit with code_2bb9c.c's layout (Sprite.h keeps only the tag; LIBGS.H is not included here); `GetTPage` declared locally. Sprite.h untouched.

## Lever

Two levers, three builds. (1) Retail loads `pmode` once, into `$a0`, and keeps the masked value there for GetTPage; writing `image->pmode & 3` twice reloads it after the stores (the stores through `sprite` may alias `image`), 1 word long. A local `s32 mode = image->pmode & 3;` fixes it. (2) Retail's grey is `li v0,0x80`; `SpriteRgb` is all-s8 (Sprite.h, kept for Sprite__SetColor's lb/sb copy), so a constant 0x80 stored to it folds to `li v0,-0x80`, and the chained `r = g = b = 0x80` does the same. An `s32 grey = 0x80;` local stored three times (b, g, r, matching retail's store order) keeps the full-width constant.

## Source

```c
/* Fill a GsSPRITE from a texture image and a cell: colour mode and tpage
 * from the image, size and u,v from the cell, the pivot at its centre,
 * neutral colour, scale 1.0 and no rotation. */
/* Fill a GsSPRITE from a texture image and a cell: colour mode and tpage
 * from the image, size and u,v from the cell, the pivot at its centre,
 * neutral colour, scale 1.0 and no rotation. */
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, struct GsIMAGE *image) {
    s32 mode = image->pmode & 3;
    s32 grey = 0x80;

    sprite->attribute = mode << 24;
    sprite->x = 0;
    sprite->y = 0;
    sprite->w = rect->w;
    sprite->h = rect->h;
    sprite->mx = sprite->w >> 1;
    sprite->my = sprite->h >> 1;
    sprite->tpage = GetTPage(mode, abr, image->px, image->py);
    sprite->u = rect->u;
    sprite->v = rect->v;
    sprite->cx = image->cx;
    sprite->cy = image->cy;
    sprite->rgb.b = grey;
    sprite->rgb.g = grey;
    sprite->rgb.r = grey;
    sprite->rotate = 0;
    sprite->scalex = 0x1000;
    sprite->scaley = 0x1000;
}
```

### Proposed learning

A byte store of a constant >= 0x80 into an s8 field compiles to `li reg,-0x80..`; retail's `li reg,0x80` is the value held in an int-typed local and stored through the narrow field (the conversion happens at the store, not at the constant).

## Round 95 (alpha, track 6: Sony headers)

Its prototype (include/Sprite.h) still spells the parameter `struct GsIMAGE *`, an incomplete tag now that TimImage's GsIMAGE is Sony's anonymous typedef; the body reads through a `GsIMAGE *tim = (GsIMAGE *)image` local (was `image->`). Interim until Sprite.h takes `GsIMAGE *` (see Sprite__Reset). Byte-identical.

Round 96 (alpha, track 6). The parameter is Sony's `GsIMAGE *tim` in the
prototype (include/Sprite.h) and the definition; the `GsIMAGE *tim =
(GsIMAGE *)image` local is gone and the body reads `tim->` directly.
Byte-identical. `sprite` stays the local SpriteGs rather than Sony's
GsSPRITE: Sprite__SetColor's whole-struct copy needs `rgb` as one SpriteRgb,
and GetSetBitField takes `attribute` as u32 * (Sony's is unsigned long), so
Sony's type would add four casts to save two (Viewport__DrawNode).

## Track 7 (round 99, charlie)

`& 3` -> `& 0x3` (a mask); `mode << 24` -> `mode << SPRITE_ATTR_MODE_SHIFT`; `0x80` -> `SPRITE_RGB_NEUTRAL` (128, decimal: a colour); `0x1000` -> Sony's `ONE` (scale 1.0 in 4.12). The shifts and the grey are defined in include/Sprite.h beside SpriteGs. Byte-exact.
