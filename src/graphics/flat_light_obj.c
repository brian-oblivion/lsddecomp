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
    FLATLIGHTOBJ_CLASS_ID,
    (void *)BasicClass__Release,
    FlatLightObj__FlatLightObj,
    (void *)BasicClass__Finalize,
    (void *)BasicClass__AddChild,
    (void *)BasicClass__RemoveChild,
    (void *)BasicClass__RemoveAllChildren,
    (void *)BasicClass__GetNextChild,
    (void *)BasicClass__AddParentRef,
    (void *)BasicClass__RemoveParentRef,
    (void *)BasicClass__ClearParentRefs,
    (void *)BasicClass__GetNextParentRef,
    (void *)BasicClass__NotifyParents,
    BasicClass__NoOpSlot34,
    (void *)BasicClass__OnNotify,
    NULL,
    FlatLightObj__SetLightId,
    FlatLightObj__SetColor,
    FlatLightObj__SetDirection,
};
