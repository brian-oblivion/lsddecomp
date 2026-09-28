#ifndef DREAM_SYS_H
#define DREAM_SYS_H

/**
 * @file dream_sys.h
 * @brief DreamSys, the object that runs the dream in progress: its object,
 *        method table and methods, the spawn, link and flashback records it
 *        keeps, and the free functions its link tests are built from
 *        (src/world/dream_sys.c).
 */

#include "common.h"
#include "actor.h"
#include "game_files.h"
/* For StageChunk / GetMoodFromStageChunk, used by DreamSys__LogChunkMood. */
#include "stage_grid.h"
#include "sound_cue_set.h"

typedef struct DreamSys DreamSys;
typedef struct DreamSysMethods DreamSysMethods;

/** DreamSys's class id (gDreamSysMethods word +0x000). Four nibbles, so
 * `(header & 0xFFFF) == DREAMSYS_CLASS_ID` tests for it or a class below it
 * (ObjM__OnNotify). */
#define DREAMSYS_CLASS_ID 0x1F34

/** The codes a DreamSys sends its parents through notifyParents
 * (ObjM__OnDreamSysNotify switches on them). Each link code is also what
 * ExecuteLink leaves in Actor::state while that link is pending;
 * DREAMSYS_NO_LINK is the idle state every link test requires. */
enum DreamSysLinkCode {
    DREAMSYS_NO_LINK = 0,           /**< no link pending */
    DREAMSYS_TIME_UP = 10,          /**< TimerTick: tick reached dreamTimeLimit */
    DREAMSYS_LINK_DAY_START = 11,   /**< InitSpawnLoc: the day's first spawn */
    DREAMSYS_LINK_DYNAMIC = 12,     /**< DynamicLink: a random spawn */
    DREAMSYS_LINK_WALL = 13,        /**< StaticWallLink */
    DREAMSYS_LINK_FLASHBACK = 14,   /**< LoadNextFlashback */
    DREAMSYS_LINK_TUNNEL = 15,      /**< TryTunnelLink */
    DREAMSYS_LINK_STAGE_TIMER = 16, /**< TryStageTimerLink */
    DREAMSYS_LINK_TELEPORT = 17     /**< TryInstantTeleportLink */
};

/** DreamSys::moveCommand, set by the pad handler and consumed by
 * AdvanceMoveCycle / ApplyMoveCommand. sMoveCommandSigns and
 * sMoveCommandDispatch make 1/2 a +/- step along the local z axis
 * (MoveLocalZOrFindLink) and 3/4 a -/+ step along local x
 * (MoveLocalXOrFindLink); FlipMoveCommand swaps each pair. Forced and
 * staircase movement always use MOVE_COMMAND_FORWARD. */
enum DreamSysMoveCommand {
    MOVE_COMMAND_NONE = 0,    /**< standing still */
    MOVE_COMMAND_FORWARD = 1, /**< a step forward (d-pad up) */
    MOVE_COMMAND_BACK = 2,    /**< a step back (d-pad down) */
    MOVE_COMMAND_LEFT = 3,    /**< a side step left (L2) */
    MOVE_COMMAND_RIGHT = 4    /**< a side step right (R2) */
};

/** DreamSys::moveMode indexes sMoveModeSpeeds {0, 24, 64, 128, 384}. The
 * pad handler switches to MOVE_MODE_RUN only while moving forward, and a
 * staircase walk takes about a seventh of the frames in it. */
#define MOVE_MODE_RUN 4

/** DreamSys::lookCallbackMode: what SelectLookCallback installs in lookCallback. */
enum DreamSysLookCallback {
    LOOK_CALLBACK_NONE = 0,      /**< no look callback */
    LOOK_CALLBACK_STEP_LOOK = 1, /**< stepLook */
    LOOK_CALLBACK_SLOT14C = 2,   /**< slot14C, an empty method */
    LOOK_CALLBACK_SLOT150 = 3    /**< slot150, an empty method */
};

/** DreamSys::moveCallbackMode: what SelectMoveCallback installs in moveCallback. */
enum DreamSysMoveCallback {
    MOVE_CALLBACK_NONE = 0,      /**< no move callback */
    MOVE_CALLBACK_TICK_MOVE = 1, /**< tickMove */
    MOVE_CALLBACK_TICK_DRIFT = 2 /**< tickDrift, with the sound cue set running */
};

/** DreamSys::moveOverride, setMoveOverride's value: which movement tickMove
 * runs (DreamSys__TickMove). */
enum DreamSysMoveOverride {
    MOVE_OVERRIDE_NONE = 0,   /**< applyPendingTurn, then tickMoveFree: the player's own movement */
    MOVE_OVERRIDE_FORCED = 1, /**< tickMoveForced: walked forward (a tunnel link) */
    MOVE_OVERRIDE_HELD = 2 /**< tickMoveHeld: a forward command that never steps (a stage-timer link) */
};

/** The dream clock counts DreamSys::tick 15 times per unit of
 * dreamTimeLimit's public value (GetSetDreamTimeLimit scales both ways). The
 * unit is seconds: every sStageTimeLimits entry is a whole number of
 * minutes (240, 180, 480, 420, ...). */
#define DREAM_TICKS_PER_SECOND 15

/** DreamSys::moodPreviousDays holds one mood per day; AdvanceDay wraps the
 * day into the next year here. */
#define DAYS_PER_YEAR 365

/** The navigation challenges DreamSys::navChallengesArray records. */
#define NAV_CHALLENGE_COUNT 30

/** The navigation-challenge array InitNavChallengesArray was last given
 * (DreamSys::navChallengesArray): GetStaticSpawn marks a challenge complete
 * in it, CalcNavigationScore counts it. */
extern s8 (*gpNavChallengesComplete)[NAV_CHALLENGE_COUNT];

/** The dynamic-link counter InitNavChallengesArray was last given
 * (DreamSys::amountDynamicLinksDone): GetRandomSpawnFromStage counts each
 * random spawn in it, CalcNavigationScore charges each one. */
extern s32 *gpDinamicLinkPenalty;

/* SceneNode__UpdateRotation (slot +0x044, the inherited rotation setter)
 * takes three Ratio16s, degrees as {num, den}, one per axis x, y, z, and
 * either STORES them into the object's rotation vector (flag != 0) or ADDS
 * them modulo a full turn (flag == 0). Every constant this unit hands that
 * slot is a Ratio16[3]: see sRotationYaw180 / Plus45 / Minus45 and
 * sCardinalRotations (src/world/dream_sys.c). */

/**
 * @brief One layer of the mood graph: the moods logged into it since the
 *        last clear. DreamSys keeps two, areaMoods (the chunks walked
 *        through) and entityMoods (the instances met); GetMoodAverage turns
 *        one into a point and UpdateDreamChart averages the two.
 */
typedef struct {
    MoodGraphPoint lastMood; /**< the mood logged last; GetMoodAverage adds a third of it again */

    /* 2 bytes unused */
    /** @brief The running sums of both axes of the logged moods. */
    struct sumAxis {
        s32 dynamic; /**< the dynamic (horizontal) axis' sum */
        s32 upper;   /**< the upper (vertical) axis' sum */
    } sumMoods;      /**< the sums GetMoodAverage divides by amountMoods */

    s32 amountMoods; /**< the number of moods logged */
} MoodGraphContributor;

/**
 * @brief A position in a stage: a chunk of the stage's grid, a tile in that
 *        chunk, and a position relative to the tile. What a link lands on
 *        (DreamSys::linkCoordinates) and what the grid reports as the
 *        player's current position (StageMap's getTargetDescriptor).
 */
typedef struct PlayerSpawnPoint {
    /** @brief A chunk of the stage's grid (StageChunk's layout). */
    struct MapChunk {
        u8 col; /**< the chunk's column */
        u8 row; /**< the chunk's row */
    } chunk;    /**< the chunk */

    /** @brief A tile within a chunk. */
    struct MapTile {
        u8 col; /**< the tile's column */
        u8 row; /**< the tile's row */
    } tile;     /**< the tile */

    /** @brief A position relative to a tile. */
    struct RelativePos {
        s16 x;  /**< x */
        s16 y;  /**< y (height) */
        s16 z;  /**< z */
    } position; /**< the position; a spawn's comes from sSpawnPosAdjust */
} PlayerSpawnPoint;

/**
 * @brief The `chunk` + `tile` half of a PlayerSpawnPoint (4 bytes), which is
 *        also the start of a StageSpawn: the spawn lookups
 *        (GetStaticSpawn, GetRandomSpawnFromStage, GenerateInitialSpawn)
 *        copy a spawn's grid position through it, and DreamSys keeps a
 *        staircase's start as this half (staircaseGridPos) followed by the
 *        other (staircaseOrigin).
 */
typedef struct PlayerSpawnGridPos {
    struct MapChunk chunk; /**< the chunk */
    struct MapTile tile;   /**< the tile */
} PlayerSpawnGridPos;

/**
 * @brief A flashback's orientation: SceneNode__GetRotationDegrees' three
 *        Ratio16s (degrees as {num, den}, about x, y and z) as one 12-byte
 *        struct, which DreamSys__AddFlashback copies from its `angles`
 *        argument and SpawnAtLink hands back to updateRotation.
 */
typedef struct FlashbackRotation {
    /** @brief One axis' angle in degrees, as a Ratio16: angle / one. */
    struct Angle {
        s16 angle;        /**< the numerator, in degrees */
        s16 one;          /**< the denominator */
    } pitch;              /**< about the x axis */
    struct Angle heading; /**< about the y axis: the yaw */
    struct Angle roll;    /**< about the z axis */
} FlashbackRotation;

/**
 * @brief One stored flashback: where, facing which way, on which day and for
 *        how long a flashback session revisits a moment of an earlier dream.
 */
typedef struct {
    s32 stageID;                /**< the stage */
    PlayerSpawnPoint position;  /**< the position in it */
    FlashbackRotation rotation; /**< the orientation */
    s16 timeLimit;              /**< seconds; SpawnAtLink sets the dream's limit to this plus 4 */
    s16 unk1C; /**< AddFlashback's fifth argument (FlashbackSaving always passes 0); nothing reads it */
    s32 day; /**< the day it was recorded; LoadNextFlashback makes it the current day */
} FlashbackEntry;

/* DreamSys::viewport is a Viewport (include/viewport.h; tag only here,
 * dream_sys.c includes the header), and soundObj a VabStreamObj
 * (include/vab_stream_obj.h); see those fields. */
struct Viewport;
struct LinkResource;
struct VabStreamObj;
struct TimImage;

/**
 * DreamSys: the dream in progress. Class id 0x1F34, table gDreamSysMethods,
 * getter GetDreamSysMethods; an Actor subclass (include/actor.h) whose ctor
 * calls Actor's first, and no class derives from it. Every method is in
 * src/world/dream_sys.c. One instance, made by GameApplication__GameApplication
 * (src/app/game_shell.c, New_DreamSys) and kept in GameApplication::dreamSys;
 * the same object is GraphRoom::dreamSys, the `target` dream_scene.c hands
 * SetDreamAuxWorld (dream_aux.c's sDreamAuxWorld), and the `peer` every
 * Entity links to. The object is 0x928 bytes (New_DreamSys); Actor's fields
 * end at +0x058 and DreamSys's own start there.
 *
 * It owns the dream clock (SceneNode's `tick`, advanced by
 * DreamSys__TimerTick, the update (+0x098) override, against
 * dreamTimeLimit), the player's movement (the pad handler OnPadEvent, +0x094,
 * sets move/turn/look commands that the tick callbacks installed by
 * SelectLookCallback/SelectMoveCallback consume), the mood graph (two
 * MoodGraphContributor accumulators averaged into a DreamColors value),
 * flashback recording and playback, and the link state machine that ends
 * one stage and starts the next (static wall links, dynamic and instance
 * links, tunnels, staircases, instant teleporters). Actor's `state` (+0x044)
 * holds the pending link type ExecuteLink writes.
 *
 * Per tick. TimerTick advances the dream clock and, at dreamTimeLimit,
 * loads the next flashback or ends the dream; below the limit it runs
 * UpdateTickState and RunTickCallbacks, which call the look and move
 * callbacks SelectLookCallback/SelectMoveCallback install. The look callback
 * is StepLook: two spring-back accumulators, StepLookOffset (the view height,
 * 600 a step up to 9000) and StepLookYaw (45 degrees a step, up to 180, and
 * back). The move callback is TickDrift or TickMove, the movement state
 * machine (free, forced or held, by moveOverride) around AdvanceMoveCycle's
 * four-tick step: a voice through the VabStreamObj in soundObj
 * (StartVoice/StopVoice), a view bob, and ApplyMoveCommand, which tries the
 * link tests before it lets the step happen.
 *
 * Link commands. The inherited dispatchers are overridden to add one case
 * each: dispatchLinkCommand (+0x09C, DispatchChunkChange) calls
 * processChunkChange for a grid (0x114) sender, onActorLinkCommand (+0x0DC,
 * DispatchInstanceEffect) calls instanceEffectsOnJournal for an Entity
 * (0x1F234) sender, and onGridCellLinkCommand (+0x0E0, WallLink) tries a
 * wall link on event 4.
 *
 * Actor::grid is the grid manager, StageMap (include/stage_map.h), which
 * dream_sys.c calls directly. The inherited updateRotation (+0x044) is the
 * rotation setter, so every constant passed to it is three degree ratios
 * (Ratio16[3], include/scene_node.h).
 *
 * The bytes from saveMagic up to newGamePending are the save block
 * (DreamSys__GetSaveBlock, DREAMSYS_SAVE_SIZE; DreamSaveBlock views it).
 */
struct DreamSys {
    ACTOR_FIELDS(DreamSysMethods);
    /* +0x058 onward: DreamSys's own */
    /** The sound bank, a VabStreamObj: StartVoice and ExecuteLink call its
     * playTone (+0x080; the voice StartVoice gets goes to voiceIndex),
     * StopVoice its stopVoice (+0x084), StartVoice its setPitchOffset
     * (+0x09C). Set by the ctor and DreamSys__SetSoundObj. */
    struct VabStreamObj *soundObj;
    /** The camera, set by the ctor and DreamSys__SetViewport; DayTask__Init
     * installs a New_NodeGuardedViewport. This class moves its GsRVIEW2
     * refView: vp and vr are the two points ProjectPointAtDistance
     * interpolates between, AdvanceMoveCycle's view bob moves vp.y and vr.y,
     * and StepLookOffset, StopDrift and TickDrift move vr.y (looking up and
     * down). Entity__MoodCue74 calls its setClearColor (+0x064). */
    struct Viewport *viewport;
    /** The ctor's `modelSource`, a LinkResource (GameApplication passes
     * New_LinkResource("ETC\DREAME5.TMD")), whose model 0 the ctor adds as a
     * child. Not read elsewhere. */
    struct LinkResource *modelSource;
    /** The TimImage DayTask__DayTask hands over through DreamSys__SetEtcTim
     * (+0x114); cleared by the ctor, never dereferenced by the DreamSys. */
    struct TimImage *etcTim;

    bool isFlashbackSession; /**< the dream replays stored flashbacks (GetSetFlashbackSession) */
    s32 moveOverride;        /**< a DreamSysMoveOverride: which movement DreamSys__TickMove runs */

    /** Non-zero blocks movement: set by the ctor and BlockMovement, cleared
     * by ResetLinkState. OnPadEvent ignores the pad, UpdateTickState and
     * TickMoveFree do nothing, and TickMoveForced cycles without stepping. */
    s32 movementBlocked;
    /** Set by the pad (the circle button) and cleared each tick by
     * UpdateTickState and by ResetLinkState; GetLinkCommandFlag reads it. */
    s32 linkCommandFlag;
    s32 unk78; /**< cleared by DreamSys__func_59598 (slot128), ResetSessionState and ResetLinkState; never read */
    s32 unk7C; /**< cleared by DreamSys__func_59590 (slot124); nothing else touches it */
    /** The look callback RunTickCallbacks calls each tick, or NULL:
     * SelectLookCallback installs it by lookCallbackMode. */
    void (*lookCallback)(struct DreamSys *self);
    s32 lookCallbackMode; /**< a DreamSysLookCallback: SelectLookCallback's last mode */
    /** A pending view-height step (1 from triangle, 2 from square; a
     * staircase walk sets 2), an index into sLookOffsetSteps and
     * sLookOffsetLimits; StepLookOffset consumes it. */
    s32 lookOffsetCommand;
    /** How far the view height is moved: StepLookOffset adds each step to
     * it and to viewport->refView.vr.y, and springs it back by 600 a tick
     * with no command pending. */
    s32 lookOffset;
    /** A pending sideways look step (1 from L1, 2 from R1), an index into
     * sLookYawSteps and sLookYawLimits; StepLookYaw consumes it. */
    s32 lookYawCommand;
    /** How far the view is turned sideways, in degrees: StepLookYaw's
     * accumulator, springing back by 45 a tick with no command pending. */
    s32 lookYaw;
    /** The move callback RunTickCallbacks calls each tick, or NULL:
     * SelectMoveCallback installs it by moveCallbackMode. */
    void (*moveCallback)(struct DreamSys *self);
    /** A DreamSysMoveCallback: SelectMoveCallback's last mode. Leaving
     * MOVE_CALLBACK_TICK_DRIFT stops the drift first. */
    s32 moveCallbackMode;
    /** The step in progress, a DreamSysMoveCommand: the pad sets it,
     * AdvanceMoveCycle clears it after four ticks, FlipMoveCommand swaps its
     * direction, and forced and staircase movement set it to
     * MOVE_COMMAND_FORWARD. */
    s32 moveCommand;
    /** A pending turn of 6 degrees (1 from d-pad left, 2 from right; the
     * staircase walks set it too), an index into sTurnRotations;
     * ApplyPendingTurn consumes it. */
    s32 turnCommand;
    /** Whether the player was stepping forward when StepLookYaw last ran;
     * FlipMoveCommand clears it. The tunnel and staircase links require it. */
    s32 moveCommandLatch;
    /** The current speed, an index into sMoveModeSpeeds. ChangeMoveMode
     * keeps the old value in previousMoveMode; GetSetMoveMode sets both. */
    s32 moveMode;
    s32 previousMoveMode; /**< the speed RestorePreviousMoveMode goes back to (releasing up after running) */
    /** Ticks into the step in progress (0..3), counted by AdvanceMoveCycle
     * while moveCommand is non-zero: the view bobs down on the first two and
     * up on the last two, and the step ends after the fourth. */
    s32 moveCycleTick;
    /** The footstep sound: the low seven bits of the floor cell's flags36
     * (NotifyLinkAttempt, when a step finds floor; 0 when out of range, 2 on
     * a tunnel link that found none), an index into sVoiceBySelect and
     * sVoicePitchBySelect. 0 is silent; 11 plays two more tones; only 22's
     * voice is kept in voiceIndex. */
    s32 voiceSelect;
    /** The voice StartVoice keeps playing (footstep 22), for StopVoice to
     * stop; -1 when none. */
    s32 voiceIndex;
    u8 padC0[4];
    s32 driftActive; /**< TickDrift moves the object while set: SelectMoveCallback's drift mode sets it, StopDrift clears it */
    s32 cueServiceActive; /**< TickDrift services soundCueSet while set: set with driftActive, StopDrift sets it to keepCues */
    /** The sound cue set drifting runs: SelectMoveCallback's drift mode
     * starts it with soundCueCallback, TickDrift services it, StopDrift
     * flushes it. */
    SoundCueSet soundCueSet;

    s32 tickPeriod; /**< the period of tickBoundary, in ticks (SetTickPeriod) */
    /** Whether tick is a multiple of tickPeriod (UpdateTickState); a wall
     * that is not a static link links dynamically only on such a tick. */
    s32 tickBoundary;
    /** Set with tickBoundary by DreamSys__SetGateFlags (+0x18C), each only
     * when its argument is >= 0; ResetLinkState sets all three to 1. No code
     * reads them. */
    s32 gateFlags[3];

    /** The dream's length in ticks; a negative value never runs out (the
     * timer compares unsigned). GetSetDreamTimeLimit converts to seconds. */
    s32 dreamTimeLimit;
    u8 pad138[12];

    MoodGraphContributor areaMoods;   /**< the moods of the chunks walked through (LogChunkMood) */
    MoodGraphContributor entityMoods; /**< the moods of the instances met (LogInstanceMood) */
    s32 currentStage; /**< the stage the player is on; a negative value names a link target for DynamicLink */
    CinematicCall nextCinematic; /**< the cinematic GetCinematic hands out: a special day's, or an instance's event video */
    /** The position a link lands on (and, for a wall link, the cell hit):
     * the link tests write it, SpawnAtLink loads it. */
    PlayerSpawnPoint linkCoordinates;
    /* 2 bytes unused */
    s32 saveMagic;                 /**< the save block's first word, sSaveMagic (InitNewGame) */
    s32 currentYear;               /**< years completed; AdvanceDay counts them */
    s32 currentDay;                /**< the day of the year, 0..364 */
    s32 totalFlashbackUnlockScore; /**< the two scores below added (CalcUnlockScore) */
    s32 navigationFlashbackUnlockScore; /**< CalcNavigationScore's result */
    s32 instanceFlashbackUnlockScore; /**< the instances' unlock effects, summed, clamped to 0..50000000 */
    /** Each day's mood, logged by EndDay (the dream chart's average) and
     * indexed by day of the year: the mood ring the graph shows. */
    MoodGraphPoint moodPreviousDays[DAYS_PER_YEAR];
    /* 2 bytes unused */
    s32 amountFlashbacksAvailable;       /**< entries in storedFlashbacks */
    FlashbackEntry storedFlashbacks[10]; /**< the flashbacks recorded (AddFlashback) */

    s8 unk5D8; /**< InitNewGame clears it; nothing reads it */
    u8 pad5D9[6];
    s8 graphScored; /**< DreamSaveBlock's graphScored: GraphRoom__ScoreDayLog sets it; InitNewGame clears it */

    s8 navChallengesArray[NAV_CHALLENGE_COUNT]; /**< one flag per navigation challenge completed */
    /* 2 bytes unused */
    s32 amountDynamicLinksDone; /**< the random spawns taken, each a penalty in CalcNavigationScore */
    u8 pad604[116];

    bool screenShakeOn; /**< the view bobs as the player walks (GetSetScreenShake; InitNewGame sets it) */
    s32 unk67C;         /**< InitNewGame clears it; nothing else touches it */
    s32 unk680;         /**< InitNewGame clears it; nothing else touches it */
    s8 unk684[500]; /**< InitNewGame clears it; nothing else touches it */

    /** Set by the ctor and by EndDay's new-game outcome, cleared by
     * ClearNewGameFlag; the save block ends before it. */
    s32 newGamePending;
    s32 currentFlashbackIndex; /**< the next flashback to play (StartDay rewinds it, SpawnAtLink advances it) */
    /** GetStageLinkAngle()'s result, stored by TryStageTimerLink right before
     * its ExecuteLink; no reader in src/. */
    s32 stageLinkAngle;
    /** A sCardinalRotations entry (its address, or 0): the heading a tunnel
     * or staircase is entered at (CheckTunnelHeading, CheckStaircaseHeading),
     * applied when forced movement starts (SetMoveOverride) and when a
     * staircase walk starts. */
    s32 enterRotation;
    /** The same for the heading the link leaves the player at, applied by
     * SpawnAtLink under forced movement; TryStageTimerLink zeroes both. */
    s32 exitRotation;

    s32 storedDay; /**< the day StartDay began; EndDay restores currentDay from it */

    /** The coordinate as SaveLinkSnapshot kept it before a step, which
     * RestoreLinkSnapshot puts back (undoing the step) and then clears
     * coord2->flg. */
    GsCOORDINATE2 coord2Snapshot;
    GsCOORD2PARAM coord2ParamSnapshot; /**< coord2->param as SaveLinkSnapshot kept it */
    s32 staircaseActive; /**< a staircase walk is running: OnPadEvent ignores the pad */
    /** Non-zero while a staircase walk runs; the step then passes `notify`
     * 0 to the move (ApplyMoveCommand). Its writers store 0 or 1. */
    u32 staircaseMoveGate;
    /** The staircase walk's tick, one of sStaircaseTickFns (chosen by
     * GetLastSpawnExtra()), or NULL: TryStaircaseLink calls it each step
     * until it returns non-zero. */
    s32 (*staircaseTickFn)(struct DreamSys *self);
    s32 staircaseFrame; /**< ticks into the staircase walk; the staircase ticks count it */
    /** The position the staircase walk began at (TryStaircaseLink copies the
     * whole PlayerSpawnPoint into this and staircaseOrigin). */
    PlayerSpawnGridPos staircaseGridPos;
    /** Its relative position, from which a staircase tick's first
     * ApplyRelativeOffset moves the object. */
    struct RelativePos staircaseOrigin;
    u8 pad922[2];
    /** GameApplication's config word +0x14, through getSetConfigOption
     * (DreamSys__GetSetConfigOption); ResetSessionState clears it. Nothing
     * reads it. */
    s32 configOption;
};

/** The bytes DreamSys__GetSaveBlock hands out, from saveMagic up to
 * newGamePending: 1792. */
#define DREAMSYS_SAVE_SIZE (offsetof(DreamSys, newGamePending) - offsetof(DreamSys, saveMagic))

/**
 * @brief What DreamSys__GetSaveBlock (+0x1B0) returns, &saveMagic, viewed
 *        from there: each field is the DreamSys field above at its DreamSys
 *        offset less saveMagic's +0x178. UpdateFlashbackLock (TitleMenu)
 *        reads the flashback pair; GraphRoom reads the year, the day and the
 *        mood ring, and sets graphScored, DreamSys +0x5DF.
 */
typedef struct DreamSaveBlock {
    u8 pad00[0x4];
    /* +0x004 */ s32 currentYear; /**< DreamSys +0x17C; nonzero: the ring is full, plot all 100 days */
    /* +0x008 */ s32 currentDay; /**< DreamSys +0x180; days logged this year, the ring's write cursor */
    /* +0x00C */ s32 totalFlashbackUnlockScore; /**< DreamSys +0x184: FLASHBACK unlocks past 9999999 */
    u8 pad10[0x18 - 0x10];
    /* +0x018 */ MoodGraphPoint moodPreviousDays[DAYS_PER_YEAR]; /**< walked backwards from currentDay - 1 */
    u8 pad2F2[0x2F4 - 0x2F2];
    /* +0x2F4 */ s32 amountFlashbacksAvailable; /**< DreamSys +0x46C: ... and only with one stored */
    u8 pad2F8[0x467 - 0x2F8];
    /* +0x467 */ s8 graphScored; /**< set once GraphRoom__ScoreDayLog's scan has succeeded */
} DreamSaveBlock;

/**
 * @brief The `extra` of the staircase spawn the last GetStaticSpawn landed
 *        on: which of sStaircaseTickFns walks it.
 * @return The spawn's `extra`, 0..3.
 */
extern s32 GetLastSpawnExtra(void);

/** @brief The eight colours of a dream's mood (CalcDreamColor). */
typedef enum DreamColors {
    DREAM_COLOR_BLACK,  /**< black */
    DREAM_COLOR_BLUE,   /**< blue */
    DREAM_COLOR_GREEN,  /**< green */
    DREAM_COLOR_CYAN,   /**< cyan */
    DREAM_COLOR_RED,    /**< red */
    DREAM_COLOR_PINK,   /**< pink */
    DREAM_COLOR_YELLOW, /**< yellow */
    DREAM_COLOR_WHITE,  /**< white */
} DreamColors;

/**
 * DreamSys's method table: Actor's slots with DreamSys's ctor parameters,
 * then DreamSys's own, each pointing at its occupant. The inherited slots
 * keep Actor's names and types; gDreamSysMethods overrides +0x008
 * (DreamSys__DreamSys), +0x040 reset (DreamSys__ResetSessionState), +0x04C
 * attachToParent (DreamSys__SpawnAtLink), +0x050 detachFromParent
 * (DreamSys__DetachFromParent), +0x088 notifyIfUnk20Active
 * (DreamSys__NotifyLinkAttempt), +0x094 onPadEvent (DreamSys__OnPadEvent),
 * +0x098 update (DreamSys__TimerTick), +0x09C dispatchLinkCommand
 * (DreamSys__DispatchChunkChange), +0x0DC onActorLinkCommand
 * (DreamSys__DispatchInstanceEffect), +0x0E0 onGridCellLinkCommand
 * (DreamSys__WallLink) and +0x0E8 slotE8 (DreamSys__NoOpSlotE8Default).
 *
 * Two inherited slots are called with a type other than their own, each
 * through a function-pointer cast: reset (+0x040), whose result the ctor
 * returns (DreamSysResetRetFn), and attachToParent (+0x04C), which
 * DreamSys__SpawnAtLink takes as (self, parent) with no offset
 * (DreamSysAttachToParentFn, ObjM__SetupSceneStyle's cast). The table is
 * 139 slots, 0x22C bytes.
 */
struct DreamSysMethods {
    ACTOR_SLOTS(DreamSys, (DreamSys * self, struct LinkResource *modelSource,
                           struct VabStreamObj *soundObj, struct Viewport *viewport));
    /* +0x0F0 */ s32 (*getSetFlashbackSession)(DreamSys *self, DreamColors *out,
                                               s32 value); /**< @see DreamSys__GetSetFlashbackSession */
    /* +0x0F4 */ void (*setMoveOverride)(DreamSys *self, s32 value); /**< @see DreamSys__SetMoveOverride */
    /* +0x0F8 */ void (*resetLinkState)(DreamSys *self, s32 moveMode,
                                        s32 tickPeriod);    /**< @see DreamSys__ResetLinkState */
    /* +0x0FC */ void (*blockMovement)(DreamSys *self);     /**< @see DreamSys__BlockMovement */
    /* +0x100 */ s32 (*getLinkCommandFlag)(DreamSys *self); /**< @see DreamSys__GetLinkCommandFlag */
    /* +0x104 */ s32 (*getSetDreamTimeLimit)(DreamSys *self, s32 time); /**< @see DreamSys__GetSetDreamTimeLimit */
    /* +0x108 */ s32 (*getDreamTimerScaled)(DreamSys *self); /**< @see DreamSys__GetDreamTimerScaled */
    /* +0x10C */ void (*setSoundObj)(DreamSys *self, struct VabStreamObj *value); /**< DreamSys__SetSoundObj: soundObj = value */
    /* +0x110 */ void (*setViewport)(DreamSys *self, struct Viewport *value); /**< @see DreamSys__SetViewport */
    /* +0x114 */ void (*setEtcTim)(DreamSys *self, struct TimImage *value); /**< DreamSys__SetEtcTim: etcTim = value */
    /* +0x118 */ void (*updateTickState)(DreamSys *self);  /**< @see DreamSys__UpdateTickState */
    /* +0x11C */ void (*runTickCallbacks)(DreamSys *self); /**< @see DreamSys__RunTickCallbacks */
    /* +0x120 */ s32 (*projectPointAtDistance)(DreamSys *self, s32 *out, s32 dist, s32 *reference,
                                               s32 tolerance); /**< @see DreamSys__ProjectPointAtDistance */
    /* +0x124 */ void (*slot124)(DreamSys *self); /**< @see DreamSys__func_59590 (never called) */
    /* +0x128 */ void (*slot128)(DreamSys *self); /**< @see DreamSys__func_59598 (never called) */
    /* +0x12C */ s32 (*slot12C)(DreamSys *self);  /**< @see DreamSys__NoOpSlot12C */
    /* +0x130 */ void (*clearTickCallbacks)(DreamSys *self, bool clearLook); /**< @see DreamSys__ClearTickCallbacks */
    /* +0x134 */ void (*setTickCallbacks)(DreamSys *self, s32 moveMode,
                                          s32 lookMode); /**< @see DreamSys__SetTickCallbacks */
    /* +0x138 */ void (*selectLookCallback)(DreamSys *self, s32 mode); /**< DreamSys__SelectLookCallback: lookCallback by a DreamSysLookCallback */
    /* +0x13C */ void (*selectMoveCallback)(DreamSys *self, s32 mode); /**< DreamSys__SelectMoveCallback: moveCallback by a DreamSysMoveCallback; the drift mode starts soundCueSet */
    /* +0x140 */ void (*stepLook)(DreamSys *self);       /**< @see DreamSys__StepLook */
    /* +0x144 */ void (*stepLookOffset)(DreamSys *self); /**< @see DreamSys__StepLookOffset */
    /* +0x148 */ void (*stepLookYaw)(DreamSys *self);    /**< @see DreamSys__StepLookYaw */
    /* +0x14C */ void (*slot14C)(DreamSys *self); /**< @see DreamSys__NoOpSlot14C (a lookCallback choice) */
    /* +0x150 */ void (*slot150)(DreamSys *self); /**< @see DreamSys__NoOpSlot150 (a lookCallback choice) */
    /* +0x154 */ s32 (*tickMove)(DreamSys *self);       /**< @see DreamSys__TickMove */
    /* +0x158 */ s32 (*tickMoveFree)(DreamSys *self);   /**< @see DreamSys__TickMoveFree */
    /* +0x15C */ s32 (*tickMoveForced)(DreamSys *self); /**< @see DreamSys__TickMoveForced */
    /* +0x160 */ s32 (*tickMoveHeld)(DreamSys *self);   /**< @see DreamSys__TickMoveHeld */
    /* +0x164 */ s32 (*advanceMoveCycle)(DreamSys *self, s32 bob); /**< @see DreamSys__AdvanceMoveCycle */
    /* +0x168 */ void (*startVoice)(DreamSys *self);               /**< @see DreamSys__StartVoice */
    /* +0x16C */ void (*stopVoice)(DreamSys *self);                /**< @see DreamSys__StopVoice */
    /* +0x170 */ s32 (*applyMoveCommand)(DreamSys *self, s32 command); /**< @see DreamSys__ApplyMoveCommand */
    /* +0x174 */ void (*applyPendingTurn)(DreamSys *self); /**< @see DreamSys__ApplyPendingTurn */
    /* +0x178 */ void (*tickDrift)(DreamSys *self);        /**< @see DreamSys__TickDrift */
    /* +0x17C */ void (*stopDrift)(DreamSys *self, s32 keepCues); /**< @see DreamSys__StopDrift */
    /* +0x180 */ s32 (*getSetMoveMode)(DreamSys *self, s32 value); /**< @see DreamSys__GetSetMoveMode */
    /* +0x184 */ void (*changeMoveMode)(DreamSys *self, s32 value); /**< @see DreamSys__ChangeMoveMode */
    /* +0x188 */ void (*restorePreviousMoveMode)(DreamSys *self); /**< @see DreamSys__RestorePreviousMoveMode */
    /* +0x18C */ void (*setGateFlags)(DreamSys *self, s32 a, s32 b, s32 c, s32 d); /**< @see DreamSys__SetGateFlags */
    /* +0x190 */ void (*setTickPeriod)(DreamSys *self, s32 value); /**< @see DreamSys__SetTickPeriod */
    /* +0x194 */ void (*soundCueCallback)(void *owner, SoundCueSet *set); /**< @see DreamSys__SoundCueCallback (only its value is read: InitSoundCueSet's callback) */
    /* +0x198 */ void (*initNewGame)(DreamSys *self); /**< @see DreamSys__InitNewGame */
    /* +0x19C */ void (*getSetScreenShake)(DreamSys *self, bool *value); /**< @see DreamSys__GetSetScreenShake */
    /* +0x1A0 */ s32 (*getCurrentDayAndYear)(DreamSys *self, s32 *outYear); /**< @see DreamSys__GetCurrentDayAndYear */
    /* +0x1A4 */ s32 (*advanceDay)(DreamSys *self);        /**< @see DreamSys__AdvanceDay */
    /* +0x1A8 */ void (*clearNewGameFlag)(DreamSys *self); /**< @see DreamSys__ClearNewGameFlag */
    /* +0x1AC */ s32 (*getNewGameFlag)(DreamSys *self);    /**< @see DreamSys__GetNewGameFlag */
    /* +0x1B0 */ s32 *(*getSaveBlock)(DreamSys *self, s32 *outSize); /**< @see DreamSys__GetSaveBlock */
    /* +0x1B4 */ s32 (*startDay)(DreamSys *self);                    /**< @see DreamSys__StartDay */
    /* +0x1B8 */ s32 (*endDay)(DreamSys *self, s32 outcome);         /**< @see DreamSys__EndDay */
    /* +0x1BC */ CinematicCall (*getCinematic)(DreamSys *self); /**< @see DreamSys__GetCinematic */
    /* +0x1C0 */ void (*initSpawnLoc)(DreamSys *self);          /**< @see DreamSys__InitSpawnLoc */
    /* +0x1C4 */ void (*dynamicLink)(DreamSys *self);           /**< @see DreamSys__DynamicLink */
    /* +0x1C8 */ bool (*staticWallLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /**< @see DreamSys__StaticWallLink */
    /* +0x1CC */ bool (*loadNextFlashback)(DreamSys *self, bool quiet); /**< @see DreamSys__LoadNextFlashback */
    /* +0x1D0 */ bool (*tryTunnelLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /**< @see DreamSys__TryTunnelLink */
    /* +0x1D4 */ bool (*tryStageTimerLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /**< @see DreamSys__TryStageTimerLink */
    /* +0x1D8 */ bool (*tryInstantTeleportLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /**< @see DreamSys__TryInstantTeleportLink */
    /* +0x1DC */ bool (*tryStaircaseLink)(DreamSys *self, PlayerSpawnPoint *currentPos); /**< @see DreamSys__TryStaircaseLink */
    /* +0x1E0 */ s32 (*getCurrentStage)(DreamSys *self); /**< @see DreamSys__GetCurrentStage */
    /* +0x1E4 */ void (*processChunkChange)(DreamSys *self, void *entity,
                                            s32 effect); /**< DreamSys__ProcessChunkChange: dispatchLinkCommand's grid (0x114) case; logs the new chunk's mood */
    /* +0x1E8 */ void (*instanceEffectsOnJournal)(DreamSys *self, void *entity,
                                                  s32 effect); /**< @see DreamSys__InstanceEffectsOnJournal */
    /* +0x1EC */ void (*getPreviousDayMood)(DreamSys *self, MoodGraphPoint *target,
                                            bool lastDayOnly); /**< @see DreamSys__GetPreviousDayMood */
    /* +0x1F0 */ void (*initMoodContributors)(DreamSys *self, MoodGraphPoint *special); /**< @see DreamSys__InitMoodContributors */
    /* +0x1F4 */ void (*logChunkMood)(DreamSys *self, PlayerSpawnPoint *currentPos); /**< @see DreamSys__LogChunkMood */
    /* +0x1F8 */ void (*logInstanceMood)(DreamSys *self, MoodGraphPoint *source); /**< @see DreamSys__LogInstanceMood */
    /* +0x1FC */ void (*updateDreamChart)(DreamSys *self, MoodGraphPoint *ret); /**< @see DreamSys__UpdateDreamChart */
    /* +0x200 */ DreamColors (*getDreamColor)(DreamSys *self); /**< @see DreamSys__GetDreamColor */
    /* +0x204 */ void (*clearMoodGraph)(DreamSys *self, MoodGraphContributor *contributor); /**< @see DreamSys__ClearMoodGraph */
    /* +0x208 */ void (*logMood)(DreamSys *self, MoodGraphContributor *layer,
                                 MoodGraphPoint *mood); /**< @see DreamSys__LogMood */
    /* +0x20C */ void (*getMoodAverage)(DreamSys *self, MoodGraphContributor *layer,
                                        MoodGraphPoint *ret); /**< @see DreamSys__GetMoodAverage */
    /* +0x210 */ void (*calcUnlockScore)(DreamSys *self);     /**< @see DreamSys__CalcUnlockScore */
    /* +0x214 */ void (*addFlashback)(DreamSys *self, s32 stage, PlayerSpawnPoint *pos, s32 *angles,
                                      s32 unknown, s32 time, s32 day); /**< @see DreamSys__AddFlashback */
    /* +0x218 */ void (*flashbackSaving)(DreamSys *self, s32 unknown,
                                         s32 timeLimit);     /**< @see DreamSys__FlashbackSaving */
    /* +0x21C */ void (*resetFlashbackList)(DreamSys *self); /**< @see DreamSys__ResetFlashbackList */
    /* +0x220 */ void (*saveLinkSnapshot)(DreamSys *self);   /**< @see DreamSys__SaveLinkSnapshot */
    /* +0x224 */ void (*restoreLinkSnapshot)(DreamSys *self); /**< @see DreamSys__RestoreLinkSnapshot */
    /* +0x228 */ s32 (*getSetConfigOption)(DreamSys *self, s32 value); /**< @see DreamSys__GetSetConfigOption */
};

/** reset (+0x040) as DreamSys__DreamSys calls it, with a result the ctor
 * returns. ResetSessionState, the occupant, returns none, and New_DreamSys
 * ignores the ctor's result. */
typedef DreamSys *(*DreamSysResetRetFn)(DreamSys *self);

/** attachToParent (+0x04C) as its occupant, DreamSys__SpawnAtLink, takes it:
 * (self, parent), no offset. ObjM__SetupSceneStyle (dream_scene.c) calls it
 * through this cast. */
typedef void (*DreamSysAttachToParentFn)(DreamSys *self, void *parent);

/**
 * @brief One spawn point of a stage's spawn tables: a grid position, an
 *        index of the relative position to land at, and a per-table extra.
 */
typedef struct StageSpawn {
    struct MapChunk chunk; /**< the chunk */
    struct MapTile tile;   /**< the tile */
    u8 adjustment;         /**< an index into sSpawnPosAdjust: the relative position to land at */
    /** Per table: a navigation challenge's index (GetStaticSpawn's flag
     * marks it complete), or a staircase's walk (GetLastSpawnExtra). */
    s8 extra;
} StageSpawn;

/**
 * @brief One link trigger of a stage's trigger tables: the grid position
 *        that triggers it and the spawn it leads to.
 */
typedef struct StaticLinkTrigger {
    struct MapChunk chunk; /**< the chunk that triggers the link */

    /** @brief The tile that triggers it, or a negative `value` for any tile
     *         of the chunk. */
    union TriggerTile {
        struct MapTile axis; /**< the tile */
        s16 value;           /**< both bytes as one value, for the compare */
    } tile;                  /**< the tile */

    s8 stage;           /**< the destination stage */
    s8 spawnpointIndex; /**< the destination spawn in that stage's spawn table */
} StaticLinkTrigger;

/** DreamSys's method table (class id 0x1F34). */
extern DreamSysMethods gDreamSysMethods;

/**
 * @brief The link test shared by TestForStaticLink, TestForTunnelLinks,
 *        TestForStaircaseNodes and TestForInstantTeleporters: finds the
 *        first of the stage's triggers whose chunk, and tile unless the
 *        trigger takes any, match `currentPos`, and writes the spawn it leads
 *        to. The match is recorded in sLinkSrcStage, sLinkTriggerIndex,
 *        sLinkDstStage and sLinkSpawnIndex.
 * @param target      Where to write the destination spawn's position.
 * @param currentPos  The player's position.
 * @param stage       The current stage, indexing the three tables.
 * @param triggerLens Each stage's number of triggers.
 * @param triggers    Each stage's triggers.
 * @param spawns      Each stage's spawns, which the triggers index.
 * @param flag        Non-zero marks the spawn's navigation challenge
 *                    (its `extra`) complete.
 * @return The destination stage, or -1 when no trigger matches.
 */
extern s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                          s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag);

/**
 * @brief The stage links TryStageTimerLink takes: only stages 1, 3, 5, 9 and
 *        12 have one. Stage 5 links below y -4095 or at one grid position,
 *        stage 9 at y 2048 and up, the others anywhere. On an odd timer the
 *        link lands on stage 12, otherwise on a random other stage.
 * @param target     Where to write the destination spawn.
 * @param stage      The current stage.
 * @param currentPos The player's position.
 * @param timer      The dream clock; its low bit picks the destination.
 * @return The destination stage (also sLinkDstStage), or -1 for no link.
 */
extern s32 TestForStageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos,
                                  s32 timer);

/**
 * @brief The rotation a stage-timer link leaves the player at.
 * @return A pointer to a half-turn rotation (as an s32), or 0 when the last
 *         link's destination was stage 12.
 */
extern s32 GetStageLinkAngle(void);

/**
 * @brief Whether the player faces the way the tunnel GetStaticSpawn last
 *        matched must be entered (within 44 degrees of its cardinal
 *        heading), and if so the rotations to enter and leave it at.
 * @param outExit  Where to write the sCardinalRotations entry the tunnel's
 *                 exit faces, or NULL.
 * @param outEnter Where to write the entry the tunnel is entered at, or NULL.
 * @param rotation The player's rotation, SceneNode__GetRotationDegrees' form.
 * @return 1 when aligned (and the rotations written), 0 otherwise.
 */
extern s32 DreamSys__CheckTunnelHeading(s32 *outExit, s32 *outEnter, void *rotation);

/**
 * @brief The staircase link test: GetStaticSpawn over the staircase tables,
 *        on stage 0 only.
 * @param target     Where to write the destination spawn.
 * @param currentPos The player's position.
 * @param stage      The current stage.
 * @return The destination stage, or -1 for no staircase.
 */
extern s32 TestForStaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);

/**
 * @brief DreamSys__CheckTunnelHeading for the staircase GetStaticSpawn last
 *        matched, over the staircase heading tables.
 * @param outExit  Where to write the exit's sCardinalRotations entry, or NULL.
 * @param outEnter Where to write the entry's, or NULL.
 * @param rotation The player's rotation, SceneNode__GetRotationDegrees' form.
 * @return 1 when aligned (and the rotations written), 0 otherwise.
 */
extern s32 DreamSys__CheckStaircaseHeading(s32 *outExit, s32 *outEnter, void *rotation);

/**
 * @brief The instant-teleporter link test: GetStaticSpawn over the
 *        teleporter tables, while teleporters are enabled.
 * @param target     Where to write the destination spawn.
 * @param currentPos The player's position.
 * @param stage      The current stage.
 * @return The destination stage, or -1 for no teleporter (or disabled).
 */
extern s32 TestForInstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);

/**
 * @brief The seconds a teleport adds to the dream's time limit.
 * @return 10 when the last link started on stage 0, else 0.
 */
extern s32 GetTeleportTimeBonus(void);

/**
 * @brief Slot +0x0E0, onGridCellLinkCommand: Actor's, then, when a wall
 *        cell reports SCENENODE_EVENT_LINKED with no link pending, records
 *        the cell as linkCoordinates and tries a static wall link, else (on
 *        a tickBoundary tick) a dynamic link. The step is then undone
 *        (restoreLinkSnapshot) and slotE8 runs.
 * @param self   The DreamSys.
 * @param sender The grid cell.
 * @param event  The cell's event.
 */
void DreamSys__WallLink(DreamSys *self, void *sender, int event);

/**
 * @brief Sets the dream's time limit and returns the old one.
 * @param self  The DreamSys.
 * @param value The new limit in seconds; a negative value is stored as it is
 *              and never runs out.
 * @return The old limit in seconds, or the old negative value.
 */
s32 DreamSys__GetSetDreamTimeLimit(DreamSys *self, s32 value);

/**
 * @brief Slot +0x198: starts a new game's save data: the save magic, year
 *        and day 0, no unlock scores or flashbacks, the view bob on, and the
 *        navigation challenges cleared.
 * @param self The DreamSys.
 */
void DreamSys__InitNewGame(DreamSys *self);

/**
 * @brief Slot +0x19C: swaps screenShakeOn (the view bob) with `*value`.
 * @param self  The DreamSys.
 * @param value The new setting; receives the old one.
 */
void DreamSys__GetSetScreenShake(DreamSys *self, bool *value);

/**
 * @brief Slot +0x1A4: moves to the next day, into the next year after day
 *        364.
 * @param self The DreamSys.
 * @return The new currentDay, 0..364.
 */
s32 DreamSys__AdvanceDay(DreamSys *self);

/**
 * @brief Slot +0x1B4: begins a dream. Rewinds the clock and the flashbacks
 *        and keeps the day; in a flashback session loads the first
 *        flashback, otherwise starts the mood graph (with a special day's
 *        mood, if it is one) and, on an ordinary day, the day's first spawn.
 * @param self The DreamSys.
 * @return The stage to start on, or -1 on a special day (a cinematic, not
 *         a dream).
 */
s32 DreamSys__StartDay(DreamSys *self);

/**
 * @brief Slot +0x1B8: ends a dream. Restores the day StartDay began (a
 *        flashback moves it); outside a flashback session, outcome 0 scores
 *        the unlock progress, logs the day's mood into moodPreviousDays and
 *        advances the day, and outcome 2 starts a new game.
 * @param self    The DreamSys.
 * @param outcome 0 the dream ended normally, 1 closed, 2 closed into a new
 *                game.
 * @return isFlashbackSession.
 */
s32 DreamSys__EndDay(DreamSys *self, s32 outcome);

/**
 * @brief Slot +0x1BC: the cinematic to play after the dream.
 * @param self The DreamSys.
 * @return nextCinematic; its entry is -1 for none.
 */
CinematicCall DreamSys__GetCinematic(DreamSys *self);

/**
 * @brief Slot +0x1C0: the day's first spawn, chosen from the previous day's
 *        mood (GenerateInitialSpawn): sets currentStage, linkCoordinates and
 *        the time limit, and marks the link DREAMSYS_LINK_DAY_START.
 * @param self The DreamSys.
 */
void DreamSys__InitSpawnLoc(DreamSys *self);

/**
 * @brief Slot +0x1C4: with no link pending, links to a random spawn
 *        (GetRandomSpawnFromStage): on another stage, or on stage
 *        -currentStage when currentStage is negative (an instance link).
 * @param self The DreamSys.
 */
void DreamSys__DynamicLink(DreamSys *self);

/**
 * @brief Slot +0x1C8: with no link pending, links through the static wall
 *        link at `currentPos`, if there is one.
 * @param self       The DreamSys.
 * @param currentPos The wall cell's position.
 * @return true when a link was found.
 */
bool DreamSys__StaticWallLink(DreamSys *self, PlayerSpawnPoint *currentPos);

/**
 * @brief Slot +0x1CC: loads the next stored flashback as the pending link:
 *        its day, stage and position.
 * @param self  The DreamSys.
 * @param quiet false tells the parents (DREAMSYS_LINK_FLASHBACK).
 * @return false when every flashback has been played, true otherwise.
 */
bool DreamSys__LoadNextFlashback(DreamSys *self, bool quiet);

/**
 * @brief Starts a link: records `linkType` in the DreamSys's state and tells
 *        the parents, which may refuse it by clearing the state. An accepted
 *        link sets the stage, restarts a flashback's clock and optionally
 *        plays the link tone.
 * @param system    The DreamSys.
 * @param stage     The destination stage.
 * @param linkType  A DreamSysLinkCode.
 * @param playSound Non-zero plays the link tone.
 * @return true when the parents accepted the link.
 */
bool ExecuteLink(DreamSys *system, s32 stage, s32 linkType, s32 playSound);

/**
 * @brief Slot +0x1E8: what an Entity's instance effect does to the dream,
 *        while no link is pending. SCENENODE_EVENT_LINKED is answered with
 *        the same event; ENTITY_EFFECT_LOG_MOOD logs the entity's mood, adds
 *        its unlock score and may record a flashback; ENTITY_EFFECT_LINK_STAGE
 *        links to the stage it names; ENTITY_EFFECT_EVENT_VIDEO ends the
 *        dream into its event video; ENTITY_EFFECT_END_DREAM ends the dream.
 *        The mood, video and end effects do nothing in a flashback session.
 * @param self   The DreamSys.
 * @param entity The Entity.
 * @param effect The effect.
 */
void DreamSys__InstanceEffectsOnJournal(DreamSys *self, void *entity, s32 effect);

/**
 * @brief Slot +0x1EC: the mood of the days before this one.
 * @param self        The DreamSys.
 * @param target      Where to write it.
 * @param lastDayOnly true: the entry before currentDay (0, 0 before the first
 *                    day); false: the average of every day logged (this
 *                    year's days, or all 365 after the first year).
 */
void DreamSys__GetPreviousDayMood(DreamSys *self, MoodGraphPoint *target, bool lastDayOnly);

/**
 * @brief Slot +0x1F0: clears both mood contributors for a new dream.
 * @param self    The DreamSys.
 * @param special If not NULL, logged into both (a special day's mood).
 */
void DreamSys__InitMoodContributors(DreamSys *self, MoodGraphPoint *special);

/**
 * @brief Slot +0x1F4: logs the mood of the chunk at `currentPos` into
 *        areaMoods.
 * @param self       The DreamSys.
 * @param currentPos The position; its chunk is looked up on currentStage.
 */
void DreamSys__LogChunkMood(DreamSys *self, PlayerSpawnPoint *currentPos);

/**
 * @brief Slot +0x1F8: logs an instance's mood into entityMoods.
 * @param self   The DreamSys.
 * @param source The mood.
 */
void DreamSys__LogInstanceMood(DreamSys *self, MoodGraphPoint *source);

/**
 * @brief Slot +0x1FC: the dream's overall mood so far: the average of the two
 *        contributors' averages (areaMoods' alone while no instance mood is
 *        logged).
 * @param self The DreamSys.
 * @param ret  Where to write the point.
 */
void DreamSys__UpdateDreamChart(DreamSys *self, MoodGraphPoint *ret);

/**
 * @brief Slot +0x200: the colour of the dream's overall mood so far.
 * @param self The DreamSys.
 * @return CalcDreamColor of UpdateDreamChart's point.
 */
DreamColors DreamSys__GetDreamColor(DreamSys *self);

/**
 * @brief The colour of a mood: each axis classed low (< -3), middle or high
 *        (>= 4), and the pair looked up in sDreamColorTable.
 * @param mood The point.
 * @return The colour.
 */
DreamColors CalcDreamColor(MoodGraphPoint *mood);

/**
 * @brief Slot +0x204: empties a contributor.
 * @param self        The DreamSys (unused).
 * @param contributor The contributor to clear.
 */
void DreamSys__ClearMoodGraph(DreamSys *self, MoodGraphContributor *contributor);

/**
 * @brief Slot +0x208: logs a mood into a contributor: it becomes lastMood and
 *        is added to the sums and the count.
 * @param self  The DreamSys (unused).
 * @param layer The contributor.
 * @param mood  The mood.
 */
void DreamSys__LogMood(DreamSys *self, MoodGraphContributor *layer, MoodGraphPoint *mood);

/**
 * @brief Slot +0x20C: a contributor's point: each axis through CalcMoodAxis,
 *        or lastMood when nothing is logged.
 * @param self  The DreamSys (unused).
 * @param layer The contributor.
 * @param ret   Where to write the point, each axis -9..9.
 */
void DreamSys__GetMoodAverage(DreamSys *self, MoodGraphContributor *layer, MoodGraphPoint *ret);

/**
 * @brief One axis of a contributor's point: the mean plus a third of the last
 *        mood, a result past either end wrapping round to the other (10 is
 *        -9, -10 is 9).
 * @param last   The last mood's axis.
 * @param sum    The logged moods' axis sum.
 * @param amount The number logged; not zero.
 * @return The axis, -9..9.
 */
s32 CalcMoodAxis(s32 last, s32 sum, s32 amount);

/**
 * @brief Slot +0x210: recalculates the progress towards unlocking the
 *        flashback feature: the navigation score plus the instance score,
 *        which is first clamped to 0..50000000.
 * @param self The DreamSys.
 */
void DreamSys__CalcUnlockScore(DreamSys *self);

/**
 * @brief Slot +0x214: stores a flashback: appended while fewer than ten are
 *        stored, otherwise over entry tick % 9.
 * @param self    The DreamSys.
 * @param stage   The stage.
 * @param pos     The position in it.
 * @param angles  The orientation, a FlashbackRotation.
 * @param unknown Stored in the entry and never read.
 * @param time    The time limit, in seconds.
 * @param day     The day.
 */
void DreamSys__AddFlashback(DreamSys *self, s32 stage, PlayerSpawnPoint *pos, s32 *angles,
                            s32 unknown, s32 time, s32 day);

/**
 * @brief Slot +0x21C: forgets every stored flashback (the Grey Man's doing).
 * @param self The DreamSys.
 */
void DreamSys__ResetFlashbackList(DreamSys *self);

/**
 * @brief The DreamSys method table.
 * @return &gDreamSysMethods.
 */
DreamSysMethods *GetDreamSysMethods(void);

/**
 * @brief Allocates a DreamSys (0x928 bytes) and runs its ctor through the
 *        table.
 * @param modelSource The LinkResource whose model 0 the DreamSys shows.
 * @param soundObj    The sound bank.
 * @param viewport    The camera.
 * @return The new DreamSys, or NULL when the allocation fails.
 */
DreamSys *New_DreamSys(struct LinkResource *modelSource, struct VabStreamObj *soundObj,
                       struct Viewport *viewport);

/**
 * @brief Clears the navigation challenges and the dynamic-link counter, and
 *        makes them the ones GetStaticSpawn, GetRandomSpawnFromStage and
 *        CalcNavigationScore use.
 * @param arrayMem    The challenge flags.
 * @param linkCounter The dynamic-link counter.
 */
void InitNavChallengesArray(s8 (*arrayMem)[30], s32 *linkCounter);

/**
 * @brief The navigation part of the flashback unlock score: 1000000 per
 *        challenge completed (50000000 for all 30), less 11024 per dynamic
 *        link, and not below 0.
 * @return The score, 0..50000000.
 */
s32 CalcNavigationScore(void);

/**
 * @brief A day's first spawn: the first stage chunk that owns `mood`, and a
 *        spawn point of that stage in that chunk (or one picked by the chunk
 *        when none is); with no stage owning it, a random spawn on stage 1.
 * @param dest      Where to write the spawn.
 * @param timeLimit Where to write the stage's time limit, in seconds.
 * @param mood      The mood to spawn by (the previous day's).
 * @param day       Passed to GetRandomSpawnFromStage, which ignores it.
 * @return The stage.
 */
s32 GenerateInitialSpawn(PlayerSpawnPoint *dest, s32 *timeLimit, MoodGraphPoint *mood, s32 day);

/**
 * @brief A random spawn point, counted as a dynamic link. With `fromStage`
 *        >= 0 the stage is a random one of stages 0..5 other than
 *        `fromStage` (a dynamic link away from it); with `fromStage`
 *        negative it is stage -fromStage (a link onto it, as instances make).
 * @param target    Where to write the spawn.
 * @param fromStage The current stage, or the negated target stage.
 * @param unused    Not read.
 * @return The spawn's stage.
 */
s32 GetRandomSpawnFromStage(PlayerSpawnPoint *target, s32 fromStage, s32 unused);

/**
 * @brief Whether a day is one of the special days; if so, picks one of its
 *        cinematics at random.
 * @param cinematic Where to write the pick; unwritten when the day is not
 *                  special.
 * @param day       The day, from 1.
 * @return The special day's mood, or NULL when the day is not special.
 */
MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day);

/* The occupants of gDreamSysMethods not declared above, in slot order. */

/**
 * @brief Constructor (slot +0x008): Actor's ctor, then the sound bank,
 *        camera and model, no time limit, movement blocked, a new game's
 *        data, and reset.
 * @param self        The object to construct.
 * @param modelSource The LinkResource whose model 0 becomes a child.
 * @param soundObj    The sound bank.
 * @param viewport    The camera.
 * @return Whatever reset leaves as its result (ResetSessionState returns
 *         none); New_DreamSys ignores it.
 */
DreamSys *DreamSys__DreamSys(DreamSys *self, struct LinkResource *modelSource,
                             struct VabStreamObj *soundObj, struct Viewport *viewport);

/**
 * @brief Slot +0x040, reset: hides the object, turns it to face yaw 180, and
 *        clears the tick callbacks, the sound cue set, the staircase walk
 *        and the config option.
 * @param self The DreamSys.
 */
void DreamSys__ResetSessionState(DreamSys *self);

/**
 * @brief Slot +0x04C, attachToParent: joins the grid at linkCoordinates
 *        (loading the chunks around it) and adds it as a child. A flashback
 *        link then takes the flashback's orientation and time limit (plus 4
 *        seconds) and moves to the next flashback; under forced movement the
 *        exit heading is applied.
 * @param self The DreamSys.
 * @param grid The stage's grid.
 */
void DreamSys__SpawnAtLink(DreamSys *self, struct StageMap *grid);

/**
 * @brief Slot +0x050, detachFromParent: disables the grid, removes it as a
 *        child, then Actor's detachFromParent.
 * @param self The DreamSys.
 */
void DreamSys__DetachFromParent(DreamSys *self);

/**
 * @brief Slot +0x088: Actor's notifyWithHull, then the answer to a step's
 *        floor search (Actor__FindNearbyLink). -1, floor found: its cell
 *        picks the footstep sound (voiceSelect), and on stage 9 a stage-timer
 *        link is tried. -2, no floor: a stage-timer link is tried where the
 *        chunk has loaded, otherwise the step is undone.
 * @param self  The DreamSys.
 * @param event The event.
 */
void DreamSys__NotifyLinkAttempt(DreamSys *self, s32 event);

/**
 * @brief Slot +0x094: the pad handler, ignored under a move override, with
 *        movement blocked or on a staircase. Held d-pad up/down step
 *        forward/back and left/right turn; held L2/R2 side-step; held
 *        triangle/square step the view height; held L1/R1 look sideways; held cross
 *        runs while stepping forward; pressing circle sets
 *        linkCommandFlag; releasing up restores the walking speed.
 * @param self   The DreamSys.
 * @param sender The pad (unused).
 * @param event  The pad event plus the button.
 */
void DreamSys__OnPadEvent(DreamSys *self, s32 sender, s32 event);

/**
 * @brief Slot +0x098, update: on each running FrameClock tick, advances the
 *        dream clock and runs the tick state and callbacks; at the time
 *        limit, a flashback session moves to the next flashback (or, with
 *        none left, reports DREAMSYS_TIME_UP), and an ordinary dream may
 *        record a flashback and reports DREAMSYS_TIME_UP.
 * @param self   The DreamSys.
 * @param sender The FrameClock (unused).
 * @param event  The FrameClock event.
 */
void DreamSys__TimerTick(DreamSys *self, s32 sender, s32 event);

/**
 * @brief Slot +0x09C, dispatchLinkCommand: Actor's, then processChunkChange
 *        when the sender is the grid.
 * @param self   The DreamSys.
 * @param sender The sender.
 * @param event  The event.
 */
void DreamSys__DispatchChunkChange(DreamSys *self, void *sender, s32 event);

/**
 * @brief Slot +0x0DC, onActorLinkCommand: Actor's, then
 *        instanceEffectsOnJournal when the sender is an Entity.
 * @param self   The DreamSys.
 * @param sender The sender.
 * @param effect The effect.
 */
void DreamSys__DispatchInstanceEffect(DreamSys *self, void *sender, s32 effect);

/** @brief Slot +0x0E8, slotE8: does nothing. */
void DreamSys__NoOpSlotE8Default(void);

/**
 * @brief Slot +0x0F0: reads the flashback-session flag, and either sets it
 *        or reports the day's colour.
 * @param self  The DreamSys.
 * @param out   With `value` < 0, receives the colour of moodPreviousDays'
 *              entry for currentDay.
 * @param value The new flag, or < 0 to keep it.
 * @return The old flag.
 */
s32 DreamSys__GetSetFlashbackSession(DreamSys *self, DreamColors *out, s32 value);

/**
 * @brief Slot +0x0F4: sets the move override; any override also sets speed 1
 *        and turns the object to enterRotation, if set.
 * @param self  The DreamSys.
 * @param value A DreamSysMoveOverride.
 */
void DreamSys__SetMoveOverride(DreamSys *self, s32 value);

/**
 * @brief Slot +0x0F8: readies the DreamSys on a new stage: logs the spawn
 *        chunk's mood, installs the step-look and move callbacks, sets the
 *        speed and tick period, clears every command, the voice, the pending
 *        link and the staircase walk, unblocks movement, clears the
 *        cinematic, and levels the roll.
 * @param self       The DreamSys.
 * @param moveMode   The speed.
 * @param tickPeriod The tickBoundary period.
 */
void DreamSys__ResetLinkState(DreamSys *self, s32 moveMode, s32 tickPeriod);

/**
 * @brief Slot +0x0FC: blocks movement (movementBlocked = 1).
 * @param self The DreamSys.
 */
void DreamSys__BlockMovement(DreamSys *self);

/**
 * @brief Slot +0x100: linkCommandFlag.
 * @param self The DreamSys.
 * @return Non-zero when circle was pressed this tick.
 */
s32 DreamSys__GetLinkCommandFlag(DreamSys *self);

/**
 * @brief Slot +0x108: the dream clock in seconds.
 * @param self The DreamSys.
 * @return tick / 15.
 */
s32 DreamSys__GetDreamTimerScaled(DreamSys *self);

/**
 * @brief Slot +0x110: sets the camera.
 * @param self  The DreamSys.
 * @param value The viewport.
 */
void DreamSys__SetViewport(DreamSys *self, struct Viewport *value);

/**
 * @brief Slot +0x118: while movement is not blocked, clears linkCommandFlag
 *        and sets tickBoundary to whether tick is a multiple of tickPeriod.
 * @param self The DreamSys.
 */
void DreamSys__UpdateTickState(DreamSys *self);

/**
 * @brief Slot +0x11C: calls the look callback, then the move callback, each
 *        if set.
 * @param self The DreamSys.
 */
void DreamSys__RunTickCallbacks(DreamSys *self);

/**
 * @brief Slot +0x120: the world position `dist` ahead of the object, at the
 *        height of the camera's view line there plus the object's own, and
 *        optionally whether it is near a reference point.
 * @param self      The DreamSys.
 * @param out       Where to write the position (three words), or NULL.
 * @param dist      The distance ahead, along local z.
 * @param reference A position to compare with, or NULL.
 * @param tolerance The largest per-axis difference that counts as near.
 * @return With `reference`, whether the position is within `tolerance` of it
 *         (IsVec3WithinRange); 0 without.
 */
s32 DreamSys__ProjectPointAtDistance(DreamSys *self, s32 *out, s32 dist, s32 *reference, s32 tolerance);

/**
 * @brief Slot +0x124 (never called): clears unk7C.
 * @param self The DreamSys.
 */
void DreamSys__func_59590(DreamSys *self);

/**
 * @brief Slot +0x128 (never called): clears unk78.
 * @param self The DreamSys.
 */
void DreamSys__func_59598(DreamSys *self);

/**
 * @brief Slot +0x12C: does nothing; ApplyMoveCommand calls it before each
 *        step.
 * @param self The DreamSys (unused).
 * @return 0.
 */
s32 DreamSys__NoOpSlot12C(DreamSys *self);

/**
 * @brief Slot +0x130: removes the move callback and, optionally, the look
 *        callback.
 * @param self      The DreamSys.
 * @param clearLook true removes the look callback too.
 */
void DreamSys__ClearTickCallbacks(DreamSys *self, bool clearLook);

/**
 * @brief Slot +0x134: selects the move callback, then the look callback.
 * @param self     The DreamSys.
 * @param moveMode A DreamSysMoveCallback.
 * @param lookMode A DreamSysLookCallback.
 */
void DreamSys__SetTickCallbacks(DreamSys *self, s32 moveMode, s32 lookMode);

/**
 * @brief Slot +0x140, the step-look callback: stepLookOffset, then
 *        stepLookYaw.
 * @param self The DreamSys.
 */
void DreamSys__StepLook(DreamSys *self);

/**
 * @brief Slot +0x144: applies a pending look up/down step to the view height
 *        (refView.vr.y) unless it would pass the step's limit, or, with none
 *        pending, springs the view back towards level by 600.
 * @param self The DreamSys.
 */
void DreamSys__StepLookOffset(DreamSys *self);

/**
 * @brief Slot +0x148: latches whether the player is stepping forward, then
 *        turns the object by a pending sideways look step unless it would
 *        pass the step's limit, or, with none pending, turns back by 45
 *        degrees; after either, FlipMoveCommand.
 * @param self The DreamSys.
 */
void DreamSys__StepLookYaw(DreamSys *self);

/** @brief Slot +0x14C, a look-callback choice: does nothing. */
void DreamSys__NoOpSlot14C(void);

/** @brief Slot +0x150, a look-callback choice: does nothing. */
void DreamSys__NoOpSlot150(void);

/**
 * @brief Slot +0x154, the move callback: by moveOverride, the pending turn
 *        and the free move, the forced move, or the held move.
 * @param self The DreamSys.
 * @return The move's result.
 */
s32 DreamSys__TickMove(DreamSys *self);

/**
 * @brief Slot +0x158: the player's own movement: one tick of the step cycle
 *        with the view bob, then the step.
 * @param self The DreamSys.
 * @return movementBlocked when it is set; otherwise nothing meaningful.
 */
s32 DreamSys__TickMoveFree(DreamSys *self);

/**
 * @brief Slot +0x15C: forced movement: steps forward, without stepping or
 *        bobbing while movement is blocked.
 * @param self The DreamSys.
 * @return AdvanceMoveCycle's result while blocked; otherwise nothing
 *         meaningful.
 */
s32 DreamSys__TickMoveForced(DreamSys *self);

/**
 * @brief Slot +0x160: held movement: sets moveCommand forward and does
 *        nothing else.
 * @param self The DreamSys.
 * @return MOVE_COMMAND_FORWARD.
 */
s32 DreamSys__TickMoveHeld(DreamSys *self);

/**
 * @brief Slot +0x164: one tick of the four-tick step cycle. Plays the
 *        footstep at the end of the step (and mid-step when running),
 *        otherwise stops it; bobs the view down then up when screen shake is
 *        on; ends the step after the fourth tick.
 * @param self The DreamSys.
 * @param bob  Non-zero allows the view bob.
 * @return The step's moveCommand, or 0 when not stepping.
 */
s32 DreamSys__AdvanceMoveCycle(DreamSys *self, s32 bob);

/**
 * @brief Slot +0x168: plays the footstep voiceSelect names (none for 0) at
 *        its pitch; footstep 11 adds two tones, and only footstep 22's voice
 *        is kept in voiceIndex.
 * @param self The DreamSys.
 */
void DreamSys__StartVoice(DreamSys *self);

/**
 * @brief Slot +0x16C: stops the kept voice, if any.
 * @param self The DreamSys.
 */
void DreamSys__StopVoice(DreamSys *self);

/**
 * @brief Slot +0x170: takes a step: tries a staircase, a teleporter and a
 *        tunnel link at the player's position, and with none, saves the
 *        coordinate and moves by the step's signed speed (which finds the
 *        floor and may link). On stage 0, a position with y < -2000 and
 *        x >= -499 is then treated as a wall hit (onGridCellLinkCommand,
 *        event 4).
 * @param self    The DreamSys.
 * @param command A DreamSysMoveCommand; 0 does nothing.
 * @return Nothing meaningful.
 */
s32 DreamSys__ApplyMoveCommand(DreamSys *self, s32 command);

/**
 * @brief Slot +0x174: applies a pending turn (6 degrees left or right).
 * @param self The DreamSys.
 */
void DreamSys__ApplyPendingTurn(DreamSys *self);

/**
 * @brief Slot +0x178, the drift move callback: while drifting, moves the
 *        object by sDriftStep and lowers the view's refView.vr.y by 600 a
 *        tick; while cues
 *        are active, services the sound cue set.
 * @param self The DreamSys.
 */
void DreamSys__TickDrift(DreamSys *self);

/**
 * @brief Slot +0x17C: stops drifting.
 * @param self     The DreamSys.
 * @param keepCues Non-zero keeps servicing the cue set, flushing it first.
 */
void DreamSys__StopDrift(DreamSys *self, s32 keepCues);

/**
 * @brief Slot +0x180: reads the speed and optionally sets it (and the
 *        previous speed).
 * @param self  The DreamSys.
 * @param value The new speed, or < 0 to keep it.
 * @return The old speed.
 */
s32 DreamSys__GetSetMoveMode(DreamSys *self, s32 value);

/**
 * @brief Slot +0x184: changes the speed, keeping the old one as the
 *        previous speed when it differs.
 * @param self  The DreamSys.
 * @param value The new speed.
 */
void DreamSys__ChangeMoveMode(DreamSys *self, s32 value);

/**
 * @brief Slot +0x188: goes back to the previous speed.
 * @param self The DreamSys.
 */
void DreamSys__RestorePreviousMoveMode(DreamSys *self);

/**
 * @brief Slot +0x18C: sets tickBoundary and the three gate flags, each only
 *        when its argument is >= 0.
 * @param self The DreamSys.
 * @param a    tickBoundary.
 * @param b    gateFlags[0].
 * @param c    gateFlags[1].
 * @param d    gateFlags[2].
 */
void DreamSys__SetGateFlags(DreamSys *self, s32 a, s32 b, s32 c, s32 d);

/**
 * @brief Slot +0x190: sets tickPeriod.
 * @param self  The DreamSys.
 * @param value The period, in ticks; not zero.
 */
void DreamSys__SetTickPeriod(DreamSys *self, s32 value);

/**
 * @brief Slot +0x194, the drift's cue callback: for a tag-1 set, gives cue
 *        slot 0 (every 20th tick) or slot 1 (otherwise) program 9, one
 *        octave down.
 * @param owner The DreamSys (unused).
 * @param set   The cue set.
 */
void DreamSys__SoundCueCallback(void *owner, SoundCueSet *set);

/**
 * @brief Slot +0x1A0: the current day and year.
 * @param self    The DreamSys.
 * @param outYear Where to write the year, or NULL.
 * @return The day of the year, from 1.
 */
s32 DreamSys__GetCurrentDayAndYear(DreamSys *self, s32 *outYear);

/**
 * @brief Slot +0x1A8: clears newGamePending.
 * @param self The DreamSys.
 */
void DreamSys__ClearNewGameFlag(DreamSys *self);

/**
 * @brief Slot +0x1AC: newGamePending.
 * @param self The DreamSys.
 * @return Non-zero after the ctor or a new-game EndDay, until
 *         ClearNewGameFlag.
 */
s32 DreamSys__GetNewGameFlag(DreamSys *self);

/**
 * @brief Slot +0x1B0: the save block (TitleMenu and GraphRoom read it).
 * @param self    The DreamSys.
 * @param outSize Where to write its size, DREAMSYS_SAVE_SIZE, or NULL.
 * @return &saveMagic (DreamSaveBlock's view).
 */
s32 *DreamSys__GetSaveBlock(DreamSys *self, s32 *outSize);

/**
 * @brief Slot +0x1D0: with no link pending, links through the tunnel at
 *        `currentPos`, when the player faces its way in while stepping
 *        forward.
 * @param self       The DreamSys.
 * @param currentPos The player's position.
 * @return true when the tunnel link was started.
 */
bool DreamSys__TryTunnelLink(DreamSys *self, PlayerSpawnPoint *currentPos);

/**
 * @brief Slot +0x1D4: with no link pending, takes the stage's timer link
 *        (TestForStageTransition), if there is one here.
 * @param self       The DreamSys.
 * @param currentPos The player's position.
 * @return true when the link was started.
 */
bool DreamSys__TryStageTimerLink(DreamSys *self, PlayerSpawnPoint *currentPos);

/**
 * @brief Slot +0x1D8: takes the instant teleporter at `currentPos`: an
 *        accepted teleport moves the object to the destination cell at once,
 *        clears the pending link and, outside a flashback session, adds the
 *        time bonus.
 * @param self       The DreamSys.
 * @param currentPos The player's position.
 * @return true when a teleporter is here.
 */
bool DreamSys__TryInstantTeleportLink(DreamSys *self, PlayerSpawnPoint *currentPos);

/**
 * @brief Slot +0x1DC: runs the staircase walk: a walk in progress takes its
 *        next tick (and, when done, ends and leaves running speed); otherwise
 *        a staircase at `currentPos`, entered facing its way while stepping
 *        forward, starts one.
 * @param self       The DreamSys.
 * @param currentPos The player's position.
 * @return false.
 */
bool DreamSys__TryStaircaseLink(DreamSys *self, PlayerSpawnPoint *currentPos);

/**
 * @brief Slot +0x1E0: currentStage.
 * @param self The DreamSys.
 * @return The stage.
 */
s32 DreamSys__GetCurrentStage(DreamSys *self);

/**
 * @brief Slot +0x218: one time in three, records the present moment as a
 *        flashback (stage, position, orientation, day).
 * @param self      The DreamSys.
 * @param unknown   Stored in the entry and never read.
 * @param timeLimit The flashback's time limit, in seconds.
 */
void DreamSys__FlashbackSaving(DreamSys *self, s32 unknown, s32 timeLimit);

/**
 * @brief Slot +0x220: keeps the coordinate and its parameters, so a step can
 *        be undone.
 * @param self The DreamSys.
 */
void DreamSys__SaveLinkSnapshot(DreamSys *self);

/**
 * @brief Slot +0x224: puts back the coordinate SaveLinkSnapshot kept (undoing
 *        the step) and marks it for recalculation.
 * @param self The DreamSys.
 */
void DreamSys__RestoreLinkSnapshot(DreamSys *self);

/**
 * @brief Slot +0x228: reads configOption and optionally sets it.
 * @param self  The DreamSys.
 * @param value The new value, or < 0 to keep it.
 * @return The old value.
 */
s32 DreamSys__GetSetConfigOption(DreamSys *self, s32 value);

/**
 * @brief Moves the object by (a - b) with y forced to 0 (addTranslation):
 *        a staircase tick's first move onto its path.
 * @param self The DreamSys.
 * @param a    The target relative position.
 * @param b    The position to move from.
 */
void DreamSys__ApplyRelativeOffset(DreamSys *self, struct RelativePos *a, struct RelativePos *b);

/**
 * @brief Enables or disables the instant teleporters TestForInstantTeleporters
 *        tests (dream_aux.c's SetTeleportsEnabled sets it per stage).
 * @param value true enables them.
 */
void SetInstantTeleportersEnabled(bool value);

#endif
