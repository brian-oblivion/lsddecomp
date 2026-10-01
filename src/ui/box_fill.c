/*
 * BoxFill's methods (include/box_fill.h: a flat-coloured GsBOXF rectangle,
 * the SceneNode FadeBox derives from), in ROM order: the allocator, ctor
 * and reset, attachToParent, the display, semi-transparency and colour
 * setters, ApplyColor, position and size, AttachAbsolute, SetPri and
 * SetMask, ending with its getter GetBoxFillMethods; its method table
 * closes the file. A (void *) entry in it is a method whose declared type
 * differs from its slot's, usually one inherited from a parent class and
 * declared on the parent's type.
 */
#include "common.h"
#include "box_fill.h"
#include "bmem_pmgr.h"

/* BoxFill__Reset's colour when it is given none: mid grey. */
static u8 sBoxFillDefaultColor[3] SDATA = {128, 128, 128};

BoxFill *New_BoxFill(void *size, void *color, s32 pri) {
    BoxFill *self;

    self = BMemPMgrAlloc(sizeof(BoxFill));
    if (self != NULL) {
        GetBoxFillMethods()->ctor(self, size, color, pri);
        return self;
    }
    return NULL;
}

void BoxFill__BoxFill(BoxFill *self, BoxFillSize *size, void *color, s32 pri) {
    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetBoxFillMethods();
    ((BoxFillResetFn)self->methods->reset)(self, size, color, pri);
}

void BoxFill__Reset(BoxFill *self, BoxFillSize *size, void *color, s32 pri) {
    BoxFillMethods *methods;

    self->pri = pri;
    self->relative = 1;
    self->attachArg = 0;
    self->boxAttribute = 0;
    self->boxX = 0;
    self->boxY = 0;
    self->boxW = size->w;
    self->boxH = size->h;
    methods = self->methods;
    if (color == NULL) {
        color = sBoxFillDefaultColor;
    }
    methods->setColor(self, 1, color);
    self->methods->setMask(self, 13);
}

void BoxFill__AttachToParent(BoxFill *self, SceneNode *parent, BoxFillPos *pos) {
    if (self->parent == NULL) {
        GetSceneNodeMethods()->attachToParent((SceneNode *)self, parent, 0);
        self->methods->setPosition(self, pos);
    }
}

s32 BoxFill__SetDisplay(BoxFill *self, s32 on) {
    return GetSetBitField(&self->boxAttribute, BOXFILL_ATTR_DOFF_SHIFT, 1, on == 0) == 0;
}

s32 BoxFill__SetSemiTrans(BoxFill *self, s32 on) {
    return GetSetBitField(&self->boxAttribute, BOXFILL_ATTR_ALON_SHIFT, 1, on != 0);
}

s32 BoxFill__SetSemiTransRate(BoxFill *self, s32 rate) {
    return GetSetBitField(&self->boxAttribute, BOXFILL_ATTR_RATE_SHIFT, 2, rate);
}

void BoxFill__SetColor(BoxFill *self, s32 overwrite, u8 *rgb) {
    BoxFill__ApplyColor(self, self->color, rgb, overwrite);
}

void BoxFill__ApplyColor(BoxFill *self, u8 *dst, u8 *src, s32 overwrite) {
    if (overwrite) {
        *(ColorRgb *)dst = *(ColorRgb *)src;
    } else {
        dst[0] += src[0];
        dst[1] += src[1];
        dst[2] += src[2];
    }
}

void BoxFill__SetPosition(BoxFill *self, BoxFillPos *pos) {
    if (self->parent != NULL) {
        *(BoxFillPos *)&self->posX = *pos;
    }
}

void BoxFill__SetSize(BoxFill *self, s32 *size) {
    if (self->parent != NULL) {
        self->boxW = ((BoxFillSize *)size)->w;
        self->boxH = ((BoxFillSize *)size)->h;
    }
}

/* attachToParent is called through an unprototyped pointer with attachArg
 * as a fourth argument, which its occupant ignores. */
void BoxFill__AttachAbsolute(BoxFill *self, SceneNode *parent, BoxFillPos *pos, s32 attachArg) {
    void (*fn)();

    fn = (void (*)())self->methods->attachToParent;
    /* MATCHING: without the do/while(0) the entry code comes out in another order. */
    do {
        fn(self, parent, pos, attachArg);
        self->relative = 0;
        self->attachArg = attachArg;
    } while (0);
}

void BoxFill__SetPri(BoxFill *self, s32 pri) {
    self->pri = pri;
}

s32 BoxFill__SetMask(BoxFill *self, s32 bits) {
    return self->mask = (1 << bits) - 1;
}

BoxFillMethods *GetBoxFillMethods(void) {
    return &gBoxFillMethods;
}

/* BoxFill (include/box_fill.h): SceneNode's table with reset, the
 * attach, display and semi-transparency overrides, then its colour, position,
 * size, priority and mask setters. */
BoxFillMethods gBoxFillMethods = {
    /* +0x000 header */ BOXFILL_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)BoxFill__BoxFill,
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
    /* +0x040 reset */ (void *)BoxFill__Reset,
    /* +0x044 updateRotation */ (void *)SceneNode__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)BoxFill__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ BoxFill__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)BoxFill__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)BoxFill__SetSemiTransRate,
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
    /* +0x0B8 setColor */ (void *)BoxFill__SetColor,
    /* +0x0BC setPosition */ BoxFill__SetPosition,
    /* +0x0C0 setSize */ BoxFill__SetSize,
    /* +0x0C4 attachAbsolute */ BoxFill__AttachAbsolute,
    /* +0x0C8 setPri */ BoxFill__SetPri,
    /* +0x0CC setMask */ BoxFill__SetMask,
};
