/*
 * FlatLightObj: one Psy-Q flat light as a BasicClass object. This file holds
 * the whole class: the allocator New_FlatLightObj, the constructor, its three
 * own slots (setLightId, setColor, setDirection) and the table getter. The
 * object and its method table are declared in include/FlatLightObj.h.
 *
 * setColor and setDirection update the object's copy of the light, then hand
 * all of it to Sony's GsSetFlatLight under the object's light id, so a light
 * is pushed to libgs on every change. The copy is a FlatLightParams, Sony's
 * GsF_LIGHT layout with r,g,b grouped as one struct, hence the casts at the
 * two calls. LightRig (src/Sprite.c) creates the three the game uses, light
 * ids 0, 1 and 2.
 *
 * Edges (track 8): the binary fixes both. The file sits between two placed
 * Sony objects, libgs/gs_110 (GsSetAmbient) before and libgs/gs_107
 * (GsSetFlatLight) after, so neither neighbour can be merged into it and its
 * start and end are real file boundaries. Inside, tools/tuboundary.py finds
 * no rodata tying or splitting the six functions (all five gaps "boundary
 * possible"; the forced boundary it notes spans Sprite.c's jump table at
 * 0x80011290 to DayTaskStageMap.c's at 0x8001140C and is met by Sony edges
 * elsewhere, so it forces nothing here). Content decided the rest: one class,
 * whole, is one file, named for it.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "FlatLightObj.h"

extern void *BMemPMgrAlloc(s32 size);

FlatLightObj *New_FlatLightObj(s32 lightId) {
    FlatLightObj *self;

    self = BMemPMgrAlloc(sizeof(FlatLightObj));
    if (self != NULL) {
        Get_vtable_FlatLightObj()->ctor(self, lightId);
        return self;
    }
    return NULL;
}

void FlatLightObj__FlatLightObj(FlatLightObj *self, s32 lightId) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_FlatLightObj();
    self->methods->setLightId(self, lightId);
}

void FlatLightObj__SetLightId(FlatLightObj *self, s32 lightId) {
    self->lightId = lightId;
}

void FlatLightObj__SetColor(FlatLightObj *self, s32 update, FlatLightColor *rgb) {
    if (update) {
        /* MATCHING: a whole-struct copy; three per-byte stores compile 3 words longer */
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

FlatLightObjMethods *Get_vtable_FlatLightObj(void) {
    return &gFlatLightObjMethods;
}
