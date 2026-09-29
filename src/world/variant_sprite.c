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

/** @brief A CLUT's position in VRAM. */
typedef struct {
    s16 x; /**< the column */
    s16 y; /**< the row */
} VariantClutPos;

/* VariantSprite's data, in address order. */

/* VariantSprite's method table, class id 0x1F44: Sprite's slots, with
 * VariantSprite's overrides. A slot whose function is declared for another
 * class's `self` takes a `void *` cast. */
/* clang-format off */
VariantSpriteMethods gVariantSpriteMethods = {
    /* +0x000 header */ 0x1F44,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)VariantSprite__VariantSprite,
    /* +0x00C finalize */ (void *)SceneNode__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ (void *)VariantSprite__SetVariantClut,
    /* +0x044 updateRotation */ (void *)Sprite__UpdateRotation,
    /* +0x048 updateScale */ (void *)VariantSprite__UpdateScale,
    /* +0x04C attachToParent */ (void *)SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)Sprite__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)Sprite__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)Sprite__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)SceneNode__NotifyWithHull,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ VariantSprite__Update,
    /* +0x09C dispatchLinkCommand */ (void *)SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setColor */ (void *)Sprite__SetColor,
    /* +0x0BC slotBC */ VariantSprite__NoOpSlotBC,
    /* +0x0C0 slotC0 */ VariantSprite__NoOpSlotC0,
    /* +0x0C4 slotC4 */ VariantSprite__NoOpSlotC4,
};
/* clang-format on */

/* VariantSprite's two texture cells, forwarded as the Sprite ctor's `rect`
 * (Sprite__Reset copies it into Sprite.rect): 16x16 at u 0x00 and 0x10,
 * v 0x120. */
SpriteRect sVariantSpriteCells[2] = {{0x00, 0x120, 16, 16}, {0x10, 0x120, 16, 16}};

/* The two variants' CLUTs: adjacent 16-colour rows on VRAM's bottom line. */
VariantClutPos sVariantSpriteClut[2] = {{976, 511}, {992, 511}};

VariantSprite *New_VariantSprite(s32 variant, void *resetArg, void *texture) {
    void *obj = BMemPMgrAlloc(sizeof(VariantSprite));
    if (obj != NULL) {
        GetVariantSpriteMethods()->ctor(obj, variant, resetArg, texture);
        return obj;
    }
    return NULL;
}

/* Sprite's ctor with the variant's cell, then this class's table, and the
 * reset slot (VariantSprite__SetVariantClut) with the variant, through
 * VariantSpriteResetFn. Returns nothing: it ends in that call. */
void VariantSprite__VariantSprite(VariantSprite *self, s32 variant, void *resetArg, void *texture) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, &sVariantSpriteCells[variant], resetArg, 0);
    self->methods = GetVariantSpriteMethods();
    self->unkA4 = 0;
    ((VariantSpriteResetFn)self->methods->reset)(self, variant);
}

void VariantSprite__SetVariantClut(VariantSprite *self, s32 variant) {
    self->variant = variant;
    self->sprite.cx = sVariantSpriteClut[variant].x;
    self->sprite.cy = sVariantSpriteClut[variant].y;
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
