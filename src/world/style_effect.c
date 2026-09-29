/*
 * StyleEffect's methods (include/style_effect.h: the Actor the style layer
 * keeps at an offset from the scene's target), in ROM order:
 *  - its slot occupants (ctor, finalize, reset = SetParams, +0x0EC =
 *    Update) and its `New_` allocator;
 *  - the three switches on `kind` its ctor, update and finalize run:
 *    InitByKind (attach at pos + offset, link the model, build the kind's
 *    children), UpdateByKind (follow pos + offset and the viewport's
 *    viewpoint y, then the kind's per-frame step) and ReleaseByKind;
 *  - the model-row kind's children (lay out, drift, release), among them
 *    the helpers AddVec3 and AttachWithRotScale, and the sprite kinds'
 *    five sprites: BuildRandomSprites, SpawnSprites, SpawnPlainSprites,
 *    RandomizeSprites (the per-frame re-shape), NoOpIgnoreArgs (the empty
 *    step) and the identical releases ReleaseSprites / ReleaseJitterSprites;
 *  - GetStyleEffectMethods, then SetStyleEffectSources, which records the
 *    TMD resource, TIM image and viewport every StyleEffect draws from.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "actor.h"
#include "style_effect.h"
#include "variant_sprite.h"
#include "viewport.h"
#include "tmd_model.h"
#include "bmem_pmgr.h"
#include <rand.h>

/* Ticks (StyleEffect::tick) before the model children start to drift. */
#define MODEL_CHILD_DRIFT_DELAY 500
/* The z distance a child drifts before it snaps back: the reset period is
 * this over the child's per-tick step, sModelChildDriftZ[tableIndex]. */
#define MODEL_CHILD_DRIFT_RANGE 24500

StyleEffect *New_StyleEffect(s32 kind, StyleEffectParams *params, SceneNode *parent, LongVec3 *pos) {
    StyleEffect *self = BMemPMgrAlloc(sizeof(StyleEffect));

    if (self != NULL) {
        if (GetStyleEffectMethods()->ctor(self, kind, params, parent, pos) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}

/* `kind` goes into Actor's pendingExtra (+0x054): see include/style_effect.h. */
StyleEffect *StyleEffect__StyleEffect(StyleEffect *self, s32 kind, StyleEffectParams *params,
                                      SceneNode *parent, LongVec3 *pos) {
    if (GetActorMethods()->ctor((Actor *)self) == NULL) {
        goto fail;
    }
    self->methods = GetStyleEffectMethods();
    self->state = 0;
    self->pendingExtra = kind;
    ((StyleEffectSetParamsFn)self->methods->reset)(self, params);
    StyleEffect__InitByKind(self, parent, pos);
    return self;
fail:
    return NULL;
}

/* Actor's finalize (SceneNode__Finalize) returns nothing, so neither does this. */
void StyleEffect__Finalize(StyleEffect *self) {
    StyleEffect__ReleaseByKind(self);
    GetActorMethods()->finalize((Actor *)self);
}

void StyleEffect__SetParams(StyleEffect *self, StyleEffectParams *params) {
    self->params = *params; /* MATCHING: one struct copy: the 4-aligned block moves 4 words a loop */
    self->tick = 0;
}

/* `pos` arrives from StyleUpdateEffectSlots and is forwarded untouched. */
void StyleEffect__Update(StyleEffect *self, LongVec3 *pos) {
    self->tick = self->tick + 1;
    StyleEffect__UpdateByKind(self, pos);
}

/* The class and its children: include/style_effect.h (the owner),
 * include/actor.h (modelChildren) and include/variant_sprite.h (sprites). */

extern s32 sSpriteShiftX[];
extern Ratio16 sSpriteScaleLarge[3];
extern Ratio16 sSpriteScaleHalf[3];
extern Ratio16 sSpriteScaleSmall[3];
extern LongVec3 sSpriteShiftScratch;

void AddVec3(LongVec3 *dst, LongVec3 *a, LongVec3 *b);
void AttachWithRotScale(Actor *node, void *parent, void *trans, void *rotation, void *scale);

/* MATCHING: unprototyped, as four StyleEffect__ helpers are: the calls below load a dead 2nd argument */
extern void NoOpIgnoreArgs();

/* New_VariantSprite: include/variant_sprite.h. */

/* What SetStyleEffectSources (at the end of the file) records: the
 * DREAMER.TMD Actor the model kinds fetch their
 * model from (setBackClip), the TIM image New_VariantSprite is handed, and
 * the scene's Viewport, whose viewpoint y (refView.vp.y) InitByKind
 * snapshots into sStyleEffectBaseViewY and UpdateByKind follows. */
extern Actor *sStyleEffectTmd; /* the Actor SetStyleEffectSources ran on */
extern void *sStyleEffectTim;
extern Viewport *sStyleEffectViewport;
extern s32 sStyleEffectBaseViewY;
extern s32 sStyleEffectModelIds[3];

/* Called once, from the class's ctor (StyleEffect__StyleEffect): snapshot the
 * viewpoint y, place self under `parent` at pos + offset, then build the
 * per-kind parts. The two model kinds link a model fetched from
 * sStyleEffectTmd by sStyleEffectModelIds[kind], and STYLE_EFFECT_MODEL_ROW
 * also gets its two model children; STYLE_EFFECT_SPRITES gets five
 * randomised sprites, STYLE_EFFECT_JITTER_SPRITES five plain ones
 * (StyleEffect__SpawnPlainSprites is StyleEffect__SpawnSprites(self, 0, 0,
 * NULL)). */
void StyleEffect__InitByKind(StyleEffect *self, SceneNode *parent, LongVec3 *pos) {
    LongVec3 placed;
    s32 kind;

    sStyleEffectBaseViewY = sStyleEffectViewport->refView.vp.y;
    AddVec3(&placed, pos, &self->params.offset);
    AttachWithRotScale((Actor *)self, parent, &placed, self->params.rotation, self->params.scale);

    kind = self->pendingExtra;
    if (kind <= STYLE_EFFECT_MODEL) {
        s32 model = sStyleEffectTmd->methods->setBackClip(sStyleEffectTmd, sStyleEffectModelIds[kind]);
        SceneNode__LinkModel((SceneNode *)self, (void *)model);
        kind = self->pendingExtra;
    }

    switch (kind) {
        case STYLE_EFFECT_MODEL_ROW:
            StyleEffect__PlaceModelChildren(self, 0);
            break;
        case STYLE_EFFECT_SPRITES:
            StyleEffect__BuildRandomSprites(self, 0);
            break;
        case STYLE_EFFECT_JITTER_SPRITES:
            StyleEffect__SpawnPlainSprites(self, 0);
            break;
        default:
            break;
    }
}

/* Called every frame from the class's slot +0x0EC (StyleEffect__Update, right
 * after it increments `tick`): set self's translation (Actor's
 * setTranslation) to pos + offset, plus however far the viewpoint y has moved
 * since StyleEffect__InitByKind snapshotted it, then run the
 * per-kind update. */
void StyleEffect__UpdateByKind(StyleEffect *self, LongVec3 *pos) {
    LongVec3 placed;

    AddVec3(&placed, pos, &self->params.offset);
    placed.y += sStyleEffectViewport->refView.vp.y - sStyleEffectBaseViewY;
    self->methods->setTranslation(self, &placed);

    switch (self->pendingExtra) {
        case STYLE_EFFECT_MODEL_ROW:
            StyleEffect__DriftModelChildren(self, pos);
            break;
        case STYLE_EFFECT_SPRITES:
            NoOpIgnoreArgs(self, pos);
            break;
        case STYLE_EFFECT_JITTER_SPRITES:
            StyleEffect__RandomizeSprites(self, pos);
            break;
        default:
            break;
    }
}

/* Called from the class's dtor (StyleEffect__Finalize): release whichever child
 * array this kind built (both sprite kinds release `sprites`, through two
 * identical functions below). */
void StyleEffect__ReleaseByKind(StyleEffect *self) {
    switch (self->pendingExtra) {
        case STYLE_EFFECT_MODEL_ROW:
            StyleEffect__ReleaseModelChildren(self);
            break;
        case STYLE_EFFECT_SPRITES:
            StyleEffect__ReleaseSprites(self);
            break;
        case STYLE_EFFECT_JITTER_SPRITES:
            StyleEffect__ReleaseJitterSprites(self);
            break;
        default:
            break;
    }
}

/* Plain Vec3 add: dst = a + b. Frameless -- no self/vtable involved. */
void AddVec3(LongVec3 *dst, LongVec3 *a, LongVec3 *b) {
    dst->x = a->x + b->x;
    dst->y = a->y + b->y;
    dst->z = a->z + b->z;
}

/* Attach `node` under `parent` at translation `trans`, then assign (set = 1)
 * its rotation and scale. Used on the owner itself and on each model child. */
void AttachWithRotScale(Actor *node, void *parent, void *trans, void *rotation, void *scale) {
    node->methods->attachToParent(node, parent, trans);
    node->methods->updateRotation(node, 1, rotation);
    node->methods->updateScale(node, 1, scale);
}

/* Lay the two model children out in a row: child i sits at (i + 1) *
 * sModelChildSpacing[modelChildLayout], along x (scaled by scale's x
 * numerator) for layouts 1-2 and along y for 3-4. reuse = 0 creates them
 * (New_Actor, sharing the owner's model, attached to the owner);
 * reuse = 1 only resets their translation (setTranslation). */
extern LongVec3 sModelChildOffsetInit;
extern s32 sModelChildSpacing[];

void StyleEffect__PlaceModelChildren(StyleEffect *self, s32 reuse) {
    LongVec3 childPos;
    Actor **slot;
    s32 i;
    s32 layout = self->params.modelChildLayout;

    if (layout == 0) {
        return;
    }
    childPos = sModelChildOffsetInit;
    slot = self->modelChildren;
    for (i = 0; i < ARRAY_COUNT(self->modelChildren); i++, slot++) {
        if (layout < 3) {
            childPos.x += self->params.scale[0].num * sModelChildSpacing[layout];
        } else {
            childPos.y += sModelChildSpacing[layout];
        }
        if (reuse) {
            Actor *child = *slot;
            child->methods->setTranslation(child, &childPos);
        } else {
            Actor *child = New_Actor();
            *slot = child;
            SceneNode__LinkModel((SceneNode *)child, self->model);
            AttachWithRotScale(*slot, self, &childPos, self->params.rotation, self->params.scale);
        }
    }
}

/* Per-`tableIndex` z step for the model children (0 = no drift), also the
 * divisor of MODEL_CHILD_DRIFT_RANGE for the reset period below. Same index space as
 * sSpriteShiftX. */
extern s32 sModelChildDriftZ[];
/* All-zero LongVec3, the start value of each child's per-frame z delta. */
extern LongVec3 sModelChildDriftInit;
/* Ratio triple {0/1, 1/10, 0/1}: the per-frame rotation increment
 * updateRotation(.., 0, ..) adds to self and to each model child. */
extern Ratio16 sSpinRotStep[3];

/* Once tick passes MODEL_CHILD_DRIFT_DELAY, for a layout with model
 * children and a nonzero sModelChildDriftZ step: spin self and both
 * children, move child i along z by step + 3 * i, and every
 * MODEL_CHILD_DRIFT_RANGE / step ticks (the period's magnitude, whatever
 * the step's sign) snap them back to their layout. Always marks self's
 * coord2 for recompute. */
/* MATCHING: unprototyped in style_effect.h: UpdateByKind's call loads a dead 2nd argument */
void StyleEffect__DriftModelChildren(StyleEffect *self) {
    s32 tableIndex;
    s32 extraZ;
    s32 period;
    s32 tick;
    Actor **slot;
    s32 i;
    s32 *stepZ;

    tableIndex = self->params.tableIndex;
    if (self->params.modelChildLayout != 0 && sModelChildDriftZ[tableIndex] != 0 &&
        (u32)self->tick > MODEL_CHILD_DRIFT_DELAY) {
        slot = self->modelChildren;
        self->methods->updateRotation(self, 0, sSpinRotStep);
        i = 0;
        /* MATCHING: the step's pointer is taken here, after the call; held earlier, two registers swap */
        stepZ = &sModelChildDriftZ[tableIndex];
        extraZ = 0;
        for (; i < ARRAY_COUNT(self->modelChildren); i++) {
            LongVec3 delta = sModelChildDriftInit;
            delta.z += extraZ + *stepZ;
            (*slot)->methods->addTranslation(*slot, &delta);
            extraZ += 3;
            (*slot)->methods->updateRotation(*slot, 0, sSpinRotStep);
            slot++;
        }

        period = MODEL_CHILD_DRIFT_RANGE / sModelChildDriftZ[tableIndex];
        tick = self->tick;
        if (period >= 0) {
            if ((u32)tick % (u32)period == 0) {
                StyleEffect__PlaceModelChildren(self, 1);
            }
        } else {
            u32 absPeriod = ~period + 1;
            if ((u32)tick % absPeriod == 0) {
                StyleEffect__PlaceModelChildren(self, 1);
            }
        }
    }
    self->coord2->flg = 0;
}

/* Release the two model children, if this layout made any. */
void StyleEffect__ReleaseModelChildren(StyleEffect *self) {
    if (self->params.modelChildLayout != 0) {
        ReleaseBasicClassArray((BasicClass **)self->modelChildren, ARRAY_COUNT(self->modelChildren));
    }
}

/* Kind 2's init: five sprites, all scaled by sSpriteScaleHalf on an even
 * rand(); then sprites[1] is either shifted along x by
 * sSpriteShiftX[tableIndex] and recoloured (tableIndex >= 2) or made
 * semi-transparent (rate 0) and rescaled, and sprites[2] is hidden. */
/* MATCHING: unprototyped in style_effect.h: InitByKind's call loads a dead 2nd argument */
void StyleEffect__BuildRandomSprites(StyleEffect *self) {
    s32 parity = rand() % 2;
    void *scale = parity ? NULL : sSpriteScaleHalf;
    VariantSprite *sprite;
    ColorRgb *color;

    StyleEffect__SpawnSprites(self, 0, 0, scale);

    if (self->params.tableIndex >= 2) {
        VariantSpriteMethods *methods;

        sprite = self->sprites[1];
        sSpriteShiftScratch.x = sSpriteShiftX[self->params.tableIndex];
        /* Called directly, not through the sprite's table: a VariantSprite
         * is a Sprite, not an Actor, and the function only touches the
         * SceneNode coord2 both share. */
        Actor__AddTranslation((Actor *)sprite, &sSpriteShiftScratch);
        methods = sprite->methods;
        color = (self->params.altColor != NULL) ? self->params.altColor : self->params.color;
        methods->setColor(sprite, color);
    } else {
        sprite = self->sprites[1];
        sprite->methods->setSemiTransOn(sprite, 1);
        sprite->methods->setSemiTransRate(sprite, 0);
        sprite->methods->updateScale(sprite, 1, (parity != 0) ? sSpriteScaleLarge : sSpriteScaleSmall);
    }

    self->sprites[2]->methods->setDisplay(self->sprites[2], 0);
}

/* Create the five sprites (New_VariantSprite), attach each to self at no offset,
 * give each self's colour (Sprite's setColor sets GsSPRITE r,g,b), and
 * assign `scale` as their scale when non-NULL. `self` is `void *`, the
 * prototype SpawnPlainSprites calls through. */
/* MATCHING: self stays void * and is cast at each use; a typed local alias does not match. */
void StyleEffect__SpawnSprites(void *self, s32 unused, s32 variant, void *scale) {
    VariantSprite **slot = ((StyleEffect *)self)->sprites;
    VariantSprite *sprite;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(((StyleEffect *)self)->sprites); i++, slot++) {
        sprite = New_VariantSprite(variant, 0, sStyleEffectTim);
        *slot = sprite;
        sprite->methods->attachToParent(sprite, self, 0);
        (*slot)->methods->setColor(*slot, ((StyleEffect *)self)->params.color);
        if (scale != 0) {
            (*slot)->methods->updateScale(*slot, 1, scale);
        }
    }
}

/* STYLE_EFFECT_SPRITES' per-frame step: nothing. Its caller passes
 * (self, pos), which it does not read. */
void NoOpIgnoreArgs(void) {}

/* ------------------------------------------------------------------ *
 * StyleEffect's sprite kinds (STYLE_EFFECT_SPRITES, _JITTER_SPRITES).
 * ------------------------------------------------------------------ */

/* Six scale tables, each three Ratio16s (x, y, z), for the jittering
 * sprites: a thin streak along y or along x, {1/16, 7/1}, {7/1, 1/16}, then
 * the same with 3 and 2; z is 1/1. VariantSprite__UpdateScale reads x and y. */
extern Ratio16 sStyleEffectJitterScales[6][3];

/* Kind 2's release; ReleaseJitterSprites, kind 3's, is the same body. */
void StyleEffect__ReleaseSprites(StyleEffect *self) {
    ReleaseBasicClassArray((BasicClass **)self->sprites, ARRAY_COUNT(self->sprites));
}

/* Five sprites of variant 0 at their default scale. */
/* MATCHING: unprototyped in style_effect.h: InitByKind's call loads a dead 2nd argument */
void StyleEffect__SpawnPlainSprites(StyleEffect *self) {
    StyleEffect__SpawnSprites(self, 0, 0, 0);
}

/* STYLE_EFFECT_JITTER_SPRITES' per-frame step: every sprite but the first
 * takes a random streak shape and a random whole-degree rotation. */
/* MATCHING: unprototyped in style_effect.h: UpdateByKind's call loads a dead 2nd argument */
void StyleEffect__RandomizeSprites(StyleEffect *self) {
    VariantSprite **sprite = &self->sprites[1];
    s32 i;

    for (i = 0; i < ARRAY_COUNT(self->sprites) - 1; i++, sprite++) {
        u32 pick = rand();

        (*sprite)->methods->updateScale(
            *sprite, 1, sStyleEffectJitterScales[pick % ARRAY_COUNT(sStyleEffectJitterScales)]);
        (*sprite)->sprite.rotate = (rand() % 360) * ONE; /* 4096ths of a degree */
    }
}

void StyleEffect__ReleaseJitterSprites(StyleEffect *self) {
    ReleaseBasicClassArray((BasicClass **)self->sprites, ARRAY_COUNT(self->sprites));
}

/* StyleEffect's table getter. */
StyleEffectMethods *GetStyleEffectMethods(void) {
    return &gStyleEffectMethods;
}

/* SetStyleEffectSources records what StyleEffect's methods draw from: the
 * scene's TMD resource, its TIM image and the viewport. The TMD resource's
 * getModel slot sits where Actor has setBackClip, hence the Actor view. */
extern s16 sStyleEffectClutPos[2];

/* Records the three sources, then points the first primitive of the TMD's
 * models 0 and 2 (sStyleEffectModelIds) at the CLUT at sStyleEffectClutPos. */
void SetStyleEffectSources(s32 unused, Actor *tmd, s32 tim, s32 viewport) {
    s32 i;
    TmdModel *model;

    sStyleEffectTmd = tmd;
    sStyleEffectTim = (void *)tim;
    sStyleEffectViewport = (Viewport *)viewport;
    i = 0;
    do {
        model = (TmdModel *)tmd->methods->setBackClip(tmd, sStyleEffectModelIds[i]);
        TmdModel__SetFirstPrimClut(model, sStyleEffectClutPos);
        i++;
    } while (i < 2);
}
