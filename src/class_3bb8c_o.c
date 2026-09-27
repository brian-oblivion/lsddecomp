/*
 * class_3bb8c_o -- functions 54..73 of the 113-function `class_3bb8c_n`
 * remainder, 0x475F0..0x47CC4 (vram 0x80056DF0..0x800574C4). Carved round 17
 * (2026-09-04); `class_3bb8c_n` keeps its name for the 54 functions in front
 * of this slice and `class_3bb8c_q` is the 19-function tail behind
 * `class_3bb8c_p`. All 20 functions matched, byte-exact; zero INCLUDE_ASM
 * stalls, zero NON_MATCHING bodies. Owns no switch jump table.
 *
 * Named round 52 (runner bravo). SPANS TWO CLASSES, cut at a ROM address,
 * not a class boundary (`tools/classtable.py`, confirmed round 17/52):
 *
 *  - StyleEffect's sprite-array helpers (`NoOpIgnoreArgs`,
 *    `StyleEffect__ReleaseSprites[B]`, `StyleEffect__RandomizeSprites`,
 *    `StyleEffect__SpawnPlainSprites`; include/StyleEffect.h). Two more of
 *    its pieces sit inside the Actor group by ROM address: its table getter
 *    `GetStyleEffectMethods`, and `SetStyleEffectSources`, which stores the
 *    TMD resource, TIM image and viewport its methods read.
 *  - `Actor` (`GetStyleEffectMethods` onward; include/Actor.h, unified round
 *    82): the base class of `DreamSys`, `TodActor` and `StyleEffect`.
 *    `Actor__Actor` is the BASE's own constructor: `TodActor__TodActor`
 *    calls it to chain to the base first, then overwrites `self->methods`
 *    with its own table. `GetStyleEffectMethods` is the StyleEffect
 *    subclass's table getter (named round 73), placed here by ROM address.
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

void NoOpIgnoreArgs(void) {}

/* ------------------------------------------------------------------ *
 * Group 1: StyleEffect's sprite-array helpers (include/StyleEffect.h), called
 * only from its per-kind dispatch in class_3bb8c_s.c. ReleaseSprites and
 * ReleaseSpritesB are two identical, separate ROM functions (see their
 * reports). RandomizeSprites walks sprites[1..4]: each gets a random scale
 * ratio table through its updateScale slot (VariantSprite__UpdateScale) and a
 * random sprite.rotate, `(rand() % 360) << 12` (4096 per degree).
 * ------------------------------------------------------------------ */

extern void ReleaseBasicClassArray(void **array, s32 count);

/* Six scale tables, each three Ratio16s (x, y, z), for the jittering
 * sprites: a thin streak along y or along x, {1/16, 7/1}, {7/1, 1/16}, then
 * the same with 3 and 2; z is 1/1. VariantSprite__UpdateScale reads x and y. */
extern Ratio16 gStyleEffectJitterScales[6][3];

void StyleEffect__ReleaseSprites(StyleEffect *self) {
    ReleaseBasicClassArray((void **)self->sprites, ARRAY_COUNT(self->sprites));
}

void StyleEffect__SpawnPlainSprites(StyleEffect *self) {
    StyleEffect__SpawnSprites(self, 0, 0, 0);
}

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

/* ------------------------------------------------------------------ *
 * Group 2: GetStyleEffectMethods onward -- Actor (include/Actor.h), the
 * shared base of DreamSys, TodActor and StyleEffect.
 * ------------------------------------------------------------------ */

/* Actor__NotifyMove's model-data buffer, filled by readUnk20Data and handed
 * to RotateAndOffsetHullList and transformAndNotifyParents. Retail's frame needs it to
 * be 0x38 bytes (sp+0x10 .. sp+0x47, the saved registers from sp+0x48); a
 * smaller buffer shifts everything after the function. Its layout is not
 * established here. */
typedef struct Buf38O {
    TmdHull hull;
    u8 pad34[4];
} Buf38O;

/* StyleEffect's getter (include/StyleEffect.h), placed here by ROM address. */
StyleEffectMethods *GetStyleEffectMethods(void) {
    return &gStyleEffectMethods;
}

/* Captured here for StyleEffect's methods (class_3bb8c_s.c, which declares
 * the same three with the same types; track 4b, round 85): the Actor this
 * runs on, and two objects passed through. */
extern Actor *gStyleEffectTmd;
extern void *gStyleEffectTim;
extern void *gStyleEffectViewport;
extern s32 gStyleEffectModelIds[3];
extern s16 gStyleEffectClutPos[2];

extern void TmdModel__SetFirstPrimClut(TmdModel *self, s16 *xy);

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

void Actor__Reset(Actor *self) {
    self->lastOffsetValue = 300;
    self->pendingExtra = 0;
}

extern void RotateAndOffsetHullList(TmdHull *hull, s32 turn, s32 back, s32 delta);

void Actor__NotifyMove(Actor *self, s32 event) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, event);
    /* Written as two nested guards, not a combined `event >= 5 && event < 9`
     * range test -- the combined form optimizes into a single unsigned
     * `(event-5) < 4` comparison, which is not what retail does (two
     * separate `slti`s). */
    if (event <= ACTOR_EVENT_MOVED_Y) {
        if (event >= ACTOR_EVENT_UNSWEPT) {
            Buf38O hullBuf;

            if (self->model != NULL && TmdModel__GetBoundsCount(self->model)) {
                self->methods->getModelHull(self, &hullBuf);
                if (event != ACTOR_EVENT_UNSWEPT) {
                    s16 offset = self->lastOffsetValue;
                    s32 alongX = (event == ACTOR_EVENT_MOVED_X);
                    s32 forward = (offset >= 0);
                    s32 delta;

                    /* `goto`, not `if/else`, to match retail's actual
                     * branch shape (see the match report). */
                    if (offset < 0) {
                        goto backward;
                    }
                    delta = offset + self->pendingExtra;
                    goto offsetHull;
                backward:
                    delta = offset - self->pendingExtra;
                offsetHull:
                    RotateAndOffsetHullList(&hullBuf.hull, alongX, forward, delta);
                }
                self->methods->transformAndNotifyParents(self, &hullBuf.hull, event);
                /* The link target's class byte: an Actor gets slotE8. */
                if (self->linkTarget != NULL) {
                    if ((u8)self->linkTarget->methods->header == ACTOR_CLASS_ID) {
                        ((Actor *)self->linkTarget)->methods->slotE8((Actor *)self->linkTarget);
                    }
                }
            }
        }
    }
}

/* Only the low byte of the sender's class id is read. Both targets are
 * called with self alone in C terms, but they read the sender and event
 * this function received: $a1/$a2 are never touched before the call. */
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

/* coord2->coord.t (+0x018 of the GsCOORDINATE2) is set (set != 0) or added
 * to, then flg is cleared so the coordinate is recomputed. The assignment
 * is a whole-Vec3 copy. */
void Actor__UpdateTranslation(Actor *self, s32 set, LongVec3 *v) {
    Actor *actor = self; /* MATCHING: a second name for self; without it $a0 is used, not $t0 */
    GsCOORDINATE2 *coord = actor->coord2;

    if (set) {
        *(LongVec3 *)coord->coord.t = *v;
    } else {
        coord->coord.t[0] += v->x;
        coord->coord.t[1] += v->y;
        coord->coord.t[2] += v->z;
    }
    actor->coord2->flg = 0;
}

void Actor__AddLocalTranslation(Actor *self, s16 *local) {
    LongVec3 delta;

    SceneNode__RotateLocalVector((SceneNode *)self, &delta, local);
    self->methods->addTranslation(self, &delta);
}

/* z component of the local move vector gActorLocalMove (class_3bb8c_p.c). */
extern s16 gActorLocalMoveZ;

void Actor__MoveLocalZ(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMoveZ, val, notify, ACTOR_EVENT_MOVED_Z);
}
