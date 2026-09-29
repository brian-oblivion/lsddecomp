/**
 * @file entity.h
 * @brief Entity, the TOD-animated actor a dream places for each mood-table
 *        row, its method table, the mood row and its column values, and the
 *        effects an Entity sends the dream.
 *
 * Spawned by dream_aux.c (SetDreamAuxWorld, SpawnDreamAuxTriggerEntity).
 */
#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"
#include "tod_actor.h"
#include "fade_box.h"
#include "sound_cue_set.h"

/** gEntityMethods' class id (TodActor's 0xF234 with a 1 above it):
 * `(header & CLASS_ID_LEVEL5_MASK) == ENTITY_CLASS_ID` is its is-kind-of test. */
#define ENTITY_CLASS_ID 0x1F234

typedef struct Entity Entity;
typedef struct EntityMethods EntityMethods;
typedef struct EntityMoodRow EntityMoodRow;

/** A mood row's handler: the SoundCueSet callback Entity__StartSoundCue
 * installs, given the Entity as its owner. */
typedef void (*EntityMoodCueFn)(Entity *self, SoundCueSet *out);

/** Rows in sEntityMoodTable: one per moodIndex, Entity__MoodCue00 to 129. */
#define ENTITY_MOOD_ROW_COUNT 130

/**
 * @brief Entity's method table: TodActor's slots, then Entity's own; 97
 *        slots, 0x184 bytes.
 *
 * Overrides: +0x008 Entity__Entity, +0x00C Entity__Finalize, +0x040
 * Entity__Reset, +0x04C Entity__AttachToParent, +0x050
 * Entity__DetachFromParent, +0x098 Entity__Update, +0x0DC
 * Entity__NotifyLinkStage, +0x0E0 Entity__OnGridCellLinkCommand, +0x11C
 * Entity__TickSoundCue.
 */
struct EntityMethods {
    TODACTOR_SLOTS(Entity, (Entity * self, s32 moodIndex, void *desc, void *sound));
    /* +0x144 */ s32 (*distanceToPeer)(Entity *self, TodActor *peer); /**< @see Entity__DistanceToPeer */
    /* +0x148 */ s32 (*getProximityRatio)(Entity *self); /**< @see Entity__GetProximityRatio */
    /* +0x14C */ EntityMoodRow *(*getMoodEffect)(Entity *self); /**< @see Entity__GetMoodEffect */
    /* +0x150 */ s32 (*getUnlockEffect)(Entity *self);          /**< @see Entity__GetUnlockEffect */
    /* +0x154 */ s32 (*getLinkStage)(Entity *self);             /**< @see Entity__GetLinkStage */
    /* +0x158 */ s32 (*getEventVideo)(Entity *self);            /**< @see Entity__GetEventVideo */
    /* +0x15C */ void (*activate)(Entity *self);                /**< @see Entity__Activate */
    /* +0x160 */ void (*deactivate)(Entity *self);              /**< @see Entity__Deactivate */
    /* +0x164 */ void (*setTargetReached)(Entity *self, s32 reached); /**< @see Entity__SetTargetReached */
    /* +0x168 */ void (*startSoundCue)(Entity *self);        /**< @see Entity__StartSoundCue */
    /* +0x16C */ void (*stopSoundCue)(Entity *self);         /**< @see Entity__StopSoundCue */
    /* +0x170 */ s32 (*updateActivationState)(Entity *self); /**< @see Entity__UpdateActivationState */
    /* +0x174 */ s32 (*updateDeactivationState)(Entity *self); /**< @see Entity__UpdateDeactivationState */
    /* +0x178 */ s32 (*updateTargetProximity)(Entity *self); /**< @see Entity__UpdateTargetProximity */
    /* +0x17C */ s32 (*updateSoundCueStart)(Entity *self); /**< @see Entity__UpdateSoundCueStart */
    /* +0x180 */ void (*updateSoundCueStop)(Entity *self); /**< @see Entity__UpdateSoundCueStop */
};

/**
 * @brief Entity: a TodActor (a TOD-animated Actor) driven by one row of the
 *        mood table, which says when it appears, when it goes, what sound
 *        cue it runs and what it tells the dream when the player reaches it.
 *
 * Class id 0x1F234 (ENTITY_CLASS_ID), table gEntityMethods, parent TodActor
 * (include/tod_actor.h; the ctor calls TodActor's first), TodActor's one
 * subclass; no class derives from it. Methods and the MoodCue handlers in
 * src/world/entity.c. The object is 0x108 bytes (New_Entity); TodActor's
 * fields end at +0x098.
 *
 * The mood row. New_Entity's first argument is `moodIndex`, which selects a
 * 16-byte row (EntityMoodRow) of sEntityMoodTable. Every tick, update
 * (+0x098) runs updateActivationState (activate when the row's activateKind
 * condition holds), updateDeactivationState, the sound-cue start/stop pair
 * and updateTargetProximity, then TodActor's update.
 *
 * The peer is the player. attachToParent's (self, peer, companion, parent,
 * offset) is TodActor's; dream_aux.c passes sDreamAuxWorld as the peer, and
 * the slots Entity calls on `peer` (+0x100, +0x120, +0x1A0, +0x200, +0x21C)
 * lie past the end of TodActor's table: their occupants in gDreamSysMethods
 * are DreamSys__GetLinkCommandFlag, DreamSys__ProjectPointAtDistance,
 * DreamSys__GetCurrentDayAndYear, DreamSys__GetDreamColor and
 * DreamSys__ResetFlashbackList. The units that call it include
 * include/dream_sys.h and cast `peer` (TodActor's field, typed TodActor *)
 * to DreamSys *. The `parent` attachToParent is given is the grid manager, a
 * StageMap (include/stage_map.h; dream_aux.c passes sDreamAuxStageMap), kept
 * in Actor's `grid` field; some MoodCue handlers call its startScaleRamp.
 *
 * Sound cues. startSoundCue calls InitSoundCueSet on `soundCueSet` with
 * TodActor's `sound` (the ctor's third argument; dream_aux.c passes
 * sDreamAuxSound) as the sound object, itself as the owner and the mood
 * row's handler as the callback, and reset selects tick callback 'B'
 * (Entity__TickSoundCue, +0x11C), which services the set once per tick.
 *
 * The MoodCue handlers. Each row's `handler` (sEntityMoodTable, at the end of entity.c) is
 * one `Entity__Cue<Behaviour>` function, named for the small script it runs
 * (Entity__CueHoverOverDreamerOnBlueElseRise, Entity__CueTone23Once); a
 * comment on each in src/world/entity.c names its row. They are not in the
 * method table. A handler is a SoundCueSet callback (include/sound_cue_set.h):
 * ServiceSoundCueSet calls it once per tick as (owner, set) with this Entity
 * as the owner. It requests tones by filling the set's slots (a VAB program
 * of the cue's sound object, or SOUND_CUE_STOP); moves, turns and scales the
 * entity (or the player, its `peer`) on moodTimer (the ticks since
 * startSoundCue), on the cue set's own `tick`, or on todFrame (the frame of
 * its TOD animation); and sends the dream an EntityEffect through
 * notifyParents. Some rows share a handler, and some have none; a handler
 * whose body repeats another's keeps a Row suffix
 * (Entity__CueSixfoldSizeRow117, Entity__CueWalkWithTurnsMaybeGiantRow113).
 * In the names the dreamer is the player (`peer`); Walk, Run and Creep are
 * moves along local -z, Rise along -y; Link, Video and EndDream are the
 * EntityEffect sent; `Tone<N>` is VAB program N. Which dream object owns each
 * row is not established.
 *
 * Inherited slot types Entity's callers depend on: moveLocalZ (+0x0C4) and
 * moveLocalY (+0x0CC) return void; applyTodFrame (+0x134) returns the next
 * frame, which Entity__CueFadeSkipTodThenLink and
 * Entity__CueAwaitReachThenLinkAfterTod thread through it; attachToParent
 * (+0x04C) keeps SceneNode's type, so callers of Entity's occupant cast to
 * TodActorAttachToParentFn (include/tod_actor.h); playTod (+0x12C) returns a
 * flag Entity's callers do not read, and they call it through
 * EntityPlayTodFn.
 */
struct Entity {
    TODACTOR_FIELDS(EntityMethods);
    /* +0x098 */ s32 moodIndex; /**< New_Entity's first argument: the row of sEntityMoodTable */
    /* +0x09C */ SoundCueSet soundCueSet; /**< startSoundCue starts it; the MoodCue handlers are its callback */
    /* +0x0F0 */ s32 active;              /**< set by activate, cleared by deactivate */
    /* +0x0F4 */ s32 targetReached;  /**< setTargetReached; latched by updateTargetProximity */
    /* +0x0F8 */ s32 soundCueActive; /**< set by startSoundCue, cleared by stopSoundCue */
    /* +0x0FC */ s32 moodTimer; /**< zeroed by startSoundCue, counted by Entity__TickSoundCue */
    /* +0x100 */ FadeBox *fadeBox; /**< made by Entity__GetOrCreateFadeBox (New_FadeBox); released by Entity__Finalize */
    /* +0x104 */ BasicClass *ownedObject; /**< NULLed by the ctor and released by Entity__Finalize; no code sets it, so nothing shows its class */
};

/** playTod (+0x12C) as Entity's callers call it: its occupant returns a flag,
 * which they do not read. Every Entity call site casts the slot to this. */
typedef void (*EntityPlayTodFn)(Entity *self);

/** The codes an Entity sends its parents through notifyParents. DreamSys's
 * onActorLinkCommand (DreamSys__DispatchInstanceEffect) passes an Entity
 * sender's codes to DreamSys__InstanceEffectsOnJournal, whose switch is what
 * each does; it ignores them all while a link is pending.
 * Entity__SetTargetReached sends ENTITY_EFFECT_LOG_MOOD, and
 * Entity__NotifyLinkStage picks one of the other three from the mood row's
 * linkStage and eventVideo. */
enum EntityEffect {
    ENTITY_EFFECT_LOG_MOOD = 9, /**< log getMoodEffect's mood, add getUnlockEffect to the unlock score */
    ENTITY_EFFECT_LINK_STAGE = 10,  /**< link to the stage getLinkStage names */
    ENTITY_EFFECT_EVENT_VIDEO = 11, /**< end the dream into the video getEventVideo names */
    ENTITY_EFFECT_END_DREAM = 12    /**< end the dream */
};

/** Actor::state 1: Entity__UpdateActivationState does not activate an
 * inactive Entity whose state is 1, and Entity__UpdateSoundCueStart does not
 * restart its cue. The MoodCue handlers store it when their cue has run its
 * course (usually after deactivate or stopSoundCue); the other values they
 * store are each handler's own phases. */
#define ENTITY_STATE_DONE 1

/** Entity's method table (class id 0x1F234). */
extern EntityMethods gEntityMethods;

/**
 * @brief The Entity method table.
 * @return &gEntityMethods.
 */
extern EntityMethods *GetEntityMethods(void);

/**
 * @brief One row of the mood table, 16 bytes: New_Entity's moodIndex selects
 *        it, and every per-mood setting of an Entity is a column of it.
 *
 * Signed columns are `s8` (plain `char` is unsigned in this build). The
 * table itself, ENTITY_MOOD_ROW_COUNT rows, is defined at the end of
 * src/world/entity.c.
 */
struct EntityMoodRow {
    s8 unread00[2]; /**< +0x00: no code reads it; every value is in -10..10, the range of a mood graph axis */
    s8 unlockKind; /**< +0x02: times 1000 is the unlock score (Entity__GetUnlockEffect); 1 to 9: Entity__Reset turns fog on; -9 to -1: Entity__IsNearTarget moves the tested point by it times 1024 in y */
    s8 activateKind; /**< +0x03: an EntityActivateKind, read by Entity__UpdateActivationState; 0: Entity__AttachToParent activates at once */
    u8 deactivateKind; /**< +0x04: an EntityDeactivateKind, read by Entity__UpdateDeactivationState */
    s8 activeRange; /**< +0x05: Entity__IsNearTarget's distance for the activation and deactivation range tests; 0: no range test */
    s8 proximityRange; /**< +0x06: Entity__UpdateTargetProximity: its magnitude is the distance within which targetReached is raised; a NEGATIVE value also makes the entity face its target every tick */
    s8 linkStage; /**< +0x07: the stage the Entity links to (Entity__GetLinkStage, Entity__NotifyLinkStage); ENTITY_LINK_STAGE_END_DREAM ends the dream; 0 and below: no link */
    s8 eventVideo; /**< +0x08: the video it ends the dream into, plus 1 (Entity__GetEventVideo); 0: none */
    s8 nearTolerance; /**< +0x09: Entity__IsNearTarget's tolerance for every range test on this row (activation, deactivation, proximity, cue start/stop) */
    s8 proximityThreshold; /**< +0x0A: Entity__GetProximityRatio's range, in ENTITY_RANGE_UNITs */
    s8 cueRange; /**< +0x0B: 0: the cue starts at attach (when the entity activated there) and never on range; else its magnitude is the distance within which the cue starts (Entity__UpdateSoundCueStart), and a NEGATIVE value also stops it once the target leaves that range (Entity__UpdateSoundCueStop) */
    EntityMoodCueFn handler; /**< +0x0C: the `Entity__Cue<Behaviour>` handler Entity__StartSoundCue installs; NULL: the row has no cue script */
};

/** activateKind: when Entity__UpdateActivationState activates an inactive
 * Entity (never while its state is ENTITY_STATE_DONE). "Near" is
 * Entity__IsNearTarget on the row's activeRange and nearTolerance; the
 * random ones pass on one tick in 128. */
enum EntityActivateKind {
    ENTITY_ACTIVATE_AT_ATTACH = 0,   /**< Entity__AttachToParent activates it at once */
    ENTITY_ACTIVATE_NEAR = 1,        /**< while near */
    ENTITY_ACTIVATE_FAR = 2,         /**< while not near */
    ENTITY_ACTIVATE_NEAR_RANDOM = 3, /**< at random while near */
    ENTITY_ACTIVATE_RANDOM = 4       /**< at random */
};

/** deactivateKind: when Entity__UpdateDeactivationState deactivates an active
 * Entity. 0 and 3 make no test; 1 and 2 test as activateKind does; from
 * ENTITY_DEACTIVATE_TIMED up, it deactivates on the tick that equals
 * deactivateKind * 15. */
enum EntityDeactivateKind {
    ENTITY_DEACTIVATE_NONE = 0, /**< never */
    ENTITY_DEACTIVATE_NEAR = 1, /**< while near */
    ENTITY_DEACTIVATE_FAR = 2,  /**< while not near */
    ENTITY_DEACTIVATE_NONE_ALT = 3, /**< never either: what sets it apart from 0 is not in Entity code */
    ENTITY_DEACTIVATE_TIMED = 10 /**< and above: on tick deactivateKind * 15 */
};

/** A linkStage of 127 ends the dream instead of linking: Entity__NotifyLinkStage
 * sends ENTITY_EFFECT_EVENT_VIDEO when the row has an eventVideo, else
 * ENTITY_EFFECT_END_DREAM. Other positive values send
 * ENTITY_EFFECT_LINK_STAGE; 0 and below send nothing there. */
#define ENTITY_LINK_STAGE_END_DREAM 127

/** @name The mood row's range unit
 * The unit of the mood row's ranges and tolerances, in world units: what
 * Entity__IsNearTarget and Entity__GetProximityRatio scale them by (the
 * grid's cell size, STAGE_CELL_SIZE in stage_map.h, has the same value).
 * @{ */
#define ENTITY_RANGE_SHIFT 11                       /**< log2 of ENTITY_RANGE_UNIT */
#define ENTITY_RANGE_UNIT (1 << ENTITY_RANGE_SHIFT) /**< one range unit */
/** @} */

/**
 * @brief Allocates an Entity (0x108 bytes) and runs its ctor through the
 *        table; frees it again when the ctor fails.
 * @param moodIndex The mood-table row.
 * @param desc Handed to TodActor's ctor.
 * @param sound The sound object its cues play on.
 * @return The new Entity, or NULL.
 */
Entity *New_Entity(s32 moodIndex, void *desc, void *sound);

/**
 * @brief Constructor (slot +0x008): TodActor's ctor, then installs
 *        gEntityMethods, takes the mood row, clears the cue set's tag, the
 *        fade box and ownedObject, and resets.
 * @param self The entity.
 * @param moodIndex The mood-table row.
 * @param desc Handed to TodActor's ctor.
 * @param sound The sound object.
 * @return self, or NULL when TodActor's ctor fails.
 */
Entity *Entity__Entity(Entity *self, s32 moodIndex, void *desc, void *sound);

/**
 * @brief The screen fade some handlers run: makes the entity's FadeBox on
 *        first use, then (re)attaches it to the entity and sets its step.
 * @param self The entity.
 * @param size The box's size, or NULL for {320, 240}; used only when the box
 *        is made.
 * @param offset Its attach offset, or NULL for {-100, -100}.
 * @param step The fade step (FadeBox's setStep).
 * @param pri The box's priority; used only when the box is made.
 * @return The FadeBox, or NULL when it could not be made.
 */
FadeBox *Entity__GetOrCreateFadeBox(Entity *self, void *size, void *offset, void *step, s32 pri);

/**
 * @brief finalize (slot +0x00C): releases the fade box and ownedObject, then
 *        TodActor's finalize.
 * @param self The entity.
 */
void Entity__Finalize(Entity *self);

/**
 * @brief reset (slot +0x040): turns fog on for an unlockKind of 1 to 9,
 *        selects tick callback B (Entity__TickSoundCue) and deactivates.
 * @param self The entity.
 */
void Entity__Reset(Entity *self);

/**
 * @brief attachToParent (slot +0x04C): when not attached yet, TodActor's
 *        attachToParent, keeps `parent` as the grid, and for a row with no
 *        activation condition activates at once and, if the row has no cue
 *        range either, starts the sound cue.
 * @param self The entity.
 * @param peer The player (a DreamSys).
 * @param companion Handed to TodActor's attachToParent.
 * @param parent The grid manager, kept in `grid`.
 * @param offset The attach offset.
 */
void Entity__AttachToParent(Entity *self, TodActor *peer, void *companion, struct StageMap *parent,
                            void *offset);

/**
 * @brief detachFromParent (slot +0x050): when attached, deactivates,
 *        TodActor's detachFromParent, and forgets the grid.
 * @param self The entity.
 */
void Entity__DetachFromParent(Entity *self);

/**
 * @brief update (slot +0x098): the per-tick state: activation (then
 *        deactivation, while active), sound-cue start (then stop, while
 *        running), target proximity, then TodActor's update.
 * @param self The entity.
 * @param sender The tick's sender, handed on.
 * @param event The tick's event, handed on.
 */
void Entity__Update(Entity *self, void *sender, s32 event);

/**
 * @brief onActorLinkCommand (slot +0x0DC): passes a link command to
 *        TodActor's handler and, when the command is SCENENODE_EVENT_LINKED
 *        and the row has a link stage, sends the row's effect to the parents
 *        (LINK_STAGE; for ENTITY_LINK_STAGE_END_DREAM, EVENT_VIDEO or
 *        END_DREAM). A row with no link stage drops the hull and move
 *        commands.
 * @param self The entity.
 * @param sender The command's sender.
 * @param event The command.
 */
void Entity__NotifyLinkStage(Entity *self, void *sender, s32 event);

/**
 * @brief onGridCellLinkCommand (slot +0x0E0): TodActor's handler, then
 *        deactivates on SCENENODE_EVENT_LINKED.
 * @param self The entity.
 * @param sender The command's sender.
 * @param event The command.
 */
void Entity__OnGridCellLinkCommand(Entity *self, void *sender, s32 event);

/**
 * @brief Tick callback B (slot +0x11C): services the sound cue set (which
 *        runs the MoodCue handler) and counts moodTimer.
 * @param self The entity.
 */
void Entity__TickSoundCue(Entity *self);

/**
 * @brief Whether a position lies near the point a distance ahead of the
 *        player (DreamSys__ProjectPointAtDistance). For an unlockKind of -9
 *        to -1 the position is moved by unlockKind * 1024 in y first.
 * @param self The entity.
 * @param pos The position (a LongVec3).
 * @param range How far ahead of the player, in ENTITY_RANGE_UNITs.
 * @param tolerance How near, in ENTITY_RANGE_UNITs; a negative -n means
 *        ENTITY_RANGE_UNIT / n.
 * @return Non-zero when near.
 */
s32 Entity__IsNearTarget(Entity *self, void *pos, s32 range, s32 tolerance);

/**
 * @brief distanceToPeer (slot +0x144): |dx| + |dz| from the entity's local
 *        translation to the peer's world translation.
 * @param self The entity.
 * @param peer The player; it must be attached.
 * @return The distance in world units.
 */
s32 Entity__DistanceToPeer(Entity *self, TodActor *peer);

/**
 * @brief getProximityRatio (slot +0x148): the sound attenuation step for the
 *        player's distance.
 * @param self The entity.
 * @return -1 without a peer or beyond the row's proximityThreshold;
 *         otherwise the distance in steps of threshold / the cue set's
 *         attenuationSteps, 0 nearest.
 */
s32 Entity__GetProximityRatio(Entity *self);

/**
 * @brief getMoodEffect (slot +0x14C): the entity's mood row.
 * @param self The entity.
 * @return &sEntityMoodTable[moodIndex].
 */
EntityMoodRow *Entity__GetMoodEffect(Entity *self);

/**
 * @brief getUnlockEffect (slot +0x150): the unlock score the entity adds.
 * @param self The entity.
 * @return The row's unlockKind * 1000.
 */
s32 Entity__GetUnlockEffect(Entity *self);

/**
 * @brief getLinkStage (slot +0x154): the stage index the row's linkStage
 *        names.
 * @param self The entity.
 * @return n - 1 for a positive linkStage n, ~n for a negative one (so -1 is
 *         stage 0).
 */
s32 Entity__GetLinkStage(Entity *self);

/**
 * @brief getEventVideo (slot +0x158): the video the row's eventVideo names.
 * @param self The entity.
 * @return eventVideo - 1 (-1 for none).
 */
s32 Entity__GetEventVideo(Entity *self);

/**
 * @brief activate (slot +0x15C): shows the entity, sets `active` and
 *        restarts the actor's tick count.
 * @param self The entity.
 */
void Entity__Activate(Entity *self);

/**
 * @brief deactivate (slot +0x160): hides the entity, stops its sound cue,
 *        clears targetReached and `active`.
 * @param self The entity.
 */
void Entity__Deactivate(Entity *self);

/**
 * @brief setTargetReached (slot +0x164): stores the flag; setting it sends
 *        ENTITY_EFFECT_LOG_MOOD to the parents.
 * @param self The entity.
 * @param reached Non-zero once the player has reached the entity.
 */
void Entity__SetTargetReached(Entity *self, s32 reached);

/**
 * @brief startSoundCue (slot +0x168): starts the row's cue set with its
 *        MoodCue handler, plays the TOD animation, enables the tick callback
 *        and zeroes moodTimer.
 * @param self The entity.
 */
void Entity__StartSoundCue(Entity *self);

/**
 * @brief stopSoundCue (slot +0x16C): flushes the cue set, stops the TOD
 *        animation and disables the tick callback.
 * @param self The entity.
 */
void Entity__StopSoundCue(Entity *self);

/**
 * @brief updateActivationState (slot +0x170): activates an inactive entity
 *        not in ENTITY_STATE_DONE when its row's activateKind condition
 *        holds.
 * @param self The entity.
 * @return `active`.
 */
s32 Entity__UpdateActivationState(Entity *self);

/**
 * @brief updateDeactivationState (slot +0x174): for an active entity, sends
 *        the event-video range effect (Entity__NotifyIfTargetInRange), then
 *        deactivates when its row's deactivateKind condition holds.
 * @param self The entity.
 * @return `active`.
 */
s32 Entity__UpdateDeactivationState(Entity *self);

/**
 * @brief updateTargetProximity (slot +0x178): while active, raises
 *        targetReached once the player is within the row's proximityRange;
 *        a negative proximityRange also turns the entity to face the player.
 * @param self The entity.
 * @return targetReached.
 */
s32 Entity__UpdateTargetProximity(Entity *self);

/**
 * @brief updateSoundCueStart (slot +0x17C): starts the sound cue of an
 *        active entity not in ENTITY_STATE_DONE once the player is within the
 *        row's cueRange (a row with cueRange 0 never starts it here).
 * @param self The entity.
 * @return soundCueActive.
 */
s32 Entity__UpdateSoundCueStart(Entity *self);

/**
 * @brief For a row whose linkStage is negative and that has an eventVideo,
 *        sends ENTITY_EFFECT_LINK_STAGE while the player is within
 *        eventVideo * 512 world units (Entity__IsTargetInRange).
 * @param self The entity.
 * @param unused Not read; the one caller passes 0.
 */
void Entity__NotifyIfTargetInRange(Entity *self, s32 unused);

/**
 * @brief Whether the player is within 512 world units of the entity's height
 *        and closer than a distance (distanceToPeer, |dx| + |dz|).
 * @param self The entity.
 * @param range The distance in world units.
 * @return 1 when in range, else 0.
 */
s32 Entity__IsTargetInRange(Entity *self, s32 range);

/**
 * @brief updateSoundCueStop (slot +0x180): stops a running sound cue once the
 *        player leaves the range of a row whose cueRange is negative.
 * @param self The entity.
 * @return soundCueActive.
 */
s32 Entity__UpdateSoundCueStop(Entity *self);

/**
 * @brief Row 51's handler, run by row 113's too (Entity__CueWalkWithTurnsMaybeGiantRow113): at
 *        moodTimer 0 in state 0, one time in five, scales to sScaleSix,
 *        moves 800 in local y and leaves state 0, so only once; plays program 8 every
 *        fifth tick; turns by -90, +90 and (at random) 180 degrees at
 *        moodTimer 90, 160 and 220; moves -80 a tick in local z.
 * @param self The entity.
 * @param out Its sound cue set.
 */
void Entity__CueWalkWithTurnsMaybeGiant(Entity *self, SoundCueSet *out);

/**
 * @brief Row 71's handler, run by row 108's before it scales the entity
 *        (Entity__CueWalkInRandomLaneGiant): on the cue's first tick plays program 0 and
 *        moves 0, 51200 or 102400 in local x at random; faces the player
 *        after moodTimer 2400; moves -30 a tick in local z.
 * @param self The entity.
 * @param out Its sound cue set.
 */
void Entity__CueWalkInRandomLane(Entity *self, SoundCueSet *out);

/**
 * @brief A shared handler body (Entity__CueRunOffOrStopAndJitterDepth, Entity__CueWanderPauseOnPink):
 *        plays program 4 on three voices on tick 6, turns 1 degree a tick in
 *        three moodTimer windows from windowStart (+0..91, +341..433,
 *        +698..791), moves zStep a tick, and at deactivateTimer deactivates
 *        and sets ENTITY_STATE_DONE.
 * @param self The entity.
 * @param out Its sound cue set.
 * @param windowStart The moodTimer the first turning window opens at.
 * @param deactivateTimer The moodTimer at which it deactivates.
 * @param zStep The local z step per tick.
 */
void Entity__StepYawInWindowsThenDeactivate(Entity *self, SoundCueSet *out, s32 windowStart,
                                            s32 deactivateTimer, s32 zStep);

#endif
