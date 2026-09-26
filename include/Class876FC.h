#ifndef CLASS876FC_H
#define CLASS876FC_H

#include "Actor.h"
#include "VariantSprite.h"

/*
 * Class876FC -- class id 0xEF34, method table gClass876FCMethods: Actor's
 * subclass (its ctor chains to GetActorMethods()->ctor first, so the id tree
 * 0x34 -> 0xEF34 is the ctor chain). Slot occupants in src/class_3bb8c_r.c
 * (New_, ctor, Finalize, SetParams, Update), the private helpers in
 * src/class_3bb8c_s.c and src/class_3bb8c_o.c (the four sprite-array
 * helpers), the getter in src/class_3bb8c_o.c. No class derives from it.
 *
 * Construction: class_3bb8c_n.c's StyleFillEffectKind0..3 build one per
 * effect slot (gStyleEffectSlots) as New_Class876FC(kind, params, parent,
 * pos), with `kind` 0..3, `params` the 0x24-byte block at gStyleSpawnOffsetX (seven
 * separately-declared symbols gStyleSpawnOffsetX..gStyleSpawnColors in class_3bb8c_n.c;
 * one Class876FCParams in the bytes: the callers pass `&gStyleSpawnOffsetX` or
 * `&gStyleSpawnRotation - 0xC`), `parent` gStyleGrid and `pos` the caller's
 * position. The ctor stores `kind`, copies `params` through the reset slot
 * and attaches self under `parent` at pos + params.offset
 * (Class876FC__InitByKind). Per kind: 0 and 1 link a model fetched through
 * D_8008ACA4; 0 also builds two Actor `modelChildren` in a row; 2 and 3 build
 * five VariantSprite `sprites` (2 randomised once, 3 re-randomised every frame).
 *
 * What it changes, from its own methods (`classtable.py gClass876FCMethods
 * --vs gActorMethods`):
 *  - +0x040 reset: the occupant, Class876FC__SetParams, takes the params
 *    block where the slot (SceneNode's) takes none. It copies the block
 *    into `params` and zeroes `tick`, as SceneNode__Reset zeroes `tick`.
 *    The ctor calls it through Class876FCSetParamsFn (a cast, no code);
 *  - +0x0EC setPendingExtra: the occupant, Class876FC__Update, is the
 *    per-frame update: it increments `tick` and runs the per-kind update
 *    with its second argument, the position. Its only caller,
 *    StyleUpdateEffectSlots (class_3bb8c_n.c), passes that position and
 *    calls through Class876FCUpdateFn (a cast, no code). The slot keeps
 *    Actor's type;
 *  - +0x008 ctor and +0x00C finalize (Class876FC__Finalize releases the
 *    per-kind children, then chains Actor's finalize).
 *
 * Inherited fields it uses differently: the ctor stores `kind` in Actor's
 * +0x054 `pendingExtra` (Class876FC__Class876FC; read by every per-kind
 * switch in class_3bb8c_s.c), and it overrides pendingExtra's setter slot
 * (+0x0EC, above), so nothing else writes it. The ctor also zeroes Actor's
 * +0x044 `state` again after Actor's ctor.
 *
 * That says what it builds, not what it is in the game, so the name stays
 * the table's address.
 *
 * The object is 0x98 bytes (New_Class876FC), exactly where `sprites` ends.
 */

typedef struct Class876FC Class876FC;
typedef struct Class876FCMethods Class876FCMethods;
typedef struct Class876FCParams Class876FCParams;

/* The 0x24-byte parameter block the reset slot (Class876FC__SetParams)
 * copies in whole. Every member is 4-aligned, so the whole-struct copy is
 * retail's aligned 4-word-per-iteration block move. */
struct Class876FCParams {
    /* +0x000 */ LongVec3 offset; /* added to the caller's position (InitByKind, UpdateByKind) */
    /* +0x00C */ void *rotation;  /* updateRotation's ratio triple (AttachWithRotScale) */
    /* +0x010 */ void *scale; /* updateScale's ratio triple; its first s16 also scales the model-child spacing (PlaceModelChildren) */
    /* +0x014 */ s32 modelChildLayout; /* 0 = no modelChildren, else an index 1..4 into gModelChildSpacing: 1-2 along x, 3-4 along y */
    /* +0x018 */ s32 tableIndex;   /* index into gModelChildDriftZ and gSpriteShiftX */
    /* +0x01C */ SpriteRgb *color; /* every sprite's setColor (SpawnSprites) */
    /* +0x020 */ SpriteRgb *altColor; /* sprites[1]'s colour instead, when non-NULL (BuildRandomSprites) */
};

/* Actor's slots, then this class's own: none. The overrides of inherited
 * slots are the ctor, finalize, reset and +0x0EC (see the banner). */
/* clang-format off */
#define CLASS876FC_SLOTS(Self, CtorParams)                                                         \
    ACTOR_SLOTS(Self, CtorParams) /* no own slots; 59 words, the last Class876FC__Update at +0x0EC */
/* clang-format on */

/* clang-format off */
#define CLASS876FC_FIELDS(Methods)                                                                 \
    ACTOR_FIELDS(Methods);                                                                         \
    /* +0x058 */ Class876FCParams params; /* the reset slot's copy (Class876FC__SetParams) */      \
    /* +0x07C */ Actor *modelChildren[2]; /* kind 0: New_Actor children (PlaceModelChildren) */    \
    /* +0x084 */ VariantSprite *sprites[5]   /* kinds 2/3: New_VariantSprite children (SpawnSprites). The object is 0x98 bytes (New_Class876FC) */
/* clang-format on */

struct Class876FCMethods {
    CLASS876FC_SLOTS(Class876FC, (Class876FC * self, s32 kind, Class876FCParams *params,
                                  SceneNode *parent, LongVec3 *pos));
};

struct Class876FC {
    CLASS876FC_FIELDS(Class876FCMethods);
};

/* The reset and +0x0EC occupants as their callers call them (see the banner). */
typedef void (*Class876FCSetParamsFn)(Class876FC *self, Class876FCParams *params);
typedef void (*Class876FCUpdateFn)(Class876FC *self, LongVec3 *pos);

extern Class876FCMethods gClass876FCMethods;
extern Class876FCMethods *GetClass876FCMethods(void); /* class_3bb8c_o.c; returns &gClass876FCMethods */

/* The class's own methods, in address order (class_3bb8c_r, _s, then _o).
 * Four are declared WITHOUT a prototype on purpose: each is one-parameter,
 * but a caller in class_3bb8c_s.c passes a dead second argument that is
 * byte-load-bearing (the `arity-ok` notes there and in the reports). */
Class876FC *New_Class876FC(s32 kind, Class876FCParams *params, SceneNode *parent,
                           LongVec3 *pos); /* BMemPMgrAlloc(0x98), then ctor */
Class876FC *Class876FC__Class876FC(Class876FC *self, s32 kind, Class876FCParams *params,
                                   SceneNode *parent, LongVec3 *pos);
void Class876FC__Finalize(Class876FC *self);
void Class876FC__SetParams(Class876FC *self, Class876FCParams *params);
void Class876FC__Update(Class876FC *self, LongVec3 *pos);
void Class876FC__InitByKind(Class876FC *self, SceneNode *parent, LongVec3 *pos);
void Class876FC__UpdateByKind(Class876FC *self, LongVec3 *pos);
void Class876FC__ReleaseByKind(Class876FC *self);
void Class876FC__PlaceModelChildren(Class876FC *self, s32 reuse);
void Class876FC__DriftModelChildren(); /* (Class876FC *self); see above */
void Class876FC__ReleaseModelChildren(Class876FC *self);
void Class876FC__BuildRandomSprites(); /* (Class876FC *self); see above */
void Class876FC__SpawnSprites(void *self, s32 unused, s32 variant, void *scale);
void Class876FC__ReleaseSprites(Class876FC *self);
void Class876FC__SpawnPlainSprites(); /* (Class876FC *self); see above */
void Class876FC__RandomizeSprites();  /* (Class876FC *self); see above */
void Class876FC__ReleaseSpritesB(Class876FC *self);

#endif
