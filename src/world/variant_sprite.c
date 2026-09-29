/*
 * VariantSprite's methods (include/variant_sprite.h: a Sprite whose variant,
 * 0 or 1, picks its texture cell and CLUT; StyleEffect's sprites), in ROM
 * order: allocator, ctor, SetVariantClut, UpdateScale, the four empty leaves
 * and the table getter GetVariantSpriteMethods.
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
#include "variant_sprite.h"
#include "bmem_pmgr.h"

/* One {x, y} entry of sVariantSpriteClutX/Y's table, in s16s: the stride
 * both lookups index by. */
#define VARIANT_CLUT_STRIDE 2

VariantSprite *New_VariantSprite(s32 variant, void *resetArg, void *texture) {
    void *obj = BMemPMgrAlloc(sizeof(VariantSprite));
    if (obj != NULL) {
        GetVariantSpriteMethods()->ctor(obj, variant, resetArg, texture);
        return obj;
    }
    return NULL;
}

/* VariantSprite's two texture cells, forwarded as the Sprite ctor's `rect`
 * (Sprite__Reset copies it into Sprite.rect): u,v = (0x00,0x20) and
 * (0x10,0x20), 16x16. */
extern SpriteRect sVariantSpriteCells[2];

/* Sprite's ctor with the variant's cell, then this class's table, and the
 * reset slot (VariantSprite__SetVariantClut) with the variant, through
 * VariantSpriteResetFn. Returns nothing: it ends in that call. */
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *resetArg, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &sVariantSpriteCells[variant], resetArg, 0);
    self->methods = GetVariantSpriteMethods();
    self->unkA4 = 0;
    ((VariantSpriteResetFn)self->methods->reset)(self, variant);
}

/* The two variants' CLUT positions, one {x, y} table in VRAM:
 * {976, 511} and {992, 511}, adjacent 16-colour rows on the bottom line. */
/* MATCHING: two externs, one address base for x and one for y. */
extern const s16 sVariantSpriteClutX[];
extern const s16 sVariantSpriteClutY[];

void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant) {
    self->variant = variant;
    self->sprite.cx = sVariantSpriteClutX[variant * VARIANT_CLUT_STRIDE];
    self->sprite.cy = sVariantSpriteClutY[variant * VARIANT_CLUT_STRIDE];
}

/*
 * Overrides SceneNode__UpdateScale. `ratios` is two num/den pairs, x then y
 * (the callers' tables hold three, sSpriteScaleLarge's {6,5} and
 * sSpriteScaleSmall's {4,6}; the third is not read), each turned into 20.12
 * by the split division RatioToFixed12 uses. The ratios go to the GsSPRITE's
 * scalex/scaley, or, while Sprite's accumulateScale is set, multiply accumScaleX/Y
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
    xScale = (s16)xRatio; /* MATCHING: truncated here; in the else arm it is a word shorter */

    yWhole = ratios[1].num / ratios[1].den;
    yRem = ratios[1].num % ratios[1].den;
    yFrac = (yRem << FIX12_SHIFT) / ratios[1].den;
    yRatio = (yWhole << FIX12_SHIFT) + yFrac;
    yScale = (s16)yRatio;

    if (self->accumulateScale != 0) {
        self->accumScaleX = ((s16)xRatio * self->accumScaleX) >> FIX12_SHIFT;
        self->accumScaleY = ((s16)yRatio * self->accumScaleY) >> FIX12_SHIFT;
    } else {
        self->sprite.scalex = xScale;
        self->sprite.scaley = yScale;
    }
}

/* The four empty leaves. VariantSprite__Update is the +0x098 update override of
 * Sprite__Update, typed as that slot; the other three occupy the class's own
 * slots +0x0BC/+0x0C0/+0x0C4, which nothing calls. */
void VariantSprite__Update(VariantSprite *self, void *sender, s32 event) {}

void VariantSprite__NoOpSlotBC(void) {}

void VariantSprite__NoOpSlotC0(void) {}

void VariantSprite__NoOpSlotC4(void) {}

VariantSpriteMethods *GetVariantSpriteMethods(void) {
    return &gVariantSpriteMethods;
}
