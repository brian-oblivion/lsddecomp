#ifndef ACTOR_H
#define ACTOR_H

#include "SceneNode.h"

/*
 * Actor -- a positioned scene object that moves (class id 0x34, method table
 * gActorMethods): a SceneNode subclass. Methods in src/class_3bb8c_o.c and
 * src/class_3bb8c_p.c. Three classes derive from it directly
 * (`typeviews.py --tree`): Class65650 (0x234, code_55dd4; Entity below it),
 * DreamSys (0x1F34) and Class876FC (0xEF34, class_3bb8c_r/s).
 *
 * Children and companions. addChild/removeChild/removeAllChildren chain
 * SceneNode's and also record two companions by the child's class id:
 * `(id & 0xFFF) == 0x114` is the grid manager (Class866E8, include/
 * Class866E8.h), kept in `grid`; `(id & 0xF) == 5` is a FrameClock object,
 * kept in `ticker` (SceneNode's onNotify routes that class's events to
 * `update`, +0x098, which the subclasses override as Class65650's
 * Update (formerly OnClass6EF50Notify), Entity__Update and DreamSys__TimerTick).
 *
 * Movement. setTranslation/addTranslation (+0x0B8/+0x0BC) set or add
 * coord2->coord.t and mark the coordinate for recompute; addLocalTranslation
 * (+0x0C0) rotates a local s16 vector by the object's orientation first.
 * moveLocalZ/X/Y (+0x0C4/+0x0C8/+0x0CC) put `val` into one component of the
 * s16 vector at D_8008ABA4 (x at ABA4, y at ABA6, z at ABA8), apply it through
 * addLocalTranslation, clear the component, keep `val` in lastOffsetValue
 * and, when `notify` is non-NULL, call notifyIfUnk20Active with 6, 7 or 8
 * (Actor__MoveAlongLocalAxis). The override of that slot, Actor__NotifyMove,
 * handles exactly events 5..8. moveLocalZOrFindLink/moveLocalXOrFindLink
 * (+0x0D0/+0x0D4) clear linkTarget, move, and when nothing set linkTarget
 * search the grid for a nearby link (Actor__FindNearbyLink).
 *
 * Link commands. The inherited dispatchLinkCommand (+0x09C) is overridden to
 * route by the SENDER's class byte: an Actor (0x34) to onActorLinkCommand
 * (+0x0DC), a Class86AA0 (0x24) to onClass86AA0LinkCommand (+0x0E0). Both
 * base occupants chain SceneNode's dispatchLinkCommand; the first also runs
 * tryAttachNearby for events 5..8.
 *
 * The object is 0x58 bytes (New_Actor). Its ctor returns self or NULL, as
 * SceneNode's does.
 */

typedef struct Actor Actor;
typedef struct ActorMethods ActorMethods;

/* The grid manager (include/Class866E8.h); only its address is kept here. */
struct Class866E8;

/* Occupants in gActorMethods named at each slot; `tools/classtable.py
 * <subclass table> --vs gActorMethods` lists a subclass's overrides. The
 * inherited slots keep SceneNode's names; this class overrides +0x008,
 * +0x010, +0x014, +0x018 (Actor__AddChild/RemoveChild/RemoveAllChildren),
 * +0x040 (Actor__Reset), +0x088 (Actor__NotifyMove) and +0x09C
 * (Actor__DispatchLinkCommand). */
/* clang-format off */
#define ACTOR_SLOTS(Self, CtorParams)                                                              \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setTranslation)(Self *self, Vec3_d294 *v);          /* Actor__SetTranslation */ \
    /* +0x0BC */ void (*addTranslation)(Self *self, Vec3_d294 *delta);      /* Actor__AddTranslation */ \
    /* +0x0C0 */ void (*addLocalTranslation)(Self *self, s16 *local);       /* Actor__AddLocalTranslation */ \
    /* +0x0C4 */ void (*moveLocalZ)(Self *self, s32 val, void *notify);     /* Actor__MoveLocalZ: event 6 */ \
    /* +0x0C8 */ void (*moveLocalX)(Self *self, s32 val, void *notify);     /* Actor__MoveLocalX: event 7 */ \
    /* +0x0CC */ void (*moveLocalY)(Self *self, s32 val, void *notify);     /* Actor__MoveLocalY: event 8 */ \
    /* +0x0D0 */ void (*moveLocalZOrFindLink)(Self *self, s32 val, void *notify); /* Actor__MoveLocalZOrFindLink */ \
    /* +0x0D4 */ void (*moveLocalXOrFindLink)(Self *self, s32 val, void *notify); /* Actor__MoveLocalXOrFindLink */ \
    /* +0x0D8 */ void (*slotD8)(void);                                      /* Actor__NoOpSlotD8, empty */ \
    /* +0x0DC */ void (*onActorLinkCommand)(Self *self, void *sender, s32 event);      /* Actor__OnActorLinkCommand */ \
    /* +0x0E0 */ void (*onClass86AA0LinkCommand)(Self *self, void *sender, s32 event); /* Actor__OnClass86AA0LinkCommand */ \
    /* +0x0E4 */ void (*setLastOffsetValue)(Self *self, s16 val);           /* Actor__SetLastOffsetValue */ \
    /* +0x0E8 */ void (*slotE8)(Self *self);                                /* Actor__NoOpSlotE8, empty; NotifyMove calls it on an Actor linkTarget */ \
    /* +0x0EC */ void (*setPendingExtra)(Self *self, s32 extra)             /* Actor__SetPendingExtra */
/* clang-format on */

/* clang-format off */
#define ACTOR_FIELDS(Methods)                                                                      \
    SCENENODE_FIELDS(Methods);                                                                    \
    /* +0x044 */ s32 state;               /* zeroed by the ctor; a subclass's state code (DreamSys and Entity: include/DreamSys.h, include/Entity.h) */ \
    /* +0x048 */ s16 lastOffsetValue;     /* MoveAlongLocalAxis's val, setLastOffsetValue; Reset: 300; NotifyMove's magnitude */ \
    /* +0x04A */ u8 pad4A[2];                                                                      \
    /* +0x04C */ struct Class866E8 *grid; /* the class-0x114 child addChild recorded; FindNearbyLink queries it */ \
    /* +0x050 */ BasicClass *ticker;      /* the class-5 (FrameClock) child addChild recorded */   \
    /* +0x054 */ s32 pendingExtra         /* setPendingExtra; Reset: 0; NotifyMove adds it to |lastOffsetValue|. The object is 0x58 bytes (New_Actor) */
/* clang-format on */

struct ActorMethods {
    ACTOR_SLOTS(Actor, (Actor * self));
};

struct Actor {
    ACTOR_FIELDS(ActorMethods);
};

extern ActorMethods gActorMethods;
extern ActorMethods *GetActorMethods(void); /* returns &gActorMethods */

/* The class's own methods, in ROM order (class_3bb8c_o, then class_3bb8c_p),
 * then its non-slot helpers. A subclass reaches the base ones through
 * GetActorMethods() and upcasts. */
void Actor__func_56f5c(s32 unused, Actor *self, s32 arg2, s32 arg3);
void *New_Actor(void);
Actor *Actor__Actor(Actor *self);
void Actor__AddChild(Actor *self, BasicClass *child);
void Actor__RemoveChild(Actor *self, BasicClass *child);
void Actor__RemoveAllChildren(Actor *self);
void Actor__Reset(Actor *self);
void Actor__NotifyMove(Actor *self, s32 event);
void Actor__DispatchLinkCommand(Actor *self, BasicClass *sender, s32 event);
void Actor__SetTranslation(Actor *self, Vec3_d294 *v);
void Actor__AddTranslation(Actor *self, Vec3_d294 *delta);
void Actor__UpdateTranslation(Actor *self, s32 set, Vec3_d294 *v);
void Actor__AddLocalTranslation(Actor *self, s16 *local);
void Actor__MoveLocalZ(Actor *self, s32 val, void *notify);
void Actor__MoveAlongLocalAxis(Actor *self, s16 *axis, s32 val, void *notify, volatile s32 event);
void Actor__MoveLocalX(Actor *self, s32 val, void *notify);
void Actor__MoveLocalY(Actor *self, s32 val, void *notify);
void Actor__MoveLocalZOrFindLink(Actor *self, s32 val, void *notify);
void Actor__MoveLocalXOrFindLink(Actor *self, s32 val, void *notify);
void Actor__NoOpSlotD8(void);
void Actor__MoveOrFindNearbyLink(Actor *self, void (*move)(Actor *, s32, void *), s32 val, void *notify);
s32 Actor__FindNearbyLink(Actor *self);
void Actor__OnActorLinkCommand(Actor *self, void *sender, s32 event);
void Actor__OnClass86AA0LinkCommand(Actor *self, void *sender, s32 event);
void Actor__SetLastOffsetValue(Actor *self, s16 val);
void Actor__NoOpSlotE8(void);
void Actor__SetPendingExtra(Actor *self, s32 extra);

#endif
