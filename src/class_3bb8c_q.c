/*
 * class_3bb8c_q -- 0x485BC..0x48738, two methods of Class879C4 (table
 * gClass879C4Methods, 49 slots; include/Class879C4.h). Class879C4 is a 0xA8-byte
 * sprite object: a subclass of the sprite class whose table is gSpriteMethods
 * (tag 0x44; GsSPRITE embedded at +0x64, drawn by Viewport__DrawNode through
 * GsSortSprite), which is itself a SceneNode subclass. Its ctor and
 * allocator (Class879C4__Class879C4, New_Class879C4) are in class_3bb8c_p.c,
 * its empty leaves and table getter in class_3bb8c_t.c; Class876FC
 * (class_3bb8c_s.c) builds five of them per instance.
 *
 * - Class879C4__SetVariantClut (slot +0x040, tail-called by the ctor with its
 *   arg1): records the variant and points the sprite's CLUT at that
 *   variant's palette row, overriding the one the base init took from the
 *   texture.
 * - Class879C4__UpdateScale (slot +0x048, overrides SceneNode__UpdateScale):
 *   the 2-D version, two num/den ratios into GsSPRITE scalex/scaley.
 *
 * Named round 79; tiers in the reports. Game-level role of the sprites is
 * not established.
 */

#include "common.h"
#include "Class879C4.h"

/*
 * Two parallel lookup tables, 2 entries each, stride 4 bytes (indexed as
 * `variant * 2` of `s16`, i.e. `variant * 4` bytes) even though each holds only
 * a 2-byte element at that stride -- retail takes two SEPARATE %hi/%lo
 * bases (gClass879C4ClutX, gClass879C4ClutY) rather than one struct array, so this is
 * the only spelling that reproduces the two relocations. Values:
 * gClass879C4ClutX = {0x03D0, 0x03E0}, gClass879C4ClutY = {0x01FF, 0x01FF} once read
 * at the real stride (confirmed against asm/data/76DC8.data.s).
 */
extern const s16 gClass879C4ClutX[];
extern const s16 gClass879C4ClutY[];

void Class879C4__SetVariantClut(Class879C4 *self, s32 variant) {
    self->variant = variant;
    self->sprite.cx = gClass879C4ClutX[variant * 2];
    self->sprite.cy = gClass879C4ClutY[variant * 2];
}

/*
 * Slot +0x048, overriding SceneNode__UpdateScale (tools/classtable.py
 * gClass879C4Methods --vs gSceneNodeMethods). `ratios` is two s16 num/den pairs
 * (x at +0x0/+0x2, y at +0x4/+0x6) -- every caller passes one of
 * gSpriteScaleLarge/gSpriteScaleSmall ({6,5}, {4,6}) or a table forwarded
 * from Class876FC__SpawnSprites -- each turned into a 20.12 fixed-point
 * ratio by the split-division idiom RatioToFixed12 uses (code_d294_c.c).
 * Unlike the base method, `set` is never read: every path assigns. Read as
 * a raw `s16 *` per the RatioToFixed12 precedent for this shape.
 */
void Class879C4__UpdateScale(Class879C4 *self, s32 set, s16 *ratios) {
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
