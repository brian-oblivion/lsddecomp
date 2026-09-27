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
#include "Actor.h"
#include "TmdModel.h"
#include "StyleEffect.h"

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
extern s32 rand(void);

typedef struct Vec3O {
    s32 x, y, z;
} Vec3O;

/* A rand()-indexed table of 6 Vec3-shaped entries, handed to each sprite's
 * updateScale. */
extern Vec3O gStyleEffectJitterScales[];

void StyleEffect__ReleaseSprites(StyleEffect *self) {
    ReleaseBasicClassArray((void **)self->sprites, 5);
}

void StyleEffect__SpawnPlainSprites(StyleEffect *self) {
    StyleEffect__SpawnSprites(self, 0, 0, 0);
}

void StyleEffect__RandomizeSprites(StyleEffect *self) {
    VariantSprite **p = &self->sprites[1];
    s32 i;

    for (i = 0; i < 4; i++, p++) {
        u32 r = rand();

        (*p)->methods->updateScale(*p, 1, &gStyleEffectJitterScales[r % 6]);
        (*p)->sprite.rotate = (rand() % 360) << 12;
    }
}

void StyleEffect__ReleaseSpritesB(StyleEffect *self) {
    ReleaseBasicClassArray((void **)self->sprites, 5);
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
    u8 raw[0x38];
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
extern s32 gStyleEffectClutPos;

extern void TmdModel__SetFirstPrimClut(void *arg0, void *arg1);

void SetStyleEffectSources(s32 unused, Actor *self, s32 arg2, s32 arg3) {
    s32 i;
    void *ret;

    gStyleEffectTmd = self;
    gStyleEffectTim = (void *)arg2;
    gStyleEffectViewport = (void *)arg3;
    i = 0;
    do {
        ret = (void *)self->methods->setBackClip(self, gStyleEffectModelIds[i]);
        TmdModel__SetFirstPrimClut(ret, &gStyleEffectClutPos);
        i++;
    } while (i < 2);
}

extern void *BMemPMgrAlloc(s32 size);
extern void *BMemPMgrFree(void *ptr);

void *New_Actor(void) {
    Actor *self = BMemPMgrAlloc(0x58);

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
    s32 tag;

    GetSceneNodeMethods()->addChild((SceneNode *)self, child);
    tag = child->methods->header;
    if ((tag & 0xFFF) == 0x114) {
        self->grid = (struct StageMap *)child;
    } else if ((tag & 0xF) == 5) {
        self->ticker = child;
    }
}

void Actor__RemoveChild(Actor *self, BasicClass *child) {
    s32 tag = child->methods->header;

    if ((tag & 0xFFF) == 0x114) {
        self->grid = NULL;
    } else if ((tag & 0xF) == 5) {
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
    self->lastOffsetValue = 0x12C;
    self->pendingExtra = 0;
}

extern void RotateAndOffsetHullList(Buf38O *out, s32 arg1, s32 arg2, s32 arg3);

void Actor__NotifyMove(Actor *self, s32 event) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, event);
    /* Written as two nested guards, not a combined `event >= 5 && event < 9`
     * range test -- the combined form optimizes into a single unsigned
     * `(event-5) < 4` comparison, which is not what retail does (two
     * separate `slti`s). */
    if (event < 9) {
        if (event >= 5) {
            Buf38O buf;

            if (self->model != NULL && TmdModel__GetBoundsCount(self->model)) {
                self->methods->getModelHull(self, &buf);
                if (event != 5) {
                    s16 h = self->lastOffsetValue;
                    s32 isSeven = (event == 7);
                    s32 nonneg = (h >= 0);
                    s32 adjusted;

                    /* `goto`, not `if/else`, to match retail's actual
                     * branch shape (see the match report). */
                    if (h < 0) {
                        goto negative;
                    }
                    adjusted = h + self->pendingExtra;
                    goto joinAdjust;
                negative:
                    adjusted = h - self->pendingExtra;
                joinAdjust:
                    RotateAndOffsetHullList(&buf, isSeven, nonneg, adjusted);
                }
                self->methods->transformAndNotifyParents(self, (TmdHull *)&buf, event);
                /* The link target's class byte: an Actor gets slotE8. */
                if (self->linkTarget != NULL) {
                    if (*(u8 *)self->linkTarget->methods == 0x34) {
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
    if (*(u8 *)sender->methods == 0x34) {
        self->methods->onActorLinkCommand(self, sender, event);
    } else if (*(u8 *)sender->methods == 0x24) {
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
    Actor *t = self;
    GsCOORDINATE2 *u = t->coord2;

    if (set) {
        *(LongVec3 *)u->coord.t = *v;
    } else {
        u->coord.t[0] += v->x;
        u->coord.t[1] += v->y;
        u->coord.t[2] += v->z;
    }
    t->coord2->flg = 0;
}

void Actor__AddLocalTranslation(Actor *self, s16 *local) {
    LongVec3 buf;

    SceneNode__RotateLocalVector((SceneNode *)self, &buf, local);
    self->methods->addTranslation(self, &buf);
}

/* z component of the local move vector gActorLocalMove (class_3bb8c_p.c). */
extern s16 gActorLocalMoveZ;

void Actor__MoveLocalZ(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &gActorLocalMoveZ, val, notify, 6);
}
