#ifndef LIGHT_RIG_H
#define LIGHT_RIG_H

#include "scene_node.h"

/**
 * @file light_rig.h
 * @brief LightRig, the scene node that owns the three flat lights and the
 *        ambient colour.
 *
 * The name is for what the class's own methods do, and the evidence is this:
 *  - The ctor makes three FlatLightObj children (New_FlatLightObj(0), (1),
 *    (2): one Psy-Q flat light each, set through GsSetFlatLight;
 *    include/flat_light_obj.h), keeps them in `lights` and adds each as a
 *    child.
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
 * its FrameClock object (+0x010) to it as a child.
 */

typedef struct LightRig LightRig;
typedef struct LightRigMethods LightRigMethods;

/** LightRig's class id (gLightRigMethods word +0x000): 0x1 under SceneNode's
 * 0x4. */
#define LIGHTRIG_CLASS_ID 0x14

/**
 * SceneNode's slots, then LightRig's own, for LightRigMethods and StageMap's
 * table to expand first. gLightRigMethods overrides the ctor, finalize,
 * reset and dispatchLinkCommand with the LightRig__ methods below.
 */
/* clang-format off */
#define LIGHTRIG_SLOTS(Self, CtorParams)                                                           \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ BasicClass *(*getLight)(Self *self, s32 index); /* LightRig__GetLight; StageMap inherits it */ \
    /* +0x0BC */ void (*setAmbientColor)(Self *self, ColorRgb *rgb, s32 swap) /* LightRig__SetAmbientColor */
/* clang-format on */

/** SceneNode's fields, then LightRig's own, for StageMap to expand first. */
/* clang-format off */
#define LIGHTRIG_FIELDS(Methods)                                                                   \
    SCENENODE_FIELDS(Methods);                                                                    \
    /* +0x044 */ BasicClass *lights[3]; /* the ctor's New_FlatLightObj(0..2), also children */    \
    /* +0x050 */ ColorRgb ambient    /* setAmbientColor; GsSetAmbient gets each << 4. The object is 0x54 bytes (New_LightRig) */
/* clang-format on */

/** LightRig's method table: LIGHTRIG_SLOTS with its ctor parameters. */
struct LightRigMethods {
    LIGHTRIG_SLOTS(LightRig, (LightRig * self));
};

/**
 * LightRig: a SceneNode owning three flat lights and the ambient colour.
 * Class id 0x14, table gLightRigMethods, parent SceneNode, whose ctor it
 * chains to first; methods in src/graphics/light_rig.c. One class derives
 * from it: StageMap (0x114, the grid manager, include/stage_map.h), whose ctor
 * and finalize chain to this class's and whose table inherits getLight
 * unchanged. The object is 0x54 bytes (New_LightRig): `lights` at +0x044,
 * `ambient` at +0x050, then one byte of word padding; StageMap's first own
 * field is at +0x054.
 *
 * The ctor returns nothing, but the slot keeps SceneNode's `void *` ctor
 * type; no caller of this class's ctor reads a result (New_LightRig returns
 * the allocation, StageMap__StageMap discards it).
 */
struct LightRig {
    LIGHTRIG_FIELDS(LightRigMethods);
};

/** LightRig's method table (class id 0x14). */
extern LightRigMethods gLightRigMethods;

/**
 * @brief The LightRig method table.
 * @return &gLightRigMethods.
 */
extern LightRigMethods *GetLightRigMethods(void);

/**
 * @brief Allocates a LightRig and runs its ctor through the table.
 * @return The new rig with its three lights, or NULL when the allocation
 *         fails.
 */
LightRig *New_LightRig(void);

/**
 * @brief Constructor (slot +0x008): SceneNode's ctor, installs
 *        gLightRigMethods, makes the three flat lights (New_FlatLightObj(0..2))
 *        and adds each as a child, then calls reset.
 * @param self The rig.
 */
void LightRig__LightRig(LightRig *self);

/**
 * @brief Finalizer (slot +0x00C): releases the three lights (fetched through
 *        getLight), then SceneNode's finalize.
 * @param self The rig.
 */
void LightRig__Finalize(LightRig *self);

/**
 * @brief Reset (slot +0x040): clears coord2->flg so libgs recomputes the
 *        coordinate; the transform is left as it is.
 * @param self The rig.
 */
void LightRig__Reset(LightRig *self);

/**
 * @brief dispatchLinkCommand (slot +0x09C): empty, so a rig takes no part in
 *        the link test.
 * @param self The rig.
 * @param sender The SceneNode that sent the event.
 * @param event The event.
 */
void LightRig__DispatchLinkCommand(LightRig *self, void *sender, s32 event);

/**
 * @brief getLight (slot +0x0B8): one of the three flat lights.
 * @param self The rig.
 * @param index 0..2; not checked.
 * @return lights[index], a FlatLightObj.
 */
BasicClass *LightRig__GetLight(LightRig *self, s32 index);

/**
 * @brief setAmbientColor (slot +0x0BC): sets the ambient colour and hands it
 *        to GsSetAmbient, each channel << 4 (0..255 to 0..4080 of ONE).
 * @param self The rig.
 * @param rgb The new colour; with `swap`, receives the old one.
 * @param swap Non-zero to return the old colour in *rgb.
 */
void LightRig__SetAmbientColor(LightRig *self, ColorRgb *rgb, s32 swap);

#endif
