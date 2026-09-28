#ifndef CLASS_DREAMSYS
#define CLASS_DREAMSYS

/*
 * DreamSys -- the dream in progress (class id 0x1F34, method table
 * gDreamSysMethods, getter Get_vtable_DreamSys): an Actor subclass
 * (include/Actor.h); no class derives from it. The ctor calls Actor's first
 * (DreamSys__DreamSys: GetActorMethods()->ctor), so the id parent is the
 * ctor-chain parent. Every method is in src/DreamSys.c. One instance, made
 * by GameApplication__GameApplication (src/GameApplicationFileResource.c, New_DreamSys) and kept in
 * GameApplication::dreamSys; the same object is GraphRoom::dreamSys, the
 * `target` ObjMStyleActor hands SetDreamAuxWorld (DreamAux's
 * gDreamAuxWorld), and the `peer` every Entity links to.
 *
 * It owns the dream clock (SceneNode's `tick`, advanced by
 * DreamSys__TimerTick, the update (+0x098) override, against
 * dreamTimeLimit), the player's movement (the pad handler OnPadEvent,
 * +0x094, sets move/turn/look commands that the tick callbacks installed by
 * SelectCallback80/98 consume), the mood graph (two MoodGraphContributor
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
 * Round 66's naming (per-name evidence in docs/match-reports/<func>.md)
 * rests on two cross-unit identifications: updateRotation (+0x044) is the
 * rotation setter, so every constant passed to it is three degree ratios
 * (RotationRatios below); and soundObj is a VabStreamObj, whose +0x080/
 * +0x084/+0x09C (playTone/stopVoice/setPitchOffset) name voiceSelect and
 * voiceIndex.
 *
 * The object is 0x928 bytes (New_DreamSys); Actor's fields end at +0x058.
 * The ctor returns whatever its last call, reset, leaves in $v0
 * (DreamSysResetRetFn); New_DreamSys ignores it.
 */

#include "common.h"
#include "Actor.h"
/* For StageChunk / GetMoodFromStageChunk, used by DreamSys__LogChunkMood
   (round 2026-08-30-d). */
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

/* DreamSys::lookCallbackMode: what SelectCallback80 installs in lookCallback. */
enum DreamSysLookCallback {
    LOOK_CALLBACK_NONE = 0,
    LOOK_CALLBACK_STEP_LOOK = 1, /* stepLook */
    LOOK_CALLBACK_SLOT14C = 2,   /* an empty slot */
    LOOK_CALLBACK_SLOT150 = 3    /* an empty slot */
};

/* DreamSys::moveCallbackMode: what SelectCallback98 installs in moveCallback. */
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

/* .sbss values. The 7B4B4 sbss segment that actually holds these is still
   plain `data` (un-flipped to dot-form), so it already provides these
   symbols; declaring them `extern` here lets this header be #included
   without a multiple-definition link error. Whoever flips that segment to
   `.data, DreamSys` should drop `extern` here in the same commit. */
extern s8 (*gpNavChallengesComplete)[NAV_CHALLENGE_COUNT];
extern s32 *gpDinamicLinkPenalty;
extern s32 gLinkSrcStage;
extern s32 gLinkTriggerIndex;
extern s32 gLinkDstStage;
extern s32 gLinkSpawnIndex;

/* Delta/threshold table pairs consumed by DreamSys__StepLookOffset (sLookOffsetSteps /
   sLookOffsetLimits, indexed by DreamSys::lookOffsetCommand) and DreamSys__StepLookYaw (sLookYawSteps /
   sLookYawLimits, indexed by DreamSys::lookYawCommand). Index 0 is unused/zero in both
   pairs; indices 1 and 2 are the negative/positive delta and its matching
   threshold. Still raw `nonmatching` data (round 2026-08-30). */
extern s32 sLookOffsetSteps[3];
extern s32 sLookOffsetLimits[3];
extern s32 sLookYawSteps[3];
extern s32 sLookYawLimits[3];

/* Consumed by DreamSys__ApplyMoveCommand (round 2026-09-06), both indexed by that
   function's own `arg1` (a mood/day-type selector, range implied by the
   table sizes below): `sMoveCommandSigns[arg1] * sMoveModeSpeeds[this->moveMode]` forms
   a signed delta, then `sMoveCommandDispatch[arg1]` is called with it. Index 0 is
   unused/null in sMoveCommandDispatch (arg1 == 0 returns before reaching any of
   these, per that function's own guard) -- consistent with sMoveCommandSigns[0]
   being 0 too. sMoveModeSpeeds is indexed separately by DreamSys::moveMode (its
   own "Current" value, see that field), not by arg1. */
extern s32 sMoveModeSpeeds[5];
extern s8 sMoveCommandSigns[8];

/* Declared further down (after the real `DreamSys` typedef exists) as
   `extern void (*sMoveCommandDispatch[5])(DreamSys *this, s32 val, void *extra);` --
   same element type as Actor__MoveLocalZOrFindLink/Actor__MoveLocalXOrFindLink (Actor +0x0D0/+0x0D4), which this table
   holds pointers to. */

/* A single {numerator, denominator} degree ratio. This is not a guess about
   the LAYOUT any more (round 66): SceneNode__UpdateRotation -- vtable slot +0x044, the
   inherited rotation setter, MATCHED in src/SceneNode.c -- reads exactly
   three of these from its `data` argument, one per axis, converts each with
   RatioToFixed12 and divides by 360, then either STORES them into the
   object's rotation vector (flag != 0) or ADDS them modulo a full turn
   (flag == 0). Every constant this unit hands that slot is three of these,
   and every one of them decodes to a plausible angle: see
   sRotationYaw180 / _PLUS45 / _MINUS45 and CARDINAL_ROTATIONS below. */
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

/* One 12-byte-stride RotationRatios array that splat had to split across two
   labels, because DreamSys__StepLookYaw references its SECOND word (entry
   0's yaw numerator, which it overwrites with its own per-tick delta) while
   DreamSys__ApplyPendingTurn address-takes whole entries. The round-2026-08-30-b
   note here read the 4-byte and 12-byte views as "likely two unrelated
   globals"; they are one table, and the data says so -- entry 0 is
   (0 deg, 45 deg, 0 deg) with the 45 being exactly the +-0x2D
   DreamSys__StepLookYaw writes, entry 1 is (0, -6, 0) and entry 2 is
   (0, +6, 0), which are DreamSys::turnCommand's two values 1 and 2. */
extern RotationRatio TURN_ROTATION_YAW[]; /* == &TURN_ROTATIONS[0].y */
extern RotationRatios TURN_ROTATIONS[];

/* (0 deg, 180 deg, 0 deg). Address-of only, forwarded as SceneNode__UpdateRotation's
   arg2 with flag 1 (absolute) by DreamSys__ResetSessionState. */
extern RotationRatios sRotationYaw180;

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

/* The `chunk`+`tile` half of a PlayerSpawnPoint (4 bytes), reinterpreted as
   one struct so a single whole-struct assignment reproduces retail's
   unaligned 4-byte `lwl`/`lwr` + `swl`/`swr` copy -- DreamSys__TryStaircaseLink (round
   2026-09-06) copies a `PlayerSpawnPoint *currentPos` piecewise into
   DreamSys::staircaseGridPos (this type) and DreamSys::staircaseOrigin (a plain
   `struct RelativePos`, the `position` half) rather than as one 10-byte
   copy, matching retail's own two separate unaligned-copy instruction
   groups. */
typedef struct PlayerSpawnGridPos {
    struct MapChunk chunk;
    struct MapTile tile;
} PlayerSpawnGridPos;

/* pitch/heading/roll grouped into one 12-byte nested struct (round
   2026-09-02, DreamSys__AddFlashback): that function block-copies all
   three from its `angles` argument in ONE retail load-all-then-store-all
   sequence (six unaligned lwl/lwr loads, all before any of the six
   unaligned swl/swr stores) -- the same "whole-struct assignment, not a
   per-word copy" idiom already documented for other block moves in this
   project. A per-field or looped copy would not reproduce that
   instruction ordering. */
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
    /* Was `s32`; retyped (round 2026-09-02): DreamSys__AddFlashback writes
	   it with a bare `sh` (halfword store) from an `s32` argument, which
	   only makes sense if the field itself is 2 bytes -- an `s32` field
	   fed by an `s32` argument would store all 4 bytes (`sw`), not 2. The
	   remaining 2 bytes before `day` are ordinary C struct alignment
	   padding, not a separate field. */
    s16 unknown_value_0x1c;
    s32 day;
} FlashbackEntry;


/* DreamSys::viewport is a Viewport (include/Viewport.h; tag only here,
   DreamSys.c includes the header). Named heightCurve / DreamSysUnk5C until
   track 4 (round 88); the offsets that view named are the Viewport's
   GsRVIEW2 refView: +0x014 vp and +0x020 vr (the two "points"
   ProjectPointAtDistance interpolates between), +0x018 vp.y and +0x024
   vr.y (AdvanceMoveCycle's view bob moves both; StepLookOffset, StopDrift
   and TickDrift move vr.y, i.e. look up and down). DayTask__Init installs
   a New_NodeGuardedViewport through setViewport, and Entity__MoodCue74 calls its
   setClearColor (+0x064). */
struct Viewport;

/* DreamSys::soundObj is a VabStreamObj (include/VabStreamObj.h): DreamSys.c
   casts it there. StartVoice / ExecuteLink call playTone (+0x080; the voice
   it returns goes to voiceIndex), StopVoice calls stopVoice (+0x084), and
   StartVoice calls setPitchOffset (+0x09C). This header used to carry that
   view as DreamSysUnk58 / DreamSysUnk58Vtable (deleted round 87, track 4). */

struct LinkResource;

/* DreamSys__DreamSys's `arg1` is a LinkResource (include/LinkResource.h;
   GameApplication__GameApplication passes New_LinkResource("ETC\DREAME5.TMD")): the
   ctor keeps it in modelSource and adds its getModel(0), a TmdModel, as a
   child. This header used to carry that view as DreamSysCtorArgObj /
   DreamSysCtorArgMethods (deleted round 89, track 4). */


/* Actor::grid is the grid manager, StageMap (include/StageMap.h, track 4
   round 89); DreamSys.c includes that header and calls it directly. */

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

/* DreamSys__TickDrift's per-tick addTranslation (+0x0BC) step. */
extern LongVec3 DRIFT_STEP;

/* gProjectOffsetZ is the LAST word of an unnamed 3-word (LongVec3-shaped)
   global scratch vector; the other two words are NOT independently named
   -- splat's dlabel boundary put them inside `VOICE_PITCH_BY_SELECT`'s dlabel as
   unlabeled tail bytes (asm/data/783DC.data.s), because nothing took their
   address directly until DreamSys__ProjectPointAtDistance (round 19). Do not rename/resegment
   this round (config/ out of scope); reach the vector's start with pointer
   arithmetic off this symbol instead: `(LongVec3 *)((s32 *)&gProjectOffsetZ
   - 2)`.

   Two independent pieces of evidence pin this down, not a guess:
   - `DreamSys__NotifyLinkAttempt` (this unit, already matched) clamps
     `this->voiceSelect = (this->linkTarget->flags36 & 0x7F); if (voiceSelect >=
     0x18) voiceSelect = 0;` -- i.e. `voiceSelect` is bounded to [0, 0x18). Both
     `VOICE_BY_SELECT` and `VOICE_PITCH_BY_SELECT` (each already-named 24+-byte byte
     tables) are indexed by this SAME bounded value in `DreamSys__StartVoice`
     (`VOICE_PITCH_BY_SELECT[voiceSelect]`), so `VOICE_PITCH_BY_SELECT`'s real, ever-read extent is
     exactly 24 bytes (`0x80087EC8`-`0x80087EDF`) -- the 8 trailing zero
     bytes splat lumped into its dlabel (`0x80087EE0`-`0x80087EE7`) are
     never reached by that indexed access and belong to something else.
   - `SceneNode__LocalOffsetToWorldPos` (SceneNode, already matched) forwards its own `src`
     parameter to `ApplyMatrixToLVArray(dst, src, 1, buf)`, and `ApplyMatrixToLVArray`'s
     own doc comment (include/SceneNode.h) confirms it treats both
     pointers as 0xC-byte (3-word) elements. `DreamSys__ProjectPointAtDistance` passes
     `(s32 *)&gProjectOffsetZ - 2` as that exact `src` argument, which only
     type-checks sensibly as a 3-word vector's start -- matching the 8
     "spare" bytes above exactly (2 words = 8 bytes immediately before
     `gProjectOffsetZ`, which is the vector's 3rd word). */
extern s32 gProjectOffsetZ;

/* A `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a` argument
   by DreamSys__TickStaircaseCase2 (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_2;

/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase0 -- same call shape as STAIRCASE_OFFSET_2 above, just a
   different constant (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_0;

/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase1 -- same call shape as STAIRCASE_OFFSET_0/STAIRCASE_OFFSET_2
   above, just a different constant (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_1;

/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase3 -- same call shape as STAIRCASE_OFFSET_2/STAIRCASE_OFFSET_0
   above, just a different constant (round 2026-09-02). */
extern struct RelativePos STAIRCASE_OFFSET_3;

/* (0 deg, +45 deg, 0 deg), forwarded as vtable slot +0x044's (SceneNode__UpdateRotation)
   arg2 with flag 0 (relative) by DreamSys__TickStaircaseCase0 and
   DreamSys__TickStaircaseCase2. Typed RotationRatios round 66: its three
   {numerator, denominator} words are {0,1} {0x2D,1} {0,1}, byte-identical in
   form to sRotationYaw180 and to every CARDINAL_ROTATIONS entry. */
extern RotationRatios ROTATION_YAW_PLUS45;

/* (0 deg, -45 deg, 0 deg) -- the mirror of ROTATION_YAW_PLUS45 above
   ({0,1} {0xFFD3,1} {0,1}), used the same way by
   DreamSys__TickStaircaseCase1 and DreamSys__TickStaircaseCase3. */
extern RotationRatios ROTATION_YAW_MINUS45;

/* Argument shape for InterpolateKeyframeValue: two "keyframe" points, each with a
   value (+0x4) and a position/time (+0x8); offset +0x0 unconfirmed
   (unread by this function). Called by still-INCLUDE_ASM DreamSys__ProjectPointAtDistance as
   InterpolateKeyframeValue(&viewport->refView.vp, &viewport->refView.vr, dist) --
   the viewpoint and the reference point, read as {x, y = value, z =
   position} (round 2026-08-30-d; the Viewport identification is round 88). */
typedef struct DreamSysInterpPoint {
    s8 unknown_values_0x0[4];
    s32 value;
    s32 position;
} DreamSysInterpPoint;

/* 3x3 lookup table indexed by [dynamicClass][upperClass], each axis
   classified into {0,1,2} by CalcDreamColor first (round 2026-08-30-d). */
extern s8 sDreamColorTable[9];

/* Byte tables indexed by DreamSys::voiceSelect (already bounded to [0,0x18) at
   the write site -- see that field's own comment). DreamSys__StartVoice
   (round 2026-09-06) reads both: VOICE_BY_SELECT[voiceSelect] (values 0..0x1E) feeds
   VabStreamObj playTone's `index` argument (program << 4, tone 0);
   VOICE_PITCH_BY_SELECT[voiceSelect] (values include -2..2, hence `s8` not `u8`) feeds
   setPitchOffset's `octave` argument directly. VOICE_PITCH_BY_SELECT's real extent is exactly
   these 24 bytes -- the trailing zero bytes splat lumped into its dlabel
   belong to the gProjectOffsetZ vector documented above, not to this table. */
extern const s8 VOICE_BY_SELECT[0x18];
extern const s8 VOICE_PITCH_BY_SELECT[0x18];

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
    /* Set unconditionally to the constructor's `arg1` by DreamSys__DreamSys
	   (round 2026-09-02): the LinkResource its model 0 came from. No other
	   observed use in this unit's queued functions. */
    struct LinkResource *modelSource;
    /* The TimImage DayTask__DayTask hands over through
       DreamSys__SetEtcTim (vtable +0x114); cleared by the ctor, never
       dereferenced by the DreamSys. */
    s32 etcTim;

    bool isFlashbackSession;
    /* Read by DreamSys__TickMove; compared against 0 / 1, else-branch otherwise.
	   Meaning unidentified beyond that (round 2026-08-30). */
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
    /* Set by DreamSys__SelectCallback80(this, arg1): NULL when arg1==0, otherwise one of
	   three vtable-slot function pointers selected by arg1 (1/2/3). Called
	   with (this) by DreamSys__RunTickCallbacks, if non-NULL. */
    void (*lookCallback)(struct DreamSys *this);
    /* Set unconditionally to arg1 by DreamSys__SelectCallback80(this, arg1); no other
	   observed use (round 2026-08-30). */
    s32 lookCallbackMode;
    /* Index into the (sLookOffsetSteps, sLookOffsetLimits) delta/threshold table pair,
	   consumed and reset to 0 by DreamSys__StepLookOffset (round 2026-08-30). */
    s32 lookOffsetCommand;
    /* Running accumulator nudged by lookOffsetCommand's table entry, or decayed by
	   600/call towards 0 when lookOffsetCommand is 0; also propagated into
	   viewport->refView.vr.y. Set by DreamSys__StepLookOffset (round 2026-08-30). */
    s32 lookOffset;
    /* Index into the (sLookYawSteps, sLookYawLimits) delta/threshold table pair,
	   consumed and reset to 0 by DreamSys__StepLookYaw (round 2026-08-30). */
    s32 lookYawCommand;
    /* Running delta accumulator paired with lookYawCommand; see DreamSys__StepLookYaw
	   (round 2026-08-30). */
    s32 lookYaw;
    /* Set by DreamSys__SelectCallback80(this, arg1) exactly like lookCallback, but from
	   a *different* trio of vtable slots. Called with (this) by
	   DreamSys__RunTickCallbacks, if non-NULL. */
    void (*moveCallback)(struct DreamSys *this);
    /* "Mode" field read/written by DreamSys__SelectCallback98(this, arg1): when ==2 on
	   entry, this->methods->stopDrift(this, 0) fires first; then it is set
	   unconditionally to arg1 (round 2026-08-30). */
    s32 moveCallbackMode;
    /* (this->moveCommand ^ 1) < 1u, i.e. (moveCommand == 1), written by
	   DreamSys__StepLookYaw; also toggled/incremented by DreamSys__FlipMoveCommand and forced
	   to 1 by DreamSys__TickMoveForced (round 2026-08-30). */
    s32 moveCommand;
    /* Index into the 12-byte-stride TURN_ROTATIONS table; consumed and reset
	   to 0 by DreamSys__ApplyPendingTurn (round 2026-08-30-b). */
    s32 turnCommand;
    /* (moveCommand == 1) as computed by DreamSys__StepLookYaw; unconditionally cleared
	   to 0 by DreamSys__FlipMoveCommand on every call (round 2026-08-30). */
    s32 moveCommandLatch;
    /* "Current" value; DreamSys__RestorePreviousMoveMode overwrites this with previousMoveMode.
	   DreamSys__GetSetMoveMode's bounds-checked setter (vtable +0x180) writes both
	   this and previousMoveMode together; DreamSys__ChangeMoveMode copies the OLD value of
	   this into previousMoveMode before overwriting it, when the new value
	   differs (round 2026-08-30-b). */
    s32 moveMode;
    /* "Previous"/paired value; see moveMode (round 2026-08-30-b). */
    s32 previousMoveMode;
    /* Attempt/beat counter incremented (and bounded to [0,4)) by
	   DreamSys__AdvanceMoveCycle on every call while moveCommand is nonzero; reset to 0 once
	   moveCommand goes back to 0. Compared against 3 there to pick a +-50
	   nudge applied to viewport->refView's vp.y and vr.y, and against 4 (together with
	   moveMode) to force moveCommand back to 0 (round 2026-09-02). */
    s32 moveCycleTick;
    /* Derived from `linkTarget->flags36` masked to 0x7F, or forced to
	   0 (if >= 0x18) or 2 (if `state == 15` and this is still 0)
	   by DreamSys__NotifyLinkAttempt's `arg1 == -1` path (round 2026-09-02). Also an index:
	   DreamSys__StartVoice (round 2026-09-06) does nothing when this is 0, else
	   uses it to index VOICE_BY_SELECT/VOICE_PITCH_BY_SELECT (see those externs), compares
	   it against 0x16 (22) to decide whether to keep or discard
	   voiceIndex's new value, and against 0xB (11) to gate two extra vtable
	   calls. */
    s32 voiceSelect;
    /* Gate flag: DreamSys__StopVoice runs its body (a call through
	   soundObj's stopVoice (+0x084), then resets this to -1) only while this is
	   >= 0 (round 2026-08-30-b). */
    s32 voiceIndex;
    s8 unknown_values_0xC0[4];
    /* Set to 1 by DreamSys__SelectCallback98's arg1==2 case, alongside cueServiceActive and
	   moveCallback (round 2026-08-30). */
    s32 driftActive;
    /* Set to 1 by DreamSys__SelectCallback98's arg1==2 case, alongside driftActive
	   (round 2026-08-30). */
    s32 cueServiceActive;
    /* The sound cue drifting runs: SelectCallback98 mode 2 starts it with
     * soundCueCallback, TickDrift services it, ClearTickCallbacks flushes it. */
    SoundCueSet soundCueSet;

    /* Divisor for DreamSys__UpdateTickState's (tick % tickPeriod) check. */
    s32 tickPeriod;
    /* Result of DreamSys__UpdateTickState's (tick % tickPeriod == 0) check. */
    s32 tickBoundary;
    /* tickBoundary/0x128/0x12C/0x130 are also bounds-checked-set as a group of
	   four by DreamSys__SetGateFlags (vtable +0x18C): each is overwritten with the
	   corresponding argument only when that argument is >= 0
	   (round 2026-08-30-b). */
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
    s32 totalFlasbackUnlockScore;
    s32 navigationFlasbackUnlockScore;
    s32 instanceFlasbackUnlockScore;
    MoodGraphPoint moodPreviousDays[DAYS_PER_YEAR];
    /* 2 bytes unused */
    s32 amountFlashbacksAvailable;
    FlashbackEntry storedFlasbacks[10];

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
	   right before an ExecuteLink (round 2026-09-02). */
    s32 stageLinkAngle;
    /* Gate flag read by DreamSys__SetMoveOverride (round 2026-08-30-b): when nonzero
	   (reusing the SAME loaded value, not a fresh 0/1 test), forwarded as
	   SceneNode__UpdateRotation's arg2 -- cast from s32 to void*, not dereferenced. */
    s32 enterRotation;
    /* Zeroed (whole word) by DreamSys__TryStageTimerLink alongside enterRotation
	   (round 2026-09-02). */
    s32 exitRotation;

    s32 storedDay;

    /* The struct previously ended here (0x890), but New_DreamSys allocates
	   sizeof(DreamSys) via a literal `ori $a0, $zero, 0x928` -- 0x98 bytes
	   more than any field so far discovered accounts for. Extended to the
	   allocator's real size (round 2026-08-30-b); the four words
	   DreamSys__ResetSessionState clears are named, the rest of the tail is still
	   unclaimed. The first 0x78 bytes of that tail are a save/restore
	   scratch buffer for the coordinate: DreamSys__SaveLinkSnapshot copies
	   *coord2 (the 0x50-byte GsCOORDINATE2) then *coord2->param (the
	   0x28-byte GsCOORD2PARAM) into these two fields;
	   DreamSys__RestoreLinkSnapshot copies them back and then clears
	   coord2->flg (round 2026-09-02; SceneNode's own types, track 4). */
    GsCOORDINATE2 coord2Snapshot;
    GsCOORD2PARAM coord2ParamSnapshot;
    s32 staircaseActive;
    /* Compared with an UNSIGNED `< 1` (sltiu) by DreamSys__ApplyMoveCommand (round
	   2026-09-06) -- typed `u32` rather than `s32` to reproduce that,
	   confirmed safe since its only two writers (round 2026-08-30) both
	   set it to the literal 0. */
    u32 staircaseMoveGate;
    /* Function pointer, called as `staircaseTickFn(this)` and its `s32` result
	   used as a truth value (DreamSys__TryStaircaseLink, round 2026-09-06); set from
	   `STAIRCASE_TICK_FNS[GetLastSpawnExtra()]` (both MATCHED) or NULLed --
	   0 is a valid state, tested with a plain `!= 0`/`== 0` before ever
	   being called through. */
    s32 (*staircaseTickFn)(struct DreamSys *this);
    /* A retry/attempt counter (round 2026-09-02, DreamSys__TickStaircaseCase2): read as a
	   whole word, compared against several literal bands, and incremented
	   by 1 at that function's normal exit. */
    s32 staircaseFrame;
    /* See PlayerSpawnGridPos's own comment -- the `chunk`+`tile` half of a
	   PlayerSpawnPoint whole-struct-copied here by DreamSys__TryStaircaseLink. */
    PlayerSpawnGridPos staircaseGridPos;
    /* A `struct RelativePos`, address-taken and passed to DreamSys__ApplyRelativeOffset as
	   its `b` argument (round 2026-09-02, DreamSys__TickStaircaseCase2) -- carved out of
	   what was raw padding in the same 0x10-byte block as staircaseFrame above. */
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
    /* +0x00C */ s32 totalFlasbackUnlockScore; /* DreamSys +0x184: FLASHBACK unlocks past 9999999 */
    u8 pad10[0x18 - 0x10];
    /* +0x018 */ MoodGraphPoint moodPreviousDays[DAYS_PER_YEAR]; /* walked backwards from currentDay - 1 */
    u8 pad2F2[0x2F4 - 0x2F2];
    /* +0x2F4 */ s32 amountFlashbacksAvailable; /* DreamSys +0x46C: ... and only with one stored */
    u8 pad2F8[0x467 - 0x2F8];
    /* +0x467 */ s8 graphScored; /* set once GraphRoom__ScoreDayLog's scan has succeeded */
} DreamSaveBlock;

/* Dispatch table indexed by DreamSys__ApplyMoveCommand's `arg1`; see that table's own
   comment near sMoveModeSpeeds/sMoveCommandSigns above. Same element signature as
   Actor__MoveLocalZOrFindLink/Actor__MoveLocalXOrFindLink (Actor +0x0D0/+0x0D4). */
extern void (*sMoveCommandDispatch[5])(DreamSys *this, s32 val, void *extra);

/* 4-entry table of `s32 (DreamSys *this)` functions (DreamSys__TickStaircaseCase0,
   DreamSys__TickStaircaseCase1, DreamSys__TickStaircaseCase2, DreamSys__TickStaircaseCase3, all already matched with
   exactly that signature), indexed by GetLastSpawnExtra()'s return value and
   stashed into DreamSys::staircaseTickFn by DreamSys__TryStaircaseLink (round 2026-09-06). */
extern s32 (*STAIRCASE_TICK_FNS[4])(DreamSys *this);

/* Called by DreamSys__TryStaircaseLink with NO explicit argument setup (the disassembly's
   call site leaves `$a0` holding an unrelated leftover value from the
   preceding statement, same "empty delay slot, no a0-a3 setup" shape as
   GetStageLinkAngle above); return value used as STAIRCASE_TICK_FNS's index. MATCHED
   round 43 (2026-09-15) -- both the gp-relative and addiu_at blockers it was
   filed under are resolved (see docs/research/gp-relative-blocker.md and
   docs/research/addiu-at-blocker.md), and the one-line body
   `STAIRCASE_SPAWNS[gLinkDstStage][gLinkSpawnIndex].extra` matched on the first rebuild
   (docs/match-reports/GetLastSpawnExtra.md). Still declared here to type
   DreamSys__TryStaircaseLink's call site, which remains INCLUDE_ASM in this unit. */
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
                                          s32 arg2); /* DreamSys__SetTickCallbacks: selectCallback98(arg1), selectCallback80(arg2) */
    /* +0x138 */ void (*selectCallback80)(DreamSys *self, s32 arg1); /* DreamSys__SelectCallback80 */
    /* +0x13C */ void (*selectCallback98)(DreamSys *self, s32 arg1); /* DreamSys__SelectCallback98 */
    /* +0x140 */ void (*stepLook)(DreamSys *self);                   /* DreamSys__StepLook */
    /* +0x144 */ void (*stepLookOffset)(DreamSys *self);             /* DreamSys__StepLookOffset */
    /* +0x148 */ void (*stepLookYaw)(DreamSys *self);                /* DreamSys__StepLookYaw */
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
    /* +0x1F0 */ void (*initMoodContributors)(DreamSys *self, MoodGraphPoint *special); /* DreamSys__InitMoodContibutors */
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
    /* u8, not s8 (round 2026-09-08, GenerateInitialSpawn): retail reads it
	   with `lbu` -- it indexes sSpawnPosAdjust, so must zero-extend. */
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

extern s16 sStageTimeLimits[];

extern struct RelativePos sSpawnPosAdjust[];

extern StageSpawn *sStageSpawnPoints[];
/* Retyped u8 (round 2026-09-08, GenerateInitialSpawn): retail reads it with
   `lbu`, and the surrounding loop guard (`count != 0` implying `count > 0`,
   a single `beqz`) only holds if it can't be negative -- a signed `s8` here
   forces GCC to add a second `blez` check that retail does not have. */
extern u8 sStageSpawnPointsCount[];

extern StageSpawn *sStagePermalinkSpawns[];
extern StaticLinkTrigger *sStagePermalinkTriggers[];
extern s8 sStagePermalinkTriggersCount[];

extern s16 sSpecialDays[];

/* The fixed "special day" mood, returned by IsDaySpecial on a match
   (round 2026-09-02); only ever address-taken there, never dereferenced by
   this unit's queued functions. */
extern MoodGraphPoint SPECIAL_DAY_MOOD;

/* Also declared in Entity.h for the same libc-style function. */
extern s32 rand(void);

extern s8 sSpecialColors[];

/* Shared by TestForStaticLink/Test4TunnelLinks/Test4StaircaseNodes/
   Test4InstantTeleporters, each of which forwards its own three args
   straight through and appends a fixed trailing quadruple (length table,
   trigger table, spawn table, literal 1). Defined later in this unit's own
   ROM order; this is a forward declaration for the earlier call sites
   above, not a cross-unit prototype. MATCHED (the gp-relative/addiu_at
   blockers this was once filed under are resolved, see CLAUDE.md); return
   type is confirmed s32 by every call site's `bltz` check, not just a guess
   -- CLAUDE.md's tail-call-wrapper warning no longer applies once a
   function is its own real C body, only while it is still INCLUDE_ASM
   (round 2026-08-30-c note superseded). */
extern s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                          s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag);

/* Called by DreamSys__TryStageTimerLink as (&this->linkCoordinates, this->currentStage,
   currentPos, this->tick); result compared with `bltz` exactly like
   TestForStaticLink's call site, so s32 (round 2026-09-02). MATCHED, defined
   later in this unit's own ROM order -- this is a forward declaration, not a
   cross-unit prototype (the gp-relative blocker this was once filed under is
   resolved; see docs/match-reports/Test4StageTransition.md). */
extern s32 Test4StageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos,
                                s32 timer);

/* Called by DreamSys__TryStageTimerLink with NO arguments (the disassembly's call site has
   an empty delay slot and no a0-a3 setup); its return value is stored whole
   into this->stageLinkAngle, hence s32 (round 2026-09-02). MATCHED, defined later
   in this unit's own ROM order -- forward declaration only (gp-relative
   blocker resolved; see docs/match-reports/GetStageLinkAngle.md). */
extern s32 GetStageLinkAngle(void);

/* SceneNode__GetRotationDegrees (DreamSys__TryTunnelLink fills its 0x10-byte
   `local` with it): include/SceneNode.h. */

/* Called by DreamSys__TryTunnelLink as (&this->exitRotation, &this->enterRotation, &local) --
   same `local` buffer SceneNode__GetRotationDegrees fills above; result used as a truth
   value (`beqz`), so s32 (round 2026-09-02). MATCHED, defined later in
   this unit's own ROM order -- forward declaration only (the gp-relative
   and addiu_at blockers this was once filed under are both resolved; see
   docs/match-reports/DreamSys__CheckTunnelHeading.md). */
extern s32 DreamSys__CheckTunnelHeading(s32 *arg0, s32 *arg1, void *arg2);

/* Called by DreamSys__TryStaircaseLink (round 2026-09-06) as (&this->linkCoordinates,
   currentPos, this->currentStage) -- same forwarding shape as
   Test4TunnelLinks/TestForStaticLink above. Defined later in this unit's own
   ROM order (`src/DreamSys.c`); this is a forward declaration for that
   earlier call site, not a cross-unit prototype. */
extern s32 Test4StaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 arg2);

/* Called by DreamSys__TryStaircaseLink as (&this->exitRotation, &this->enterRotation, &local) --
   identical call shape to DreamSys__CheckTunnelHeading above (same `local` buffer, same two
   `this` fields), so the same signature. MATCHED, defined later in this
   unit's own ROM order -- forward declaration only (the gp-relative and
   addiu_at blockers this was once filed under are both resolved; see
   docs/match-reports/DreamSys__CheckStaircaseHeading.md). */
extern s32 DreamSys__CheckStaircaseHeading(s32 *arg0, s32 *arg1, void *arg2);

/* Same (target, currentPos, stage) forwarding shape as Test4TunnelLinks
   above (see that function's own comment) -- called by DreamSys__TryInstantTeleportLink as
   (&this->linkCoordinates, currentPos, this->currentStage), result compared
   with `bltz` (round 2026-09-02). MATCHED, defined later in this unit's own
   ROM order -- forward declaration only (gp-relative blocker resolved; see
   docs/match-reports/Test4InstantTeleporters.md). */
extern s32 Test4InstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);

/* Called by DreamSys__TryInstantTeleportLink with NO arguments, same shape as GetStageLinkAngle
   above; return value is forwarded straight into ExecuteLink's stage-type
   argument, hence s32 (round 2026-09-02). MATCHED, defined later in this
   unit's own ROM order -- forward declaration only (gp-relative blocker
   resolved; see docs/match-reports/GetTeleportTimeBonus.md). */
extern s32 GetTeleportTimeBonus(void);

/* Table triple for Test4TunnelLinks (round 2026-08-30-d), same roles as the
   STAGE_PERMALINK_* triple above but for tunnel links specifically. */
extern s8 LEN_TUNNEL_TRIGGERS[];
extern StaticLinkTrigger *TUNNEL_TRIGGERS[];
extern StageSpawn *TUNNEL_SPAWNS[];

/* Table triple for Test4StaircaseNodes (round 2026-08-30-d). */
extern s8 LEN_STAIRCASE_TRIGGERS[];
extern StaticLinkTrigger *STAIRCASE_TRIGGERS[];
extern StageSpawn *STAIRCASE_SPAWNS[];

/* This function might be called when the player hits a wall?
It tries to do an static link first, then a dynamic one */
void DreamSys__WallLink(DreamSys *this, void *unk_class_86aa0, int arg2);

/* @brief Sets the overall time limit for the dream and returns its previous value. */
/* @param value The new time limit, in seconds. Negative values are stored as-is. */
/* @return The previous time limit, in seconds, or -1. */
s32 DreamSys__GetSetDreamTimeLimit(DreamSys *this, s32 value);

/* @brief (Re)initializes playthrough-relevant data, like day number, flashabcks, etc. */
void DreamSys__InitNewGame(DreamSys *this);

/* @brief Sets whether the camera should shake when the player walks. */
/* @param value If True, the screen will shake when the player walks. */
/* @return To &value, the previous value of ScreenShakeOn */
void DreamSys__GetSetScreenShake(DreamSys *this, bool *value);

/* @brief Moves the currentDay counter foward one day, looping over on new years. */
/* @return Integer between 0 and 364, of the new currentDay value. */
s32 DreamSys__AdvanceDay(DreamSys *this);

/* @brief Checks what kind of dream comes next, and executes the appropiate start-of-dream actions. */
/* @return ID of the Stage to spawn on. Or -1 if the dream is Special (i.e. non-interactive). */
s32 DreamSys__StartDay(DreamSys *this);

/* @brief Executes various end-of-dream actions. */
s32 DreamSys__EndDay(DreamSys *this, s32 arg1);

/* @brief Gets the indicies of the Cinematic to be played next, if any. */
/* @return CinematicCall with the currently stored indicies. An Entry value of -1 means no cinematic. */
CinematicCall DreamSys__GetCinematic(DreamSys *this);

/* @brief Sets the next spawnpoint to be the intial spawn appropiate for the last day's graph. */
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
void DreamSys__InitMoodContibutors(DreamSys *this, MoodGraphPoint *special);

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
/* @param layer The contributor that will recieve the mood. */
/* @param mood The mood contribution to be logged. */
void DreamSys__LogMood(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *mood);

/* @brief Calculates the average point of a given Contributor. */
/* @param layer The MoodGraphContributor to be calculated. */
/* @param ret Pointer where this contributor's average point will be written to. */
/* @return To &ret, MoodPoint between (-9,-9) and (9,9). */
void DreamSys__GetMoodAverage(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *ret);

/* @brief Turns the values of a given mood contributor axis into an useable average */
/* @param lank The mood contribution that happened last, which recieves a boost in the code */
/* @param sum The cumulative value from all mood contributions */
/* @param amount The amount of mood contributions adquired */
/* @return Normalized integer between -9 and 9 */
s32 CalcMoodAxis(s32 lank, s32 sum, s32 amount);

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
DreamSysMethods *Get_vtable_DreamSys(void);

/* @brief Allocates and constructs a DreamSys instance.
 * Still INCLUDE_ASM in src/DreamSys.c; declared here so other units'
 * matched C (e.g. GameApplication__GameApplication in src/GameApplicationFileResource.c) can call it -- see
 * "Calling into a function that is still INCLUDE_ASM in another unit is
 * fine" in docs/DECOMPILATION_LEARNINGS.md. */
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