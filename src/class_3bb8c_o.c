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
 *  - `LinkOwnerObj`/`LinkElemObj` (`NoOpIgnoreArgs`, `LinkOwnerObj__ReleaseLinks[B]`,
 *    `LinkOwnerObj__RandomizeLinks`, `LinkOwnerObj__func_56e1c`): a small
 *    object holding an inline 5-element link array (`links`). This is the
 *    SAME node `class_3bb8c_s.c` independently calls `LinkNode` -- kept as
 *    this unit's own local view per the multiple-independent-local-views
 *    convention, unrelated to the class below.
 *  - `Actor` (`GetClass876FCMethods` onward; include/Actor.h, unified round
 *    82): the base class of `DreamSys`, `Class65650` and `Class876FC`.
 *    `Actor__Actor` is the BASE's own constructor: `Class65650__Class65650`
 *    calls it to chain to the base first, then overwrites `self->methods`
 *    with its own table. `GetClass876FCMethods` is the Class876FC
 *    subclass's table getter (named round 73), placed here by ROM address.
 */
#include "common.h"
#include "Actor.h"
#include "TmdModel.h"

void NoOpIgnoreArgs(void) {
}

/* ------------------------------------------------------------------ *
 * Group 1: LinkOwnerObj__ReleaseLinks / LinkOwnerObj__ReleaseLinksB /
 * LinkOwnerObj__RandomizeLinks / LinkOwnerObj__func_56e1c.
 * Self is some larger object with an inline 5-element `BasicClass *`
 * array at +0x084 (`links`). ReleaseLinks/ReleaseLinksB (two identical,
 * separate ROM functions -- see their own reports) release the whole
 * array via ReleaseBasicClassArray, already established elsewhere as
 * `void ReleaseBasicClassArray(BasicClass **array, s32 count)` in
 * code_8220_b.c -- kept generic `void **` here per this project's
 * per-unit convention for that symbol, e.g. code_2cc8c.h's own looser
 * reading. RandomizeLinks walks array indices [1..4] (self+0x88 ..
 * self+0x94), which is exactly inside the same 5-element array, and for
 * each element calls its own vtable slot +0x048 with a random Vec3-ish
 * table entry, then sets the element's own `angle` field to a random
 * value (`(rand() % 360) << 12`, a degrees->fixed-point conversion).
 * ------------------------------------------------------------------ */

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 rand(void);

typedef struct Vec3O {
    s32 x, y, z;
} Vec3O;

/* A rand()-indexed table of 6 Vec3-shaped entries, passed to each link
 * element's own slot48. */
extern Vec3O gLinkElemVec3Table[];

typedef struct LinkElemObj LinkElemObj;
typedef struct LinkElemMethods {
    u8 pad0[0x48];
    void (*slot48)(LinkElemObj *self, s32 arg1, Vec3O *arg2); /* +0x048 */
} LinkElemMethods;
struct LinkElemObj {
    LinkElemMethods *methods; /* +0x000 */
    u8 pad4[0x80];             /* +0x004 .. +0x083, unknown */
    s32 angle;                   /* +0x084, a random "angle" set by LinkOwnerObj__RandomizeLinks */
};

typedef struct LinkOwnerObj {
    u8 pad0[0x84];              /* +0x000 .. +0x083, unknown */
    LinkElemObj *links[5];        /* +0x084 .. +0x097 */
} LinkOwnerObj;

void LinkOwnerObj__ReleaseLinks(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->links, 5);
}

extern void Class876FC__SpawnSprites(void *arg0, s32 arg1, s32 arg2, s32 arg3);

void LinkOwnerObj__func_56e1c(void *this) {
    Class876FC__SpawnSprites(this, 0, 0, 0);
}

void LinkOwnerObj__RandomizeLinks(LinkOwnerObj *this) {
    LinkElemObj **p = &this->links[1];
    s32 i;

    for (i = 0; i < 4; i++, p++) {
        u32 r = rand();

        (*p)->methods->slot48(*p, 1, &gLinkElemVec3Table[r % 6]);
        (*p)->angle = (rand() % 360) << 12;
    }
}

void LinkOwnerObj__ReleaseLinksB(LinkOwnerObj *this) {
    ReleaseBasicClassArray((void **)this->links, 5);
}

/* ------------------------------------------------------------------ *
 * Group 2: GetClass876FCMethods onward -- Actor (include/Actor.h), the
 * shared base of DreamSys, Class65650 and Class876FC.
 * ------------------------------------------------------------------ */

/* Actor__NotifyMove's model-data buffer, filled by readUnk20Data and handed
 * to RotateAndOffsetHullList and transformAndNotifyParents. Retail's frame needs it to
 * be 0x38 bytes (sp+0x10 .. sp+0x47, the saved registers from sp+0x48); a
 * smaller buffer shifts everything after the function. Its layout is not
 * established here. */
typedef struct Buf38O {
    u8 raw[0x38];
} Buf38O;

/* Class876FC's table (class_3bb8c_r/s); typed as its parent's layout. */
extern ActorMethods gClass876FCMethods;

ActorMethods *GetClass876FCMethods(void) {
    return &gClass876FCMethods;
}

/* Captured here for Class876FC's methods (class_3bb8c_s.c, which declares
 * the same three with the same types; track 4b, round 85): the Actor this
 * runs on, and two objects passed through. */
extern Actor *D_8008ACA4;
extern void *D_8008ACA8;
extern void *D_8008ACAC;
extern s32 D_8008AB98[3];
extern s32 D_8008AB94;

extern void SetTargetOffset(void *arg0, void *arg1);

void Actor__func_56f5c(s32 unused, Actor *self, s32 arg2, s32 arg3) {
    s32 i;
    void *ret;

    D_8008ACA4 = self;
    D_8008ACA8 = (void *) arg2;
    D_8008ACAC = (void *) arg3;
    i = 0;
    do {
        ret = (void *) self->methods->getSetUnk10Flag8(self, D_8008AB98[i]);
        SetTargetOffset(ret, &D_8008AB94);
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
    if (GetClass6B5CCMethods()->ctor((Class6B5CC *)self) == NULL) {
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

    GetClass6B5CCMethods()->addChild((Class6B5CC *)self, child);
    tag = child->methods->header;
    if ((tag & 0xFFF) == 0x114) {
        self->grid = (struct Class866E8 *)child;
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
    GetClass6B5CCMethods()->removeChild((Class6B5CC *)self, child);
}

void Actor__RemoveAllChildren(Actor *self) {
    self->grid = NULL;
    self->ticker = NULL;
    GetClass6B5CCMethods()->removeAllChildren((Class6B5CC *)self);
}

void Actor__Reset(Actor *self) {
    self->lastOffsetValue = 0x12C;
    self->pendingExtra = 0;
}

extern void RotateAndOffsetHullList(Buf38O *out, s32 arg1, s32 arg2, s32 arg3);

void Actor__NotifyMove(Actor *self, s32 event) {
    GetClass6B5CCMethods()->notifyIfUnk20Active((Class6B5CC *)self, event);
    /* Written as two nested guards, not a combined `event >= 5 && event < 9`
     * range test -- the combined form optimizes into a single unsigned
     * `(event-5) < 4` comparison, which is not what retail does (two
     * separate `slti`s). */
    if (event < 9) {
        if (event >= 5) {
            Buf38O buf;

            if (self->model != NULL && TmdModel__GetBoundsCount(self->model)) {
                self->methods->readUnk20Data(self, &buf);
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
                self->methods->transformAndNotifyParents(self, (GenericCountList_d294 *)&buf, event);
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
        self->methods->onClass86AA0LinkCommand(self, sender, event);
    }
}

void Actor__SetTranslation(Actor *self, Vec3_d294 *v) {
    Actor__UpdateTranslation(self, 1, v);
}

void Actor__AddTranslation(Actor *self, Vec3_d294 *delta) {
    Actor__UpdateTranslation(self, 0, delta);
}

/* coord2->coord.t (+0x018 of the GsCOORDINATE2) is set (set != 0) or added
 * to, then flg is cleared so the coordinate is recomputed. The assignment
 * is a whole-Vec3 copy. */
void Actor__UpdateTranslation(Actor *self, s32 set, Vec3_d294 *v) {
    Actor *t = self;
    Class6B5CCSub14 *u = t->coord2;

    if (set) {
        *(Vec3_d294 *)&u->tx = *v;
    } else {
        u->tx += v->x;
        u->ty += v->y;
        u->tz += v->z;
    }
    t->coord2->flg = 0;
}

void Actor__AddLocalTranslation(Actor *self, s16 *local) {
    Vec3_d294 buf;

    Class6B5CC__RotateLocalVector((Class6B5CC *)self, &buf, local);
    self->methods->addTranslation(self, &buf);
}

/* z component of the local move vector D_8008ABA4 (class_3bb8c_p.c). */
extern s16 D_8008ABA8;

void Actor__MoveLocalZ(Actor *self, s32 val, void *notify) {
    Actor__MoveAlongLocalAxis(self, &D_8008ABA8, val, notify, 6);
}
