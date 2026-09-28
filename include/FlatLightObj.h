#ifndef FLATLIGHTOBJ_H
#define FLATLIGHTOBJ_H

#include "BasicClass.h"

/*
 * FlatLightObj -- one Psy-Q flat light, class id 0x6, method table
 * gFlatLightObjMethods, a direct BasicClass subclass (`tools/classtable.py
 * gFlatLightObjMethods --vs gBasicClassMethods`: overrides the ctor, adds three
 * slots). No class derives from it. Methods in src/FlatLightObj.c, which holds
 * the whole class: allocator, ctor, the three own slots and the getter.
 *
 * "FlatLight" is Sony's own name (LIBGS.H's GsF_LIGHT and GsSetFlatLight),
 * not a guess: the object keeps a light id at +0x00C and a GsF_LIGHT at
 * +0x010, and setColor/setDirection update the GsF_LIGHT and hand it by
 * address to GsSetFlatLight(lightId, &light). FlatLightParams is GsF_LIGHT's
 * layout (`int vx,vy,vz; unsigned char r,g,b;`, same offsets) with r,g,b
 * grouped as FlatLightColor, which setColor's whole-struct copy needs;
 * src/FlatLightObj.c takes GsSetFlatLight from <libgs.h> and casts to GsF_LIGHT *.
 *
 * Who holds one: LightRig__LightRig (src/Sprite.c, include/LightRig.h)
 * makes three with New_FlatLightObj(0), (1), (2), keeps them in
 * LightRig::lights and adds each as a child; LightRig__Finalize releases
 * them. The one caller of setColor (+0x044) and setDirection (+0x048) is
 * StageMap__SetChildParams (src/class_39e08.c), through LightRig's
 * getLight, with update = 1 and sources stepping 3 bytes (an r,g,b) and 6
 * bytes (an s16 vx,vy,vz) per light. That call site casts getLight's
 * BasicClass * to FlatLightObj * and its s32 sources to the slots' types
 * (track 4, round 89).
 *
 * The object is 0x20 bytes (New_FlatLightObj's allocation).
 */

typedef struct FlatLightObj FlatLightObj;
typedef struct FlatLightObjMethods FlatLightObjMethods;

/* The colour triple. Copied as a whole struct (FlatLightObj__SetColor): the
 * all-s8 members give it alignment 1, and GCC's block move is what loads all
 * three bytes before storing any. */
typedef struct {
    s8 r, g, b;
} FlatLightColor;

/* == LIBGS.H GsF_LIGHT: the direction, then the colour. */
typedef struct {
    /* +0x000 */ s32 vx, vy, vz;
    /* +0x00C */ FlatLightColor rgb;
} FlatLightParams;

struct FlatLightObjMethods {
    BASICCLASS_SLOTS(FlatLightObj, (FlatLightObj * self, s32 lightId)); /* ctor: FlatLightObj__FlatLightObj */
    /* +0x040 */ void (*setLightId)(FlatLightObj *self, s32 lightId); /* FlatLightObj__SetLightId */
    /* +0x044 */ void (*setColor)(FlatLightObj *self, s32 update,
                                  FlatLightColor *rgb); /* FlatLightObj__SetColor: copy *rgb if update, then GsSetFlatLight */
    /* +0x048 */ void (*setDirection)(FlatLightObj *self, s32 update,
                                      s16 *dir); /* FlatLightObj__SetDirection: widen dir[0..2] if update, then GsSetFlatLight */
};

struct FlatLightObj {
    BASICCLASS_FIELDS(FlatLightObjMethods);
    /* +0x00C */ s32 lightId;           /* the GsSetFlatLight id: 0, 1 or 2 from LightRig */
    /* +0x010 */ FlatLightParams light; /* ends at +0x020, the object's size */
};

extern FlatLightObjMethods gFlatLightObjMethods;
extern FlatLightObjMethods *Get_vtable_FlatLightObj(void); /* returns &gFlatLightObjMethods */

/* The class's own methods, FlatLightObj, in ROM order. */
FlatLightObj *New_FlatLightObj(s32 lightId);
void FlatLightObj__FlatLightObj(FlatLightObj *self, s32 lightId);
void FlatLightObj__SetLightId(FlatLightObj *self, s32 lightId);
void FlatLightObj__SetColor(FlatLightObj *self, s32 update, FlatLightColor *rgb);
void FlatLightObj__SetDirection(FlatLightObj *self, s32 update, s16 *dir);

#endif
