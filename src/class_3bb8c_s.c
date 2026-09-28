/*
 * class_3bb8c_s -- StyleEffect's per-kind work (include/StyleEffect.h).
 *
 * The three switches on `kind` (StyleEffectKind) that its ctor, update and
 * finalize run: InitByKind (attach at pos + offset, link the model, build
 * the kind's children), UpdateByKind (follow pos + offset and the
 * viewport's viewpoint y, then the kind's per-frame step) and
 * ReleaseByKind. Then what they call for the model-row kind -- lay out,
 * drift and release the two model children -- and for the sprite kinds,
 * build the five sprites. Two small helpers every kind uses sit among
 * them: AddVec3 and AttachWithRotScale (attach, then set the rotation and
 * scale). The slot occupants themselves are in class_3bb8c_r.c; the rest
 * of the sprite helpers, and SetStyleEffectSources, in class_3bb8c_o.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "Actor.h"
#include "VariantSprite.h"
#include "StyleEffect.h"
#include "Viewport.h"

/* The class and its children: include/StyleEffect.h (the owner),
 * include/Actor.h (modelChildren) and include/VariantSprite.h (sprites). */

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 gSpriteShiftX[];
extern Ratio16 gSpriteScaleLarge[3];
extern Ratio16 gSpriteScaleHalf[3];
extern Ratio16 gSpriteScaleSmall[3];
extern LongVec3 gSpriteShiftScratch;

void AddVec3(LongVec3 *dst, LongVec3 *a, LongVec3 *b);
void AttachWithRotScale(Actor *node, void *parent, void *trans, void *rotation, void *scale);

/* MATCHING: StyleEffect__SpawnPlainSprites, __RandomizeSprites,
 * __BuildRandomSprites and __DriftModelChildren read only `self`, but the
 * calls below pass a second, dead argument that retail loads, so
 * include/StyleEffect.h declares them without a prototype. NoOpIgnoreArgs
 * (class_3bb8c_o.c, empty) is declared the same way here. */
extern void NoOpIgnoreArgs();

/* New_VariantSprite: include/VariantSprite.h. */

/* What SetStyleEffectSources (class_3bb8c_o.c) recorded, declared there
 * with the same types: the DREAMER.TMD Actor the model kinds fetch their
 * model from (setBackClip), the TIM image New_VariantSprite is handed, and
 * the scene's Viewport, whose viewpoint y (refView.vp.y) InitByKind
 * snapshots into gStyleEffectBaseViewY and UpdateByKind follows. The
 * viewport stays `void *` because that is how class_3bb8c_o.c declares it. */
extern Actor *gStyleEffectTmd; /* the Actor SetStyleEffectSources ran on */
extern void *gStyleEffectTim;
extern Viewport *gStyleEffectViewport;
extern s32 gStyleEffectBaseViewY;
extern s32 gStyleEffectModelIds[];

/* Called once, from the class's ctor (StyleEffect__StyleEffect): snapshot the
 * viewpoint y, place self under `parent` at pos + offset, then build the
 * per-kind parts. The two model kinds link a model fetched from
 * gStyleEffectTmd by gStyleEffectModelIds[kind], and STYLE_EFFECT_MODEL_ROW
 * also gets its two model children; STYLE_EFFECT_SPRITES gets five
 * randomised sprites, STYLE_EFFECT_JITTER_SPRITES five plain ones
 * (StyleEffect__SpawnPlainSprites is StyleEffect__SpawnSprites(self, 0, 0,
 * NULL)). */
void StyleEffect__InitByKind(StyleEffect *self, SceneNode *parent, LongVec3 *pos) {
    LongVec3 placed;
    s32 kind;

    gStyleEffectBaseViewY = gStyleEffectViewport->refView.vp.y;
    AddVec3(&placed, pos, &self->params.offset);
    AttachWithRotScale((Actor *)self, parent, &placed, self->params.rotation, self->params.scale);

    kind = self->pendingExtra;
    if (kind <= STYLE_EFFECT_MODEL) {
        s32 model = gStyleEffectTmd->methods->setBackClip(gStyleEffectTmd, gStyleEffectModelIds[kind]);
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
    placed.y += gStyleEffectViewport->refView.vp.y - gStyleEffectBaseViewY;
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
 * identical class_3bb8c_o.c functions). */
void StyleEffect__ReleaseByKind(StyleEffect *self) {
    switch (self->pendingExtra) {
        case STYLE_EFFECT_MODEL_ROW:
            StyleEffect__ReleaseModelChildren(self);
            break;
        case STYLE_EFFECT_SPRITES:
            StyleEffect__ReleaseSprites(self);
            break;
        case STYLE_EFFECT_JITTER_SPRITES:
            StyleEffect__ReleaseSpritesB(self);
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
 * gModelChildSpacing[modelChildLayout], along x (scaled by scale's x
 * numerator) for layouts 1-2 and along y for 3-4. reuse = 0 creates them
 * (New_Actor, sharing the owner's model, attached to the owner);
 * reuse = 1 only resets their translation (setTranslation). */
extern LongVec3 gModelChildOffsetInit;
extern s32 gModelChildSpacing[];

void StyleEffect__PlaceModelChildren(StyleEffect *self, s32 reuse) {
    LongVec3 childPos;
    Actor **slot;
    s32 i;
    s32 layout = self->params.modelChildLayout;

    if (layout == 0) {
        return;
    }
    childPos = gModelChildOffsetInit;
    slot = self->modelChildren;
    for (i = 0; i < ARRAY_COUNT(self->modelChildren); i++, slot++) {
        if (layout < 3) {
            childPos.x += self->params.scale[0].num * gModelChildSpacing[layout];
        } else {
            childPos.y += gModelChildSpacing[layout];
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
 * gSpriteShiftX. */
extern s32 gModelChildDriftZ[];
/* All-zero LongVec3, the start value of each child's per-frame z delta. */
extern LongVec3 gModelChildDriftInit;
/* Ratio triple {0/1, 1/10, 0/1}: the per-frame rotation increment
 * updateRotation(.., 0, ..) adds to self and to each model child. */
extern Ratio16 gSpinRotStep[3];

/* Ticks (StyleEffect::tick) before the model children start to drift. */
#define MODEL_CHILD_DRIFT_DELAY 500
/* The z distance a child drifts before it snaps back: the reset period is
 * this over the child's per-tick step, gModelChildDriftZ[tableIndex]. */
#define MODEL_CHILD_DRIFT_RANGE 24500

/* Once tick passes MODEL_CHILD_DRIFT_DELAY, for a layout with model
 * children and a nonzero gModelChildDriftZ step: spin self and both
 * children, move child i along z by step + 3 * i, and every
 * MODEL_CHILD_DRIFT_RANGE / step ticks (the period's magnitude, whatever
 * the step's sign) snap them back to their layout. Always marks self's
 * coord2 for recompute. */
void StyleEffect__DriftModelChildren(StyleEffect *self) {
    s32 tableIndex;
    s32 extraZ;
    s32 period;
    s32 tick;
    Actor **slot;
    s32 i;
    s32 *stepZ;

    tableIndex = self->params.tableIndex;
    if (self->params.modelChildLayout != 0 && gModelChildDriftZ[tableIndex] != 0 &&
        (u32)self->tick > MODEL_CHILD_DRIFT_DELAY) {
        slot = self->modelChildren;
        self->methods->updateRotation(self, 0, gSpinRotStep);
        i = 0;
        /* MATCHING: the guard reads the step from the table and the pointer
         * is taken only here, after the call; either held earlier in a
         * local swaps two registers. */
        stepZ = &gModelChildDriftZ[tableIndex];
        extraZ = 0;
        for (; i < ARRAY_COUNT(self->modelChildren); i++) {
            LongVec3 delta = gModelChildDriftInit;
            delta.z += extraZ + *stepZ;
            (*slot)->methods->addTranslation(*slot, &delta);
            extraZ += 3;
            (*slot)->methods->updateRotation(*slot, 0, gSpinRotStep);
            slot++;
        }

        period = MODEL_CHILD_DRIFT_RANGE / gModelChildDriftZ[tableIndex];
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
        ReleaseBasicClassArray((void **)self->modelChildren, ARRAY_COUNT(self->modelChildren));
    }
}

/* Kind 2's init: five sprites, all scaled by gSpriteScaleHalf on an even
 * rand(); then sprites[1] is either shifted along x by
 * gSpriteShiftX[tableIndex] and recoloured (tableIndex >= 2) or made
 * semi-transparent (rate 0) and rescaled, and sprites[2] is hidden. */
void StyleEffect__BuildRandomSprites(StyleEffect *self) {
    s32 parity = rand() % 2;
    void *scale = parity ? NULL : gSpriteScaleHalf;
    VariantSprite *sprite;
    SpriteRgb *color;

    StyleEffect__SpawnSprites(self, 0, 0, scale);

    if (self->params.tableIndex >= 2) {
        VariantSpriteMethods *methods;

        sprite = self->sprites[1];
        gSpriteShiftScratch.x = gSpriteShiftX[self->params.tableIndex];
        /* Called directly, not through the sprite's table: a VariantSprite
         * is a Sprite, not an Actor, and the function only touches the
         * SceneNode coord2 both share. */
        Actor__AddTranslation((Actor *)sprite, &gSpriteShiftScratch);
        methods = sprite->methods;
        color = (self->params.altColor != NULL) ? self->params.altColor : self->params.color;
        methods->setColor(sprite, color);
    } else {
        sprite = self->sprites[1];
        sprite->methods->setSemiTransOn(sprite, 1);
        sprite->methods->setSemiTransRate(sprite, 0);
        sprite->methods->updateScale(sprite, 1, (parity != 0) ? gSpriteScaleLarge : gSpriteScaleSmall);
    }

    self->sprites[2]->methods->setDisplay(self->sprites[2], 0);
}

/* Create the five sprites (New_VariantSprite), attach each to self at no offset,
 * give each self's colour (Sprite's setColor sets GsSPRITE r,g,b), and
 * assign `scale` as their scale when non-NULL. `self` stays `void *`: it is
 * the prototype class_3bb8c_o.c calls through, and a typed local alias of
 * it costs a callee-saved register (see this function's report). */
void StyleEffect__SpawnSprites(void *self, s32 unused, s32 variant, void *scale) {
    VariantSprite **slot = ((StyleEffect *)self)->sprites;
    VariantSprite *sprite;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(((StyleEffect *)self)->sprites); i++, slot++) {
        sprite = New_VariantSprite(variant, 0, gStyleEffectTim);
        *slot = sprite;
        sprite->methods->attachToParent(sprite, self, 0);
        (*slot)->methods->setColor(*slot, ((StyleEffect *)self)->params.color);
        if (scale != 0) {
            (*slot)->methods->updateScale(*slot, 1, scale);
        }
    }
}
