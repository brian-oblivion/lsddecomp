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

void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant) {
    self->variant = variant;
    self->sprite.cx = gVariantSpriteClutX[variant * 2];
    self->sprite.cy = gVariantSpriteClutY[variant * 2];
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
void VariantSprite__UpdateScale(VariantSprite *self, s32 set, s16 *ratios) {
    s32 q1, r1, q2, ratio1;
    s32 q3, r3, q4, ratio2;
    s16 short1, short2;

    q1 = ratios[0] / ratios[1];
    r1 = ratios[0] % ratios[1];
    q2 = (r1 << 12) / ratios[1];
    ratio1 = (q1 << 12) + q2;
    short1 = (s16)ratio1;

    q3 = ratios[2] / ratios[3];
    r3 = ratios[2] % ratios[3];
    q4 = (r3 << 12) / ratios[3];
    ratio2 = (q3 << 12) + q4;
    short2 = (s16)ratio2;

    if (self->unk58 != 0) {
        self->unk5C = ((s16)ratio1 * self->unk5C) >> 12;
        self->unk60 = ((s16)ratio2 * self->unk60) >> 12;
    } else {
        self->sprite.scalex = short1;
        self->sprite.scaley = short2;
    }
}
