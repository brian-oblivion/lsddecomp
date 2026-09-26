#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"
#include "Class65650.h"
#include "Class6E99C.h"

/*
 * Entity -- a Class65650 (TOD-animated Actor) driven by a per-mood row of
 * tables (class id 0x1F234, method table ENTITY_METHODS, getter
 * Get_vtable_Entity): Class65650's one subclass (include/Class65650.h); no
 * class derives from it. The ctor calls Class65650's first
 * (Get_vtable_Class65650()->ctor), so the id parent is the ctor-chain
 * parent. Methods in src/Entity.c (New_Entity .. Entity__UpdateDeactivationState)
 * and src/Entity_b.c (the last three slots, the range helpers, the getter).
 * The MoodCue handlers in Entity_b..Entity_g are not in the table: they are
 * the `handler` of gEntityMoodHandlerTable's rows. Spawned by code_4cd08
 * (SetDreamAuxWorld, SpawnDreamAuxTriggerEntity).
 *
 * The mood row. New_Entity's first argument is `moodIndex`, which selects a
 * 16-byte row of gEntityMoodTable and of the parallel byte tables below.
 * Every tick, update (+0x098) runs updateActivationState (activate when the
 * row's detachKind condition holds), updateDeactivationState, the sound-cue
 * start/stop pair and updateTargetProximity, then Class65650's update.
 *
 * The peer is the player. attachToParent's (self, peer, companion, parent,
 * offset) is Class65650's; code_4cd08 passes gDreamAuxWorld as the peer, and
 * the slots Entity calls on `peer` (+0x100, +0x120, +0x1A0, +0x200, +0x21C)
 * lie past the end of Class65650's table: their occupants in
 * DREAMSYS_METHODS are DreamSys__GetLinkCommandFlag,
 * DreamSys__ProjectPointAtDistance, DreamSys__GetCurrentDayAndYear,
 * DreamSys__GetDreamColor and DreamSys__ResetFlashbackList. `Unk94Obj` below
 * is this header's view of that object (DreamSys is not unified); accessors
 * cast `peer` to it. Entity's attachToParent keeps its `parent` argument in
 * Actor's `grid` field (+0x04C).
 *
 * Sound cues. startSoundCue calls InitSoundCueSet on `soundCueSet` with
 * Class65650's `arg2` (the ctor's third argument; code_4cd08 passes
 * D_8008AC04) as the sound object, itself as the owner and the mood row's
 * handler as the callback, and selects tick callback 'B' in reset
 * (Entity__TickSoundCue, +0x11C), which services the set once per tick. So a
 * MoodCue handler is a SoundCueSet callback: ServiceSoundCueSet calls it as
 * (owner, set), and EntityMoodHandlerArg is the SoundCueSet
 * (src/code_179d8_l.c's view has the same offsets).
 *
 * Inherited slot types, settled by callers' bytes (keep them):
 *  - moveLocalZ (+0x0C4) and moveLocalY (+0x0CC) return void. Entity__MoodCue00
 *    tail-merges two moveLocalZ calls and Entity__MoodCue115 merges a
 *    moveLocalY call with void siblings; GCC 2.6.3 cannot cross-jump a
 *    value-returning call with a void one (docs/match-reports/
 *    Entity__MoodCue115.md). Entity__MoodCue32/37 compile the same either way.
 *  - applyTodFrame (+0x134) returns the next frame: Entity__MoodCue91/92
 *    thread todFramePtr through it.
 *  - attachToParent (+0x04C) keeps Class6B5CC's type; callers of Entity's
 *    occupant cast to Class65650AttachToParentFn (Class65650.h's banner).
 *
 * The object is 0x108 bytes (New_Entity); Class65650's fields end at +0x098.
 */

typedef struct Entity Entity;
typedef struct EntityMethods EntityMethods;
typedef struct Unk94Obj Unk94Obj;
typedef struct Unk94Methods Unk94Methods;
typedef struct Unk5CObj Unk5CObj;
typedef struct Unk5CMethods Unk5CMethods;
typedef struct Unk4CObj Unk4CObj;
typedef struct Unk4CMethods Unk4CMethods;
typedef struct EntityMoodRow EntityMoodRow;
typedef struct EntityMoodHandlerArg EntityMoodHandlerArg;

/* Class65650's slots (overrides: +0x008 Entity__Entity, +0x00C
 * Entity__Finalize, +0x040 Entity__Reset, +0x04C Entity__AttachToParent,
 * +0x050 Entity__DetachFromParent, +0x098 Entity__Update, +0x0DC
 * Entity__NotifyLinkStage, +0x0E0 Entity__OnClass86AA0LinkCommand, +0x11C
 * Entity__TickSoundCue; `tools/classtable.py ENTITY_METHODS --vs
 * gClass65650Methods`), then this class's own. */
struct EntityMethods {
    CLASS65650_SLOTS(Entity, (Entity *self, s32 moodIndex, void *desc, void *arg2));
    /* +0x144 */ s32 (*distanceToPeer)(Entity *self, Class65650 *peer);  /* Entity__DistanceToPeer: |dx| + |dz| from coord2's translation to peer's world position */
    /* +0x148 */ s32 (*getProximityRatio)(Entity *self);                 /* Entity__GetProximityRatio: -1 when no peer or out of range */
    /* +0x14C */ EntityMoodRow *(*getMoodEffect)(Entity *self);          /* Entity__GetMoodEffect: &gEntityMoodTable[moodIndex] */
    /* +0x150 */ s32 (*getUnlockEffect)(Entity *self);                   /* Entity__GetUnlockEffect */
    /* +0x154 */ s32 (*getLinkStage)(Entity *self);                      /* Entity__GetLinkStage */
    /* +0x158 */ s32 (*getEventVideo)(Entity *self);                     /* Entity__GetEventVideo */
    /* +0x15C */ void (*activate)(Entity *self);                         /* Entity__Activate: setDisplay(1), active = 1 */
    /* +0x160 */ void (*deactivate)(Entity *self);                       /* Entity__Deactivate: setDisplay(0), stopSoundCue, setTargetReached(0) */
    /* +0x164 */ void (*setTargetReached)(Entity *self, s32 reached);    /* Entity__SetTargetReached: notifyParents(9) when set */
    /* +0x168 */ void (*startSoundCue)(Entity *self);                    /* Entity__StartSoundCue */
    /* +0x16C */ void (*stopSoundCue)(Entity *self);                     /* Entity__StopSoundCue */
    /* +0x170 */ s32 (*updateActivationState)(Entity *self);             /* Entity__UpdateActivationState: returns active */
    /* +0x174 */ s32 (*updateDeactivationState)(Entity *self);           /* Entity__UpdateDeactivationState: returns active */
    /* +0x178 */ s32 (*updateTargetProximity)(Entity *self);             /* Entity__UpdateTargetProximity: returns targetReached */
    /* +0x17C */ s32 (*updateSoundCueStart)(Entity *self);               /* Entity__UpdateSoundCueStart: returns soundCueActive */
    /* +0x180 */ void (*updateSoundCueStop)(Entity *self);               /* Entity__UpdateSoundCueStop */
};                                   /* 97 slots, 0x184 bytes */

/* The MoodCue handlers' second argument: the SoundCueSet they are the
 * callback of (see the banner). In src/code_179d8_l.c's names: +0x04 the
 * service count (unk4), +0x10 the skip word, and from +0x18 three 0x14-byte
 * slots {index, note, pitchOffset, word2, word3}, so unk1C/unk20/unk24/unk28
 * are slot 0's note/pitchOffset/word2/word3, unk30/unk34 slot 1's
 * note/pitchOffset and unk44/unk48 slot 2's. Entity embeds one at +0x09C. */
struct EntityMoodHandlerArg {
    s32 tag;     /* +0x00, InitSoundCueSet's tag (moodIndex + 1); serviced only while > 0; zeroed by Entity__Entity */
    s32 unk4;    /* +0x04, the service count ServiceSoundCueSet increments; the handlers time their tone requests on it (Entity__MoodCue05/07/09/10/13: `out->unk4 % N`) */
    u8 pad08[0x08];
    s32 unk10;    /* +0x10, written by Entity__MoodCue15/Entity__MoodCue05/Entity__MoodCue10/... */
    s32 unk14;    /* +0x14, the divisor InitSoundCueSet sets (10); Entity__GetProximityRatio divides by it */
    u8 pad18[0x04];
    s32 unk1C;     /* +0x1C, written by Entity__MoodCue15/Entity__MoodCue05/Entity__MoodCue10/... */
    s32 unk20;      /* +0x20, written by Entity__MoodCue07 only (paired with unk1C the same round) */
    s32 unk24;       /* +0x24, written by Entity__MoodCue40 (Entity_d) */
    s32 unk28;        /* +0x28, written by Entity__MoodCue40 (Entity_d) */
    u8 pad2C[0x04];
    s32 unk30;      /* +0x30, written by Entity__MoodCue10/Entity__MoodCue07 */
    s32 unk34;       /* +0x34, written by Entity__MoodCue07 only (paired with unk30) */
    u8 pad38[0x0C];
    s32 unk44;       /* +0x44, written by Entity__MoodCue10/Entity__MoodCue07 */
    s32 unk48;        /* +0x48, written by Entity__MoodCue07 only (paired with unk44) */
    u8 pad4C[0x54 - 0x4C];  /* slot 2's word2/word3; the set is 0x54 bytes */
};

struct Entity {
    CLASS65650_FIELDS(EntityMethods);
    /* +0x098 */ s32 moodIndex;         /* New_Entity's first argument: the row of gEntityMoodTable and the byte tables */
    /* +0x09C */ EntityMoodHandlerArg soundCueSet; /* the SoundCueSet startSoundCue initialises and the MoodCue handlers fill */
    /* +0x0F0 */ s32 active;            /* activate / deactivate */
    /* +0x0F4 */ s32 targetReached;     /* setTargetReached; latched by updateTargetProximity */
    /* +0x0F8 */ s32 soundCueActive;    /* startSoundCue / stopSoundCue */
    /* +0x0FC */ s32 moodTimer;         /* zeroed by startSoundCue, counted by Entity__TickSoundCue */
    /* +0x100 */ Class6E99C *unk100;    /* made by Entity__GetOrCreateUnk100 (New_Class6E99C); released by Entity__Finalize */
    /* +0x104 */ BasicClass *unk104;    /* released by Entity__Finalize, never set in Entity code: nothing shows its class. The object is 0x108 bytes (New_Entity) */
};

/* playTod (+0x12C) is Class65650's slot and returns the flag its occupant
 * sets, but Entity's callers call it as void: Entity__MoodCue39/57 (Entity_d)
 * and Entity__MoodCue86 (Entity_f) cross-jump a playTod call with a void
 * sibling (stopTod), which GCC 2.6.3 does only when both are void (the
 * moveLocalZ case in the banner); through the s32 slot they grow 3 words
 * each. Every Entity call site casts the slot to this typedef, which emits
 * no code. */
typedef void (*EntityPlayTodFn)(Entity *self);

extern EntityMethods ENTITY_METHODS;
extern EntityMethods *Get_vtable_Entity(void); /* returns &ENTITY_METHODS */

/* The peer's view (see the banner: the player, a DreamSys). Slots are named
 * by offset, with the DREAMSYS_METHODS occupant (or the inherited slot's
 * name) and the Entity callers beside each. */
struct Unk94Methods {
    u8 pad000[0x44];
    void (*slot44)(Unk94Obj *self, s32 arg1, void *arg2);  /* updateRotation: Entity__MoodCue49 (Entity_d) */
    u8 pad048[0x94 - 0x48];
    void (*slot94)(Unk94Obj *self, s32 arg1, s32 arg2);    /* DreamSys__OnPadEvent: Entity__MoodCue123 (Entity_g), as (0, 2) and (0, 7) */
    u8 pad098[0xB8 - 0x98];
    void (*slotB8)(Unk94Obj *self, void *arg1);            /* setTranslation: Entity__MoodCue49 (Entity_d) */
    u8 pad0BC[0xC4 - 0xBC];
    void (*slotC4)(Unk94Obj *self, s32 arg1, s32 arg2);    /* moveLocalZ: Entity__MoodCue74 (Entity_e) */
    void (*slotC8)(Unk94Obj *self, s32 arg1, s32 arg2);    /* moveLocalX: Entity__MoodCue58 (Entity_d) */
    void (*slotCC)(Unk94Obj *self, s32 arg1, s32 arg2);    /* moveLocalY: Entity__MoodCue47/58 (Entity_d) */
    u8 pad0D0[0x100 - 0xD0];
    s32 (*slot100)(Unk94Obj *self);                        /* DreamSys__GetLinkCommandFlag: compared against 0 */
    u8 pad104[0x120 - 0x104];
    s32 (*slot120)(Unk94Obj *self, s32 arg1, s32 arg2, void *arg3, s32 arg4); /* DreamSys__ProjectPointAtDistance: Entity__IsNearTarget's tail call */
    u8 pad124[0x130 - 0x124];
    void (*slot130)(Unk94Obj *self, s32 arg1);             /* DreamSys__ClearTickCallbacks: (peer, 0) and (peer, 1) */
    void (*slot134)(Unk94Obj *self, s32 arg1, s32 arg2);   /* DreamSys__SetTickCallbacks: Entity__MoodCue115 (Entity_d), Entity__MoodCue74 (Entity_e) */
    u8 pad138[0x1A0 - 0x138];
    s32 (*slot1A0)(Unk94Obj *self, s32 arg1);              /* DreamSys__GetCurrentDayAndYear: Entity__MoodCue46 (Entity_d), taken mod 3 */
    u8 pad1A4[0x200 - 0x1A4];
    s32 (*slot200)(Unk94Obj *self);                        /* DreamSys__GetDreamColor: compared against 1, 5, 6, 7 */
    u8 pad204[0x21C - 0x204];
    void (*slot21C)(Unk94Obj *self);                       /* DreamSys__ResetFlashbackList: Entity__MoodCue98 (Entity_g) */
};

struct Unk94Obj {
    Unk94Methods *methods;      /* +0x00 */
    u8 pad04[0x14 - 0x04];
    Class6B5CCSub14 *coord2;    /* +0x14, Class6B5CC's coord2 */
    u8 pad18[0x5C - 0x18];
    Unk5CObj *unk5C;            /* +0x5C (Class65650's modelData offset), dereferenced through its own table by Entity__MoodCue74 (Entity_e) */
};

/* The object at the peer's +0x5C; one slot reached. */
struct Unk5CMethods {
    u8 pad000[0x64];
    void (*slot64)(Unk5CObj *self, void *arg1); /* Entity__MoodCue74 (Entity_e) as slot64(peer->unk5C, D_8008AC1C) */
};

struct Unk5CObj {
    Unk5CMethods *methods; /* +0x00 */
};

/* The object Entity__AttachToParent keeps in Actor's `grid` field (+0x04C):
 * its `parent` argument (code_4cd08 passes D_8008ABFC). Actor.h types the
 * field as the grid manager (struct Class866E8); this is the one slot Entity
 * reaches on it. Accessors cast `grid` to it. */
struct Unk4CMethods {
    u8 pad000[0x138];
    void (*slot138)(Unk4CObj *self, s32 arg1, s32 arg2); /* Entity__MoodCue12; result discarded */
};

struct Unk4CObj {
    Unk4CMethods *methods; /* +0x00 */
};

/* Default arguments Entity__GetOrCreateUnk100 substitutes when its own
 * `name`/`arg2` parameters are NULL -- both plain 2-word buffers
 * (asm/data/7B3F8.sdata.s): {0x140, 0xF0} (320, 240) and {-100, -100}, the
 * size and offset Viewport gives its Class6E99C (Class6E99C.h). */
extern s32 gEntityDefaultPos[2];
extern s32 gEntityDefaultOffset[2];

/* The mood-indexed table lookups. `gEntityMoodTable` is a real struct array (16
 * bytes/entry, `this->moodIndex` selects the row) -- Entity__UpdateActivationState reads
 * its +0x3 (signed) and Entity__UpdateDeactivationState its +0x4 (UNSIGNED) as two DIFFERENT
 * small-enum fields, not the same byte reinterpreted; both also read +0x5
 * (signed) and +0x9 (signed). `gEntityUnlockKindTable`/`gEntityLinkStageTable`/`gEntityEventVideoTable` are
 * SEPARATE global arrays (own base symbols, own `lui`/`addiu`), each also
 * 16-byte/entry and independently `this->moodIndex`-indexed -- despite the
 * base addresses' proximity, they are not sub-fields of the gEntityMoodTable row.
 * Table element types past what's listed here are `s8` (signed byte loads),
 * not `char`, despite `-funsigned-char` making plain `char` unsigned project-
 * wide -- these tables are explicitly `lb`, not `lbu`, in every user seen so
 * far (contrast `linkKind`, `gEntityUnlockKindTable`, both `lbu`/`lb`-mixed by design,
 * not by the project's usual char convention). */
struct EntityMoodRow {
    u8 pad00[0x03];
    s8 detachKind;   /* +0x03, read by Entity__UpdateActivationState */
    u8 linkKind;      /* +0x04, read by Entity__UpdateDeactivationState (unsigned load) */
    s8 unk5;           /* +0x05 */
    s8 proximityRange;  /* +0x06, read by Entity__UpdateTargetProximity only (compiler-checked, round 71): magnitude (after abs) is Entity__IsNearTarget's distance arg for raising targetReached via setTargetReached; a NEGATIVE value also makes the entity face its target every tick */
    u8 pad07[0x02];
    s8 unk9;              /* +0x09, distance-fixup byte shared by Entity__UpdateTargetProximity/Entity__UpdateSoundCueStart/Entity__UpdateSoundCueStop */
    u8 pad0A[0x01];
    s8 cueRange;            /* +0x0B, read by Entity__UpdateSoundCueStart/Entity__UpdateSoundCueStop only (compiler-checked, round 71): 0 = the cue never auto-starts; magnitude (after abs) is Entity__IsNearTarget's distance arg for starting the sound cue; a NEGATIVE value also stops it again once the target leaves that range. SEPARATE field from proximityRange (+0x06) */
    u8 pad0C[0x04];
};

extern EntityMoodRow gEntityMoodTable[];
extern s8 gEntityUnlockKindTable[];  /* GetUnlockEffect */
extern s8 D_80089EA7[];  /* read by Entity__AttachToParent, own base symbol immediately after gEntityUnlockKindTable, moodIndex*0x10-indexed like the rest of this family */
extern s8 gEntityLinkStageTable[];  /* GetLinkStage */
extern s8 gEntityEventVideoTable[];  /* GetEventVideo */
extern s8 gEntityProximityThresholdTable[];  /* read by Entity__GetProximityRatio, own base symbol immediately before D_80089EAF, moodIndex*0x10-indexed like the rest of this family */
extern s8 D_80089EAF[];  /* read by Entity__AttachToParent, own base symbol immediately after gEntityEventVideoTable, moodIndex*0x10-indexed like the rest of this family */

/* The class's own methods, in ROM order (Entity, then Entity_b). A caller
 * reaching the base ones goes through Get_vtable_Class65650() and upcasts. */
Entity *New_Entity(s32 moodIndex, void *desc, void *arg2);
Entity *Entity__Entity(Entity *self, s32 moodIndex, void *desc, void *arg2);
Class6E99C *Entity__GetOrCreateUnk100(Entity *self, void *name, void *arg2, void *arg3, s32 arg4);
void Entity__Finalize(Entity *self);
void Entity__Reset(Entity *self);
void Entity__AttachToParent(Entity *self, Class65650 *peer, void *companion, Unk4CObj *parent, void *offset);
void Entity__DetachFromParent(Entity *self);
void Entity__Update(Entity *self, void *sender, s32 event);
void Entity__NotifyLinkStage(Entity *self, void *sender, s32 event);
void Entity__OnClass86AA0LinkCommand(Entity *self, void *sender, s32 event);
void Entity__TickSoundCue(Entity *self);
s32 Entity__IsNearTarget(Entity *self, void *pos, s32 arg2, s32 arg3);
s32 Entity__DistanceToPeer(Entity *self, Class65650 *peer);
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
void Entity__MoodCue51(Entity *self, EntityMoodHandlerArg *out);  /* Entity_d; called by Entity__MoodCue113 (Entity_g) */
void Entity__MoodCue71(Entity *self, EntityMoodHandlerArg *out);  /* Entity_e; called by Entity__MoodCue108 (Entity_g) */
void Entity__StepYawInWindowsThenDeactivate(Entity *self, EntityMoodHandlerArg *out, s32 arg2, s32 arg3, s32 arg4); /* Entity_g; called by Entity_d */

/* Functions of other units Entity calls directly. The SoundCueSet functions
 * are defined in code_179d8_e/l as (VabStreamObj *, SoundCueSet *); these
 * declarations take Class65650's untyped `arg2` and Entity's `soundCueSet`
 * word, and their results are unused. */
extern void *BMemPMgrAlloc(s32 size);
extern void BMemPMgrFree(void *arg);
extern void ServiceSoundCueSet(void *sound, void *set);
extern void FlushSoundCueSet(void *sound, void *set);
extern s32 rand(void);

#endif
