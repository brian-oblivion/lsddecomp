/**
 * @file flat_light_obj.h
 * @brief FlatLightObj, one libgs flat light (GsF_LIGHT) as a BasicClass
 *        object, and its method table.
 */
#ifndef FLAT_LIGHT_OBJ_H
#define FLAT_LIGHT_OBJ_H

#include "basic_class.h"
#include "draw_system.h"

typedef struct FlatLightObj FlatLightObj;
typedef struct FlatLightObjMethods FlatLightObjMethods;

/** FlatLightObj's class id (gFlatLightObjMethods word +0x000). A single nibble: it derives from BasicClass alone. */
#define FLATLIGHTOBJ_CLASS_ID 0x6

/**
 * @brief A flat light's direction and colour: <libgs.h>'s GsF_LIGHT, with
 *        r, g, b grouped as one ColorRgb so setColor can copy them whole.
 *
 * The offsets are GsF_LIGHT's, so the methods pass it to GsSetFlatLight cast
 * to GsF_LIGHT *.
 */
typedef struct {
    /* +0x000 */ s32 vx,       /**< the light's direction, x */
        vy,                    /**< the light's direction, y */
        vz;                    /**< the light's direction, z */
    /* +0x00C */ ColorRgb rgb; /**< the light's colour */
} FlatLightParams;

/**
 * @brief FlatLightObj's method table, gFlatLightObjMethods: BasicClass's slots
 *        with the ctor overridden, then three of its own.
 */
struct FlatLightObjMethods {
    BASICCLASS_SLOTS(FlatLightObj, (FlatLightObj * self, s32 lightId)); /**< ctor: @see FlatLightObj__FlatLightObj */
    /* +0x040 */ void (*setLightId)(FlatLightObj *self, s32 lightId); /**< @see FlatLightObj__SetLightId */
    /* +0x044 */ void (*setColor)(FlatLightObj *self, s32 update, ColorRgb *rgb); /**< @see FlatLightObj__SetColor */
    /* +0x048 */ void (*setDirection)(FlatLightObj *self, s32 update,
                                      s16 *dir); /**< @see FlatLightObj__SetDirection */
};

/**
 * @brief One of the game's three flat lights (class id 0x6): a light id and
 *        the GsF_LIGHT it pushes to libgs on every change.
 *
 * Parent BasicClass; no class derives from it. Methods, the whole class, in
 * src/graphics/flat_light_obj.c. The object is 0x20 bytes (New_FlatLightObj).
 *
 * LightRig__LightRig (src/graphics/sprite.c) makes three, with light ids 0, 1
 * and 2, keeps them in LightRig::lights and adds each as a child;
 * LightRig__Finalize releases them. Their one caller of setColor and
 * setDirection is StageMap__SetChildParams (src/world/dream_day.c), through
 * LightRig's getLight, with update = 1 and per-light sources of an r, g, b
 * and an s16 vx, vy, vz.
 */
struct FlatLightObj {
    BASICCLASS_FIELDS(FlatLightObjMethods);
    /* +0x00C */ s32 lightId;           /**< the GsSetFlatLight id: 0, 1 or 2 from LightRig */
    /* +0x010 */ FlatLightParams light; /**< the light as last pushed; ends at +0x020, the object's size */
};

/** FlatLightObj's method table. */
extern FlatLightObjMethods gFlatLightObjMethods;

/**
 * @brief Returns FlatLightObj's method table.
 * @return &gFlatLightObjMethods.
 */
extern FlatLightObjMethods *GetFlatLightObjMethods(void);

/**
 * @brief Allocates a FlatLightObj from the pool and constructs it.
 * @param lightId The libgs flat light it drives (0, 1 or 2).
 * @return The new light, or NULL when the pool is exhausted.
 */
FlatLightObj *New_FlatLightObj(s32 lightId);

/**
 * @brief Constructor (slot +0x008): BasicClass's, then setLightId.
 * @param self    The object to construct.
 * @param lightId The libgs flat light it drives.
 */
void FlatLightObj__FlatLightObj(FlatLightObj *self, s32 lightId);

/**
 * @brief Slot +0x040: sets the libgs flat light id the object drives.
 * @param self    The light.
 * @param lightId The new id; nothing is pushed until the next setColor or
 *                setDirection.
 */
void FlatLightObj__SetLightId(FlatLightObj *self, s32 lightId);

/**
 * @brief Slot +0x044: optionally takes a new colour, then pushes the whole
 *        light to libgs with GsSetFlatLight.
 * @param self   The light.
 * @param update Nonzero to copy `rgb` first; zero only pushes.
 * @param rgb    The new colour.
 */
void FlatLightObj__SetColor(FlatLightObj *self, s32 update, ColorRgb *rgb);

/**
 * @brief Slot +0x048: optionally takes a new direction, then pushes the whole
 *        light to libgs with GsSetFlatLight.
 * @param self   The light.
 * @param update Nonzero to widen dir[0..2] into vx, vy, vz first; zero only
 *               pushes.
 * @param dir    The new direction, three s16s.
 */
void FlatLightObj__SetDirection(FlatLightObj *self, s32 update, s16 *dir);

#endif
