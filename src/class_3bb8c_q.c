/*
 * class_3bb8c_q -- two methods of VariantSprite (include/VariantSprite.h), a
 * Sprite whose variant, 0 or 1, picks its texture cell and CLUT. The ctor
 * and allocator are in class_3bb8c_p.c, the empty leaves and the table
 * getter in class_3bb8c_t.c.
 *
 * - VariantSprite__SetVariantClut (reset, +0x040, called last by the ctor
 *   with the variant): records the variant and points the GsSPRITE's CLUT
 *   at that variant's row, replacing the one Sprite's reset took from the
 *   texture.
 * - VariantSprite__UpdateScale (updateScale, +0x048): two num/den ratios
 *   into GsSPRITE scalex/scaley.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "VariantSprite.h"

/*
 * The two variants' CLUT positions, one {x, y} table in VRAM:
 * {976, 511} and {992, 511}, adjacent 16-colour rows on the bottom line.
 * MATCHING: two externs, as retail takes one %hi/%lo base for x, one for y.
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
 * Overrides SceneNode__UpdateScale. `ratios` is two num/den pairs, x then y
 * (the callers' tables hold three, gSpriteScaleLarge's {6,5} and
 * gSpriteScaleSmall's {4,6}; the third is not read), each turned into 20.12
 * by the split division RatioToFixed12 uses. The ratios go to the GsSPRITE's
 * scalex/scaley, or, while Sprite's unk58 is set, multiply accumScaleX/Y
 * instead. `set` is not read.
 */
void VariantSprite__UpdateScale(VariantSprite *self, s32 set, Ratio16 *ratios) {
    s32 xWhole, xRem, xFrac, xRatio;
    s32 yWhole, yRem, yFrac, yRatio;
    s16 xScale, yScale;

    xWhole = ratios[0].num / ratios[0].den;
    xRem = ratios[0].num % ratios[0].den;
    xFrac = (xRem << FIX12_SHIFT) / ratios[0].den;
    xRatio = (xWhole << FIX12_SHIFT) + xFrac;
    xScale = (s16)xRatio; /* MATCHING: here; in the else arm it loses a move */

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
