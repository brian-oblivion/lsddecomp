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
#include "BasicClass.h"

/*
 * FlatLightObj: a BasicClass (class id 0x6, direct BasicClass child) that
 * owns one Psy-Q flat light. 0x20 bytes (New_FlatLightObj's allocation).
 * Fields past BasicClass's are a Psy-Q light id at +0x00C and a GsF_LIGHT
 * (LIBGS.H: `int vx,vy,vz; unsigned char r,g,b;`) at +0x010, laid out here
 * as FlatLightParams -- same fields, same offsets -- and handed to
 * GsSetFlatLight by address. "FlatLight" is Sony's own name (GsF_LIGHT,
 * GsSetFlatLight), not a guess. LIBGS.H is not included: its prototypes
 * collide in shared headers, so the one used here is declared locally with
 * a local copy of the struct.
 *
 * The unit's only outside caller, LightRig__LightRig (src/code_322b4.c,
 * include/LightRig.h), calls New_FlatLightObj with light ids 0, 1, 2 and
 * keeps the three in LightRig::lights, the fixed 3-light flat-lighting rig
 * that also sets the ambient colour (round 86).
 */
typedef struct FlatLightObj FlatLightObj;
typedef struct FlatLightObjMethods FlatLightObjMethods;

/* The colour triple. Copied as a whole struct (FlatLightObj__SetColor): GCC's block
 * move is what loads all three bytes before storing any. */
typedef struct {
    s8 r, g, b;
} FlatLightColor;

typedef struct {
    /* +0x000 */ s32 vx, vy, vz;
    /* +0x00C */ FlatLightColor rgb;
} FlatLightParams; /* == GsF_LIGHT */

struct FlatLightObjMethods {
    BASICCLASS_SLOTS(FlatLightObj, (FlatLightObj *self, s32 lightId));
    /* +0x040 */ void (*setLightId)(FlatLightObj *self, s32 lightId);
    /* +0x044 */ void (*setColor)(FlatLightObj *self, s32 update, FlatLightColor *rgb);
    /* +0x048 */ void (*setDirection)(FlatLightObj *self, s32 update, s16 *dir);
};

struct FlatLightObj {
    BASICCLASS_FIELDS(FlatLightObjMethods);
    /* +0x00C */ s32 lightId;
    /* +0x010 */ FlatLightParams light;
};

extern void *BMemPMgrAlloc(s32 size);
extern int GsSetFlatLight(int id, FlatLightParams *lt);
extern FlatLightObjMethods gFlatLightObjMethods;
FlatLightObjMethods *Get_vtable_FlatLightObj(void);

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
