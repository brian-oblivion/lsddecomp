/*
 * Sprite's methods (include/sprite.h: a SceneNode drawing one GsSPRITE cut
 * from a texture, the base of ScreenSprite, CharSprite, TextRow and
 * VariantSprite), in ROM order: the allocator, ctor and reset, with
 * InitGsSprite, the GsSPRITE fill reset uses, then updateRotation,
 * setDisplay, setSemiTrans, setSemiTransRate, the empty update and
 * setColor, ending with its getter GetSpriteMethods; its method table
 * closes the file. A (void *) entry in it is a method whose declared
 * parameters differ from the slot's, usually one inherited from a parent
 * class and declared on the parent's type.
 */
#include "common.h"
#include "sprite.h"
#include "tim_image.h"
#include "bmem_pmgr.h"

/* Allocate and construct a Sprite on the texture cell `rect`. */
Sprite *New_Sprite(void *texture, s32 abr, SpriteRect *rect, void *resetArg, s32 resetWord) {
    Sprite *obj = BMemPMgrAlloc(sizeof(Sprite));

    if (obj != NULL) {
        GetSpriteMethods()->ctor(obj, texture, abr, rect, resetArg, resetWord);
        return obj;
    }
    return NULL;
}

/* Sprite's reset (+0x040) as its ctor calls it: with all five ctor
 * arguments. The slot is SceneNode's, typed without them (sprite.h, "Not
 * settled"), and Sprite__Reset reads the first three. Local, where
 * CharSpriteResetFn and VariantSpriteResetFn sit in their headers, because
 * only this ctor calls through it. */
typedef void (*SpriteResetFn)(Sprite *self, void *texture, s32 abr, SpriteRect *rect,
                              void *resetArg, s32 resetWord);

/* gSpriteMethods slot +0x008 (ctor): the SceneNode ctor, install the table,
 * and hand every argument to reset. */
void Sprite__Sprite(Sprite *self, void *texture, s32 abr, SpriteRect *rect, void *resetArg,
                    s32 resetWord) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetSpriteMethods();
    ((SpriteResetFn)self->methods->reset)(self, texture, abr, rect, resetArg, resetWord);
}

/* gSpriteMethods slot +0x040 (reset): bind the texture and cell, rebuild the GsSPRITE. */
void Sprite__Reset(Sprite *self, void *texture, s32 abr, SpriteRect *rect) {
    self->image = &((TimImage *)texture)->tim;
    self->rect = *rect;
    InitGsSprite(&self->sprite, abr, rect, self->image);
    self->accumulateScale = 0;
}

/* Fill a GsSPRITE from a texture image and a cell: colour mode and tpage
 * from the image, size and u,v from the cell, the pivot at its centre,
 * neutral colour, scale 1.0 and no rotation. `tim` is a TimImage's GsIMAGE. */
void InitGsSprite(SpriteGs *sprite, s32 abr, SpriteRect *rect, GsIMAGE *tim) {
    s32 mode = tim->pmode & TIM_PMODE_DEPTH_MASK;
    s32 grey = SPRITE_RGB_NEUTRAL;

    sprite->attribute = mode << SPRITE_ATTR_MODE_SHIFT;
    sprite->x = 0;
    sprite->y = 0;
    sprite->w = rect->w;
    sprite->h = rect->h;
    sprite->mx = sprite->w >> 1;
    sprite->my = sprite->h >> 1;
    sprite->tpage = GetTPage(mode, abr, tim->px, tim->py);
    sprite->u = rect->u;
    sprite->v = rect->v;
    sprite->cx = tim->cx;
    sprite->cy = tim->cy;
    sprite->rgb.b = grey;
    sprite->rgb.g = grey;
    sprite->rgb.r = grey;
    sprite->rotate = 0;
    sprite->scalex = ONE;
    sprite->scaley = ONE;
}

/* gSpriteMethods slot +0x044 (updateRotation): table[2] as a fraction of
 * degrees, in 4096ths; set or add to the GsSPRITE's rotate. */
void Sprite__UpdateRotation(Sprite *self, s32 set, Ratio16 *table) {
    s32 angle;

    angle = ((table[2].num / table[2].den) << FIX12_SHIFT) +
            ((table[2].num % table[2].den) << FIX12_SHIFT) / table[2].den;
    if (set) {
        self->sprite.rotate = angle;
    } else {
        self->sprite.rotate += angle;
    }
}

/* Sprite classes slot +0x060 (setDisplay): display on or off (GsDOFF, inverted);
 * returns whether it was on. */
s32 Sprite__SetDisplay(Sprite *self, s32 on) {
    return GetSetBitField(&self->sprite.attribute, SPRITE_ATTR_DOFF_SHIFT, 1, on == 0) == 0;
}

/* Sprite classes slot +0x064: semitransparency on or off (GsALON); returns the
 * old bit. */
s32 Sprite__SetSemiTrans(Sprite *self, s32 on) {
    return GetSetBitField(&self->sprite.attribute, SPRITE_ATTR_ALON_SHIFT, 1, on != 0);
}

/* Sprite classes slot +0x068: the semitransparency rate (2 bits); returns the
 * old rate. */
s32 Sprite__SetSemiTransRate(Sprite *self, s32 rate) {
    return GetSetBitField(&self->sprite.attribute, SPRITE_ATTR_RATE_SHIFT, 2, rate);
}

/* Slot +0x098 (update) of every sprite class's table but VariantSprite's: empty override. */
void Sprite__Update(Sprite *self, void *sender, s32 event) {}

/* Slot +0x0B8 of gCharSpriteMethods, gScreenSpriteMethods, gSpriteMethods and gVariantSpriteMethods (the
 * sprite classes): copy three bytes into the embedded GsSPRITE's r,g,b. */
void Sprite__SetColor(Sprite *self, ColorRgb *rgb) {
    self->sprite.rgb = *rgb;
}

/* Returns the gSpriteMethods method table. */
SpriteMethods *GetSpriteMethods(void) {
    return &gSpriteMethods;
}

/* Sprite (include/sprite.h): SceneNode's table with Sprite's ctor, reset,
 * rotation, display, semi-transparency and update, then setColor. */
SpriteMethods gSpriteMethods = {
    /* +0x000 header */ SPRITE_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)Sprite__Sprite,
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
    /* +0x040 reset */ (void *)Sprite__Reset,
    /* +0x044 updateRotation */ (void *)Sprite__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ Sprite__SetDisplay,
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
    /* +0x098 update */ Sprite__Update,
    /* +0x09C dispatchLinkCommand */ (void *)SceneNode__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setColor */ Sprite__SetColor,
};
