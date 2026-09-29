/*
 * BgLayer's methods (include/bg_layer.h: a SceneNode drawing one GsBG over
 * a TileMap, the screen's background layer), in ROM order: the allocator,
 * ctor and reset, updateRotation and updateScale, setColor and the empty
 * slot +0x0BC, ending with its getter GetBgLayerMethods; its
 * method table closes the file. A (void *) entry in it is a method whose
 * declared parameters differ from the slot's, usually one inherited from a
 * parent class and declared on the parent's type.
 */
#include "common.h"
#include "bg_layer.h"
#include "tile_map.h"
#include "bmem_pmgr.h"

/* GsBG attribute bits 24..25, the colour mode (LIBGS: 0 4-bit CLUT, 1 8-bit
 * CLUT, 2 15-bit direct). BGLAYER_MODE_SCREEN is the one New_BgLayer's caller
 * uses, over TileAtlas's 15-bit texture pages. */
#define BG_ATTR_8BIT (1 << 24)
#define BG_ATTR_15BIT (2 << 24)
#define BG_SCREEN_W 320 /* BGLAYER_MODE_SCREEN's layer size */
#define BG_SCREEN_H 240
#define BG_SCALE_MAX 30000 /* BgLayer__UpdateScale's clamp, in 20.12 */

/* Allocate a BgLayer and construct it over `src`'s map in `mode`. */
BgLayer *New_BgLayer(TileMap *src, s32 mode) {
    BgLayer *obj = BMemPMgrAlloc(sizeof(BgLayer));

    if (obj != NULL) {
        GetBgLayerMethods()->ctor(obj, src, mode);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): SceneNode's, then reset. */
void BgLayer__BgLayer(BgLayer *self, TileMap *src, s32 mode) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetBgLayerMethods();
    ((BgLayerResetFn)self->methods->reset)(self, src, mode);
}

/* reset (+0x040): lay the GsBG over `src`'s map, sized to the map
 * (BGLAYER_MODE_MAP, 8-bit CLUT) or to the screen (BGLAYER_MODE_SCREEN,
 * 15-bit), at the origin, unscaled, unrotated, pivoting on its centre. */
extern ColorRgb sBgLayerDefaultColor;

void BgLayer__Reset(BgLayer *self, TileMap *src, s32 mode) {
    if (mode == BGLAYER_MODE_MAP) {
        self->bgAttribute = BG_ATTR_8BIT;
        self->w = src->map.cellw * src->map.ncellw;
        self->h = src->map.cellh * src->map.ncellh;
    } else if (mode == BGLAYER_MODE_SCREEN) {
        self->bgAttribute = BG_ATTR_15BIT;
        self->w = BG_SCREEN_W;
        self->h = BG_SCREEN_H;
    }
    self->x = 0;
    self->y = 0;
    self->scrollx = 0;
    self->scrolly = 0;
    self->color = sBgLayerDefaultColor;
    self->map = &src->map;
    self->scalex = ONE;
    self->scaley = ONE;
    self->rotate = 0;
    self->mx = self->w / 2;
    self->my = self->h / 2;
}

/* updateRotation (+0x044): the ratio table's z entry, in 20.12, becomes the
 * GsBG's rotation when `set`, else is added to it. */
void BgLayer__UpdateRotation(BgLayer *self, s32 set, Ratio16 *table) {
    s32 num = table[2].num;
    s32 den = table[2].den;
    s32 v = ((num / den) << FIX12_SHIFT) + (((num % den) << FIX12_SHIFT) / den);

    if (set) {
        self->rotate = v;
    } else {
        self->rotate += v;
    }
}

/* updateScale (+0x048): the ratio table's x and y entries, in 20.12,
 * become the GsBG's scale when `set` (ONE for a zero divisor, at most
 * BG_SCALE_MAX), else are added to it; a sum past BG_SCALE_MAX clamps
 * there, or to 1 when that ratio had a negative term. */
void BgLayer__UpdateScale(BgLayer *self, s32 set, Ratio16 *src) {
    s32 negX;
    s32 negY;
    s32 den;
    s16 sx;
    s16 sy;
    s32 v;

    negX = 0;
    negY = 0;
    if (src[0].num < 0 || src[0].den < 0) {
        negX = 1;
    }
    if (src[1].num < 0 || src[1].den < 0) {
        negY = 1;
    }
    den = src[0].den;
    if (den != 0) {
        sx = ((src[0].num / den) << FIX12_SHIFT) + (((src[0].num % den) << FIX12_SHIFT) / den);
    }
    if (src[1].den != 0) {
        sy = ((src[1].num / src[1].den) << FIX12_SHIFT) +
             (((src[1].num % src[1].den) << FIX12_SHIFT) / src[1].den);
    }
    if (set) {
        if (den == 0) {
            self->scalex = ONE;
        } else {
            v = sx;
            if (v > BG_SCALE_MAX) {
                v = BG_SCALE_MAX;
            }
            self->scalex = v;
        }
        if (src[1].den == 0) {
            self->scaley = ONE;
        } else {
            v = sy;
            if (v > BG_SCALE_MAX) {
                v = BG_SCALE_MAX;
            }
            self->scaley = v;
        }
    } else {
        if (self->scalex + sx > BG_SCALE_MAX) {
            if (negX) {
                self->scalex = 1;
            } else {
                self->scalex = BG_SCALE_MAX;
            }
        } else {
            self->scalex = sx + self->scalex;
        }
        if (self->scaley + sy > BG_SCALE_MAX) {
            if (negY) {
                self->scaley = 1;
            } else {
                self->scaley = BG_SCALE_MAX;
            }
        } else {
            self->scaley = sy + self->scaley;
        }
    }
}

/* +0x0B8: take `rgb` as the GsBG's colour when `enable`. */
void BgLayer__SetColor(BgLayer *self, s32 enable, ColorRgb *rgb) {
    if (enable) {
        self->color = *rgb;
    }
}

void BgLayer__NoOp(void) {}

BgLayerMethods *GetBgLayerMethods(void) {
    return &gBgLayerMethods;
}

/* BgLayer: SceneNode's table with its reset, rotation and scale, then
 * setColor. */
BgLayerMethods gBgLayerMethods = {
    /* +0x000 header */ BGLAYER_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)BgLayer__BgLayer,
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
    /* +0x040 reset */ (void *)BgLayer__Reset,
    /* +0x044 updateRotation */ (void *)BgLayer__UpdateRotation,
    /* +0x048 updateScale */ (void *)BgLayer__UpdateScale,
    /* +0x04C attachToParent */ (void *)SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)SceneNode__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)SceneNode__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)SceneNode__SetSemiTransRate,
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
    /* +0x098 update */ (void *)SceneNode__Update,
    /* +0x09C dispatchLinkCommand */ (void *)SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setColor */ BgLayer__SetColor,
    /* +0x0BC slotBC */ BgLayer__NoOp,
};
