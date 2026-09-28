#ifndef STYLEEFFECT_H
#define STYLEEFFECT_H

#include "Actor.h"
#include "VariantSprite.h"

/*
 * StyleEffect -- an Actor the style layer (src/world/ObjMStyleActor.c) places at an
 * offset from the target position and keeps there: every frame it moves to
 * pos + offset, plus however far the viewport's viewpoint has risen or
 * fallen since it was built. Its `kind` picks what it carries (enum
 * StyleEffectKind): a model, the same model with two copies in a row, or a
 * cluster of five sprites. Class id 0xEF34, method table
 * gStyleEffectMethods, getter GetStyleEffectMethods. Its ctor chains to
 * Actor's (include/Actor.h); no class derives from it.
 *
 * Who builds it. StyleBuildEffectSlots, on the style layer's first tick,
 * first calls SetStyleEffectSources with the scene's DREAMER.TMD resource,
 * ETC.TIM image and viewport (sStyleEffectTmd, sStyleEffectTim,
 * gStyleEffectViewport), then StyleFillEffectKind0..3 fill
 * sStyleEffectSlots with New_StyleEffect(kind, params, gStyleGrid, pos):
 * several of kind 0, of kind 1 for variant 2, then one of kind 3 (variant 0)
 * or kind 2 (variant 2). `params` is one StyleEffectParams laid over the
 * separately-declared gStyleSpawnOffsetX .. gStyleSpawnColors, which the
 * fill functions and SetupStyleSpawnParamsRandom/B randomise before each build.
 * StyleUpdateEffectSlots runs every slot's update with the target position,
 * and StyleReleaseEffectSlots releases them.
 *
 * Lifecycle.
 *   ctor(kind, params, parent, pos)
 *            Actor's ctor, `state` 0, `kind` into Actor's `pendingExtra`
 *            (+0x054), then reset(params), then InitByKind: attach under
 *            `parent` at pos + offset with the params' rotation and scale,
 *            snapshot the viewpoint y (sStyleEffectBaseViewY), link a model
 *            from sStyleEffectTmd (kinds 0 and 1, sStyleEffectModelIds[kind])
 *            and build the kind's children.
 *   reset (+0x040)
 *            SetParams: copy the whole params block into `params`, zero
 *            `tick`. It takes the block where SceneNode's slot takes
 *            nothing; the ctor calls it through StyleEffectSetParamsFn.
 *   update (+0x0EC, Actor's setPendingExtra slot)
 *            Update: `tick` + 1, then UpdateByKind: setTranslation(pos +
 *            offset + the viewpoint y change), then the kind's per-frame
 *            step. StyleUpdateEffectSlots calls it through
 *            StyleEffectUpdateFn; overriding pendingExtra's setter is why
 *            nothing else rewrites `kind`.
 *   finalize ReleaseByKind (the kind's child array), then Actor's finalize.
 *
 * The object is 0x98 bytes (New_StyleEffect), exactly where `sprites` ends.
 */

typedef struct StyleEffect StyleEffect;
typedef struct StyleEffectMethods StyleEffectMethods;
typedef struct StyleEffectParams StyleEffectParams;

/* What each `kind` builds and does per frame (the switches in
 * ObjMStyleActor.c). */
typedef enum StyleEffectKind {
    STYLE_EFFECT_MODEL_ROW =
        0, /* model, plus two copies in a row (modelChildren) that spin and drift along z after 500 ticks */
    STYLE_EFFECT_MODEL = 1, /* model only; nothing per frame */
    STYLE_EFFECT_SPRITES = 2, /* five sprites shaped once at build (BuildRandomSprites); nothing per frame */
    STYLE_EFFECT_JITTER_SPRITES =
        3 /* five sprites, sprites[1..4] given a random scale and rotation every frame (RandomizeSprites) */
} StyleEffectKind;

/* The 0x24-byte parameter block the reset slot (StyleEffect__SetParams)
 * copies in whole. Every member is 4-aligned, so the whole-struct copy is
 * retail's aligned 4-word-per-iteration block move. */
struct StyleEffectParams {
    /* +0x000 */ LongVec3 offset; /* added to the caller's position (InitByKind, UpdateByKind) */
    /* +0x00C */ Ratio16 *rotation; /* Ratio16[3] degrees, self's and the model children's updateRotation (AttachWithRotScale) */
    /* +0x010 */ Ratio16 *scale; /* Ratio16[3], their updateScale; scale[0].num also scales the model-child spacing (PlaceModelChildren) */
    /* +0x014 */ s32 modelChildLayout; /* 0 = no modelChildren, else an index 1..4 into sModelChildSpacing: 1-2 along x, 3-4 along y */
    /* +0x018 */ s32 tableIndex;   /* index into sModelChildDriftZ and sSpriteShiftX */
    /* +0x01C */ SpriteRgb *color; /* every sprite's setColor (SpawnSprites) */
    /* +0x020 */ SpriteRgb *altColor; /* sprites[1]'s colour instead, when non-NULL (BuildRandomSprites) */
};

/* Actor's slots, then this class's own: none. The overrides of inherited
 * slots are the ctor, finalize, reset and +0x0EC (see the banner). */
/* clang-format off */
#define STYLEEFFECT_SLOTS(Self, CtorParams)                                                         \
    ACTOR_SLOTS(Self, CtorParams) /* no own slots; 59 words, the last StyleEffect__Update at +0x0EC */
/* clang-format on */

/* clang-format off */
#define STYLEEFFECT_FIELDS(Methods)                                                                 \
    ACTOR_FIELDS(Methods);                                                                         \
    /* +0x058 */ StyleEffectParams params; /* the reset slot's copy (StyleEffect__SetParams) */      \
    /* +0x07C */ Actor *modelChildren[2]; /* STYLE_EFFECT_MODEL_ROW: New_Actor children (PlaceModelChildren) */    \
    /* +0x084 */ VariantSprite *sprites[5]   /* the two sprite kinds: New_VariantSprite children (SpawnSprites). The object is 0x98 bytes (New_StyleEffect) */
/* clang-format on */

struct StyleEffectMethods {
    STYLEEFFECT_SLOTS(StyleEffect, (StyleEffect * self, s32 kind, StyleEffectParams *params,
                                    SceneNode *parent, LongVec3 *pos));
};

struct StyleEffect {
    STYLEEFFECT_FIELDS(StyleEffectMethods);
};

/* The reset and +0x0EC occupants as their callers call them (see the banner). */
typedef void (*StyleEffectSetParamsFn)(StyleEffect *self, StyleEffectParams *params);
typedef void (*StyleEffectUpdateFn)(StyleEffect *self, LongVec3 *pos);

extern StyleEffectMethods gStyleEffectMethods;
extern StyleEffectMethods *GetStyleEffectMethods(void); /* ObjMStyleActor.c; returns &gStyleEffectMethods */

/* The class's own methods, in address order (src/world/ObjMStyleActor.c).
 * Four are declared WITHOUT a prototype on purpose: each is one-parameter,
 * but a caller in ObjMStyleActor.c passes a dead second argument that is
 * byte-load-bearing (the `arity-ok` notes there and in the reports). */
StyleEffect *New_StyleEffect(s32 kind, StyleEffectParams *params, SceneNode *parent,
                             LongVec3 *pos); /* BMemPMgrAlloc(sizeof(StyleEffect)), then ctor */
StyleEffect *StyleEffect__StyleEffect(StyleEffect *self, s32 kind, StyleEffectParams *params,
                                      SceneNode *parent, LongVec3 *pos);
void StyleEffect__Finalize(StyleEffect *self);
void StyleEffect__SetParams(StyleEffect *self, StyleEffectParams *params);
void StyleEffect__Update(StyleEffect *self, LongVec3 *pos);
void StyleEffect__InitByKind(StyleEffect *self, SceneNode *parent, LongVec3 *pos);
void StyleEffect__UpdateByKind(StyleEffect *self, LongVec3 *pos);
void StyleEffect__ReleaseByKind(StyleEffect *self);
void StyleEffect__PlaceModelChildren(StyleEffect *self, s32 reuse);
void StyleEffect__DriftModelChildren(); /* (StyleEffect *self); see above */
void StyleEffect__ReleaseModelChildren(StyleEffect *self);
void StyleEffect__BuildRandomSprites(); /* (StyleEffect *self); see above */
void StyleEffect__SpawnSprites(void *self, s32 unused, s32 variant, void *scale);
void StyleEffect__ReleaseSprites(StyleEffect *self);
void StyleEffect__SpawnPlainSprites(); /* (StyleEffect *self); see above */
void StyleEffect__RandomizeSprites();  /* (StyleEffect *self); see above */
void StyleEffect__ReleaseJitterSprites(StyleEffect *self);

#endif
