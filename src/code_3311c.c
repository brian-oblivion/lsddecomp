/*
 * code_3311c -- GAME code carved from psyq_3311c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x3311C..0x3328C (vram 0x8004291C..0x80042A8C). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the whole of class
 * gFlatLightObjMethods -- allocator, ctor, its three own vtable slots
 * (+0x40/+0x44/+0x48), and the table getter.
 *
 * All six functions matched and named in round 81 (alpha).
 */
#include "common.h"
#include "FlatLightObj.h"

/*
 * The class is declared in include/FlatLightObj.h (track 4, round 87): the
 * object, its table and the evidence for the name live there. The unit's
 * only outside caller of New_FlatLightObj is LightRig__LightRig
 * (src/code_322b4.c, include/LightRig.h), with light ids 0, 1, 2.
 */

extern void *BMemPMgrAlloc(s32 size);
extern int GsSetFlatLight(int id, FlatLightParams *lt);

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
        self->light.rgb = *rgb;
    }
    GsSetFlatLight(self->lightId, &self->light);
}

void FlatLightObj__SetDirection(FlatLightObj *self, s32 update, s16 *dir) {
    if (update) {
        self->light.vx = dir[0];
        self->light.vy = dir[1];
        self->light.vz = dir[2];
    }
    GsSetFlatLight(self->lightId, &self->light);
}

FlatLightObjMethods *Get_vtable_FlatLightObj(void) {
    return &gFlatLightObjMethods;
}
