#ifndef DREAMSYS_H
#define DREAMSYS_H

/*
 * DreamSys -- the dream in progress (class id 0x1F34, method table
 * gDreamSysMethods, getter GetDreamSysMethods): an Actor subclass
 * (include/Actor.h); no class derives from it. The ctor calls Actor's first
 * (DreamSys__DreamSys: GetActorMethods()->ctor), so the id parent is the
 * ctor-chain parent. Every method is in src/world/DreamSys.c. One instance, made
 * by GameApplication__GameApplication (src/app/GameApplicationFileResource.c, New_DreamSys) and kept in
 * GameApplication::dreamSys; the same object is GraphRoom::dreamSys, the
 * `target` ObjMStyleActor hands SetDreamAuxWorld (DreamAux's
 * sDreamAuxWorld), and the `peer` every Entity links to.
 *
 * It owns the dream clock (SceneNode's `tick`, advanced by
 * DreamSys__TimerTick, the update (+0x098) override, against
 * dreamTimeLimit), the player's movement (the pad handler OnPadEvent,
 * +0x094, sets move/turn/look commands that the tick callbacks installed by
 * SelectLookCallback/SelectMoveCallback consume), the mood graph (two MoodGraphContributor
 * accumulators averaged into a DreamColors value), flashback recording and
 * playback, and the link state machine that ends one stage and starts the
 * next (static wall links, dynamic and instance links, tunnels, staircases,
 * instant teleporters). Actor's `state` (+0x044) holds the pending link
 * type ExecuteLink writes.
 *
 * Link commands. The inherited dispatchers are overridden to add one case
 * each: dispatchLinkCommand (+0x09C, DispatchChunkChange) calls
 * processChunkChange for a grid (0x114) sender, onActorLinkCommand (+0x0DC,
 * DispatchInstanceEffect) calls instanceEffectsOnJournal for an Entity
 * (0x1F234) sender, and onGridCellLinkCommand (+0x0E0, WallLink) tries a
 * wall link on event 4.
 *
 * Two cross-unit identifications carry the names below: updateRotation
 * (+0x044) is the rotation setter, so every constant passed to it is three
 * degree ratios (RotationRatios below); and soundObj is a VabStreamObj,
 * whose +0x080/+0x084/+0x09C (playTone/stopVoice/setPitchOffset) name
 * voiceSelect and voiceIndex.
 *
 * The object is 0x928 bytes (New_DreamSys); Actor's fields end at +0x058.
 * The ctor returns whatever its last call, reset, leaves in $v0
 * (DreamSysResetRetFn); New_DreamSys ignores it.
 */

#include "common.h"
#include "Actor.h"
/* For StageChunk / GetMoodFromStageChunk, used by DreamSys__LogChunkMood. */
#include "StageGrid.h"
#include "SoundCueSet.h"

typedef struct DreamSys DreamSys;
typedef struct DreamSysMethods DreamSysMethods;

/* DreamSys's class id (gDreamSysMethods word +0x000). Four nibbles, so
 * `(header & 0xFFFF) == DREAMSYS_CLASS_ID` tests for it or a class below it
 * (ObjM__OnNotify). */
#define DREAMSYS_CLASS_ID 0x1F34

/* The codes a DreamSys sends its parents through notifyParents (ObjM__OnDreamSysNotify
 * switches on them). Each link code is also what ExecuteLink leaves in
 * Actor::state while that link is pending; DREAMSYS_NO_LINK is the idle
 * state every link test requires. */
enum DreamSysLinkCode {
    DREAMSYS_NO_LINK = 0,
    DREAMSYS_TIME_UP = 10,          /* TimerTick: tick reached dreamTimeLimit */
    DREAMSYS_LINK_DAY_START = 11,   /* InitSpawnLoc: the day's first spawn */
    DREAMSYS_LINK_DYNAMIC = 12,     /* DynamicLink: a random spawn */
    DREAMSYS_LINK_WALL = 13,        /* StaticWallLink */
    DREAMSYS_LINK_FLASHBACK = 14,   /* LoadNextFlashback */
    DREAMSYS_LINK_TUNNEL = 15,      /* TryTunnelLink */
    DREAMSYS_LINK_STAGE_TIMER = 16, /* TryStageTimerLink */
    DREAMSYS_LINK_TELEPORT = 17     /* TryInstantTeleportLink */
};

/* DreamSys::moveCommand, set by the pad handler and consumed by
 * AdvanceMoveCycle / ApplyMoveCommand. sMoveCommandSigns and
 * sMoveCommandDispatch make 1/2 a +/- step along the local z axis
 * (MoveLocalZOrFindLink) and 3/4 a -/+ step along local x
 * (MoveLocalXOrFindLink); FlipMoveCommand swaps each pair. Forced and
 * staircase movement always use MOVE_COMMAND_FORWARD. */
enum DreamSysMoveCommand {
    MOVE_COMMAND_NONE = 0,
    MOVE_COMMAND_FORWARD = 1,
    MOVE_COMMAND_BACK = 2,
    MOVE_COMMAND_LEFT = 3,
    MOVE_COMMAND_RIGHT = 4
};

/* DreamSys::moveMode indexes sMoveModeSpeeds {0, 24, 64, 128, 384}. The
 * pad handler switches to MOVE_MODE_RUN only while moving forward, and a
 * staircase walk takes about a seventh of the frames in it. */
#define MOVE_MODE_RUN 4

/* DreamSys::lookCallbackMode: what SelectLookCallback installs in lookCallback. */
enum DreamSysLookCallback {
    LOOK_CALLBACK_NONE = 0,
    LOOK_CALLBACK_STEP_LOOK = 1, /* stepLook */
    LOOK_CALLBACK_SLOT14C = 2,   /* an empty slot */
    LOOK_CALLBACK_SLOT150 = 3    /* an empty slot */
};

/* DreamSys::moveCallbackMode: what SelectMoveCallback installs in moveCallback. */
enum DreamSysMoveCallback {
    MOVE_CALLBACK_NONE = 0,
    MOVE_CALLBACK_TICK_MOVE = 1, /* tickMove */
    MOVE_CALLBACK_TICK_DRIFT = 2 /* tickDrift, with the sound cue set running */
};

/* DreamSys::moveOverride, setMoveOverride's value: which movement tickMove
 * runs (DreamSys__TickMove). */
enum DreamSysMoveOverride {
    MOVE_OVERRIDE_NONE = 0,   /* applyPendingTurn, then tickMoveFree */
    MOVE_OVERRIDE_FORCED = 1, /* tickMoveForced */
    MOVE_OVERRIDE_HELD = 2    /* tickMoveHeld */
};

/* The dream clock counts DreamSys::tick 15 times per unit of
 * dreamTimeLimit's public value (GetSetDreamTimeLimit scales both ways). The
 * unit is seconds: every sStageTimeLimits entry is a whole number of
 * minutes (240, 180, 480, 420, ...). */
#define DREAM_TICKS_PER_SECOND 15

/* DreamSys::moodPreviousDays holds one mood per day; AdvanceDay wraps the
 * day into the next year here. */
#define DAYS_PER_YEAR 365

/* The navigation challenges DreamSys::navChallengesArray records. */
#define NAV_CHALLENGE_COUNT 30

/* .sbss values. The unit's sbss data segment defines them, so they are
   declared here, not defined. */
extern s8 (*gpNavChallengesComplete)[NAV_CHALLENGE_COUNT];
extern s32 *gpDinamicLinkPenalty;

/* A single {numerator, denominator} degree ratio. SceneNode__UpdateRotation
   (vtable slot +0x044, the inherited rotation setter) reads three of these
   from its `data` argument, one per axis, converts each with
   RatioToFixed12 and divides by 360, then either STORES them into the
   object's rotation vector (flag != 0) or ADDS them modulo a full turn
   (flag == 0). Every constant this unit hands that slot is three of these:
   see sRotationYaw180 / _PLUS45 / _MINUS45 below and sCardinalRotations
   (src/world/DreamSys.c). */
typedef struct RotationRatio {
    s16 numerator;
    s16 denominator;
} RotationRatio;

/* The x/y/z triple SceneNode__UpdateRotation actually consumes. */
typedef struct RotationRatios {
    RotationRatio x;
    RotationRatio y;
    RotationRatio z;
} RotationRatios;

typedef struct CinematicCall {
    s16 bank;
    s16 entry;
} CinematicCall;

typedef struct {
    MoodGraphPoint lastMood;

    /* 2 bytes unused */
    struct sumAxis {
        s32 dynamic;
        s32 upper;
    } sumMoods;

    s32 amountMoods;
} MoodGraphContributor;

typedef struct PlayerSpawnPoint {
    struct MapChunk {
        u8 col;
        u8 row;
    } chunk;

    struct MapTile {
        u8 col;
        u8 row;
    } tile;

    struct RelativePos {
        s16 x;
        s16 y;
        s16 z;
    } position;
} PlayerSpawnPoint;

/* The `chunk`+`tile` half of a PlayerSpawnPoint (4 bytes) as one struct.
   DreamSys__TryStaircaseLink copies a PlayerSpawnPoint in two halves, into
   DreamSys::staircaseGridPos (this type) and DreamSys::staircaseOrigin (the
   `position` half).
   MATCHING: two whole-struct copies (retail's two lwl/lwr + swl/swr groups), not one 10-byte copy. */
typedef struct PlayerSpawnGridPos {
    struct MapChunk chunk;
    struct MapTile tile;
} PlayerSpawnGridPos;

/* pitch/heading/roll as one 12-byte struct, which DreamSys__AddFlashback
   copies from its `angles` argument.
   MATCHING: one whole-struct assignment (all six lwl/lwr loads before the six swl/swr stores). */
typedef struct FlashbackRotation {
    struct Angle {
        s16 angle;
        s16 one;
    } pitch; /* Does not do what you think it does */
    struct Angle heading;
    struct Angle roll; /* Ditto */
} FlashbackRotation;

typedef struct {
    s32 stageID;
    PlayerSpawnPoint position;
    FlashbackRotation rotation;
    s16 timeLimit;
    /* s16: DreamSys__AddFlashback stores it with `sh`. Two bytes of
       alignment padding follow, before `day`. */
    s16 unknown_value_0x1c;
    s32 day;
} FlashbackEntry;

/* DreamSys::viewport is a Viewport (include/Viewport.h; tag only here,
   DreamSys.c includes the header). This class moves its GsRVIEW2 refView:
   +0x014 vp and +0x020 vr (the two "points"
   ProjectPointAtDistance interpolates between), +0x018 vp.y and +0x024
   vr.y (AdvanceMoveCycle's view bob moves both; StepLookOffset, StopDrift
   and TickDrift move vr.y, i.e. look up and down). DayTask__Init installs
   a New_NodeGuardedViewport through setViewport, and Entity__MoodCue74 calls its
   setClearColor (+0x064). */
struct Viewport;

/* DreamSys::soundObj is a VabStreamObj (include/VabStreamObj.h): DreamSys.c
   casts it there. StartVoice / ExecuteLink call playTone (+0x080; the voice
   it returns goes to voiceIndex), StopVoice calls stopVoice (+0x084), and
   StartVoice calls setPitchOffset (+0x09C). */

struct LinkResource;

/* DreamSys__DreamSys's `arg1` is a LinkResource (include/LinkResource.h;
   GameApplication__GameApplication passes New_LinkResource("ETC\DREAME5.TMD")): the
   ctor keeps it in modelSource and adds its getModel(0), a TmdModel, as a
   child. */

/* Actor::grid is the grid manager, StageMap (include/StageMap.h);
   DreamSys.c includes that header and calls it directly. */

/* DreamSys's base class is Actor (include/Actor.h): DreamSys's own methods
   reach the base implementations through GetActorMethods() and upcast. */

/* DreamSys__InstanceEffectsOnJournal's view of the Entity (include/Entity.h,
   class 0x1F234) that sent it an instance effect: the four Entity slots it
   calls, named as Entity.h names them. A local view because a unit including
   both this header and Entity.h sees conflicting SoundCueSet prototypes. */
typedef struct DreamSysEntityMethods {
    u8 pad00[0x14C];
    MoodGraphPoint *(*getMoodEffect)(void *self); /* Entity +0x14C */
    s32 (*getUnlockEffect)(void *self);           /* Entity +0x150 */
    s32 (*getLinkStage)(void *self);              /* Entity +0x154 */
    s32 (*getEventVideo)(void *self);             /* Entity +0x158 */
} DreamSysEntityMethods;

typedef struct DreamSysEntityObj {
    DreamSysEntityMethods *methods;
} DreamSysEntityObj;

/* Argument shape for InterpolateKeyframeValue: two "keyframe" points, each with a
   value (+0x4) and a position/time (+0x8); +0x0 is not read.
   DreamSys__ProjectPointAtDistance calls it as
   InterpolateKeyframeValue(&viewport->refView.vp, &viewport->refView.vr, dist):
   the viewpoint and the reference point, read as {x, y = value, z = position}. */
typedef struct DreamSysInterpPoint {
    s8 unknown_values_0x0[4];
    s32 value;
    s32 position;
} DreamSysInterpPoint;

/* BasicClass-family allocator; see GameApplicationFileResource.h / TodActor.c / Entity.h /
   Pad.c for the other units that also declare it locally. */
extern void *BMemPMgrAlloc(s32 size);

/* The object. Actor's fields (include/Actor.h) run to +0x058; DreamSys's
 * own start there. New_DreamSys allocates 0x928 bytes. */
struct DreamSys {
    ACTOR_FIELDS(DreamSysMethods);
    /* +0x058 onward: DreamSys's own */
    /* Set by DreamSys__SetSoundObj(this, value): a VabStreamObj, cast to
           one where it is called through (include/VabStreamObj.h). */
    s32 soundObj;
    /* The camera: set by DreamSys__SetViewport (and the ctor's arg3); see
           `struct Viewport` above for the refView fields this class moves. */
    struct Viewport *viewport;
    /* Set by the ctor to its `arg1`: the LinkResource its model 0 came
       from. Not read elsewhere. */
    struct LinkResource *modelSource;
    /* The TimImage DayTask__DayTask hands over through
       DreamSys__SetEtcTim (vtable +0x114); cleared by the ctor, never
       dereferenced by the DreamSys. */
    s32 etcTim;

    bool isFlashbackSession;
    /* A DreamSysMoveOverride: which movement DreamSys__TickMove runs. */
    s32 moveOverride;

    /* Gate flag: DreamSys__BlockMovement sets it to 1; DreamSys__GetLinkCommandFlag reads it back;
           DreamSys__UpdateTickState skips its whole body while this is nonzero. */
    s32 movementBlocked;
    /* Cleared to 0, then set to (tick % tickPeriod == 0) by
           DreamSys__UpdateTickState. */
    s32 linkCommandFlag;
    /* Cleared to 0 by DreamSys__func_59598; no other observed use. */
    s32 unk_0x78;
    /* Cleared to 0 by DreamSys__func_59590; no other observed use. */
    s32 unk_0x7C;
    /* Set by DreamSys__SelectLookCallback(this, arg1): NULL when arg1==0, otherwise one of
           three vtable-slot function pointers selected by arg1 (1/2/3). Called
           with (this) by DreamSys__RunTickCallbacks, if non-NULL. */
    void (*lookCallback)(struct DreamSys *this);
    /* Set unconditionally to arg1 by DreamSys__SelectLookCallback(this, arg1);
       no other observed use. */
    s32 lookCallbackMode;
    /* Index into the (sLookOffsetSteps, sLookOffsetLimits) delta/threshold table pair,
       consumed and reset to 0 by DreamSys__StepLookOffset. */
    s32 lookOffsetCommand;
    /* Running accumulator nudged by lookOffsetCommand's table entry, or decayed by
       600/call towards 0 when lookOffsetCommand is 0; also propagated into
       viewport->refView.vr.y. Set by DreamSys__StepLookOffset. */
    s32 lookOffset;
    /* Index into the (sLookYawSteps, sLookYawLimits) delta/threshold table pair,
       consumed and reset to 0 by DreamSys__StepLookYaw. */
    s32 lookYawCommand;
    /* Running delta accumulator paired with lookYawCommand; see DreamSys__StepLookYaw. */
    s32 lookYaw;
    /* Set by DreamSys__SelectMoveCallback(this, arg1) as lookCallback is by
       SelectLookCallback, from a different trio of vtable slots. Called with
       (this) by DreamSys__RunTickCallbacks, if non-NULL. */
    void (*moveCallback)(struct DreamSys *this);
    /* "Mode" field read/written by DreamSys__SelectMoveCallback(this, arg1): when ==2 on
       entry, this->methods->stopDrift(this, 0) fires first; then it is set
       unconditionally to arg1. */
    s32 moveCallbackMode;
    /* (this->moveCommand ^ 1) < 1u, i.e. (moveCommand == 1), written by
       DreamSys__StepLookYaw; also toggled/incremented by DreamSys__FlipMoveCommand and forced
       to 1 by DreamSys__TickMoveForced. */
    s32 moveCommand;
    /* Index into the 12-byte-stride sTurnRotations table; consumed and reset
       to 0 by DreamSys__ApplyPendingTurn. */
    s32 turnCommand;
    /* (moveCommand == 1) as computed by DreamSys__StepLookYaw; unconditionally cleared
       to 0 by DreamSys__FlipMoveCommand on every call. */
    s32 moveCommandLatch;
    /* "Current" value; DreamSys__RestorePreviousMoveMode overwrites this with previousMoveMode.
       DreamSys__GetSetMoveMode's bounds-checked setter (vtable +0x180) writes both
       this and previousMoveMode together; DreamSys__ChangeMoveMode copies the OLD value of
       this into previousMoveMode before overwriting it, when the new value
       differs. */
    s32 moveMode;
    /* "Previous"/paired value; see moveMode. */
    s32 previousMoveMode;
    /* Attempt/beat counter incremented (and bounded to [0,4)) by
           DreamSys__AdvanceMoveCycle on every call while moveCommand is nonzero; reset to 0 once
           moveCommand goes back to 0. Compared against 3 there to pick a +-50
           nudge applied to viewport->refView's vp.y and vr.y, and against 4 (together with
       moveMode) to force moveCommand back to 0. */
    s32 moveCycleTick;
    /* Derived from `linkTarget->flags36` masked to 0x7F, or forced to
       0 (if >= 0x18) or 2 (if `state == 15` and this is still 0)
       by DreamSys__NotifyLinkAttempt's `arg1 == -1` path. Also an index:
       DreamSys__StartVoice does nothing when this is 0, else
       uses it to index sVoiceBySelect/sVoicePitchBySelect (see those externs), compares
       it against 0x16 (22) to decide whether to keep or discard
       voiceIndex's new value, and against 0xB (11) to gate two extra vtable
       calls. */
    s32 voiceSelect;
    /* Gate flag: DreamSys__StopVoice runs its body (a call through
       soundObj's stopVoice (+0x084), then resets this to -1) only while this is
       >= 0. */
    s32 voiceIndex;
    s8 unknown_values_0xC0[4];
    /* Set to 1 by DreamSys__SelectMoveCallback's arg1==2 case, alongside cueServiceActive and
       moveCallback. */
    s32 driftActive;
    /* Set to 1 by DreamSys__SelectMoveCallback's arg1==2 case, alongside driftActive. */
    s32 cueServiceActive;
    /* The sound cue drifting runs: SelectMoveCallback mode 2 starts it with
     * soundCueCallback, TickDrift services it, ClearTickCallbacks flushes it. */
    SoundCueSet soundCueSet;

    /* Divisor for DreamSys__UpdateTickState's (tick % tickPeriod) check. */
    s32 tickPeriod;
    /* Result of DreamSys__UpdateTickState's (tick % tickPeriod == 0) check. */
    s32 tickBoundary;
    /* tickBoundary/0x128/0x12C/0x130 are also bounds-checked-set as a group of
       four by DreamSys__SetGateFlags (vtable +0x18C): each is overwritten with the
       corresponding argument only when that argument is >= 0. */
    s32 unk_0x128;
    s32 unk_0x12C;
    s32 unk_0x130;

    s32 dreamTimeLimit;
    s8 unknown_values_0x138[12];

    MoodGraphContributor areaMoods;
    MoodGraphContributor entityMoods;
    s32 currentStage;
    CinematicCall nextCinematic;
    PlayerSpawnPoint linkCoordinates;
    /* 2 bytes unused */
    s32 saveMagic;
    s32 currentYear;
    s32 currentDay;
    s32 totalFlashbackUnlockScore;
    s32 navigationFlashbackUnlockScore;
    s32 instanceFlashbackUnlockScore;
    MoodGraphPoint moodPreviousDays[DAYS_PER_YEAR];
    /* 2 bytes unused */
    s32 amountFlashbacksAvailable;
    FlashbackEntry storedFlashbacks[10];

    s8 unknown_values_0x5d8[8];

    s8 navChallengesArray[NAV_CHALLENGE_COUNT];
    /* 2 bytes unused */
    s32 amountDynamicLinksDone;
    s8 unknown_values_0x604[116];

    bool screenShakeOn;
    s32 unknown_word_0x67c;
    s32 unknown_word_0x680;
    s8 unknown_values_0x684[500];

    s32 newGamePending;
    s32 currentFlashbackIndex;
    /* Set (whole word) by DreamSys__TryStageTimerLink to GetStageLinkAngle()'s return value,
       right before an ExecuteLink. */
    s32 stageLinkAngle;
    /* Gate flag read by DreamSys__SetMoveOverride: when nonzero
       (reusing the SAME loaded value, not a fresh 0/1 test), forwarded as
       SceneNode__UpdateRotation's arg2 -- cast from s32 to void*, not dereferenced. */
    s32 enterRotation;
    /* Zeroed (whole word) by DreamSys__TryStageTimerLink alongside enterRotation. */
    s32 exitRotation;

    s32 storedDay;

    /* The tail of New_DreamSys's 0x928-byte allocation. Its first 0x78 bytes
       are a save/restore scratch buffer for the coordinate:
       DreamSys__SaveLinkSnapshot copies *coord2 (the 0x50-byte
       GsCOORDINATE2) then *coord2->param (the 0x28-byte GsCOORD2PARAM) into
       these two fields; DreamSys__RestoreLinkSnapshot copies them back and
       then clears coord2->flg. DreamSys__ResetSessionState clears the four
       words after them; the rest of the tail has no known reader. */
    GsCOORDINATE2 coord2Snapshot;
    GsCOORD2PARAM coord2ParamSnapshot;
    s32 staircaseActive;
    /* MATCHING: u32, for DreamSys__ApplyMoveCommand's unsigned `< 1` (sltiu); both writers store 0. */
    u32 staircaseMoveGate;
    /* Called as `staircaseTickFn(this)`, its result a truth value
       (DreamSys__TryStaircaseLink); set from
       `sStaircaseTickFns[GetLastSpawnExtra()]` or NULLed, and tested
       against 0 before every call. */
    s32 (*staircaseTickFn)(struct DreamSys *this);
    /* A retry/attempt counter (DreamSys__TickStaircaseYawPlus45): read as a
       whole word, compared against several literal bands, and incremented
       by 1 at that function's normal exit. */
    s32 staircaseFrame;
    /* See PlayerSpawnGridPos's own comment -- the `chunk`+`tile` half of a
           PlayerSpawnPoint whole-struct-copied here by DreamSys__TryStaircaseLink. */
    PlayerSpawnGridPos staircaseGridPos;
    /* A `struct RelativePos`, address-taken and passed to DreamSys__ApplyRelativeOffset as
       its `b` argument (DreamSys__TickStaircaseYawPlus45). */
    struct RelativePos staircaseOrigin;
    s8 unknown_values_0x922[2];
    s32 unk_0x924;
};

/* The bytes DreamSys__GetSaveBlock hands out, from saveMagic up to
   newGamePending: 1792. */
#define DREAMSYS_SAVE_SIZE (offsetof(DreamSys, newGamePending) - offsetof(DreamSys, saveMagic))

/* What DreamSys__GetSaveBlock (+0x1B0) returns, &saveMagic, viewed from
 * there: each field is the DreamSys field above at its DreamSys offset less
 * saveMagic's +0x178. UpdateFlashbackLock (TitleMenu) reads the flashback
 * pair; GraphRoom reads the year, the day and the mood ring, and sets
 * graphScored, DreamSys +0x5DF, the last byte of unknown_values_0x5d8[8]. */
typedef struct DreamSaveBlock {
    u8 pad00[0x4];
    /* +0x004 */ s32 currentYear; /* DreamSys +0x17C; nonzero: the ring is full, plot all 100 days */
    /* +0x008 */ s32 currentDay; /* DreamSys +0x180; days logged this year, the ring's write cursor */
    /* +0x00C */ s32 totalFlashbackUnlockScore; /* DreamSys +0x184: FLASHBACK unlocks past 9999999 */
    u8 pad10[0x18 - 0x10];
    /* +0x018 */ MoodGraphPoint moodPreviousDays[DAYS_PER_YEAR]; /* walked backwards from currentDay - 1 */
    u8 pad2F2[0x2F4 - 0x2F2];
    /* +0x2F4 */ s32 amountFlashbacksAvailable; /* DreamSys +0x46C: ... and only with one stored */
    u8 pad2F8[0x467 - 0x2F8];
    /* +0x467 */ s8 graphScored; /* set once GraphRoom__ScoreDayLog's scan has succeeded */
} DreamSaveBlock;

/* Called by DreamSys__TryStaircaseLink with no argument setup; its return
   value indexes sStaircaseTickFns. Defined after its caller in
   src/world/DreamSys.c. */
extern s32 GetLastSpawnExtra(void);

typedef enum DreamColors {
    DREAM_COLOR_BLACK,
    DREAM_COLOR_BLUE,
    DREAM_COLOR_GREEN,
    DREAM_COLOR_CYAN,
    DREAM_COLOR_RED,
    DREAM_COLOR_PINK,
    DREAM_COLOR_YELLOW,
    DREAM_COLOR_WHITE,
} DreamColors;

/* Occupants in gDreamSysMethods named at each slot (`tools/classtable.py
 * gDreamSysMethods --vs gActorMethods`). The inherited slots keep Actor's
 * names and types; this class overrides +0x008 (DreamSys__DreamSys), +0x040
 * reset (DreamSys__ResetSessionState), +0x04C attachToParent
 * (DreamSys__SpawnAtLink), +0x050 detachFromParent
 * (DreamSys__DetachFromParent), +0x088 notifyIfUnk20Active
 * (DreamSys__NotifyLinkAttempt), +0x094 onPadEvent (DreamSys__OnPadEvent),
 * +0x098 update (DreamSys__TimerTick), +0x09C dispatchLinkCommand
 * (DreamSys__DispatchChunkChange), +0x0DC onActorLinkCommand
 * (DreamSys__DispatchInstanceEffect), +0x0E0 onGridCellLinkCommand
 * (DreamSys__WallLink) and +0x0E8 slotE8 (DreamSys__NoOpSlotE8Default).
 *
 * Two inherited slots are called with a type other than their own, each
 * through a function-pointer cast (no code), as TodActor.h's banner does:
 *  - reset (+0x040): DreamSys__DreamSys returns what the call leaves in $v0
 *    (`return this->methods->reset(this)`, a tail position retail keeps), so
 *    it calls through DreamSysResetRetFn below.
 *  - attachToParent (+0x04C): DreamSys__SpawnAtLink takes (self, parent),
 *    no offset; ObjM__SetupSceneStyle calls it through
 *    DreamSysAttachToParentFn. */
struct DreamSysMethods {
    ACTOR_SLOTS(DreamSys, (DreamSys * self, struct LinkResource *arg1, s32 arg2, s32 arg3));
    /* +0x0F0 */ s32 (*getSetFlashbackSession)(DreamSys *self, DreamColors *out,
                                               s32 value); /* DreamSys__GetSetFlashbackSession: value < 0 writes the day's colour to *out */
    /* +0x0F4 */ void (*setMoveOverride)(DreamSys *self, s32 value); /* DreamSys__SetMoveOverride */
    /* +0x0F8 */ void (*resetLinkState)(DreamSys *self, s32 moveMode, s32 tickPeriod); /* DreamSys__ResetLinkState */
    /* +0x0FC */ void (*blockMovement)(DreamSys *self); /* DreamSys__BlockMovement: movementBlocked = 1 */
    /* +0x100 */ s32 (*getLinkCommandFlag)(DreamSys *self); /* DreamSys__GetLinkCommandFlag */
    /* +0x104 */ s32 (*getSetDreamTimeLimit)(DreamSys *self, s32 time); /* DreamSys__GetSetDreamTimeLimit */
    /* +0x108 */ s32 (*getDreamTimerScaled)(DreamSys *self); /* DreamSys__GetDreamTimerScaled: tick / 15 */
    /* +0x10C */ void (*setSoundObj)(DreamSys *self, s32 value); /* DreamSys__SetSoundObj */
    /* +0x110 */ void (*setViewport)(DreamSys *self, struct Viewport *value); /* DreamSys__SetViewport */
    /* +0x114 */ void (*setEtcTim)(DreamSys *self, s32 value); /* DreamSys__SetEtcTim: etcTim = value */
    /* +0x118 */ void (*updateTickState)(DreamSys *self);      /* DreamSys__UpdateTickState */
    /* +0x11C */ void (*runTickCallbacks)(DreamSys *self);     /* DreamSys__RunTickCallbacks */
    /* +0x120 */ s32 (*projectPointAtDistance)(DreamSys *self, s32 *out, s32 dist, s32 *reference,
                                               s32 tolerance); /* DreamSys__ProjectPointAtDistance */
    /* +0x124 */ void (*slot124)(DreamSys *self); /* DreamSys__func_59590: unk_0x7C = 0 */
    /* +0x128 */ void (*slot128)(DreamSys *self); /* DreamSys__func_59598: unk_0x78 = 0 */
    /* +0x12C */ s32 (*slot12C)(DreamSys *self);  /* DreamSys__NoOpSlot12C */
    /* +0x130 */ void (*clearTickCallbacks)(DreamSys *self, bool arg1); /* DreamSys__ClearTickCallbacks */
    /* +0x134 */ void (*setTickCallbacks)(DreamSys *self, s32 arg1,
                                          s32 arg2); /* DreamSys__SetTickCallbacks: selectMoveCallback(arg1), selectLookCallback(arg2) */
    /* +0x138 */ void (*selectLookCallback)(DreamSys *self, s32 arg1); /* DreamSys__SelectLookCallback */
    /* +0x13C */ void (*selectMoveCallback)(DreamSys *self, s32 arg1); /* DreamSys__SelectMoveCallback */
    /* +0x140 */ void (*stepLook)(DreamSys *self);                     /* DreamSys__StepLook */
    /* +0x144 */ void (*stepLookOffset)(DreamSys *self); /* DreamSys__StepLookOffset */
    /* +0x148 */ void (*stepLookYaw)(DreamSys *self);    /* DreamSys__StepLookYaw */
    /* +0x14C */ void (*slot14C)(DreamSys *self); /* DreamSys__NoOpSlot14C, empty; a lookCallback choice */
    /* +0x150 */ void (*slot150)(DreamSys *self); /* DreamSys__NoOpSlot150, empty; a lookCallback choice */
    /* +0x154 */ s32 (*tickMove)(DreamSys *self);                   /* DreamSys__TickMove */
    /* +0x158 */ s32 (*tickMoveFree)(DreamSys *self);               /* DreamSys__TickMoveFree */
    /* +0x15C */ s32 (*tickMoveForced)(DreamSys *self);             /* DreamSys__TickMoveForced */
    /* +0x160 */ s32 (*tickMoveHeld)(DreamSys *self);               /* DreamSys__TickMoveHeld */
    /* +0x164 */ s32 (*advanceMoveCycle)(DreamSys *self, s32 arg1); /* DreamSys__AdvanceMoveCycle */
    /* +0x168 */ void (*startVoice)(DreamSys *self);                /* DreamSys__StartVoice */
    /* +0x16C */ void (*stopVoice)(DreamSys *self);                 /* DreamSys__StopVoice */
    /* +0x170 */ s32 (*applyMoveCommand)(DreamSys *self, s32 arg1); /* DreamSys__ApplyMoveCommand */
    /* +0x174 */ void (*applyPendingTurn)(DreamSys *self);          /* DreamSys__ApplyPendingTurn */
    /* +0x178 */ void (*tickDrift)(DreamSys *self);                 /* DreamSys__TickDrift */
    /* +0x17C */ void (*stopDrift)(DreamSys *self, s32 arg1);       /* DreamSys__StopDrift */
    /* +0x180 */ s32 (*getSetMoveMode)(DreamSys *self, s32 value);  /* DreamSys__GetSetMoveMode */
    /* +0x184 */ void (*changeMoveMode)(DreamSys *self, s32 value); /* DreamSys__ChangeMoveMode */
    /* +0x188 */ void (*restorePreviousMoveMode)(DreamSys *self); /* DreamSys__RestorePreviousMoveMode */
    /* +0x18C */ void (*setGateFlags)(DreamSys *self, s32 a, s32 b, s32 c, s32 d); /* DreamSys__SetGateFlags */
    /* +0x190 */ void (*setTickPeriod)(DreamSys *self, s32 value); /* DreamSys__SetTickPeriod */
    /* +0x194 */ void (*soundCueCallback)(void *owner, SoundCueSet *set); /* DreamSys__SoundCueCallback: only its VALUE is read (InitSoundCueSet's callback) */
    /* +0x198 */ void (*initNewGame)(DreamSys *self);                    /* DreamSys__InitNewGame */
    /* +0x19C */ void (*getSetScreenShake)(DreamSys *self, bool *value); /* DreamSys__GetSetScreenShake */
    /* +0x1A0 */ s32 (*getCurrentDayAndYear)(DreamSys *self, s32 *outYear); /* DreamSys__GetCurrentDayAndYear */
    /* +0x1A4 */ s32 (*advanceDay)(DreamSys *self);        /* DreamSys__AdvanceDay */
    /* +0x1A8 */ void (*clearNewGameFlag)(DreamSys *self); /* DreamSys__ClearNewGameFlag */
    /* +0x1AC */ s32 (*getNewGameFlag)(DreamSys *self);    /* DreamSys__GetNewGameFlag */
    /* +0x1B0 */ s32 *(*getSaveBlock)(DreamSys *self, s32 *outSize); /* DreamSys__GetSaveBlock: &saveMagic; *outSize = 0x700 */
    /* +0x1B4 */ s32 (*startDay)(DreamSys *self);               /* DreamSys__StartDay */
    /* +0x1B8 */ s32 (*endDay)(DreamSys *self, s32 arg1);       /* DreamSys__EndDay */
    /* +0x1BC */ CinematicCall (*getCinematic)(DreamSys *self); /* DreamSys__GetCinematic */
    /* +0x1C0 */ void (*initSpawnLoc)(DreamSys *self);          /* DreamSys__InitSpawnLoc */
    /* +0x1C4 */ void (*dynamicLink)(DreamSys *self);           /* DreamSys__DynamicLink */
    /* +0x1C8 */ bool (*staticWallLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /* DreamSys__StaticWallLink */
    /* +0x1CC */ bool (*loadNextFlashback)(DreamSys *self, bool unknown); /* DreamSys__LoadNextFlashback */
    /* +0x1D0 */ bool (*tryTunnelLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /* DreamSys__TryTunnelLink */
    /* +0x1D4 */ bool (*tryStageTimerLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /* DreamSys__TryStageTimerLink */
    /* +0x1D8 */ bool (*tryInstantTeleportLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /* DreamSys__TryInstantTeleportLink */
    /* +0x1DC */ bool (*tryStaircaseLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /* DreamSys__TryStaircaseLink */
    /* +0x1E0 */ s32 (*getCurrentStage)(DreamSys *self); /* DreamSys__GetCurrentStage */
    /* +0x1E4 */ void (*processChunkChange)(DreamSys *self, void *entity,
                                            s32 effect); /* DreamSys__ProcessChunkChange: dispatchLinkCommand's grid (0x114) case */
    /* +0x1E8 */ void (*instanceEffectsOnJournal)(DreamSys *self, void *entity,
                                                  s32 effect); /* DreamSys__InstanceEffectsOnJournal: onActorLinkCommand's Entity (0x1F234) case */
    /* +0x1EC */ void (*getPreviousDayMood)(DreamSys *self, MoodGraphPoint *target,
                                            bool unknown); /* DreamSys__GetPreviousDayMood */
    /* +0x1F0 */ void (*initMoodContributors)(DreamSys *self, MoodGraphPoint *special); /* DreamSys__InitMoodContributors */
    /* +0x1F4 */ void (*logChunkMood)(DreamSys *self, PlayerSpawnPoint *currentPos); /* DreamSys__LogChunkMood */
    /* +0x1F8 */ void (*logInstanceMood)(DreamSys *self, MoodGraphPoint *source); /* DreamSys__LogInstanceMood */
    /* +0x1FC */ void (*updateDreamChart)(DreamSys *self, MoodGraphPoint *ret); /* DreamSys__UpdateDreamChart */
    /* +0x200 */ DreamColors (*getDreamColor)(DreamSys *self); /* DreamSys__GetDreamColor */
    /* +0x204 */ void (*clearMoodGraph)(DreamSys *self, MoodGraphContributor *contributor); /* DreamSys__ClearMoodGraph */
    /* +0x208 */ void (*logMood)(DreamSys *self, MoodGraphContributor *layer,
                                 MoodGraphPoint *mood); /* DreamSys__LogMood */
    /* +0x20C */ void (*getMoodAverage)(DreamSys *self, MoodGraphContributor *layer,
                                        MoodGraphPoint *ret); /* DreamSys__GetMoodAverage */
    /* +0x210 */ void (*calcUnlockScore)(DreamSys *self);     /* DreamSys__CalcUnlockScore */
    /* +0x214 */ void (*addFlashback)(DreamSys *self, s32 stage, PlayerSpawnPoint *pos, s32 *angles,
                                      s32 unknown, s32 time, s32 day); /* DreamSys__AddFlashback */
    /* +0x218 */ void (*flashbackSaving)(DreamSys *self, s32 arg1, s32 arg2); /* DreamSys__FlashbackSaving */
    /* +0x21C */ void (*resetFlashbackList)(DreamSys *self); /* DreamSys__ResetFlashbackList */
    /* +0x220 */ void (*saveLinkSnapshot)(DreamSys *self); /* DreamSys__SaveLinkSnapshot: coord2 and its param into the snapshot fields */
    /* +0x224 */ void (*restoreLinkSnapshot)(DreamSys *self); /* DreamSys__RestoreLinkSnapshot */
    /* +0x228 */ s32 (*slot228)(DreamSys *self, s32 value); /* DreamSys__func_5ba20: get/set; GameApplication__GameApplication calls it */
}; /* 139 slots, 0x22C bytes */

/* reset (+0x040) as DreamSys__DreamSys calls it (see above). */
typedef DreamSys *(*DreamSysResetRetFn)(DreamSys *self);

/* attachToParent (+0x04C) as its occupant, DreamSys__SpawnAtLink, takes it:
 * (self, parent), no offset. ObjM__SetupSceneStyle (ObjMStyleActor) calls it
 * through this cast. */
typedef void (*DreamSysAttachToParentFn)(DreamSys *self, void *parent);

typedef struct StageSpawn {
    struct MapChunk chunk;
    struct MapTile tile;
    /* Indexes sSpawnPosAdjust, read with `lbu` (GenerateInitialSpawn). */
    u8 adjustment;
    s8 extra;
} StageSpawn;

typedef struct StaticLinkTrigger {
    struct MapChunk chunk;

    union TriggerTile {
        struct MapTile axis;
        s16 value;
    } tile;

    s8 stage;
    s8 spawnpointIndex;
} StaticLinkTrigger;

/* Jumptable holding all of DreamSys "virtual" methods */
extern DreamSysMethods gDreamSysMethods;

/* Also declared in Entity.h for the same libc-style function. */
extern s32 rand(void);

/* Shared by TestForStaticLink/TestForTunnelLinks/TestForStaircaseNodes/
   TestForInstantTeleporters, each of which forwards its own three args
   straight through and appends a fixed trailing quadruple (length table,
   trigger table, spawn table, literal 1); every call site tests the result
   with `bltz`. Defined after those callers in src/world/DreamSys.c. */
extern s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                          s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag);

/* Called by DreamSys__TryStageTimerLink as (&this->linkCoordinates, this->currentStage,
   currentPos, this->tick); result tested with `bltz`, like
   TestForStaticLink's. Defined after its caller. */
extern s32 TestForStageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos,
                                  s32 timer);

/* Called by DreamSys__TryStageTimerLink with no arguments; its return value
   is stored whole into this->stageLinkAngle. Defined after its caller. */
extern s32 GetStageLinkAngle(void);

/* SceneNode__GetRotationDegrees (DreamSys__TryTunnelLink fills its 0x10-byte
   `local` with it): include/SceneNode.h. */

/* Called by DreamSys__TryTunnelLink as (&this->exitRotation, &this->enterRotation, &local) --
   the `local` buffer SceneNode__GetRotationDegrees fills; result a truth
   value (`beqz`). Defined after its caller. */
extern s32 DreamSys__CheckTunnelHeading(s32 *arg0, s32 *arg1, void *arg2);

/* Called by DreamSys__TryStaircaseLink as (&this->linkCoordinates,
   currentPos, this->currentStage) -- same forwarding shape as
   TestForTunnelLinks/TestForStaticLink above. Defined after its caller. */
extern s32 TestForStaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 arg2);

/* Called by DreamSys__TryStaircaseLink as (&this->exitRotation, &this->enterRotation, &local) --
   identical call shape to DreamSys__CheckTunnelHeading above (same `local` buffer, same two
   `this` fields), so the same signature. Defined after its caller. */
extern s32 DreamSys__CheckStaircaseHeading(s32 *arg0, s32 *arg1, void *arg2);

/* Same (target, currentPos, stage) forwarding shape as TestForTunnelLinks
   above -- called by DreamSys__TryInstantTeleportLink as
   (&this->linkCoordinates, currentPos, this->currentStage), result tested
   with `bltz`. Defined after its caller. */
extern s32 TestForInstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);

/* Called by DreamSys__TryInstantTeleportLink with no arguments, like
   GetStageLinkAngle; its return value goes straight into ExecuteLink's
   stage-type argument. Defined after its caller. */
extern s32 GetTeleportTimeBonus(void);

/* This function might be called when the player hits a wall?
It tries to do an static link first, then a dynamic one */
void DreamSys__WallLink(DreamSys *this, void *unk_class_86aa0, int arg2);

/* @brief Sets the overall time limit for the dream and returns its previous value. */
/* @param value The new time limit, in seconds. Negative values are stored as-is. */
/* @return The previous time limit, in seconds, or -1. */
s32 DreamSys__GetSetDreamTimeLimit(DreamSys *this, s32 value);

/* @brief (Re)initializes playthrough-relevant data, like day number, flashbacks, etc. */
void DreamSys__InitNewGame(DreamSys *this);

/* @brief Sets whether the camera should shake when the player walks. */
/* @param value If True, the screen will shake when the player walks. */
/* @return To &value, the previous value of ScreenShakeOn */
void DreamSys__GetSetScreenShake(DreamSys *this, bool *value);

/* @brief Moves the currentDay counter foward one day, looping over on new years. */
/* @return Integer between 0 and 364, of the new currentDay value. */
s32 DreamSys__AdvanceDay(DreamSys *this);

/* @brief Checks what kind of dream comes next, and executes the appropriate start-of-dream actions. */
/* @return ID of the Stage to spawn on. Or -1 if the dream is Special (i.e. non-interactive). */
s32 DreamSys__StartDay(DreamSys *this);

/* @brief Executes various end-of-dream actions. */
s32 DreamSys__EndDay(DreamSys *this, s32 arg1);

/* @brief Gets the indices of the Cinematic to be played next, if any. */
/* @return CinematicCall with the currently stored indices. An Entry value of -1 means no cinematic. */
CinematicCall DreamSys__GetCinematic(DreamSys *this);

/* @brief Sets the next spawnpoint to be the initial spawn appropriate for the last day's graph. */
void DreamSys__InitSpawnLoc(DreamSys *this);

/* @brief Handles either dynamic or instance links, based on the value of DreamSys.currentStage */
void DreamSys__DynamicLink(DreamSys *this);

/* @brief Test whether a given position in the current stage is a static wall link */
/* @param currentPos The player's current position on the stage */
/* @return True if a valid link was found, False otherwise */
bool DreamSys__StaticWallLink(DreamSys *this, PlayerSpawnPoint *currentPos);

/* @brief Loads the next flashback on a flashback session */
/* @return False if it is the end of the flashback session, True otherwise */
bool DreamSys__LoadNextFlashback(DreamSys *this, bool unknown);

/* Called during some links, but no idea what it actually does */
bool ExecuteLink(DreamSys *system, s32 stage, s32 unk1, s32 unk2);

/* DreamSys__ProcessChunkChange(DreamSys *this,); */

/* @brief Processes the instance actions that directly affect this class, like instance linking and flashback logging. */
/* @param entity Pointer to the instance? */
/* @param effect Index of the effect to handle. */
void DreamSys__InstanceEffectsOnJournal(DreamSys *this, void *entity, s32 effect);

void DreamSys__GetPreviousDayMood(DreamSys *this, MoodGraphPoint *target, bool unknown);

/* @brief (Re)initializes both Mood Contributors in preparation for the start of the day. */
/* @param special If not NULL, both graphs will be initialized with this point logged in. */
void DreamSys__InitMoodContributors(DreamSys *this, MoodGraphPoint *special);

/* @brief Logs the mood effect of the chunk at the given position. */
/* @param currentPos The player's current position on the stage */
void DreamSys__LogChunkMood(DreamSys *this, PlayerSpawnPoint *currentPos);

/* @brief Logs the given mood point as an instance mood. */
/* @param source Pointer to the mood to get logged. */
void DreamSys__LogInstanceMood(DreamSys *this, MoodGraphPoint *source);

/* @brief Calculates the current Overall Mood of the dream based on data from the contributors. */
/* @param ret Pointer where the final graph point will be written to. */
void DreamSys__UpdateDreamChart(DreamSys *this, MoodGraphPoint *ret);

/* @brief Gets the color value associated with the current dream mood */
/* @return Enum value of the current mood's color */
DreamColors DreamSys__GetDreamColor(DreamSys *this);

/* @brief Calculates the DreamColor for a given mood */
/* @param mood The graph point to get a color from */
/* @return Enum value of the calculated color */
DreamColors CalcDreamColor(MoodGraphPoint *mood);

/* @brief Resets all the values stored in a contributor back to zero. */
/* @param contributor MoodGraphContributor to be cleared. */
void DreamSys__ClearMoodGraph(DreamSys *this, MoodGraphContributor *contributor);

/* @brief "Logs" a given Mood Effect on the given Contributor. */
/* @param layer The contributor that will receive the mood. */
/* @param mood The mood contribution to be logged. */
void DreamSys__LogMood(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *mood);

/* @brief Calculates the average point of a given Contributor. */
/* @param layer The MoodGraphContributor to be calculated. */
/* @param ret Pointer where this contributor's average point will be written to. */
/* @return To &ret, MoodPoint between (-9,-9) and (9,9). */
void DreamSys__GetMoodAverage(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *ret);

/* @brief Turns the values of a given mood contributor axis into a usable average */
/* @param last The mood contribution that happened last, which receives a boost in the code */
/* @param sum The cumulative value from all mood contributions */
/* @param amount The amount of mood contributions acquired */
/* @return Normalized integer between -9 and 9 */
s32 CalcMoodAxis(s32 last, s32 sum, s32 amount);

/* @brief Recalculates the total progress towards unlocking the flashback feature */
void DreamSys__CalcUnlockScore(DreamSys *this);

/* @brief Saves a "Flashback Spawnpoint" into the player's flashback session. */
/* @param stage Stage index of the flashback */
/* @param pos Coordinates of the player in the stage */
/* @param angles Array of angles, used to make the player face the correct way */
/* @param unknown */
/* @param time Time limit of the flashback */
/* @param day Day number of the flashback */
void DreamSys__AddFlashback(DreamSys *this, s32 stage, PlayerSpawnPoint *pos, s32 *angles,
                            s32 unknown, s32 time, s32 day);

/* @brief Called by the Grey Man to "erase" your flashback log */
void DreamSys__ResetFlashbackList(DreamSys *this);

/* @brief Gets the jumptable of "Virtual methods" assigned to the DreamSys class. */
/* @return &gDreamSysMethods */
DreamSysMethods *GetDreamSysMethods(void);

/* @brief Allocates and constructs a DreamSys instance. */
DreamSys *New_DreamSys(struct LinkResource *arg0, s32 arg1, s32 arg2);

/* @brief Initializes the values that will be used by CalcNavigationScore. */
/* @param arrayMem Pointer to the array of challenges completed */
/* @param linkCounter Pointer to an integer counting up the dynamic/instance links */
void InitNavChallengesArray(s8 (*arrayMem)[30], s32 *linkCounter);

/* @brief Calculates a score based on amount of Navigation Challenges achieved */
/* @return Integer between 0 and 50,000,000 */
s32 CalcNavigationScore(void);

/* @brief Gets the stage, spawn point, and time limit of a given point in the graph. */
/* @param target Pointer where the spawnpoint found will be written */
/* @param timeLimit Pointer where the time limit will be written to */
/* @param mood The mood that will be used for the calculation */
/* @param day Unused? */
/* @return The stage index of the initial spawn. */
s32 GenerateInitialSpawn(PlayerSpawnPoint *target, s32 *timeLimit, MoodGraphPoint *mood, s32 day);

/* @brief Obtains a random Spawnpoint on, or away from, a given stage. */
/* @param target Pointer where the new spawn will be written to */
/* @param stg The current stage (or target stage, if negative) */
/* @return Stage the spawn belongs to */
s32 GetRandomSpawnFromStage(PlayerSpawnPoint *target, s32 stg, s32 unused);
/* This function has two modes of operation, depending on the signage of stg.
   If stg is positive or zero, it behaves as a fully dynamic link *away* from a given stage.
   If stg is negative, it behaves as a "semi-static" link *on* a given stage. (This is the kind of link normally used by instances)
   Regardless of mode, this spawn will count towards the "Dynamic link penalty" of the flashback unlock score.*/

/* @brief Checks whether a given day is Special, and loads a random cinematic if it is. */
/* @param cinematic The CinematicCall that will be written to if a match is found. */
/* @param day The day number to check against (1-indexed). */
/* @return The pointer to this dream's graph contribution, or NULL if the dream is *not* Special. */
MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day);

/* The occupants of gDreamSysMethods not declared above, in slot order. */
DreamSys *DreamSys__DreamSys(DreamSys *this, struct LinkResource *arg1, s32 arg2, s32 arg3);
void DreamSys__ResetSessionState(DreamSys *this);
void DreamSys__SpawnAtLink(DreamSys *this, struct StageMap *arg1);
void DreamSys__DetachFromParent(DreamSys *this);
void DreamSys__NotifyLinkAttempt(DreamSys *this, s32 arg1);
void DreamSys__OnPadEvent(DreamSys *this, s32 arg1, s32 mode);
void DreamSys__TimerTick(DreamSys *this, s32 arg1, s32 arg2);
void DreamSys__DispatchChunkChange(DreamSys *this, void *arg1, s32 arg2);
void DreamSys__DispatchInstanceEffect(DreamSys *this, void *arg1, s32 arg2);
void DreamSys__NoOpSlotE8Default(void);
s32 DreamSys__GetSetFlashbackSession(DreamSys *this, DreamColors *out, s32 value);
void DreamSys__SetMoveOverride(DreamSys *this, s32 value);
void DreamSys__ResetLinkState(DreamSys *this, s32 moveMode, s32 tickPeriod);
void DreamSys__BlockMovement(DreamSys *this);
s32 DreamSys__GetLinkCommandFlag(DreamSys *this);
s32 DreamSys__GetDreamTimerScaled(DreamSys *this);
void DreamSys__SetViewport(DreamSys *this, struct Viewport *value);
void DreamSys__UpdateTickState(DreamSys *this);
void DreamSys__RunTickCallbacks(DreamSys *this);
s32 DreamSys__ProjectPointAtDistance(DreamSys *this, s32 *out, s32 dist, s32 *reference, s32 tolerance);
void DreamSys__func_59590(DreamSys *this);
void DreamSys__func_59598(DreamSys *this);
s32 DreamSys__NoOpSlot12C(DreamSys *this);
void DreamSys__ClearTickCallbacks(DreamSys *this, bool arg1);
void DreamSys__SetTickCallbacks(DreamSys *this, s32 arg1, s32 arg2);
void DreamSys__StepLook(DreamSys *this);
void DreamSys__StepLookOffset(DreamSys *this);
void DreamSys__StepLookYaw(DreamSys *this);
void DreamSys__NoOpSlot14C(void);
void DreamSys__NoOpSlot150(void);
s32 DreamSys__TickMove(DreamSys *this);
s32 DreamSys__TickMoveFree(DreamSys *this);
s32 DreamSys__TickMoveForced(DreamSys *this);
s32 DreamSys__TickMoveHeld(DreamSys *this);
s32 DreamSys__AdvanceMoveCycle(DreamSys *this, s32 arg1);
void DreamSys__StartVoice(DreamSys *this);
void DreamSys__StopVoice(DreamSys *this);
s32 DreamSys__ApplyMoveCommand(DreamSys *this, s32 arg1);
void DreamSys__ApplyPendingTurn(DreamSys *this);
void DreamSys__TickDrift(DreamSys *this);
void DreamSys__StopDrift(DreamSys *this, s32 arg1);
s32 DreamSys__GetSetMoveMode(DreamSys *this, s32 value);
void DreamSys__ChangeMoveMode(DreamSys *this, s32 value);
void DreamSys__RestorePreviousMoveMode(DreamSys *this);
void DreamSys__SetGateFlags(DreamSys *this, s32 a, s32 b, s32 c, s32 d);
void DreamSys__SetTickPeriod(DreamSys *this, s32 value);
void DreamSys__SoundCueCallback(void *owner, SoundCueSet *set);
s32 DreamSys__GetCurrentDayAndYear(DreamSys *this, s32 *arg1);
void DreamSys__ClearNewGameFlag(DreamSys *this);
s32 DreamSys__GetNewGameFlag(DreamSys *this);
s32 *DreamSys__GetSaveBlock(DreamSys *this, s32 *arg1);
bool DreamSys__TryTunnelLink(DreamSys *this, PlayerSpawnPoint *currentPos);
bool DreamSys__TryStageTimerLink(DreamSys *this, PlayerSpawnPoint *currentPos);
bool DreamSys__TryInstantTeleportLink(DreamSys *this, PlayerSpawnPoint *currentPos);
bool DreamSys__TryStaircaseLink(DreamSys *this, PlayerSpawnPoint *currentPos);
s32 DreamSys__GetCurrentStage(DreamSys *this);
void DreamSys__FlashbackSaving(DreamSys *this, s32 arg1, s32 arg2);
void DreamSys__SaveLinkSnapshot(DreamSys *this);
void DreamSys__RestoreLinkSnapshot(DreamSys *this);
s32 DreamSys__func_5ba20(DreamSys *this, s32 value);

/* A non-slot helper the staircase ticks call before its definition:
   addTranslation of (a - b) with y forced to 0. */
void DreamSys__ApplyRelativeOffset(DreamSys *this, struct RelativePos *a, struct RelativePos *b);

#endif