/*
 * LightRig's methods (include/light_rig.h: three flat lights and the
 * ambient colour), in ROM order: the allocator, ctor, finalize, reset, the
 * empty dispatchLinkCommand, getLight and setAmbientColor, ending with its
 * getter GetLightRigMethods; its method table closes the file. A (void *)
 * entry in it is a method whose declared parameters differ from the slot's,
 * one inherited from a parent class and declared on the parent's type.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include "light_rig.h"
#include "flat_light_obj.h"
#include "bmem_pmgr.h"

/* A 0..255 colour channel to GsSetAmbient's 0..ONE scale (255 << 4 is 4080). */
#define AMBIENT_TO_FIX12_SHIFT 4

/* Allocate and construct a LightRig with its three lights. */
LightRig *New_LightRig(void) {
    LightRig *obj = BMemPMgrAlloc(sizeof(LightRig));

    if (obj != NULL) {
        GetLightRigMethods()->ctor(obj);
        return obj;
    }
    return NULL;
}

/* LightRig slot +0x008 (ctor): the SceneNode ctor, install the table,
 * create and add the three flat lights, then reset. */
void LightRig__LightRig(LightRig *self) {
    s32 i;
    BasicClass **light;

    GetSceneNodeMethods()->ctor((SceneNode *)self);
    self->methods = GetLightRigMethods();
    for (i = 0, light = self->lights; i < ARRAY_COUNT(self->lights); i++, light++) {
        *light = (BasicClass *)New_FlatLightObj(i);
        self->methods->addChild(self, *light);
    }
    self->methods->reset(self);
}

/* LightRig slot +0x00C (finalize): release the three lights, then the
 * SceneNode finalize. */
void LightRig__Finalize(LightRig *self) {
    s32 i;
    BasicClass *light;

    for (i = 0; i < ARRAY_COUNT(self->lights); i++) {
        light = self->methods->getLight(self, i);
        light->methods->release(light);
    }
    GetSceneNodeMethods()->finalize((SceneNode *)self);
}

/* LightRig slot +0x040 (reset): mark the coordinate for recompute. */
void LightRig__Reset(LightRig *self) {
    self->coord2->flg = 0;
}

/* LightRig slot +0x09C (dispatchLinkCommand): empty override. */
void LightRig__DispatchLinkCommand(LightRig *self, void *sender, s32 event) {}

/* LightRig slot +0x0B8 (getLight), inherited unchanged by gStageMapMethods. */
BasicClass *LightRig__GetLight(LightRig *self, s32 index) {
    return self->lights[index];
}

/* LightRig slot +0x0BC: set the ambient colour (swapping the old one out
 * into *rgb when asked) and hand it to GsSetAmbient. */
void LightRig__SetAmbientColor(LightRig *self, ColorRgb *rgb, s32 swap) {
    ColorRgb old;

    if (swap) {
        old = self->ambient;
        self->ambient = *rgb;
        *rgb = old;
    } else {
        self->ambient = *rgb;
    }
    GsSetAmbient(self->ambient.r << AMBIENT_TO_FIX12_SHIFT, self->ambient.g << AMBIENT_TO_FIX12_SHIFT,
                 self->ambient.b << AMBIENT_TO_FIX12_SHIFT);
}

/* Returns the LightRig method table. */
LightRigMethods *GetLightRigMethods(void) {
    return &gLightRigMethods;
}

/* LightRig (include/light_rig.h): SceneNode's table with its ctor, finalize,
 * reset and dispatchLinkCommand, then getLight and setAmbientColor. */
LightRigMethods gLightRigMethods = {
    /* +0x000 header */ LIGHTRIG_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)LightRig__LightRig,
    /* +0x00C finalize */ LightRig__Finalize,
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
    /* +0x040 reset */ LightRig__Reset,
    /* +0x044 updateRotation */ (void *)SceneNode__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
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
    /* +0x09C dispatchLinkCommand */ LightRig__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 getLight */ LightRig__GetLight,
    /* +0x0BC setAmbientColor */ LightRig__SetAmbientColor,
};
