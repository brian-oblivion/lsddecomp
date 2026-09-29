/*
 * FlatLightObj: one Psy-Q flat light as a BasicClass object. This file holds
 * the whole class: the allocator New_FlatLightObj, the constructor, its three
 * own slots (setLightId, setColor, setDirection), the table getter and the
 * table itself. The object and its method table are declared in
 * include/flat_light_obj.h.
 *
 * setColor and setDirection update the object's copy of the light, then hand
 * all of it to Sony's GsSetFlatLight under the object's light id, so a light
 * is pushed to libgs on every change. The copy is a FlatLightParams, Sony's
 * GsF_LIGHT layout with r,g,b grouped as one struct, hence the casts at the
 * two calls. LightRig (src/graphics/sprite.c) creates the three the game uses, light
 * ids 0, 1 and 2.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "flat_light_obj.h"
#include "bmem_pmgr.h"

FlatLightObj *New_FlatLightObj(s32 lightId) {
    FlatLightObj *self;

    self = BMemPMgrAlloc(sizeof(FlatLightObj));
    if (self != NULL) {
        GetFlatLightObjMethods()->ctor(self, lightId);
        return self;
    }
    return NULL;
}

void FlatLightObj__FlatLightObj(FlatLightObj *self, s32 lightId) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetFlatLightObjMethods();
    self->methods->setLightId(self, lightId);
}

void FlatLightObj__SetLightId(FlatLightObj *self, s32 lightId) {
    self->lightId = lightId;
}

void FlatLightObj__SetColor(FlatLightObj *self, s32 update, ColorRgb *rgb) {
    if (update) {
        self->light.rgb = *rgb;
    }
    GsSetFlatLight(self->lightId, (GsF_LIGHT *)&self->light);
}

void FlatLightObj__SetDirection(FlatLightObj *self, s32 update, s16 *dir) {
    if (update) {
        self->light.vx = dir[0];
        self->light.vy = dir[1];
        self->light.vz = dir[2];
    }
    GsSetFlatLight(self->lightId, (GsF_LIGHT *)&self->light);
}

FlatLightObjMethods *GetFlatLightObjMethods(void) {
    return &gFlatLightObjMethods;
}

/* FlatLightObj's method table (include/flat_light_obj.h): BasicClass's slots
 * with the ctor, then its three setters. A (void *) entry is a base method,
 * declared on BasicClass *. */
FlatLightObjMethods gFlatLightObjMethods = {
    /* +0x000 header */ FLATLIGHTOBJ_CLASS_ID,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ FlatLightObj__FlatLightObj,
    /* +0x00C finalize */ (void *)BasicClass__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 setLightId */ FlatLightObj__SetLightId,
    /* +0x044 setColor */ FlatLightObj__SetColor,
    /* +0x048 setDirection */ FlatLightObj__SetDirection,
};
