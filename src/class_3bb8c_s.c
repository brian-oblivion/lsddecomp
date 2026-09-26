/*
 * class_3bb8c_s -- 0x46D20..0x475F0, the private methods of the class whose
 * table is gStyleEffectMethods: StyleEffect, include/StyleEffect.h (round 88).
 * A 0x98-byte scene object built by New_StyleEffect with a `kind` 0..3: it
 * attaches itself under a parent at pos + offset, links a model for kinds
 * 0-1, and owns up to two child arrays -- two Actor model children laid
 * out in a row that spin and drift along z after frame 500 (kind 0), or
 * five VariantSprite sprites (a GsSPRITE at +0x64) that are randomised at build
 * time (kind 2) or every frame (kind 3, class_3bb8c_o.c). Entry points are
 * the class's ctor, update slot (+0x0EC) and dtor in class_3bb8c_r.c, via
 * StyleEffect__InitByKind / __UpdateByKind / __ReleaseByKind.
 *
 * All 10 matched (StyleEffect__DriftModelChildren, the last, in round 75).
 * Named round 70; tiers in the reports. Game-level role unknown.
 */
#include "common.h"
#include "Actor.h"
#include "VariantSprite.h"
#include "StyleEffect.h"

/* The class and its children: include/StyleEffect.h (the owner),
 * include/Actor.h (modelChildren) and include/VariantSprite.h (sprites). */

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 gSpriteShiftX[];
extern s32 gSpriteScaleLarge[];
extern s32 gSpriteScaleHalf[];
extern s32 gSpriteScaleSmall[];
extern LongVec3 gSpriteShiftScratch;

void AddVec3(LongVec3 *dst, LongVec3 *a, LongVec3 *b);
void AttachWithRotScale(Actor *node, void *parent, void *trans, void *rotation, void *scale);

/* Four of the class's one-parameter helpers are called here with a dead
 * second argument that is byte-load-bearing, so include/StyleEffect.h
 * declares them WITHOUT a prototype (old-style), which is what lets these
 * calls pass it:
 *  - StyleEffect__SpawnPlainSprites (class_3bb8c_o.c): its body WRITES
 *    $a1/$a2/$a3 to zero before any read, but InitByKind's retail emits
 *    `move a1,zero` at 0x80056624;
 *  - StyleEffect__RandomizeSprites (class_3bb8c_o.c): its body reads only
 *    $a0 (`addiu s0,a0,136`), but UpdateByKind's retail emits `move a1,s1`
 *    at 0x800566FC;
 *  - StyleEffect__BuildRandomSprites (below): its body reads only $a0
 *    (`move s1,a0`); InitByKind's retail emits `move a1,zero` at 0x80056614;
 *  - StyleEffect__DriftModelChildren (below): its body writes $a1 (`move
 *    a1,zero`) before any read; UpdateByKind's retail emits `move a1,s1` in
 *    the jal delay slot at 0x800566D8 (round 75).
 * NoOpIgnoreArgs (class_3bb8c_o.c, empty) is the same idiom. */
extern void NoOpIgnoreArgs();

/* New_VariantSprite: include/VariantSprite.h. */

/* Three globals class_3bb8c_o.c's Actor__func_56f5c captures once from its
 * parameters (declared there with the same types; track 4b, round 85):
 * gStyleEffectTmd is the Actor it ran on, called here through SceneNode's
 * +0x080 getSetUnk10Flag8 as that function calls it; gStyleEffectTim is
 * forwarded opaquely to New_VariantSprite as its third argument; gStyleEffectViewport's
 * pointee has a field at +0x018 that StyleEffect__InitByKind and
 * StyleEffect__UpdateByKind snapshot/diff via gStyleEffectBaseViewY. */
extern Actor *gStyleEffectTmd; /* the Actor Actor__func_56f5c ran on */
extern void *gStyleEffectTim;
extern void *gStyleEffectViewport;
extern s32 gStyleEffectBaseViewY;
extern s32 gStyleEffectModelIds[];

/* Called once, from the class's ctor (StyleEffect__StyleEffect): place self under
 * `parent` at pos + offset, then build the per-kind parts. Kinds 0 and 1
 * link a model fetched from gStyleEffectTmd by gStyleEffectModelIds[kind]; kind 0 also
 * gets two model children, kind 2 five randomised sprites, kind 3 five
 * plain sprites (StyleEffect__SpawnPlainSprites is StyleEffect__SpawnSprites(self,
 * 0, 0, NULL)). */
void StyleEffect__InitByKind(StyleEffect *self, SceneNode *parent, LongVec3 *pos) {
    LongVec3 local;
    s32 state;

    gStyleEffectBaseViewY = *(s32 *)((u8 *)gStyleEffectViewport + 0x18);
    AddVec3(&local, pos, &self->params.offset);
    AttachWithRotScale((Actor *)self, parent, &local, self->params.rotation, self->params.scale);

    state = self->pendingExtra;
    if (state < 2) {
        s32 ret = gStyleEffectTmd->methods->setBackClip(gStyleEffectTmd, gStyleEffectModelIds[state]);
        SceneNode__LinkModel((SceneNode *)self, (void *)ret);
        state = self->pendingExtra;
    }

    switch (state) {
        case 0:
            StyleEffect__PlaceModelChildren(self, 0);
            break;
        case 2:
            StyleEffect__BuildRandomSprites(self, 0);
            break;
        case 3:
            StyleEffect__SpawnPlainSprites(self, 0);
            break;
        default:
            break;
    }
}

/* Called every frame from the class's slot +0x0EC (StyleEffect__Update, right
 * after it increments `tick`): set self's translation (Actor's
 * setTranslation) to pos + offset, plus however far gStyleEffectViewport's +0x018
 * word has moved since StyleEffect__InitByKind snapshotted it, then run the
 * per-kind update. */
void StyleEffect__UpdateByKind(StyleEffect *self, LongVec3 *pos) {
    LongVec3 local;

    AddVec3(&local, pos, &self->params.offset);
    local.y += *(s32 *)((u8 *)gStyleEffectViewport + 0x18) - gStyleEffectBaseViewY;
    self->methods->setTranslation(self, &local);

    switch (self->pendingExtra) {
        case 0:
            StyleEffect__DriftModelChildren(self, pos);
            break;
        case 2:
            NoOpIgnoreArgs(self, pos);
            break;
        case 3:
            StyleEffect__RandomizeSprites(self, pos);
            break;
        default:
            break;
    }
}

/* Called from the class's dtor (StyleEffect__Finalize): release whichever child
 * array this kind built (kinds 2 and 3 both release `sprites`, through two
 * identical class_3bb8c_o.c functions). */
void StyleEffect__ReleaseByKind(StyleEffect *self) {
    switch (self->pendingExtra) {
        case 0:
            StyleEffect__ReleaseModelChildren(self);
            break;
        case 2:
            StyleEffect__ReleaseSprites(self);
            break;
        case 3:
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
    LongVec3 accum;
    Actor **p;
    s32 i;
    s32 count = self->params.modelChildLayout;

    if (count == 0) {
        return;
    }
    accum = gModelChildOffsetInit;
    p = self->modelChildren;
    for (i = 0; i < 2; i++, p++) {
        if (count < 3) {
            accum.x += *(s16 *)self->params.scale * gModelChildSpacing[count];
        } else {
            accum.y += gModelChildSpacing[count];
        }
        if (reuse) {
            Actor *child = *p;
            child->methods->setTranslation(child, &accum);
        } else {
            Actor *child = New_Actor();
            *p = child;
            SceneNode__LinkModel((SceneNode *)child, self->model);
            AttachWithRotScale(*p, self, &accum, self->params.rotation, self->params.scale);
        }
    }
}

/* Per-`tableIndex` z step for the model children (0 = no drift), also the
 * divisor of the 24500 / step reset period below. Same index space as
 * gSpriteShiftX. */
extern s32 gModelChildDriftZ[];
/* All-zero LongVec3, the start value of each child's per-frame z delta. */
extern LongVec3 gModelChildDriftInit;
/* Ratio triple {0/1, 1/10, 0/1}: the per-frame rotation increment
 * updateRotation(.., 0, ..) adds to self and to each model child. */
extern s32 gSpinRotStep[];

/* After 500 frames (tick >= 0x1F5), for kinds with model children and a
 * nonzero gModelChildDriftZ step: spin self and both children, move the
 * children along z, and every 24500 / step frames snap them back to their
 * layout. Always marks self's coord2 for recompute.
 *
 * Matched round 75: the step is read straight from the table in the guard,
 * and the loop's pointer is taken again after the call -- CSE turns that
 * second &gModelChildDriftZ[idx] into retail's `move s4,s1`. */
void StyleEffect__DriftModelChildren(StyleEffect *self) {
    s32 idx;
    s32 accumOffset;
    s32 divq;
    s32 modend;
    Actor **p;
    s32 i;
    s32 *stepZ;

    idx = self->params.tableIndex;
    if (self->params.modelChildLayout != 0 && gModelChildDriftZ[idx] != 0 && (u32)self->tick >= 0x1F5) {
        p = self->modelChildren;
        self->methods->updateRotation(self, 0, gSpinRotStep);
        i = 0;
        stepZ = &gModelChildDriftZ[idx];
        accumOffset = 0;
        for (; i < 2; i++) {
            LongVec3 local = gModelChildDriftInit;
            local.z += accumOffset + *stepZ;
            (*p)->methods->addTranslation(*p, &local);
            accumOffset += 3;
            (*p)->methods->updateRotation(*p, 0, gSpinRotStep);
            p++;
        }

        divq = 24500 / gModelChildDriftZ[idx];
        modend = self->tick;
        if (divq >= 0) {
            if ((u32)modend % (u32)divq == 0) {
                StyleEffect__PlaceModelChildren(self, 1);
            }
        } else {
            u32 adivq = ~divq + 1;
            if ((u32)modend % adivq == 0) {
                StyleEffect__PlaceModelChildren(self, 1);
            }
        }
    }
    self->coord2->flg = 0;
}

/* Release the two model children, if this layout made any. */
void StyleEffect__ReleaseModelChildren(StyleEffect *self) {
    if (self->params.modelChildLayout != 0) {
        ReleaseBasicClassArray((void **)self->modelChildren, 2);
    }
}

/* Kind 2's init: five sprites, all scaled by gSpriteScaleHalf on an even
 * rand(); then sprites[1] is either shifted along x by
 * gSpriteShiftX[tableIndex] and recoloured (tableIndex >= 2) or made
 * semi-transparent (rate 0) and rescaled, and sprites[2] is hidden. */
void StyleEffect__BuildRandomSprites(StyleEffect *self) {
    s32 parity = rand() % 2;
    void *tblOrNull = parity ? NULL : gSpriteScaleHalf;
    VariantSprite *child;
    SpriteRgb *arg;

    StyleEffect__SpawnSprites(self, 0, 0, tblOrNull);

    if (self->params.tableIndex >= 2) {
        VariantSpriteMethods *m;

        child = self->sprites[1];
        gSpriteShiftScratch.x = gSpriteShiftX[self->params.tableIndex];
        /* Called directly, not through the child's table: the child is a
         * sprite (VariantSprite, a Sprite), not an Actor, and the function only
         * touches the SceneNode coord2 both share. */
        Actor__AddTranslation((Actor *)child, &gSpriteShiftScratch);
        m = child->methods;
        arg = (self->params.altColor != NULL) ? self->params.altColor : self->params.color;
        m->setColor(child, arg);
    } else {
        child = self->sprites[1];
        child->methods->setSemiTrans(child, 1);
        child->methods->setSemiTransRate(child, 0);
        child->methods->updateScale(child, 1, (parity != 0) ? gSpriteScaleLarge : gSpriteScaleSmall);
    }

    self->sprites[2]->methods->setDisplay(self->sprites[2], 0);
}

/* Create the five sprites (New_VariantSprite), attach each to self at no offset,
 * give each self's colour (Sprite's setColor sets GsSPRITE r,g,b), and
 * assign `scale` as their scale when non-NULL. `self` stays `void *`: it is
 * the prototype class_3bb8c_o.c calls through, and a typed local alias of
 * it costs a callee-saved register (see this function's report). */
void StyleEffect__SpawnSprites(void *self, s32 unused, s32 variant, void *scale) {
    VariantSprite **p = ((StyleEffect *)self)->sprites;
    VariantSprite *node;
    s32 i;

    for (i = 0; i < 5; i++, p++) {
        node = New_VariantSprite(variant, 0, gStyleEffectTim);
        *p = node;
        node->methods->attachToParent(node, self, 0);
        (*p)->methods->setColor(*p, ((StyleEffect *)self)->params.color);
        if (scale != 0) {
            (*p)->methods->updateScale(*p, 1, scale);
        }
    }
}
