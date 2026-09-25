/*
 * code_3311c -- GAME code carved from psyq_3311c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x3311C..0x3328C (vram 0x8004291C..0x80042A8C). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the four methods of
 * D_8006F06C.
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "BasicClass.h"

/*
 * The class of D_8006F06C: a BasicClass that owns one Psy-Q flat light.
 * 0x20 bytes (func_8004291C's allocation). Fields past BasicClass's are a
 * GsF_LIGHT (LIBGS.H) at +0x010, handed to GsSetFlatLight by address.
 * LIBGS.H is not included: its prototypes collide in shared headers, so the
 * one used here is declared locally with a local copy of the struct.
 */
typedef struct FlatLightObj FlatLightObj;
typedef struct FlatLightObjMethods FlatLightObjMethods;

/* The colour triple. Copied as a whole struct (func_800429E8): GCC's block
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
extern FlatLightObjMethods D_8006F06C;
FlatLightObjMethods *func_80042A7C(void);

INCLUDE_ASM("asm/nonmatchings/code_3311c", func_8004291C);
INCLUDE_ASM("asm/nonmatchings/code_3311c", func_8004297C);
void func_800429E0(FlatLightObj *self, s32 lightId) {
    self->lightId = lightId;
}
void func_800429E8(FlatLightObj *self, s32 update, FlatLightColor *rgb) {
    if (update) {
        self->light.rgb = *rgb;
    }
    GsSetFlatLight(self->lightId, &self->light);
}
INCLUDE_ASM("asm/nonmatchings/code_3311c", func_80042A2C);
FlatLightObjMethods *func_80042A7C(void) {
    return &D_8006F06C;
}
