#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"
#include "TodActor.h"
#include "FadeBox.h"
#include "SoundCueSet.h"

/*
 * Entity -- a TodActor (TOD-animated Actor) driven by a per-mood row of
 * tables (class id 0x1F234, method table gEntityMethods, getter
 * Get_vtable_Entity): TodActor's one subclass (include/TodActor.h); no
 * class derives from it. The ctor calls TodActor's first
 * (GetTodActorMethods()->ctor), so the id parent is the ctor-chain
 * parent. Its methods and its MoodCue handlers are in src/Entity.c. The
 * MoodCue handlers are not in the table: they are
 * the `handler` of gEntityMoodHandlerTable's rows. Spawned by DreamAux
 * (SetDreamAuxWorld, SpawnDreamAuxTriggerEntity).
 *
 * The mood row. New_Entity's first argument is `moodIndex`, which selects a
 * 16-byte row of gEntityMoodTable and of the parallel byte tables below.
 * Every tick, update (+0x098) runs updateActivationState (activate when the
 * row's activateKind condition holds), updateDeactivationState, the sound-cue
 * start/stop pair and updateTargetProximity, then TodActor's update.
 *
 * The peer is the player. attachToParent's (self, peer, companion, parent,
 * offset) is TodActor's; DreamAux passes gDreamAuxWorld as the peer, and
 * the slots Entity calls on `peer` (+0x100, +0x120, +0x1A0, +0x200, +0x21C)
 * lie past the end of TodActor's table: their occupants in
 * gDreamSysMethods are DreamSys__GetLinkCommandFlag,
 * DreamSys__ProjectPointAtDistance, DreamSys__GetCurrentDayAndYear,
 * DreamSys__GetDreamColor and DreamSys__ResetFlashbackList. The units that
 * call it include include/DreamSys.h and cast `peer` (TodActor's field,
 * typed TodActor *) to DreamSys *; the Unk94Obj view that stood here was
 * deleted in track 4 (round 88). Entity's attachToParent keeps its `parent` argument in
 * Actor's `grid` field (+0x04C).
 *
 * Sound cues. startSoundCue calls InitSoundCueSet on `soundCueSet` with
 * TodActor's `sound` (the ctor's third argument; DreamAux passes
 * gDreamAuxSound) as the sound object, itself as the owner and the mood row's
 * handler as the callback, and selects tick callback 'B' in reset
 * (Entity__TickSoundCue, +0x11C), which services the set once per tick. So a
 * MoodCue handler is a SoundCueSet callback (include/SoundCueSet.h):
 * ServiceSoundCueSet calls it as (owner, set) with this Entity as the owner,
 * and it requests tones by filling the set's slots.
 *
 * Inherited slot types, settled by callers' bytes (keep them):
 *  - moveLocalZ (+0x0C4) and moveLocalY (+0x0CC) return void. Entity__MoodCue00
 *    tail-merges two moveLocalZ calls and Entity__MoodCue115 merges a
 *    moveLocalY call with void siblings; GCC 2.6.3 cannot cross-jump a
 *    value-returning call with a void one (docs/match-reports/
 *    Entity__MoodCue115.md). Entity__MoodCue32/37 compile the same either way.
 *  - applyTodFrame (+0x134) returns the next frame: Entity__MoodCue91/92
 *    thread todFramePtr through it.
 *  - attachToParent (+0x04C) keeps SceneNode's type; callers of Entity's
 *    occupant cast to TodActorAttachToParentFn (TodActor.h's banner).
 *
 * The object is 0x108 bytes (New_Entity); TodActor's fields end at +0x098.
 */

typedef struct Entity Entity;
typedef struct EntityMethods EntityMethods;
typedef struct EntityMoodRow EntityMoodRow;

/* TodActor's slots (overrides: +0x008 Entity__Entity, +0x00C
 * Entity__Finalize, +0x040 Entity__Reset, +0x04C Entity__AttachToParent,
 * +0x050 Entity__DetachFromParent, +0x098 Entity__Update, +0x0DC
 * Entity__NotifyLinkStage, +0x0E0 Entity__OnGridCellLinkCommand, +0x11C
 * Entity__TickSoundCue; `tools/classtable.py gEntityMethods --vs
 * gTodActorMethods`), then this class's own. */
struct EntityMethods {
    TODACTOR_SLOTS(Entity, (Entity * self, s32 moodIndex, void *desc, void *sound));
    /* +0x144 */ s32 (*distanceToPeer)(Entity *self, TodActor *peer); /* Entity__DistanceToPeer: |dx| + |dz| from coord2's translation to peer's world position */
    /* +0x148 */ s32 (*getProximityRatio)(Entity *self); /* Entity__GetProximityRatio: -1 when no peer or out of range */
    /* +0x14C */ EntityMoodRow *(*getMoodEffect)(Entity *self); /* Entity__GetMoodEffect: &gEntityMoodTable[moodIndex] */
    /* +0x150 */ s32 (*getUnlockEffect)(Entity *self); /* Entity__GetUnlockEffect */
    /* +0x154 */ s32 (*getLinkStage)(Entity *self);    /* Entity__GetLinkStage */
    /* +0x158 */ s32 (*getEventVideo)(Entity *self);   /* Entity__GetEventVideo */
    /* +0x15C */ void (*activate)(Entity *self); /* Entity__Activate: setDisplay(1), active = 1 */
    /* +0x160 */ void (*deactivate)(Entity *self); /* Entity__Deactivate: setDisplay(0), stopSoundCue, setTargetReached(0) */
    /* +0x164 */ void (*setTargetReached)(Entity *self, s32 reached); /* Entity__SetTargetReached: notifyParents(9) when set */
    /* +0x168 */ void (*startSoundCue)(Entity *self); /* Entity__StartSoundCue */
    /* +0x16C */ void (*stopSoundCue)(Entity *self);  /* Entity__StopSoundCue */
    /* +0x170 */ s32 (*updateActivationState)(Entity *self); /* Entity__UpdateActivationState: returns active */
    /* +0x174 */ s32 (*updateDeactivationState)(Entity *self); /* Entity__UpdateDeactivationState: returns active */
    /* +0x178 */ s32 (*updateTargetProximity)(Entity *self); /* Entity__UpdateTargetProximity: returns targetReached */
    /* +0x17C */ s32 (*updateSoundCueStart)(Entity *self); /* Entity__UpdateSoundCueStart: returns soundCueActive */
    /* +0x180 */ void (*updateSoundCueStop)(Entity *self); /* Entity__UpdateSoundCueStop */
}; /* 97 slots, 0x184 bytes */

struct Entity {
    TODACTOR_FIELDS(EntityMethods);
    /* +0x098 */ s32 moodIndex; /* New_Entity's first argument: the row of gEntityMoodTable and the byte tables */
    /* +0x09C */ SoundCueSet soundCueSet; /* startSoundCue starts it; the MoodCue handlers are its callback */
    /* +0x0F0 */ s32 active;              /* activate / deactivate */
    /* +0x0F4 */ s32 targetReached;  /* setTargetReached; latched by updateTargetProximity */
    /* +0x0F8 */ s32 soundCueActive; /* startSoundCue / stopSoundCue */
    /* +0x0FC */ s32 moodTimer;      /* zeroed by startSoundCue, counted by Entity__TickSoundCue */
    /* +0x100 */ FadeBox *fadeBox; /* made by Entity__GetOrCreateFadeBox (New_FadeBox); released by Entity__Finalize */
    /* +0x104 */ BasicClass *unk104; /* released by Entity__Finalize, never set in Entity code: nothing shows its class. The object is 0x108 bytes (New_Entity) */
};

/* playTod (+0x12C) is TodActor's slot and returns the flag its occupant
 * sets, but Entity's callers call it as void: Entity__MoodCue39/57 (Entity)
 * and Entity__MoodCue86 (Entity) cross-jump a playTod call with a void
 * sibling (stopTod), which GCC 2.6.3 does only when both are void (the
 * moveLocalZ case in the banner); through the s32 slot they grow 3 words
 * each. Every Entity call site casts the slot to this typedef, which emits
 * no code. */
typedef void (*EntityPlayTodFn)(Entity *self);

/* The codes an Entity sends its parents through notifyParents. DreamSys's
 * onActorLinkCommand (DreamSys__DispatchInstanceEffect) passes an Entity
 * sender's codes to DreamSys__InstanceEffectsOnJournal, whose switch is
 * what each does; it ignores them all while a link is pending.
 * Entity__SetTargetReached sends ENTITY_EFFECT_LOG_MOOD, and
 * Entity__NotifyLinkStage picks one of the other three from the mood row's
 * gEntityLinkStageTable and gEntityEventVideoTable entries. */
enum EntityEffect {
    ENTITY_EFFECT_LOG_MOOD = 9, /* log getMoodEffect's mood, add getUnlockEffect to the unlock score */
    ENTITY_EFFECT_LINK_STAGE = 10,  /* link to the stage getLinkStage names */
    ENTITY_EFFECT_EVENT_VIDEO = 11, /* end the dream into the video getEventVideo names */
    ENTITY_EFFECT_END_DREAM = 12    /* end the dream */
};

/* Actor::state 1: Entity__UpdateActivationState does not activate an
 * inactive Entity whose state is 1, and Entity__UpdateSoundCueStart does
 * not restart its cue. The MoodCue handlers store it when their cue has run
 * its course (usually after deactivate or stopSoundCue); the other values
 * they store are each handler's own phases. */
#define ENTITY_STATE_DONE 1

extern EntityMethods gEntityMethods;
extern EntityMethods *Get_vtable_Entity(void); /* returns &gEntityMethods */

/* The object Entity__AttachToParent keeps in Actor's `grid` field (+0x04C)
 * is the grid manager, StageMap (include/StageMap.h; DreamAux passes
 * gDreamAuxStageMap). Entity/_e/_g call its startScaleRamp (+0x138). */

/* The size and attach offset Entity__GetOrCreateFadeBox substitutes when its
 * `size`/`offset` arguments are NULL: {320, 240} and {-100, -100}, what
 * Viewport gives its FadeBox (FadeBox.h). */
extern s32 gEntityFadeBoxDefaultSize[2];
extern s32 gEntityFadeBoxDefaultOffset[2];

/* One row of the mood table (16 bytes): New_Entity's moodIndex selects it, and
 * every per-mood setting of an Entity is a column of it. Signed columns are
 * `s8` (`lb`); plain `char` would be unsigned here (-funsigned-char).
 *
 * gEntityLinkStageTable and gEntityEventVideoTable are two of its columns
 * seen as flat arrays (the row base + 7 and + 8, indexed moodIndex * 16):
 * GCC spells a constant-offset field of a global array as `%hi`/`%lo(sym +
 * off)`, which splat labels as a symbol of its own. Entity still reads them
 * that way; the field spelling compiles to the same bytes. */
struct EntityMoodRow {
    u8 pad00[0x02];
    s8 unlockKind; /* +0x02, Entity__GetUnlockEffect: times 1000 is the unlock score; 1 to 9: Entity__Reset turns fog on; -9 to -1: Entity__IsNearTarget moves the tested point by it times 1024 in y */
    s8 activateKind; /* +0x03, read by Entity__UpdateActivationState; 0: Entity__AttachToParent activates at once */
    u8 deactivateKind; /* +0x04, read by Entity__UpdateDeactivationState (unsigned load) */
    s8 activeRange; /* +0x05, Entity__IsNearTarget's distance for the activation and deactivation range tests; 0: no range test */
    s8 proximityRange; /* +0x06, read by Entity__UpdateTargetProximity: magnitude (after abs) is Entity__IsNearTarget's distance arg for raising targetReached via setTargetReached; a NEGATIVE value also makes the entity face its target every tick */
    s8 linkStage; /* +0x07, Entity__GetLinkStage and Entity__NotifyLinkStage (gEntityLinkStageTable) */
    s8 eventVideo; /* +0x08, Entity__GetEventVideo and Entity__NotifyLinkStage (gEntityEventVideoTable) */
    s8 nearTolerance; /* +0x09, Entity__IsNearTarget's tolerance for every range test on this row (activation, deactivation, proximity, cue start/stop) */
    s8 proximityThreshold; /* +0x0A, Entity__GetProximityRatio's range, in ENTITY_RANGE_UNITs */
    s8 cueRange; /* +0x0B, read by Entity__UpdateSoundCueStart/Entity__UpdateSoundCueStop and Entity__AttachToParent: 0 = the cue starts at attach (when the entity activated there) and never on range; magnitude (after abs) is Entity__IsNearTarget's distance arg for starting the sound cue; a NEGATIVE value also stops it again once the target leaves that range. SEPARATE field from proximityRange (+0x06) */
    SoundCueCallbackFn handler; /* +0x0C, the Entity__MoodCueNN Entity__StartSoundCue installs (symbol gEntityMoodHandlerTable, 0x80089EB0) */
};

/* activateKind: when Entity__UpdateActivationState activates an inactive
 * Entity (never while its state is ENTITY_STATE_DONE). "Near" is
 * Entity__IsNearTarget on the row's activeRange and nearTolerance; the
 * random ones pass on one tick in 128. */
enum EntityActivateKind {
    ENTITY_ACTIVATE_AT_ATTACH = 0,   /* Entity__AttachToParent activates it at once */
    ENTITY_ACTIVATE_NEAR = 1,        /* while near */
    ENTITY_ACTIVATE_FAR = 2,         /* while not near */
    ENTITY_ACTIVATE_NEAR_RANDOM = 3, /* at random while near */
    ENTITY_ACTIVATE_RANDOM = 4       /* at random */
};

/* deactivateKind: when Entity__UpdateDeactivationState deactivates an active
 * Entity. 0 and 3 make no test; 1 and 2 test as activateKind does; from
 * ENTITY_DEACTIVATE_TIMED up, it deactivates on the tick that equals
 * deactivateKind * 15. */
enum EntityDeactivateKind {
    ENTITY_DEACTIVATE_NONE = 0,
    ENTITY_DEACTIVATE_NEAR = 1,
    ENTITY_DEACTIVATE_FAR = 2,
    ENTITY_DEACTIVATE_NONE_ALT = 3, /* no test either: what sets it apart from 0 is not in Entity code */
    ENTITY_DEACTIVATE_TIMED = 10
};

/* A linkStage of 127 ends the dream instead of linking: Entity__NotifyLinkStage
 * sends ENTITY_EFFECT_EVENT_VIDEO when the row has an eventVideo, else
 * ENTITY_EFFECT_END_DREAM. Other positive values send
 * ENTITY_EFFECT_LINK_STAGE; 0 and below send nothing there. */
#define ENTITY_LINK_STAGE_END_DREAM 127

/* The unit of the mood row's ranges and tolerances, in world units: what
 * Entity__IsNearTarget and Entity__GetProximityRatio scale them by (the
 * grid's cell size, STAGE_CELL_SIZE in StageMap.h, has the same value). */
#define ENTITY_RANGE_SHIFT 11
#define ENTITY_RANGE_UNIT (1 << ENTITY_RANGE_SHIFT)

extern EntityMoodRow gEntityMoodTable[];
extern s8 gEntityLinkStageTable[];  /* the linkStage column (Entity) */
extern s8 gEntityEventVideoTable[]; /* the eventVideo column (Entity) */

/* The class's own methods, in ROM order (Entity, then Entity). A caller
 * reaching the base ones goes through GetTodActorMethods() and upcasts. */
Entity *New_Entity(s32 moodIndex, void *desc, void *sound);
Entity *Entity__Entity(Entity *self, s32 moodIndex, void *desc, void *sound);
FadeBox *Entity__GetOrCreateFadeBox(Entity *self, void *size, void *offset, void *step, s32 pri);
void Entity__Finalize(Entity *self);
void Entity__Reset(Entity *self);
void Entity__AttachToParent(Entity *self, TodActor *peer, void *companion, struct StageMap *parent,
                            void *offset);
void Entity__DetachFromParent(Entity *self);
void Entity__Update(Entity *self, void *sender, s32 event);
void Entity__NotifyLinkStage(Entity *self, void *sender, s32 event);
void Entity__OnGridCellLinkCommand(Entity *self, void *sender, s32 event);
void Entity__TickSoundCue(Entity *self);
s32 Entity__IsNearTarget(Entity *self, void *pos, s32 range, s32 tolerance);
s32 Entity__DistanceToPeer(Entity *self, TodActor *peer);
s32 Entity__GetProximityRatio(Entity *self);
EntityMoodRow *Entity__GetMoodEffect(Entity *self);
s32 Entity__GetUnlockEffect(Entity *self);
s32 Entity__GetLinkStage(Entity *self);
s32 Entity__GetEventVideo(Entity *self);
void Entity__Activate(Entity *self);
void Entity__Deactivate(Entity *self);
void Entity__SetTargetReached(Entity *self, s32 reached);
void Entity__StartSoundCue(Entity *self);
void Entity__StopSoundCue(Entity *self);
s32 Entity__UpdateActivationState(Entity *self);
s32 Entity__UpdateDeactivationState(Entity *self);
s32 Entity__UpdateTargetProximity(Entity *self);
s32 Entity__UpdateSoundCueStart(Entity *self);
void Entity__NotifyIfTargetInRange(Entity *self, s32 arg1);
s32 Entity__IsTargetInRange(Entity *self, s32 range);
s32 Entity__UpdateSoundCueStop(Entity *self);

/* MoodCue handlers called from another Entity unit. */
void Entity__MoodCue51(Entity *self, SoundCueSet *out); /* Entity; called by Entity__MoodCue113 (Entity) */
void Entity__MoodCue71(Entity *self, SoundCueSet *out); /* Entity; called by Entity__MoodCue108 (Entity) */
void Entity__StepYawInWindowsThenDeactivate(Entity *self, SoundCueSet *out, s32 windowStart,
                                            s32 deactivateTimer, s32 zStep); /* Entity; called by Entity */

/* The motion templates (.data, 0x80089C58..0x80089E97, in address order):
 * the constant triples the MoodCue handlers in Entity..Entity pass to
 * updateRotation (+0x044) and updateScale (+0x048) -- three Ratio16s
 * (include/SceneNode.h), degrees or scale factors, {x, y, z} -- and to
 * addTranslation (+0x0BC), three s32 deltas. Named by value. The slots take
 * the table untyped, so the element type is the reader's (SceneNode__Update-
 * Rotation/UpdateScale), not the callers'. sTranslateYMinus64's label also
 * holds a second triple, (0, -0x20, 0); SCALE_X3's z den is Entity.c's
 * sScaleTemplateZDenom. */
extern Ratio16 ROTATION_XPLUS_EIGHTH[];
extern Ratio16 ROTATION_YAW_PLUS9[];
extern Ratio16 ROTATION_YAW_MINUS9[];
extern Ratio16 ROTATION_YAW_PLUS180[];
extern Ratio16 sRotationYawPlus90[];
extern Ratio16 sRotationYawMinus90[];
extern Ratio16 sRotationYawPlus2[];
extern Ratio16 ROTATION_YAW_MINUS_THIRD[];
extern Ratio16 ROTATION_YAW_MINUS_HALF[];
extern Ratio16 ROTATION_ZPLUS9[];
extern Ratio16 ROTATION_ZPLUS1[];
extern Ratio16 ROTATION_ZMINUS9[];
extern Ratio16 sRotationYawMinus120[];
extern Ratio16 sRotationX50YMinus120Z30[];
extern Ratio16 ROTATION_YAW_PLUS4[];
extern Ratio16 ROTATION_XPLUS90[];
extern Ratio16 ROTATION_YAW_PLUS1[];
extern Ratio16 ROTATION_ZMINUS90[];
extern LongVec3 sTranslateYPlus256[];
extern LongVec3 TRANSLATE_Y_MINUS4096[];
extern LongVec3 TRANSLATE_Y_MINUS512[];
extern LongVec3 TRANSLATE_Y_PLUS64[];
extern LongVec3 TRANSLATE_Y_PLUS8[];
extern LongVec3 sTranslateYMinus64[];
extern LongVec3 TRANSLATE_Y_MINUS256[];
extern LongVec3 TRANSLATE_X_MINUS64[];
extern LongVec3 sTranslateYPlus64ZMinus64[];
extern LongVec3 TRANSLATE_Y_MINUS1500_Z_PLUS1024[];
extern LongVec3 TRANSLATE_Z_MINUS256[];
extern Ratio16 SCALE_QUARTER[];
extern Ratio16 sScaleHalf[];
extern Ratio16 SCALE_X_FOUR_FIFTHS_Y_SIX_FIFTHS[]; /* {4/5, 6/5, 5/5} */
extern Ratio16 sScaleDouble[];
extern Ratio16 SCALE_MINUS_SIXTY_FOURTH[];
extern Ratio16 SCALE_EIGHT_SEVENTHS[];
extern Ratio16 SCALE_UNIT[]; /* {1/1, 1/1, 1/1}, a .data copy of SceneNode.h's SCALE_ONE */
extern Ratio16 SCALE_EIGHTH[];
extern Ratio16 SCALE_X_EIGHTH_Y2_Z_EIGHTH[];
extern Ratio16 SCALE_SIX[];
extern Ratio16 SCALE_TWO_FIFTHS[];
extern Ratio16 SCALE_Y2[];
extern Ratio16 SCALE_Y4[];
extern Ratio16 SCALE_TRIPLE[];
extern Ratio16 SCALE_THIRTY_SECOND[];
extern Ratio16 SCALE_X3[];

/* Functions of other units Entity calls directly. The SoundCueSet functions
 * are defined in PlacementGridVabSound/l as (VabStreamObj *, SoundCueSet *); these
 * declarations take TodActor's `arg2` untyped, and their results are
 * unused. */
extern void *BMemPMgrAlloc(s32 size);
extern void BMemPMgrFree(void *arg);
extern void ServiceSoundCueSet(void *sound, SoundCueSet *set);
extern void FlushSoundCueSet(void *sound, SoundCueSet *set);
extern s32 rand(void);

#endif
