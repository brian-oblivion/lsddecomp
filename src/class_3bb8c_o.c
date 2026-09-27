/*
 * The tail of StyleEffect's sprite helpers, then the first half of Actor's
 * own methods (the second half is class_3bb8c_p.c). The file is cut by
 * address, not by class, so it holds two groups:
 *
 *  - StyleEffect (include/StyleEffect.h): the per-kind pieces for its two
 *    sprite kinds that StyleEffect__UpdateByKind and ReleaseByKind
 *    (class_3bb8c_s.c) call. SpawnPlainSprites builds the five sprites,
 *    RandomizeSprites re-shapes four of them every frame, NoOpIgnoreArgs is
 *    the empty per-frame step, and ReleaseSprites / ReleaseSpritesB are two
 *    identical functions that release them. Behind them, by address, sit the
 *    class's table getter and SetStyleEffectSources, which records the
 *    TMD resource, TIM image and viewport every StyleEffect draws from.
 *  - Actor (include/Actor.h), the base of TodActor, DreamSys and
 *    StyleEffect: New_Actor and the constructor, the child bookkeeping that
 *    keeps the grid manager and the frame clock in `grid` and `ticker`,
 *    Reset, NotifyMove (the hull sweep sent after a move),
 *    DispatchLinkCommand, and the translation setters that end in
 *    MoveLocalZ.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <rand.h>
#include "Actor.h"
#include "TmdModel.h"
#include "StyleEffect.h"
#include "FrameClock.h"
#include "GridCell.h"

/* STYLE_EFFECT_SPRITES' per-frame step: nothing. Its caller passes
 * (self, pos), which it does not read. */
void NoOpIgnoreArgs(void) {}

/* ------------------------------------------------------------------ *
 * StyleEffect's sprite kinds (STYLE_EFFECT_SPRITES, _JITTER_SPRITES).
 * ------------------------------------------------------------------ */

extern void ReleaseBasicClassArray(void **array, s32 count);

/* Six scale tables, each three Ratio16s (x, y, z), for the jittering
 * sprites: a thin streak along y or along x, {1/16, 7/1}, {7/1, 1/16}, then
 * the same with 3 and 2; z is 1/1. VariantSprite__UpdateScale reads x and y. */
extern Ratio16 gStyleEffectJitterScales[6][3];

/* Kind 2's release; ReleaseSpritesB, kind 3's, is the same body. */
void StyleEffect__ReleaseSprites(StyleEffect *self) {
    ReleaseBasicClassArray((void **)self->sprites, ARRAY_COUNT(self->sprites));
}

/* Five sprites of variant 0 at their default scale. */
void StyleEffect__SpawnPlainSprites(StyleEffect *self) {
    StyleEffect__SpawnSprites(self, 0, 0, 0);
}

/* STYLE_EFFECT_JITTER_SPRITES' per-frame step: every sprite but the first
 * takes a random streak shape and a random whole-degree rotation. */
void StyleEffect__RandomizeSprites(StyleEffect *self) {
    VariantSprite **sprite = &self->sprites[1];
    s32 i;

    for (i = 0; i < ARRAY_COUNT(self->sprites) - 1; i++, sprite++) {
        u32 pick = rand();

        (*sprite)->methods->updateScale(
            *sprite, 1, gStyleEffectJitterScales[pick % ARRAY_COUNT(gStyleEffectJitterScales)]);
        (*sprite)->sprite.rotate = (rand() % 360) * ONE; /* 4096ths of a degree */
    }
}

void StyleEffect__ReleaseSpritesB(StyleEffect *self) {
    ReleaseBasicClassArray((void **)self->sprites, ARRAY_COUNT(self->sprites));
}

/* StyleEffect's table getter. */
StyleEffectMethods *GetStyleEffectMethods(void) {
    return &gStyleEffectMethods;
}

/* What StyleEffect's methods (class_3bb8c_s.c, which declares the same
 * globals) draw from: the scene's TMD resource, its TIM image and the
 * viewport. The TMD resource's getModel slot sits where Actor has
 * setBackClip, hence the Actor view. */
extern Actor *gStyleEffectTmd;
extern void *gStyleEffectTim;
extern void *gStyleEffectViewport;
extern s32 gStyleEffectModelIds[3];
extern s16 gStyleEffectClutPos[2];

extern void TmdModel__SetFirstPrimClut(TmdModel *self, s16 *xy);

/* Records the three sources, then points the first primitive of the TMD's
 * models 0 and 2 (gStyleEffectModelIds) at the CLUT at gStyleEffectClutPos. */
void SetStyleEffectSources(s32 unused, Actor *tmd, s32 tim, s32 viewport) {
    s32 i;
    TmdModel *model;

    gStyleEffectTmd = tmd;
    gStyleEffectTim = (void *)tim;
    gStyleEffectViewport = (void *)viewport;
    i = 0;
    do {
        model = (TmdModel *)tmd->methods->setBackClip(tmd, gStyleEffectModelIds[i]);
        TmdModel__SetFirstPrimClut(model, gStyleEffectClutPos);
        i++;
    } while (i < 2);
}

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

void *New_Actor(void) {
    Actor *self = BMemPMgrAlloc(sizeof(Actor));

    if (self != NULL) {
        if (GetActorMethods()->ctor(self) != NULL) {
            return self;
        }
        BMemPMgrFree(self);
        return NULL;
    }
    return NULL;
}

Actor *Actor__Actor(Actor *self) {
    if (GetSceneNodeMethods()->ctor((SceneNode *)self) == NULL) {
        goto fail;
    }
    self->methods = GetActorMethods();
    self->state = 0;
    self->grid = NULL;
    self->ticker = NULL;
    self->methods->reset(self);
    return self;
fail:
    return NULL;
}

/* addChild, removeChild and removeAllChildren chain SceneNode's and keep
 * two companions: a StageMap child (class id 0x114, three nibbles) in
 * `grid`, a FrameClock child in `ticker`. */
void Actor__AddChild(Actor *self, BasicClass *child) {
    s32 classId;

    GetSceneNodeMethods()->addChild((SceneNode *)self, child);
    classId = child->methods->header;
    if ((classId & 0xFFF) == 0x114) {
        self->grid = (struct StageMap *)child;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->ticker = child;
    }
}

void Actor__RemoveChild(Actor *self, BasicClass *child) {
    s32 classId = child->methods->header;

    if ((classId & 0xFFF) == 0x114) {
        self->grid = NULL;
    } else if ((classId & CLASS_ID_ROOT_MASK) == FRAMECLOCK_CLASS_ID) {
        self->ticker = NULL;
    }
    GetSceneNodeMethods()->removeChild((SceneNode *)self, child);
}

void Actor__RemoveAllChildren(Actor *self) {
    self->grid = NULL;
    self->ticker = NULL;
    GetSceneNodeMethods()->removeAllChildren((SceneNode *)self);
}

/* The distance NotifyMove stretches the hull by until a move or
 * setLastOffsetValue sets one; no extra. */
void Actor__Reset(Actor *self) {
    self->lastOffsetValue = 300;
    self->pendingExtra = 0;
}

extern void RotateAndOffsetHullList(TmdHull *hull, s32 turn, s32 back, s32 delta);

/* notifyWithHull: SceneNode's, then, for events ACTOR_EVENT_UNSWEPT to
 * ACTOR_EVENT_MOVED_Y on a model with bounds, the model's hull goes to the
 * parents through transformAndNotifyParents. For a move it is first
 * stretched: one face pushed out by the last move's distance plus
 * pendingExtra, an x face for ACTOR_EVENT_MOVED_X (RotateAndOffsetHullList
 * turns the box a quarter first), a z face otherwise; the max face after a
 * forward move, the min face after a backward one. An Actor linkTarget then
 * gets slotE8, which every Actor class leaves empty. */
void Actor__NotifyMove(Actor *self, s32 event) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, event);
    /* MATCHING: two nested ifs; `&&` folds into one unsigned compare */
    if (event <= ACTOR_EVENT_MOVED_Y) {
        if (event >= ACTOR_EVENT_UNSWEPT) {
            TmdHull hull;

            if (self->model != NULL && TmdModel__GetBoundsCount(self->model)) {
                self->methods->getModelHull(self, &hull);
                if (event != ACTOR_EVENT_UNSWEPT) {
                    s16 offset = self->lastOffsetValue;
                    s32 alongX = (event == ACTOR_EVENT_MOVED_X);
                    s32 forward = (offset >= 0);
                    s32 delta;

                    /* MATCHING: goto, not if/else, for retail's branch order */
                    if (offset < 0) {
                        goto backward;
                    }
                    delta = offset + self->pendingExtra;
                    goto offsetHull;
                backward:
                    delta = offset - self->pendingExtra;
                offsetHull:
                    RotateAndOffsetHullList(&hull, alongX, forward, delta);
                }
                self->methods->transformAndNotifyParents(self, &hull, event);
                if (self->linkTarget != NULL) {
                    if ((u8)self->linkTarget->methods->header == ACTOR_CLASS_ID) {
                        ((Actor *)self->linkTarget)->methods->slotE8((Actor *)self->linkTarget);
                    }
                }
            }
        }
    }
}

/* Routes a link command by the sender's class id byte: from an Actor (or
 * any class below it) to onActorLinkCommand, from a GridCell to
 * onGridCellLinkCommand, from anything else nowhere. */
void Actor__DispatchLinkCommand(Actor *self, BasicClass *sender, s32 event) {
    if ((u8)sender->methods->header == ACTOR_CLASS_ID) {
        self->methods->onActorLinkCommand(self, sender, event);
    } else if ((u8)sender->methods->header == GRIDCELL_CLASS_ID) {
        self->methods->onGridCellLinkCommand(self, sender, event);
    }
}

void Actor__SetTranslation(Actor *self, LongVec3 *v) {
    Actor__UpdateTranslation(self, 1, v);
}

void Actor__AddTranslation(Actor *self, LongVec3 *delta) {
    Actor__UpdateTranslation(self, 0, delta);
}

/* Sets (set != 0) or adds to the offset from the parent, coord2->coord.t,
 * then clears coord2->flg so libgs recomputes the matrix. */
void Actor__UpdateTranslation(Actor *self, s32 set, LongVec3 *v) {
    Actor *actor = self; /* MATCHING: a second name for self; without it $a0 is used, not $t0 */
    GsCOORDINATE2 *coord = actor->coord2;

    if (set) {
        *(LongVec3 *)coord->coord.t = *v; /* MATCHING: one struct copy, loads before stores */
    } else {
        coord->coord.t[0] += v->x;
        coord->coord.t[1] += v->y;
        coord->coord.t[2] += v->z;
    }
    actor->coord2->flg = 0;
}

/* Moves by `local` turned by the actor's own rotation. */
void Actor__AddLocalTranslation(Actor *self, s16 *local) {
    LongVec3 delta;

    SceneNode__RotateLocalVector((SceneNode *)self, &delta, local);
    self->methods->addTranslation(self, &delta);
}

/* The z of the s16 local move vector whose x and y are gActorLocalMove
 * (class_3bb8c_p.c, with MoveLocalX/Y and MoveAlongLocalAxis). */
extern s16 gActorLocalMoveZ;

void Actor__MoveLocalZ(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMoveZ, val, notify, ACTOR_EVENT_MOVED_Z);
}
