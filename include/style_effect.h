#ifndef STYLE_EFFECT_H
#define STYLE_EFFECT_H

#include "actor.h"
#include "variant_sprite.h"

/**
 * @file style_effect.h
 * @brief StyleEffect, the Actor the style layer keeps at an offset from the
 *        scene's target: a model, a row of three models, or five sprites.
 *
 * Methods in src/world/dream_scene.c, New_StyleEffect through
 * GetStyleEffectMethods.
 */

typedef struct StyleEffect StyleEffect;
typedef struct StyleEffectMethods StyleEffectMethods;
typedef struct StyleEffectParams StyleEffectParams;

/**
 * @brief What a StyleEffect carries and does per frame, by `kind` (the
 * switches in StyleEffect__InitByKind, UpdateByKind and ReleaseByKind).
 */
typedef enum StyleEffectKind {
    STYLE_EFFECT_MODEL_ROW = 0, /**< A model plus two copies in a row that spin and drift along z after 500 ticks. */
    STYLE_EFFECT_MODEL = 1, /**< A model only; nothing per frame. */
    STYLE_EFFECT_SPRITES = 2, /**< Five sprites shaped once at build (BuildRandomSprites); nothing per frame. */
    STYLE_EFFECT_JITTER_SPRITES = 3 /**< Five sprites; four re-shaped every frame (RandomizeSprites). */
} StyleEffectKind;

/**
 * @brief The 0x24-byte parameter block the reset slot (StyleEffect__SetParams)
 * copies in whole. The style layer builds every effect from one such block,
 * laid over the separately declared sStyleSpawnOffsetX .. sStyleSpawnColors,
 * and re-randomises it before each build.
 */
struct StyleEffectParams {
    /* +0x000 */ LongVec3 offset; /**< Added to the caller's position (InitByKind, UpdateByKind). */
    /* +0x00C */ Ratio16 *rotation; /**< Ratio16[3] degrees: self's and the model children's rotation. */
    /* +0x010 */ Ratio16 *scale; /**< Ratio16[3]: their scale; scale[0].num also scales the model-child spacing. */
    /* +0x014 */ s32 modelChildLayout; /**< 0: no model children; 1..4 index sModelChildSpacing, 1-2 along x, 3-4 along y. */
    /* +0x018 */ s32 tableIndex;  /**< Index into sModelChildDriftZ and sSpriteShiftX. */
    /* +0x01C */ ColorRgb *color; /**< Every sprite's colour (SpawnSprites). */
    /* +0x020 */ ColorRgb *altColor; /**< sprites[1]'s colour instead, when non-NULL (BuildRandomSprites). */
};

/**
 * @brief StyleEffect's slots: Actor's, and no own ones. The overrides are
 * the ctor, finalize, reset (+0x040, StyleEffect__SetParams) and +0x0EC
 * (StyleEffect__Update), 59 words in all.
 */
/* clang-format off */
#define STYLEEFFECT_SLOTS(Self, CtorParams)                                                         \
    ACTOR_SLOTS(Self, CtorParams) /* no own slots; the last, StyleEffect__Update, at +0x0EC */
/* clang-format on */

/**
 * @brief StyleEffect's fields: Actor's (whose `pendingExtra` holds the
 * kind), then the parameter block and the kind's children.
 */
/* clang-format off */
#define STYLEEFFECT_FIELDS(Methods)                                                                 \
    ACTOR_FIELDS(Methods);                                                                         \
    /* +0x058 */ StyleEffectParams params; /* the reset slot's copy (StyleEffect__SetParams) */      \
    /* +0x07C */ Actor *modelChildren[2]; /* STYLE_EFFECT_MODEL_ROW: New_Actor children (PlaceModelChildren) */    \
    /* +0x084 */ VariantSprite *sprites[5]   /* the two sprite kinds: New_VariantSprite children (SpawnSprites) */
/* clang-format on */

/** @brief StyleEffect's method table (see STYLEEFFECT_SLOTS). */
struct StyleEffectMethods {
    STYLEEFFECT_SLOTS(StyleEffect, (StyleEffect * self, s32 kind, StyleEffectParams *params,
                                    SceneNode *parent, LongVec3 *pos));
};

/**
 * @brief An Actor the style layer places at an offset from the target
 * position and keeps there (class id 0xEF34): every frame it moves to pos +
 * offset, plus however far the viewport's viewpoint has risen or fallen since
 * it was built. Its ctor chains to Actor's; no class derives from it.
 *
 * Who builds it: StyleBuildEffectSlots, on the style layer's first tick,
 * calls SetStyleEffectSources with the scene's DREAMER.TMD resource, ETC.TIM
 * image and viewport, then StyleFillEffectKind0..3 fill sStyleEffectSlots
 * with New_StyleEffect(kind, params, sStyleGrid, pos): several of kind 0, of
 * kind 1 for style variant 2, then one of kind 3 (variant 0) or kind 2
 * (variant 2). StyleUpdateEffectSlots runs every slot's update with the
 * target position, and StyleReleaseEffectSlots releases them.
 *
 * Lifecycle:
 *  - ctor(kind, params, parent, pos): Actor's ctor, `state` 0, `kind` into
 *    Actor's `pendingExtra`, then reset(params), then InitByKind: attach
 *    under `parent` at pos + offset with the params' rotation and scale,
 *    snapshot the viewpoint y, link a model (kinds 0 and 1) and build the
 *    kind's children;
 *  - reset (+0x040), SetParams: copies the params block and zeroes `tick`.
 *    It takes the block where SceneNode's slot takes nothing, so the ctor
 *    calls it through StyleEffectSetParamsFn;
 *  - update (+0x0EC, Actor's setPendingExtra slot), Update: counts `tick`,
 *    then UpdateByKind follows the target and runs the kind's per-frame
 *    step. StyleUpdateEffectSlots calls it through StyleEffectUpdateFn;
 *    because the override replaces pendingExtra's setter, nothing else
 *    rewrites `kind`;
 *  - finalize: ReleaseByKind (the kind's children), then Actor's finalize.
 */
struct StyleEffect {
    STYLEEFFECT_FIELDS(StyleEffectMethods);
}; /* 0x98 bytes: New_StyleEffect */

/** @brief The reset slot as the ctor calls it (StyleEffect__SetParams). */
typedef void (*StyleEffectSetParamsFn)(StyleEffect *self, StyleEffectParams *params);

/** @brief Slot +0x0EC as StyleUpdateEffectSlots calls it (StyleEffect__Update). */
typedef void (*StyleEffectUpdateFn)(StyleEffect *self, LongVec3 *pos);

/** @brief StyleEffect's method table (see StyleEffectMethods). */
extern StyleEffectMethods gStyleEffectMethods;

/**
 * @brief Returns StyleEffect's method table.
 * @return &gStyleEffectMethods.
 */
extern StyleEffectMethods *GetStyleEffectMethods(void);

/**
 * @brief Allocates a StyleEffect from the BMemPMgr pool and constructs it.
 * @param kind   enum StyleEffectKind.
 * @param params The parameter block, copied.
 * @param parent The node it attaches under.
 * @param pos    The target position it is placed relative to.
 * @return The new effect, or NULL when the pool is exhausted or the ctor fails.
 */
StyleEffect *New_StyleEffect(s32 kind, StyleEffectParams *params, SceneNode *parent, LongVec3 *pos);

/**
 * @brief Constructor (slot +0x008): Actor's ctor, the kind, the params, then
 * InitByKind places it and builds its parts.
 * @param self   The object being constructed.
 * @param kind   enum StyleEffectKind.
 * @param params The parameter block, copied.
 * @param parent The node it attaches under.
 * @param pos    The target position it is placed relative to.
 * @return self, or NULL when Actor's ctor fails.
 */
StyleEffect *StyleEffect__StyleEffect(StyleEffect *self, s32 kind, StyleEffectParams *params,
                                      SceneNode *parent, LongVec3 *pos);

/**
 * @brief Finalizer (slot +0x00C): releases the kind's children, then Actor's
 * finalizer.
 * @param self The effect.
 */
void StyleEffect__Finalize(StyleEffect *self);

/**
 * @brief reset (slot +0x040): copies `params` into the effect and zeroes
 * `tick`.
 * @param self   The effect.
 * @param params The parameter block.
 */
void StyleEffect__SetParams(StyleEffect *self, StyleEffectParams *params);

/**
 * @brief Slot +0x0EC, the per-frame update: counts `tick`, then follows the
 * target and runs the kind's step (UpdateByKind).
 * @param self The effect.
 * @param pos  The target position.
 */
void StyleEffect__Update(StyleEffect *self, LongVec3 *pos);

/**
 * @brief Snapshots the viewpoint y, attaches the effect under `parent` at
 * pos + offset with its rotation and scale, links the model for the model
 * kinds and builds the kind's children: the model row, five randomised
 * sprites (STYLE_EFFECT_SPRITES) or five plain ones (JITTER_SPRITES).
 * @param self   The effect.
 * @param parent The node it attaches under.
 * @param pos    The target position.
 */
void StyleEffect__InitByKind(StyleEffect *self, SceneNode *parent, LongVec3 *pos);

/**
 * @brief Moves the effect to pos + offset plus the viewpoint's y change since
 * it was built, then runs the kind's step: drift the model row, or jitter the
 * sprites.
 * @param self The effect.
 * @param pos  The target position.
 */
void StyleEffect__UpdateByKind(StyleEffect *self, LongVec3 *pos);

/**
 * @brief Releases the children the kind built.
 * @param self The effect.
 */
void StyleEffect__ReleaseByKind(StyleEffect *self);

/**
 * @brief Lays the two model children out in a row, (i + 1) spacings from the
 * effect along x or y by `modelChildLayout`; does nothing for layout 0.
 * @param self  The effect.
 * @param reuse 0 creates the children (sharing the effect's model), 1 only
 *              moves them back to their places.
 */
void StyleEffect__PlaceModelChildren(StyleEffect *self, s32 reuse);

/**
 * @brief The model row's per-frame step, on a StyleEffect `self`: after
 * MODEL_CHILD_DRIFT_DELAY ticks, spins the effect and both children and moves
 * the children along z, snapping them back to their layout every period;
 * always marks the effect's coordinate for recompute. Declared without a
 * prototype, because StyleEffect__UpdateByKind passes a second argument it
 * does not read.
 */
void StyleEffect__DriftModelChildren();

/**
 * @brief Releases the two model children, if the layout made any.
 * @param self The effect.
 */
void StyleEffect__ReleaseModelChildren(StyleEffect *self);

/**
 * @brief STYLE_EFFECT_SPRITES' build, on a StyleEffect `self`: five sprites,
 * all half-scale on an even rand(); then sprites[1] is shifted along x and
 * recoloured (tableIndex 2 and up) or made semi-transparent and rescaled, and
 * sprites[2] is hidden. Declared without a prototype, because
 * StyleEffect__InitByKind passes a second argument it does not read.
 */
void StyleEffect__BuildRandomSprites();

/**
 * @brief Creates the five sprites, attached to the effect at no offset, in
 * the params' colour, scaled by `scale` when it is non-NULL.
 * @param self    The StyleEffect.
 * @param unused  Not read.
 * @param variant The VariantSprite variant (its texture cell and CLUT).
 * @param scale   A Ratio16[3] scale, or NULL for the default.
 */
void StyleEffect__SpawnSprites(void *self, s32 unused, s32 variant, void *scale);

/**
 * @brief STYLE_EFFECT_SPRITES' release: the five sprites.
 * @param self The effect.
 */
void StyleEffect__ReleaseSprites(StyleEffect *self);

/**
 * @brief STYLE_EFFECT_JITTER_SPRITES' build, on a StyleEffect `self`: five
 * sprites of variant 0 at their default scale. Declared without a prototype,
 * because StyleEffect__InitByKind passes a second argument it does not read.
 */
void StyleEffect__SpawnPlainSprites();

/**
 * @brief STYLE_EFFECT_JITTER_SPRITES' per-frame step, on a StyleEffect
 * `self`: every sprite but the first takes a random streak shape
 * (sStyleEffectJitterScales) and a random whole-degree rotation. Declared
 * without a prototype, because StyleEffect__UpdateByKind passes a second
 * argument it does not read.
 */
void StyleEffect__RandomizeSprites();

/**
 * @brief STYLE_EFFECT_JITTER_SPRITES' release: the five sprites.
 * @param self The effect.
 */
void StyleEffect__ReleaseJitterSprites(StyleEffect *self);

#endif
