#ifndef ACTOR_H
#define ACTOR_H

#include "scene_node.h"

/**
 * @file actor.h
 * @brief Actor, the SceneNode that moves: translation, local-axis moves,
 *        the hull sweep it reports after a move, and the link search that
 *        lets it stand on the grid.
 *
 * Methods in src/world/dream_scene.c, SetStyleEffectSources through
 * GetActorMethods.
 */

typedef struct Actor Actor;
typedef struct ActorMethods ActorMethods;

/* The grid manager (include/stage_map.h); only its address is kept here. */
struct StageMap;

/** Actor's class id (gActorMethods word +0x000). Two nibbles, so
 * `(u8)header == ACTOR_CLASS_ID` tests for Actor or a class below it
 * (TodActor, 0x234): StageMap__DispatchLinkCommand. */
#define ACTOR_CLASS_ID 0x34

/**
 * @brief The events an Actor's moves send through notifyWithHull
 * (Actor__MoveAlongLocalAxis), and the range Actor__NotifyMove handles: for
 * the three moves it sweeps the model's hull by lastOffsetValue before
 * passing it on, for ACTOR_EVENT_UNSWEPT it passes the hull as it is.
 */
enum ActorMoveEvent {
    ACTOR_EVENT_UNSWEPT = 5, /**< The hull as it is; no sender of 5 is in the game's code. */
    ACTOR_EVENT_MOVED_Z = 6, /**< After moveLocalZ. */
    ACTOR_EVENT_MOVED_X = 7, /**< After moveLocalX. */
    ACTOR_EVENT_MOVED_Y = 8  /**< After moveLocalY. */
};

/**
 * @brief Actor's slots: SceneNode's, then its own from +0x0B8.
 *
 * Overrides of SceneNode's slots: +0x008 Actor__Actor, +0x010
 * Actor__AddChild, +0x014 Actor__RemoveChild, +0x018
 * Actor__RemoveAllChildren, +0x040 Actor__Reset, +0x088 Actor__NotifyMove
 * and +0x09C Actor__DispatchLinkCommand. The inherited slots keep
 * SceneNode's names.
 */
/* clang-format off */
#define ACTOR_SLOTS(Self, CtorParams)                                                              \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*setTranslation)(Self *self, LongVec3 *v);          /* @see Actor__SetTranslation */ \
    /* +0x0BC */ void (*addTranslation)(Self *self, LongVec3 *delta);      /* @see Actor__AddTranslation */ \
    /* +0x0C0 */ void (*addLocalTranslation)(Self *self, s16 *local);       /* @see Actor__AddLocalTranslation */ \
    /* +0x0C4 */ void (*moveLocalZ)(Self *self, s32 val, void *notify);     /* @see Actor__MoveLocalZ */ \
    /* +0x0C8 */ void (*moveLocalX)(Self *self, s32 val, void *notify);     /* @see Actor__MoveLocalX */ \
    /* +0x0CC */ void (*moveLocalY)(Self *self, s32 val, void *notify);     /* @see Actor__MoveLocalY */ \
    /* +0x0D0 */ void (*moveLocalZOrFindLink)(Self *self, s32 val, void *notify); /* @see Actor__MoveLocalZOrFindLink */ \
    /* +0x0D4 */ void (*moveLocalXOrFindLink)(Self *self, s32 val, void *notify); /* @see Actor__MoveLocalXOrFindLink */ \
    /* +0x0D8 */ void (*slotD8)(void);                                      /* @see Actor__NoOpSlotD8 */ \
    /* +0x0DC */ void (*onActorLinkCommand)(Self *self, void *sender, s32 event);      /* @see Actor__OnActorLinkCommand */ \
    /* +0x0E0 */ void (*onGridCellLinkCommand)(Self *self, void *sender, s32 event); /* @see Actor__OnGridCellLinkCommand */ \
    /* +0x0E4 */ void (*setLastOffsetValue)(Self *self, s16 val);           /* @see Actor__SetLastOffsetValue */ \
    /* +0x0E8 */ void (*slotE8)(Self *self);                                /* @see Actor__NoOpSlotE8; NotifyMove calls it on an Actor linkTarget */ \
    /* +0x0EC */ void (*setPendingExtra)(Self *self, s32 extra)             /* @see Actor__SetPendingExtra */
/* clang-format on */

/**
 * @brief Actor's fields: SceneNode's, then its state, the last move's
 * distance, its two companions and the hull sweep's extra distance.
 */
/* clang-format off */
#define ACTOR_FIELDS(Methods)                                                                      \
    SCENENODE_FIELDS(Methods);                                                                    \
    /* +0x044 */ s32 state;               /* zeroed by the ctor; a subclass's state code (DreamSys and Entity: include/dream_sys.h, include/entity.h) */ \
    /* +0x048 */ s16 lastOffsetValue;     /* the last move's distance, or setLastOffsetValue's; Reset: 300; NotifyMove's sweep */ \
    /* +0x04A */ u8 pad4A[2];                                                                      \
    /* +0x04C */ struct StageMap *grid; /* the class-0x114 child addChild recorded; FindNearbyLink queries it */ \
    /* +0x050 */ BasicClass *ticker;      /* the class-5 (FrameClock) child addChild recorded */   \
    /* +0x054 */ s32 pendingExtra         /* setPendingExtra; Reset: 0; NotifyMove adds it to |lastOffsetValue| */
/* clang-format on */

/** @brief Actor's method table (see ACTOR_SLOTS). */
struct ActorMethods {
    ACTOR_SLOTS(Actor, (Actor * self));
};

/**
 * @brief A positioned scene object that moves (class id 0x34): a SceneNode
 * subclass. Three classes derive from it directly: TodActor (0x234,
 * src/world/tod_actor.c; Entity below it), DreamSys (0x1F34) and StyleEffect
 * (0xEF34, include/style_effect.h). A subclass reaches the base methods
 * through GetActorMethods() and upcasts.
 *
 * Children and companions: addChild/removeChild/removeAllChildren chain
 * SceneNode's and also record two companions by the child's class id: a
 * StageMap (`(id & 0xFFF) == 0x114`, include/stage_map.h) is the grid,
 * kept in `grid`; a FrameClock (`(id & 0xF) == 5`) is kept in `ticker`.
 * SceneNode's onNotify routes the FrameClock's events to `update` (+0x098),
 * which the subclasses override (TodActor's Update, Entity__Update,
 * DreamSys__TimerTick).
 *
 * Movement: setTranslation/addTranslation set or add coord2->coord.t and
 * mark the coordinate for recompute; addLocalTranslation rotates a local s16
 * vector by the object's orientation first. moveLocalZ/X/Y move along one
 * local axis, keep the distance in `lastOffsetValue` and, when `notify` is
 * non-NULL, send ACTOR_EVENT_MOVED_Z/X/Y through notifyWithHull, whose
 * override (Actor__NotifyMove) reports the hull swept by the move.
 * moveLocalZOrFindLink/moveLocalXOrFindLink clear linkTarget, move, and when
 * the move set no linkTarget search the grid for a nearby cell to stand on
 * (Actor__FindNearbyLink).
 *
 * Link commands: the inherited dispatchLinkCommand (+0x09C) is overridden
 * to route by the SENDER's class byte: an Actor (0x34) to
 * onActorLinkCommand, a GridCell (0x24) to onGridCellLinkCommand. Both base
 * occupants chain SceneNode's dispatchLinkCommand; the first also runs
 * tryAttachNearby for ACTOR_EVENT_UNSWEPT..ACTOR_EVENT_MOVED_Y.
 *
 * Its ctor returns self or NULL, as SceneNode's does.
 */
struct Actor {
    ACTOR_FIELDS(ActorMethods);
}; /* 0x58 bytes: New_Actor */

/** @brief Actor's method table (see ActorMethods). */
extern ActorMethods gActorMethods;

/**
 * @brief Returns Actor's method table.
 * @return &gActorMethods.
 */
extern ActorMethods *GetActorMethods(void);

/**
 * @brief Records what every StyleEffect draws from: the scene's TMD
 * resource, its TIM image and its viewport; then points the first primitive
 * of the TMD's models 0 and 2 at the style CLUT (sStyleEffectClutPos).
 * @param unused   The style variant; not read.
 * @param tmd      ETC\\DREAMER.TMD's resource, whose setBackClip slot yields a model.
 * @param tim      ETC\\ETC.TIM's image, for New_VariantSprite.
 * @param viewport The scene's Viewport.
 */
void SetStyleEffectSources(s32 unused, Actor *tmd, s32 tim, s32 viewport);

/**
 * @brief Allocates an Actor from the BMemPMgr pool and constructs it.
 * @return The new Actor, or NULL when the pool is exhausted or the ctor fails.
 */
void *New_Actor(void);

/**
 * @brief Constructor (slot +0x008): SceneNode's ctor, then `state`, `grid`
 * and `ticker` cleared and reset.
 * @param self The object being constructed.
 * @return self, or NULL when SceneNode's ctor fails.
 */
Actor *Actor__Actor(Actor *self);

/**
 * @brief addChild (slot +0x010): SceneNode's, then keeps a StageMap child in
 * `grid` or a FrameClock child in `ticker`.
 * @param self  The Actor.
 * @param child The child to add.
 */
void Actor__AddChild(Actor *self, BasicClass *child);

/**
 * @brief removeChild (slot +0x014): forgets the child if it is the grid or
 * the ticker, then SceneNode's removeChild.
 * @param self  The Actor.
 * @param child The child to remove.
 */
void Actor__RemoveChild(Actor *self, BasicClass *child);

/**
 * @brief removeAllChildren (slot +0x018): clears `grid` and `ticker`, then
 * SceneNode's removeAllChildren.
 * @param self The Actor.
 */
void Actor__RemoveAllChildren(Actor *self);

/**
 * @brief reset (slot +0x040): the sweep distance back to 300 and no extra.
 * @param self The Actor.
 */
void Actor__Reset(Actor *self);

/**
 * @brief notifyWithHull (slot +0x088): SceneNode's, then for
 * ACTOR_EVENT_UNSWEPT..ACTOR_EVENT_MOVED_Y on a model with bounds sends the
 * model's hull to the parents (transformAndNotifyParents). For a move the
 * hull is first swept: one face pushed out by the last move's distance plus
 * `pendingExtra`, an x face for ACTOR_EVENT_MOVED_X, a z face otherwise; the
 * max face after a forward move, the min face after a backward one. An Actor
 * linkTarget then gets slotE8.
 * @param self  The Actor.
 * @param event The event code (enum ActorMoveEvent, or any other).
 */
void Actor__NotifyMove(Actor *self, s32 event);

/**
 * @brief dispatchLinkCommand (slot +0x09C): routes a link command by the
 * sender's class byte, from an Actor (or a class below it) to
 * onActorLinkCommand, from a GridCell to onGridCellLinkCommand, from anything
 * else nowhere.
 * @param self   The Actor.
 * @param sender The object sending the command.
 * @param event  The command's event code.
 */
void Actor__DispatchLinkCommand(Actor *self, BasicClass *sender, s32 event);

/**
 * @brief setTranslation (slot +0x0B8): sets the offset from the parent.
 * @param self The Actor.
 * @param v    The new translation.
 */
void Actor__SetTranslation(Actor *self, LongVec3 *v);

/**
 * @brief addTranslation (slot +0x0BC): adds to the offset from the parent.
 * @param self  The Actor.
 * @param delta The amount to move by.
 */
void Actor__AddTranslation(Actor *self, LongVec3 *delta);

/**
 * @brief Sets or adds to coord2->coord.t, then clears coord2->flg so the
 * matrix is recomputed.
 * @param self The Actor.
 * @param set  Non-zero to set, zero to add.
 * @param v    The translation or the delta.
 */
void Actor__UpdateTranslation(Actor *self, s32 set, LongVec3 *v);

/**
 * @brief addLocalTranslation (slot +0x0C0): moves by `local` turned by the
 * Actor's own rotation.
 * @param self  The Actor.
 * @param local An s16[3] vector in the Actor's own axes.
 */
void Actor__AddLocalTranslation(Actor *self, s16 *local);

/**
 * @brief moveLocalZ (slot +0x0C4): moves `val` along the local z axis;
 * ACTOR_EVENT_MOVED_Z.
 * @param self   The Actor.
 * @param val    The distance.
 * @param notify Non-NULL to send the event through notifyWithHull.
 */
void Actor__MoveLocalZ(Actor *self, s32 val, void *notify);

/**
 * @brief Moves the Actor by `val` along one local axis, keeps `val` in
 * `lastOffsetValue` and, when `notify` is non-NULL, sends `event` through
 * notifyWithHull.
 * @param self   The Actor.
 * @param axis   That axis's component of the local move vector sActorLocalMove.
 * @param val    The distance.
 * @param notify Non-NULL to send the event.
 * @param event  ACTOR_EVENT_MOVED_Z, _X or _Y.
 */
void Actor__MoveAlongLocalAxis(Actor *self, s16 *axis, s32 val, void *notify, s32 event);

/**
 * @brief moveLocalX (slot +0x0C8): moves `val` along the local x axis;
 * ACTOR_EVENT_MOVED_X.
 * @param self   The Actor.
 * @param val    The distance.
 * @param notify Non-NULL to send the event through notifyWithHull.
 */
void Actor__MoveLocalX(Actor *self, s32 val, void *notify);

/**
 * @brief moveLocalY (slot +0x0CC): moves `val` along the local y axis;
 * ACTOR_EVENT_MOVED_Y.
 * @param self   The Actor.
 * @param val    The distance.
 * @param notify Non-NULL to send the event through notifyWithHull.
 */
void Actor__MoveLocalY(Actor *self, s32 val, void *notify);

/**
 * @brief moveLocalZOrFindLink (slot +0x0D0): moveLocalZ, then looks for a
 * link if the move made none.
 * @param self   The Actor.
 * @param val    The distance.
 * @param notify Non-NULL to send the move's event.
 */
void Actor__MoveLocalZOrFindLink(Actor *self, s32 val, void *notify);

/**
 * @brief moveLocalXOrFindLink (slot +0x0D4): moveLocalX, then looks for a
 * link if the move made none.
 * @param self   The Actor.
 * @param val    The distance.
 * @param notify Non-NULL to send the move's event.
 */
void Actor__MoveLocalXOrFindLink(Actor *self, s32 val, void *notify);

/** @brief Slot +0x0D8: does nothing. */
void Actor__NoOpSlotD8(void);

/**
 * @brief Clears `linkTarget`, moves, and when the move linked to nothing
 * looks for a nearby link (Actor__FindNearbyLink).
 * @param self   The Actor.
 * @param move   The move slot to run (moveLocalZ or moveLocalX).
 * @param val    The distance.
 * @param notify Passed to the move.
 */
void Actor__MoveOrFindNearbyLink(Actor *self, void (*move)(Actor *, s32, void *), s32 val, void *notify);

/**
 * @brief Looks in the grid for a GridCell a vertical ray from the Actor's
 * position hits: its own cell of its chunk slot and, in a vertical grid, the
 * same cell in the slots above and below. On a hit the cell becomes
 * `linkTarget`, the Actor moves onto the hit and notifyWithHull gets -1;
 * with no hit `linkTarget` is NULL and it gets -2.
 * @param self The Actor.
 * @return 1 when it linked; 0 when not, or with no grid or no slot holding
 *         its position.
 */
s32 Actor__FindNearbyLink(Actor *self);

/**
 * @brief onActorLinkCommand (slot +0x0DC): SceneNode's dispatchLinkCommand,
 * then for ACTOR_EVENT_UNSWEPT..ACTOR_EVENT_MOVED_Y tryAttachNearby(self,
 * sender, event).
 * @param self   The Actor.
 * @param sender The Actor sending the command.
 * @param event  The command's event code.
 */
void Actor__OnActorLinkCommand(Actor *self, void *sender, s32 event);

/**
 * @brief onGridCellLinkCommand (slot +0x0E0): SceneNode's
 * dispatchLinkCommand.
 * @param self   The Actor.
 * @param sender The GridCell sending the command.
 * @param event  The command's event code.
 */
void Actor__OnGridCellLinkCommand(Actor *self, void *sender, s32 event);

/**
 * @brief setLastOffsetValue (slot +0x0E4): sets the distance NotifyMove
 * sweeps the hull by.
 * @param self The Actor.
 * @param val  The distance.
 */
void Actor__SetLastOffsetValue(Actor *self, s16 val);

/** @brief Slot +0x0E8: does nothing; NotifyMove calls it on an Actor linkTarget. */
void Actor__NoOpSlotE8(void);

/**
 * @brief setPendingExtra (slot +0x0EC): sets the distance NotifyMove adds to
 * a move's sweep.
 * @param self  The Actor.
 * @param extra The extra distance.
 */
void Actor__SetPendingExtra(Actor *self, s32 extra);

#endif
