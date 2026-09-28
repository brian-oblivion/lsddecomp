/*
 * class_3bb8c_q -- two methods of VariantSprite (include/VariantSprite.h), a
 * Sprite whose variant, 0 or 1, picks its texture cell and CLUT. The ctor
 * and allocator are in class_3bb8c_p.c, the empty leaves and table getter
 * in class_3bb8c_t.c.
 *
 * - VariantSprite__SetVariantClut (reset, +0x040, called last by the ctor
 *   with the variant): records the variant and points the GsSPRITE's CLUT
 *   at that variant's row, replacing the one Sprite's reset took from the
 *   texture.
 * - VariantSprite__UpdateScale (+0x048): two num/den ratios into GsSPRITE
 *   scalex/scaley.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "VariantSprite.h"

/*
 * Two parallel lookup tables, 2 entries each, stride 4 bytes (indexed as
 * `variant * 2` of `s16`, i.e. `variant * 4` bytes) even though each holds only
 * a 2-byte element at that stride -- retail takes two SEPARATE %hi/%lo
 * bases (gVariantSpriteClutX, gVariantSpriteClutY) rather than one struct array, so this is
 * the only spelling that reproduces the two relocations. Values:
 * gVariantSpriteClutX = {0x03D0, 0x03E0}, gVariantSpriteClutY = {0x01FF, 0x01FF} once read
 * at the real stride (confirmed against asm/data/76DC8.data.s).
 */
extern const s16 gVariantSpriteClutX[];
extern const s16 gVariantSpriteClutY[];

/* One {x, y} entry of that table, in s16s: the stride both lookups index by. */
#define VARIANT_CLUT_STRIDE 2

void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant) {
    self->variant = variant;
    self->sprite.cx = gVariantSpriteClutX[variant * VARIANT_CLUT_STRIDE];
    self->sprite.cy = gVariantSpriteClutY[variant * VARIANT_CLUT_STRIDE];
}

/*
 * Slot +0x048, overriding SceneNode__UpdateScale (tools/classtable.py
 * gVariantSpriteMethods --vs gSceneNodeMethods). `ratios` is two s16 num/den pairs
 * (x at +0x0/+0x2, y at +0x4/+0x6) -- every caller passes one of
 * gSpriteScaleLarge/gSpriteScaleSmall ({6,5}, {4,6}) or a table forwarded
 * from StyleEffect__SpawnSprites -- each turned into a 20.12 fixed-point
 * ratio by the split-division idiom RatioToFixed12 uses (code_d294_c.c).
 * Unlike the base method, `set` is never read: every path assigns. Read as
 * a raw `s16 *` per the RatioToFixed12 precedent for this shape.
 */
void VariantSprite__UpdateScale(VariantSprite *self, s32 set, Ratio16 *ratios) {
    s32 xWhole, xRem, xFrac, xRatio;
    s32 yWhole, yRem, yFrac, yRatio;
    s16 xScale, yScale;

    xWhole = ratios[0].num / ratios[0].den;
    xRem = ratios[0].num % ratios[0].den;
    xFrac = (xRem << FIX12_SHIFT) / ratios[0].den;
    xRatio = (xWhole << FIX12_SHIFT) + xFrac;
    xScale = (s16)xRatio;

    yWhole = ratios[1].num / ratios[1].den;
    yRem = ratios[1].num % ratios[1].den;
    yFrac = (yRem << FIX12_SHIFT) / ratios[1].den;
    yRatio = (yWhole << FIX12_SHIFT) + yFrac;
    yScale = (s16)yRatio;

    if (self->unk58 != 0) {
        self->accumScaleX = ((s16)xRatio * self->accumScaleX) >> FIX12_SHIFT;
        self->accumScaleY = ((s16)yRatio * self->accumScaleY) >> FIX12_SHIFT;
    } else {
        self->sprite.scalex = xScale;
        self->sprite.scaley = yScale;
    }
}
