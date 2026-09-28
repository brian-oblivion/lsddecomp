#ifndef LIGHTRIG_H
#define LIGHTRIG_H

#include "scene_node.h"

/*
 * LightRig -- a scene node that owns the flat lights and the ambient colour
 * (class id 0x14, method table gLightRigMethods): SceneNode's subclass.
 * Methods in src/graphics/sprite.c. The name is for what the class's own methods
 * do, and the evidence is this:
 *  - The ctor makes three FlatLightObj children (New_FlatLightObj(0), (1),
 *    (2): one Psy-Q flat light each, set through GsSetFlatLight; src/
 *    flat_light_obj.c), keeps them in `lights` and adds each as a child.
 *  - Finalize fetches the same three through getLight (+0x0B8) and releases
 *    each before SceneNode's finalize.
 *  - setAmbientColor (+0x0BC) stores an r,g,b in `ambient` and hands it,
 *    scaled << 4, to GsSetAmbient.
 *  - The one caller of getLight outside the class, StageMap__SetChildParams,
 *    calls each returned light's +0x044 and +0x048 -- FlatLightObj's setColor
 *    and setDirection -- from sources that step by 3 bytes (an r,g,b) and by
 *    6 bytes (an s16 vx,vy,vz) per light.
 *
 * Who holds one: IntermediateBase__Init makes one with New_LightRig() when
 * its init args bring none (IntermediateBase's +0x014) and, in mode 0, adds
 * its FrameClock object (+0x010) to it as a child. One class derives from
 * it: StageMap (gStageMapMethods, 0x114, the grid manager), whose ctor and
 * finalize chain to this class's first and whose table inherits getLight
 * unchanged; it expands these macros (include/StageMap.h).
 *
 * The ctor chains to SceneNode's (GetSceneNodeMethods()->ctor), so the id
 * tree (0x4 -> 0x14) is the ctor chain. The ctor returns nothing, but the
 * slot keeps SceneNode's `void *` ctor type: no caller of this class's ctor
 * reads $v0 (New_LightRig returns the allocation, StageMap__StageMap
 * discards it), so the two spellings compile alike.
 *
 * The object is 0x54 bytes (New_LightRig): `lights` at +0x044 and `ambient`
 * at +0x050, then one byte of word padding. StageMap names nothing of its
 * own in +0x044..+0x054 (its first own field, `origin`, is at +0x054).
 */

typedef struct LightRig LightRig;
typedef struct LightRigMethods LightRigMethods;

/* SceneNode's slots, then this class's own. `tools/classtable.py
 * gLightRigMethods --vs gSceneNodeMethods` lists the overrides of the
 * inherited ones (LightRig__LightRig, __Finalize, __Reset,
 * __DispatchLinkCommand). */
/* clang-format off */
#define LIGHTRIG_SLOTS(Self, CtorParams)                                                           \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ BasicClass *(*getLight)(Self *self, s32 index); /* LightRig__GetLight: lights[index]; StageMap inherits it */ \
    /* +0x0BC */ void (*setAmbientColor)(Self *self, ColorRgb *rgb, s32 swap) /* LightRig__SetAmbientColor; swap: the old colour comes back in *rgb */
/* clang-format on */

/* clang-format off */
#define LIGHTRIG_FIELDS(Methods)                                                                   \
    SCENENODE_FIELDS(Methods);                                                                    \
    /* +0x044 */ BasicClass *lights[3]; /* the ctor's New_FlatLightObj(0..2), also children */    \
    /* +0x050 */ ColorRgb ambient    /* setAmbientColor; GsSetAmbient gets each << 4. The object is 0x54 bytes (New_LightRig) */
/* clang-format on */

struct LightRigMethods {
    LIGHTRIG_SLOTS(LightRig, (LightRig * self));
};

struct LightRig {
    LIGHTRIG_FIELDS(LightRigMethods);
};

extern LightRigMethods gLightRigMethods;
extern LightRigMethods *GetLightRigMethods(void); /* returns &gLightRigMethods */

/* The occupants of gLightRigMethods this class owns, in slot order. The
 * inherited ones are SceneNode's and BasicClass's (their headers). */
LightRig *New_LightRig(void);
void LightRig__LightRig(LightRig *self);
void LightRig__Finalize(LightRig *self);
void LightRig__Reset(LightRig *self);
void LightRig__DispatchLinkCommand(LightRig *self, void *sender, s32 event);
BasicClass *LightRig__GetLight(LightRig *self, s32 index);
void LightRig__SetAmbientColor(LightRig *self, ColorRgb *rgb, s32 swap);

#endif
