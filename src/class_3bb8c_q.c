/*
 * class_3bb8c_q -- 0x485BC..0x48738, two methods of class D800879C4 (table
 * D_800879C4, 49 slots; tools/classtable.py). D800879C4 is a 0xA8-byte
 * sprite object: a subclass of the sprite class whose table is gSpriteMethods
 * (tag 0x44; GsSPRITE embedded at +0x64, drawn by Unk18Obj__DrawNode through
 * GsSortSprite), which is itself a Class6B5CC subclass. Its ctor and
 * allocator (D800879C4__D800879C4, New_D800879C4) are in class_3bb8c_p.c,
 * its empty leaves and table getter in class_3bb8c_t.c; Class876FC
 * (class_3bb8c_s.c) builds five of them per instance.
 *
 * - D800879C4__SetVariantClut (slot +0x040, tail-called by the ctor with its
 *   arg1): records the variant and points the sprite's CLUT at that
 *   variant's palette row, overriding the one the base init took from the
 *   texture.
 * - D800879C4__UpdateScale (slot +0x048, overrides Class6B5CC__UpdateScale):
 *   the 2-D version, two num/den ratios into GsSPRITE scalex/scaley.
 *
 * Named round 79; tiers in the reports. Game-level role of the sprites is
 * not established.
 */

#include "common.h"

/*
 * This unit's own local view of the object (class_3bb8c_p.c and
 * class_3bb8c_t.c carry theirs, per the multiple-independent-local-views
 * convention). New_D800879C4 allocates 0xA8 bytes; every offset here stays
 * inside.
 */
typedef struct D800879C4Obj D800879C4Obj;
struct D800879C4Obj {
    u8 pad00[0x58];
    /* +0x058..+0x063: written by the base class's slot +0x040 init
     * (Sprite__Reset, asm/psyq_322b4.s: unk58 = 0) and read by
     * D800879C4__UpdateScale below; Unk18Obj__DrawNode's sprite draw path does
     * not read them. What sets unk58 non-zero is not established. */
    s32 unk58;                  /* +0x058, UpdateScale: non-zero selects the scale-in-place path */
    s32 unk5C;                  /* +0x05C, UpdateScale: multiplied by the x ratio when unk58 != 0 */
    s32 unk60;                  /* +0x060, UpdateScale: multiplied by the y ratio when unk58 != 0 */
    /* +0x064: an embedded GsSPRITE (layout confirmed by the base init
     * InitGsSprite, which fills attribute/w/h/tpage/u/v/cx/cy/rgb/mx/my/
     * scalex/scaley/rotate at their GsSPRITE offsets, and by
     * Unk18Obj__DrawNode, which hands self+0x64 to GsSortSprite). Only the
     * members this unit touches are spelled out. */
    u8 pad64[0x74 - 0x64];
    s16 spriteClutX;            /* +0x074, GsSPRITE.cx */
    s16 spriteClutY;            /* +0x076, GsSPRITE.cy */
    u8 pad78[0x80 - 0x78];
    s16 spriteScaleX;           /* +0x080, GsSPRITE.scalex (20.12) */
    s16 spriteScaleY;           /* +0x082, GsSPRITE.scaley (20.12) */
    u8 pad84[0xA0 - 0x84];
    /* +0x0A0: the ctor's arg1, which also picks this object's texture cell
     * (&D_80087A8C[arg1], passed to the base ctor by D800879C4__D800879C4). */
    s32 variant;
};

/*
 * Two parallel lookup tables, 2 entries each, stride 4 bytes (indexed as
 * `variant * 2` of `s16`, i.e. `variant * 4` bytes) even though each holds only
 * a 2-byte element at that stride -- retail takes two SEPARATE %hi/%lo
 * bases (gD800879C4ClutX, gD800879C4ClutY) rather than one struct array, so this is
 * the only spelling that reproduces the two relocations. Values:
 * gD800879C4ClutX = {0x03D0, 0x03E0}, gD800879C4ClutY = {0x01FF, 0x01FF} once read
 * at the real stride (confirmed against asm/data/76DC8.data.s).
 */
extern const s16 gD800879C4ClutX[];
extern const s16 gD800879C4ClutY[];

void D800879C4__SetVariantClut(D800879C4Obj *self, s32 variant) {
    self->variant = variant;
    self->spriteClutX = gD800879C4ClutX[variant * 2];
    self->spriteClutY = gD800879C4ClutY[variant * 2];
}

/*
 * Slot +0x048, overriding Class6B5CC__UpdateScale (tools/classtable.py
 * D_800879C4 --vs gClass6B5CCMethods). `ratios` is two s16 num/den pairs
 * (x at +0x0/+0x2, y at +0x4/+0x6) -- every caller passes one of
 * gSpriteScaleLarge/gSpriteScaleSmall ({6,5}, {4,6}) or a table forwarded
 * from Class876FC__SpawnSprites -- each turned into a 20.12 fixed-point
 * ratio by the split-division idiom RatioToFixed12 uses (code_d294_c.c).
 * Unlike the base method, `set` is never read: every path assigns. Read as
 * a raw `s16 *` per the RatioToFixed12 precedent for this shape.
 */
void D800879C4__UpdateScale(D800879C4Obj *self, s32 set, s16 *ratios) {
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
        self->spriteScaleX = short1;
        self->spriteScaleY = short2;
    }
}
