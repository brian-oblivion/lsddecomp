/* DreamSys's methods (include/dream_sys.h has the class), in four groups,
 * followed by the free functions the link tests are built from:
 *
 * 1. Construction and reset: New_DreamSys, the ctor, ResetSessionState,
 *    ResetLinkState, SpawnAtLink.
 * 2. The per-tick chain: TimerTick, the tick callbacks (StepLook, TickMove,
 *    TickDrift) and the step they drive (AdvanceMoveCycle, StartVoice /
 *    StopVoice, ApplyMoveCommand).
 * 3. Day, mood and flashback bookkeeping: StartDay/EndDay, the mood graph
 *    (two MoodGraphContributor accumulators that UpdateDreamChart averages
 *    and CalcDreamColor turns into a DreamColors value),
 *    AddFlashback/FlashbackSaving, CalcUnlockScore, and GetSaveBlock, which
 *    hands out the DREAMSYS_SAVE_SIZE bytes from saveMagic that InitNewGame
 *    initializes.
 * 4. Linking: WallLink/DynamicLink and the Try...Link family, over the free
 *    TestFor.../GetStaticSpawn testers and the stage spawn tables. ExecuteLink
 *    records the link (enum DreamSysLinkCode) in Actor's `state` and tells
 *    the parents. */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <memory.h>
#include <rand.h>
#include "dream_sys.h"
#include "entity.h"
#include "pad.h"
#include "frame_clock.h"
#include "link_resource.h"
#include "stage_map.h"
#include "lbd_file.h"
#include "vab_stream_obj.h"
#include "viewport.h"
#include "bmem_pmgr.h"

extern s32 sLinkSrcStage;
extern s32 sLinkTriggerIndex;
extern s32 sLinkDstStage;
extern s32 sLinkSpawnIndex;

/* The number of stages: every per-stage table below has one entry each. */

/* A trigger tile whose value is negative: any tile of the chunk triggers. */
#define ANY_TILE {{255, 255}}

/* Defined below and named only by the tables: five of DreamSys's slots and
 * the staircase walks sStaircaseTickFns lists. */
void DreamSys__SetSoundObj(DreamSys *self, VabStreamObj *value);
void DreamSys__SetEtcTim(DreamSys *self, struct TimImage *value);
void DreamSys__SelectLookCallback(DreamSys *self, s32 mode);
void DreamSys__SelectMoveCallback(DreamSys *self, s32 mode);
void DreamSys__ProcessChunkChange(DreamSys *self, void *entity, s32 effect);
s32 DreamSys__TickStaircaseYawPlus90(DreamSys *self);
s32 DreamSys__TickStaircaseYawMinus135(DreamSys *self);
s32 DreamSys__TickStaircaseYawPlus45(DreamSys *self);
s32 DreamSys__TickStaircaseYawMinus90(DreamSys *self);

/* DreamSys's data, in address order. A method-table slot whose function is
 * declared for another class's `self` takes a `void *` cast. */

/* DreamSys's method table, class id 0x1F34: Actor's slots, with DreamSys's
 * overrides, then its own from +0x0F0. */
/* clang-format off */
DreamSysMethods gDreamSysMethods = {
    /* +0x000 header */ 0x1F34,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)DreamSys__DreamSys,
    /* +0x00C finalize */ (void *)SceneNode__Finalize,
    /* +0x010 addChild */ (void *)Actor__AddChild,
    /* +0x014 removeChild */ (void *)Actor__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)Actor__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)SceneNode__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ DreamSys__ResetSessionState,
    /* +0x044 updateRotation */ (void *)SceneNode__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)DreamSys__SpawnAtLink,
    /* +0x050 detachFromParent */ (void *)DreamSys__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)SceneNode__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)SceneNode__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)SceneNode__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ DreamSys__NotifyLinkAttempt,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)DreamSys__OnPadEvent,
    /* +0x098 update */ (void *)DreamSys__TimerTick,
    /* +0x09C dispatchLinkCommand */ DreamSys__DispatchChunkChange,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ NULL,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 setTranslation */ (void *)Actor__SetTranslation,
    /* +0x0BC addTranslation */ (void *)Actor__AddTranslation,
    /* +0x0C0 addLocalTranslation */ (void *)Actor__AddLocalTranslation,
    /* +0x0C4 moveLocalZ */ (void *)Actor__MoveLocalZ,
    /* +0x0C8 moveLocalX */ (void *)Actor__MoveLocalX,
    /* +0x0CC moveLocalY */ (void *)Actor__MoveLocalY,
    /* +0x0D0 moveLocalZOrFindLink */ (void *)Actor__MoveLocalZOrFindLink,
    /* +0x0D4 moveLocalXOrFindLink */ (void *)Actor__MoveLocalXOrFindLink,
    /* +0x0D8 slotD8 */ Actor__NoOpSlotD8,
    /* +0x0DC onActorLinkCommand */ DreamSys__DispatchInstanceEffect,
    /* +0x0E0 onGridCellLinkCommand */ DreamSys__WallLink,
    /* +0x0E4 setLastOffsetValue */ (void *)Actor__SetLastOffsetValue,
    /* +0x0E8 onLinkUpdate */ (void *)DreamSys__OnLinkUpdate,
    /* +0x0EC setPendingExtra */ (void *)Actor__SetPendingExtra,
    /* +0x0F0 getSetFlashbackSession */ DreamSys__GetSetFlashbackSession,
    /* +0x0F4 setMoveOverride */ DreamSys__SetMoveOverride,
    /* +0x0F8 resetLinkState */ DreamSys__ResetLinkState,
    /* +0x0FC blockMovement */ DreamSys__BlockMovement,
    /* +0x100 getLinkCommandFlag */ DreamSys__GetLinkCommandFlag,
    /* +0x104 getSetDreamTimeLimit */ DreamSys__GetSetDreamTimeLimit,
    /* +0x108 getDreamTimerScaled */ DreamSys__GetDreamTimerScaled,
    /* +0x10C setSoundObj */ DreamSys__SetSoundObj,
    /* +0x110 setViewport */ DreamSys__SetViewport,
    /* +0x114 setEtcTim */ DreamSys__SetEtcTim,
    /* +0x118 updateTickState */ DreamSys__UpdateTickState,
    /* +0x11C runTickCallbacks */ DreamSys__RunTickCallbacks,
    /* +0x120 projectPointAtDistance */ DreamSys__ProjectPointAtDistance,
    /* +0x124 clearUnusedFlag7C */ DreamSys__ClearUnusedFlag7C,
    /* +0x128 clearUnusedFlag78 */ DreamSys__ClearUnusedFlag78,
    /* +0x12C beforeMoveCommand */ DreamSys__BeforeMoveCommand,
    /* +0x130 clearTickCallbacks */ DreamSys__ClearTickCallbacks,
    /* +0x134 setTickCallbacks */ DreamSys__SetTickCallbacks,
    /* +0x138 selectLookCallback */ DreamSys__SelectLookCallback,
    /* +0x13C selectMoveCallback */ DreamSys__SelectMoveCallback,
    /* +0x140 stepLook */ DreamSys__StepLook,
    /* +0x144 stepLookOffset */ DreamSys__StepLookOffset,
    /* +0x148 stepLookYaw */ DreamSys__StepLookYaw,
    /* +0x14C noOpLook2 */ (void *)DreamSys__NoOpLook2,
    /* +0x150 noOpLook3 */ (void *)DreamSys__NoOpLook3,
    /* +0x154 tickMove */ DreamSys__TickMove,
    /* +0x158 tickMoveFree */ DreamSys__TickMoveFree,
    /* +0x15C tickMoveForced */ DreamSys__TickMoveForced,
    /* +0x160 tickMoveHeld */ DreamSys__TickMoveHeld,
    /* +0x164 advanceMoveCycle */ DreamSys__AdvanceMoveCycle,
    /* +0x168 startVoice */ DreamSys__StartVoice,
    /* +0x16C stopVoice */ DreamSys__StopVoice,
    /* +0x170 applyMoveCommand */ DreamSys__ApplyMoveCommand,
    /* +0x174 applyPendingTurn */ DreamSys__ApplyPendingTurn,
    /* +0x178 tickDrift */ DreamSys__TickDrift,
    /* +0x17C stopDrift */ DreamSys__StopDrift,
    /* +0x180 getSetMoveMode */ DreamSys__GetSetMoveMode,
    /* +0x184 changeMoveMode */ DreamSys__ChangeMoveMode,
    /* +0x188 restorePreviousMoveMode */ DreamSys__RestorePreviousMoveMode,
    /* +0x18C setGateFlags */ DreamSys__SetGateFlags,
    /* +0x190 setTickPeriod */ DreamSys__SetTickPeriod,
    /* +0x194 soundCueCallback */ DreamSys__SoundCueCallback,
    /* +0x198 initNewGame */ DreamSys__InitNewGame,
    /* +0x19C getSetScreenShake */ DreamSys__GetSetScreenShake,
    /* +0x1A0 getCurrentDayAndYear */ DreamSys__GetCurrentDayAndYear,
    /* +0x1A4 advanceDay */ DreamSys__AdvanceDay,
    /* +0x1A8 clearNewGameFlag */ DreamSys__ClearNewGameFlag,
    /* +0x1AC getNewGameFlag */ DreamSys__GetNewGameFlag,
    /* +0x1B0 getSaveBlock */ DreamSys__GetSaveBlock,
    /* +0x1B4 startDay */ DreamSys__StartDay,
    /* +0x1B8 endDay */ DreamSys__EndDay,
    /* +0x1BC getCinematic */ DreamSys__GetCinematic,
    /* +0x1C0 initSpawnLoc */ DreamSys__InitSpawnLoc,
    /* +0x1C4 dynamicLink */ DreamSys__DynamicLink,
    /* +0x1C8 staticWallLink */ DreamSys__StaticWallLink,
    /* +0x1CC loadNextFlashback */ DreamSys__LoadNextFlashback,
    /* +0x1D0 tryTunnelLink */ DreamSys__TryTunnelLink,
    /* +0x1D4 tryStageTimerLink */ DreamSys__TryStageTimerLink,
    /* +0x1D8 tryInstantTeleportLink */ DreamSys__TryInstantTeleportLink,
    /* +0x1DC tryStaircaseLink */ DreamSys__TryStaircaseLink,
    /* +0x1E0 getCurrentStage */ DreamSys__GetCurrentStage,
    /* +0x1E4 processChunkChange */ DreamSys__ProcessChunkChange,
    /* +0x1E8 instanceEffectsOnJournal */ DreamSys__InstanceEffectsOnJournal,
    /* +0x1EC getPreviousDayMood */ DreamSys__GetPreviousDayMood,
    /* +0x1F0 initMoodContributors */ DreamSys__InitMoodContributors,
    /* +0x1F4 logChunkMood */ DreamSys__LogChunkMood,
    /* +0x1F8 logInstanceMood */ DreamSys__LogInstanceMood,
    /* +0x1FC updateDreamChart */ DreamSys__UpdateDreamChart,
    /* +0x200 getDreamColor */ DreamSys__GetDreamColor,
    /* +0x204 clearMoodGraph */ DreamSys__ClearMoodGraph,
    /* +0x208 logMood */ DreamSys__LogMood,
    /* +0x20C getMoodAverage */ DreamSys__GetMoodAverage,
    /* +0x210 calcUnlockScore */ DreamSys__CalcUnlockScore,
    /* +0x214 addFlashback */ DreamSys__AddFlashback,
    /* +0x218 flashbackSaving */ DreamSys__FlashbackSaving,
    /* +0x21C resetFlashbackList */ DreamSys__ResetFlashbackList,
    /* +0x220 saveLinkSnapshot */ DreamSys__SaveLinkSnapshot,
    /* +0x224 restoreLinkSnapshot */ DreamSys__RestoreLinkSnapshot,
    /* +0x228 getSetConfigOption */ DreamSys__GetSetConfigOption,
};
/* clang-format on */

/* clang-format off */
/* (0, 180, 0) degrees: DreamSys__ResetSessionState's absolute
 * updateRotation. */
Ratio16 sRotationYaw180[3] = {{0, 1}, {180, 1}, {0, 1}};

/* CalcDreamColor's 3 x 3 table, [dynamicClass][upperClass], each axis
 * classed 0..2 first. */
s8 sDreamColorTable[9] = {6, 7, 2, 4, 7, 1, 3, 0, 5};

/* DreamSys__ApplyMoveCommand's tables: sMoveCommandSigns[command] times
 * sMoveModeSpeeds[moveMode] is the signed step, and
 * sMoveCommandDispatch[command] (Actor +0x0D0/+0x0D4) takes it. Command 0
 * returns before reaching them. */
s32 sMoveModeSpeeds[5] = {0, 24, 64, 128, 384};
s8 sMoveCommandSigns[8] = {0, 1, -1, -1, 1, 0, 0, 0};
void (*sMoveCommandDispatch[5])(DreamSys *self, s32 val, void *extra) = {
    NULL, (void *)Actor__MoveLocalZOrFindLink, (void *)Actor__MoveLocalZOrFindLink,
    (void *)Actor__MoveLocalXOrFindLink, (void *)Actor__MoveLocalXOrFindLink,
};

/* The look steps and their limits: DreamSys__StepLookOffset's, indexed by
 * lookOffsetCommand, and DreamSys__StepLookYaw's, by lookYawCommand. Index 0
 * is unused; 1 and 2 are the negative and positive step. */
s32 sLookOffsetSteps[3] = {0, -600, 600};
s32 sLookOffsetLimits[3] = {0, 9000, 9000};
s32 sLookYawSteps[3] = {0, -45, 45};
s32 sLookYawLimits[3] = {0, 181, 181};

/* The relative turns: row 0 is StepLookYaw's, whose yaw numerator it
 * overwrites with its step before each use; rows 1 and 2, (0, -6, 0) and
 * (0, 6, 0), are DreamSys__ApplyPendingTurn's for turnCommand 1 and 2. */
Ratio16 sTurnRotations[3][3] = {
    {{0, 1}, {45, 1}, {0, 1}},
    {{0, 1}, {-6, 1}, {0, 1}},
    {{0, 1}, {6, 1}, {0, 1}},
};

/* DreamSys__TickDrift's per-tick addTranslation (+0x0BC) step. */
LongVec3 sDriftStep = {0, 512, 0};

/* Indexed by voiceSelect (0..0x17, DreamSys__NotifyLinkAttempt bounds
 * it): DreamSys__StartVoice plays program sVoiceBySelect[voiceSelect] at
 * octave offset sVoicePitchBySelect[voiceSelect]. */
s8 sVoiceBySelect[0x18] = {0, 3, 29, 18, 25, 18, 28, 29, 3, 30, 30, 3, 3, 21, 16, 16, 10, 28, 18, 6, 13, 19, 12, 30};
s8 sVoicePitchBySelect[0x18] = {0, 0, -1, -2, 2, -1, 0, 0, -1, 0, 0, -1, 1, 0, 2, -2, 2, 2, -1, 1, 2, 2, 1, -2};

/* DreamSys__ProjectPointAtDistance's local offset (0, 0, dist): it writes
 * z and hands the vector to SceneNode__LocalOffsetToWorldPos. */
LongVec3 sProjectOffset = {0, 0, 0};

/* The staircase walks, indexed by GetLastSpawnExtra() and stored in
 * staircaseTickFn by DreamSys__TryStaircaseLink. */
s32 (*sStaircaseTickFns[4])(DreamSys *self) = {
    DreamSys__TickStaircaseYawPlus90, DreamSys__TickStaircaseYawMinus135,
    DreamSys__TickStaircaseYawPlus45, DreamSys__TickStaircaseYawMinus90,
};

/* (0, +45, 0) and (0, -45, 0) degrees: the staircase walks' relative
 * updateRotation. */
Ratio16 sRotationYawPlus45[3] = {{0, 1}, {45, 1}, {0, 1}};
Ratio16 sRotationYawMinus45[3] = {{0, 1}, {-45, 1}, {0, 1}};

/* Each stage's time limit, TryStageTimerLink's. */
s16 sStageTimeLimits[STAGE_COUNT] = {240, 180, 480, 420, 120, 600, 60, 300, 240, 300, 240, 180, 120, 480};

/* The positions relative to a tile a spawn lands at, by
 * StageSpawn::adjustment. */
struct RelativePos sSpawnPosAdjust[37] = {
    /*  0 */ {0, 0, 0},
    /*  1 */ {0, -2048, 0},
    /*  2 */ {0, -4096, 0},
    /*  3 */ {0, -6144, 0},
    /*  4 */ {0, -8192, 0},
    /*  5 */ {0, -512, 0},
    /*  6 */ {0, -1024, 0},
    /*  7 */ {0, -1536, 0},
    /*  8 */ {0, -3072, 0},
    /*  9 */ {0, -10240, 0},
    /* 10 */ {0, -2048, 0},
    /* 11 */ {-1024, 512, 0},
    /* 12 */ {1024, 0, 0},
    /* 13 */ {1024, 0, 1024},
    /* 14 */ {-1024, 0, 1024},
    /* 15 */ {-256, 0, 0},
    /* 16 */ {0, -380, 0},
    /* 17 */ {0, -700, 0},
    /* 18 */ {-256, 0, 1024},
    /* 19 */ {-256, 0, 128},
    /* 20 */ {0, 0, -1024},
    /* 21 */ {0, -768, 0},
    /* 22 */ {-896, -7311, -393},
    /* 23 */ {0, 0, 256},
    /* 24 */ {512, 0, 160},
    /* 25 */ {0, -2048, 1280},
    /* 26 */ {0, -4096, 1280},
    /* 27 */ {0, -6144, 1280},
    /* 28 */ {0, -8192, 1024},
    /* 29 */ {0, -256, 0},
    /* 30 */ {0, -2489, 0},
    /* 31 */ {0, 3728, 0},
    /* 32 */ {0, 512, 0},
    /* 33 */ {0, -4800, 0},
    /* 34 */ {0, 1024, 0},
    /* 35 */ {0, 2048, 0},
    /* 36 */ {0, -14336, 0},
};

/* Each stage's spawn points, GenerateInitialSpawn's and
 * GetRandomSpawnFromStage's: a chunk, a tile in it, an sSpawnPosAdjust
 * index and the extra byte. */
StageSpawn sStg00SpawnPoints[8] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 6,  3},  0,  0},
    {{ 0,  0}, { 1,  8},  0,  0},
    {{ 0,  0}, { 6,  8},  0,  0},
    {{ 0,  1}, { 7,  9},  1,  0},
    {{ 0,  2}, { 5,  8},  2,  0},
    {{ 0,  3}, { 7,  9},  3,  0},
    {{ 0,  3}, { 4,  8},  3,  0},
    {{ 0,  4}, { 4,  8},  4,  0},
};
StageSpawn sStg01SpawnPoints[3] = {
    /* chunk      tile     adj extra */
    {{ 2,  1}, {12,  9},  0,  0},
    {{ 1,  0}, {12, 14},  0,  0},
    {{ 1,  1}, {18, 15},  0,  0},
};
StageSpawn sStg02SpawnPoints[30] = {
    /* chunk      tile     adj extra */
    {{ 0,  1}, {19,  0},  0,  0},
    {{ 1,  0}, {18,  4},  0,  0},
    {{ 2,  0}, {11, 16},  0,  0},
    {{ 4,  0}, { 4,  8},  0,  0},
    {{ 1,  1}, {11, 14},  0,  0},
    {{ 2,  1}, {11, 19},  0,  0},
    {{ 2,  1}, { 4, 14},  0,  0},
    {{ 3,  1}, {18,  9},  0,  0},
    {{ 4,  1}, {18, 19},  0,  0},
    {{ 0,  2}, {15,  6},  0,  0},
    {{ 1,  2}, {15, 12},  0,  0},
    {{ 2,  2}, {11, 19},  0,  0},
    {{ 2,  2}, {16, 11},  0,  0},
    {{ 3,  2}, {14,  6},  0,  0},
    {{ 5,  2}, { 2,  7},  0,  0},
    {{ 0,  3}, {13, 19},  0,  0},
    {{ 1,  3}, {19,  1}, 29,  0},
    {{ 2,  3}, { 2, 19}, 29,  0},
    {{ 3,  3}, {13,  3},  0,  0},
    {{ 3,  3}, { 7,  8},  0,  0},
    {{ 0,  4}, {10, 13}, 29,  0},
    {{ 2,  4}, {11,  6},  0,  0},
    {{ 3,  4}, {14,  8},  0,  0},
    {{ 4,  4}, {14,  8},  0,  0},
    {{ 4,  4}, {13,  0},  0,  0},
    {{ 2,  5}, {16, 13},  0,  0},
    {{ 3,  5}, { 1, 16},  0,  0},
    {{ 4,  5}, { 6,  0},  6,  0},
    {{ 1,  2}, {12, 19}, 29,  0},
    {{ 4,  0}, { 2, 19},  0,  0},
};
StageSpawn sStg03SpawnPoints[123] = {
    /* chunk      tile     adj extra */
    {{ 2,  0}, { 9,  9},  0,  0},
    {{ 3,  0}, { 6, 10},  0,  0},
    {{ 6,  0}, {13, 13},  1,  0},
    {{13,  0}, { 3, 16},  5,  0},
    {{ 4,  1}, { 9,  1},  0,  0},
    {{ 5,  1}, {10,  9},  1,  0},
    {{ 6,  1}, {16, 19},  0,  0},
    {{ 7,  1}, { 6, 15},  0,  0},
    {{10,  1}, { 7, 18},  6,  0},
    {{11,  1}, { 7,  0},  0,  0},
    {{ 5,  2}, { 8, 18},  0,  0},
    {{ 6,  2}, { 9, 12},  0,  0},
    {{ 7,  2}, { 0, 19},  0,  0},
    {{ 8,  2}, {15, 13},  0,  0},
    {{12,  2}, { 7,  5},  0,  0},
    {{14,  2}, { 5, 10}, 30,  0},
    {{ 3,  3}, {19, 10},  0,  0},
    {{ 4,  3}, {12, 10},  0,  0},
    {{ 5,  3}, {19, 19},  0,  0},
    {{ 7,  3}, { 4, 12}, 30,  0},
    {{ 8,  3}, { 5,  5},  0,  0},
    {{ 9,  3}, {15,  6},  0,  0},
    {{10,  3}, { 9, 19},  0,  0},
    {{11,  3}, { 9, 16},  0,  0},
    {{12,  3}, {15, 12},  1,  0},
    {{13,  3}, { 0, 10},  6,  0},
    {{15,  3}, { 6, 16},  0,  0},
    {{ 4,  4}, { 7,  4},  0,  0},
    {{ 5,  4}, { 2,  0},  0,  0},
    {{ 6,  4}, { 8,  0},  0,  0},
    {{ 7,  4}, {14,  6},  0,  0},
    {{ 8,  4}, {17, 17},  0,  0},
    {{ 9,  4}, { 0,  5},  0,  0},
    {{10,  4}, { 9, 13},  0,  0},
    {{11,  4}, {19, 19},  0,  0},
    {{12,  4}, { 4,  0},  0,  0},
    {{13,  4}, {15, 14},  7,  0},
    {{ 1,  5}, {16, 15},  0,  0},
    {{ 2,  5}, {13, 16},  6,  0},
    {{ 3,  5}, { 8, 15},  6,  0},
    {{ 4,  5}, { 3, 18},  0,  0},
    {{ 5,  5}, { 8, 12},  5,  0},
    {{ 7,  5}, {15,  8},  0,  0},
    {{ 8,  5}, {13, 10},  1,  0},
    {{ 9,  5}, {14, 19},  0,  0},
    {{11,  5}, {19, 18},  2,  0},
    {{12,  5}, { 4, 14},  2,  0},
    {{14,  5}, { 8,  6},  0,  0},
    {{ 2,  6}, {10,  8},  0,  0},
    {{ 3,  6}, {13, 13},  0,  0},
    {{ 4,  6}, {16,  4},  0,  0},
    {{ 6,  6}, { 9, 10},  6,  0},
    {{ 8,  6}, {13, 14},  0,  0},
    {{ 9,  6}, { 7,  4}, 31,  0},
    {{10,  6}, { 1,  5},  0,  0},
    {{11,  6}, {18, 12},  2,  0},
    {{12,  6}, { 2,  7},  2,  0},
    {{14,  6}, {10, 11},  0,  0},
    {{ 2,  7}, {11,  8},  0,  0},
    {{ 3,  7}, {10,  0},  0,  0},
    {{ 4,  7}, {12, 16},  0,  0},
    {{ 5,  7}, { 3,  9},  0,  0},
    {{ 6,  7}, {19,  1},  0,  0},
    {{ 7,  7}, { 6, 15},  0,  0},
    {{ 8,  7}, {16,  2},  0,  0},
    {{ 9,  7}, { 9, 15},  5,  0},
    {{10,  7}, {13,  4},  0,  0},
    {{11,  7}, {11, 15},  0,  0},
    {{12,  7}, { 6,  9},  0,  0},
    {{13,  7}, {11, 12},  0,  0},
    {{14,  7}, { 6, 19},  0,  0},
    {{ 2,  8}, { 1,  7},  0,  0},
    {{ 3,  8}, { 2,  1},  0,  0},
    {{ 4,  8}, {13, 12},  0,  0},
    {{ 6,  8}, { 3,  5},  0,  0},
    {{ 9,  8}, { 1, 11},  0,  0},
    {{10,  8}, {18, 14},  0,  0},
    {{ 2,  9}, {13,  7},  0,  0},
    {{ 4,  9}, {11, 13},  0,  0},
    {{ 5,  9}, { 1,  3},  0,  0},
    {{ 6,  9}, {12,  3},  0,  0},
    {{ 7,  9}, {17, 19},  0,  0},
    {{10,  9}, {18, 12},  0,  0},
    {{ 1, 10}, {19, 19},  0,  0},
    {{ 3, 10}, {19, 13},  0,  0},
    {{ 4, 10}, { 5, 15},  0,  0},
    {{ 5, 10}, {10, 13},  0,  0},
    {{ 9, 10}, { 4, 12},  0,  0},
    {{10, 10}, { 9,  5},  0,  0},
    {{11, 10}, { 7, 17},  0,  0},
    {{ 1, 11}, {13, 10}, 32,  0},
    {{ 2, 11}, {16,  3},  0,  0},
    {{ 4, 11}, { 8, 11},  7,  0},
    {{ 5, 11}, { 5, 12},  0,  0},
    {{ 7, 11}, { 2, 19},  0,  0},
    {{11, 11}, { 7, 11},  0,  0},
    {{ 1, 12}, {19, 11},  0,  0},
    {{ 2, 12}, {19, 19},  0,  0},
    {{ 3, 12}, { 1, 19},  0,  0},
    {{ 4, 12}, { 9, 18},  0,  0},
    {{ 5, 12}, { 8, 18},  0,  0},
    {{ 6, 12}, {13, 11},  3,  0},
    {{ 6, 12}, { 5,  7},  0,  0},
    {{ 9, 12}, {11, 17},  0,  0},
    {{10, 12}, { 8,  6},  0,  0},
    {{ 2, 13}, { 9, 12},  0,  0},
    {{ 3, 13}, {15, 14},  0,  0},
    {{ 4, 13}, { 9,  1},  0,  0},
    {{ 5, 13}, {12, 11},  2,  0},
    {{ 6, 13}, {10, 10},  1,  0},
    {{ 7, 13}, {15,  1},  8,  0},
    {{ 8, 13}, { 9,  9},  1,  0},
    {{ 9, 13}, {16, 17},  0,  0},
    {{10, 13}, { 1,  0},  0,  0},
    {{11, 13}, {17,  7},  0,  0},
    {{13, 13}, {15, 18},  0,  0},
    {{14, 13}, { 9,  6},  0,  0},
    {{ 4, 14}, {12,  5},  1,  0},
    {{ 6, 14}, { 4, 13},  0,  0},
    {{ 9, 14}, {14,  8},  0,  0},
    {{13, 14}, {15, 12},  0,  0},
    {{14, 15}, { 5,  6},  0,  0},
    {{ 9,  4}, { 8, 11}, 21,  0},
};
StageSpawn sStg04SpawnPoints[20] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 2,  3},  0,  0},
    {{ 1,  0}, {10, 19},  0,  0},
    {{ 2,  0}, { 3, 15},  0,  0},
    {{ 4,  0}, {15, 19},  0,  0},
    {{ 1,  1}, {10, 16},  6,  0},
    {{ 3,  1}, { 0,  3},  0,  0},
    {{ 4,  1}, {12, 14},  0,  0},
    {{ 1,  2}, { 4,  9},  0,  0},
    {{ 2,  2}, {10,  8}, 34,  0},
    {{ 3,  2}, { 9,  2},  0,  0},
    {{ 4,  2}, { 9,  8},  0,  0},
    {{ 0,  3}, {14,  8},  0,  0},
    {{ 1,  3}, {10, 11},  0,  0},
    {{ 2,  3}, { 5, 11},  0,  0},
    {{ 5,  3}, { 5,  9}, 16,  0},
    {{ 0,  4}, { 2,  8},  0,  0},
    {{ 1,  4}, {16, 16},  0,  0},
    {{ 2,  4}, {13,  1},  0,  0},
    {{ 3,  4}, { 0, 11},  0,  0},
    {{ 4,  4}, { 4,  5},  0,  0},
};
StageSpawn sStg05SpawnPoints[21] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, {18, 10},  1,  0},
    {{ 1,  0}, {16, 12},  0,  0},
    {{ 2,  0}, { 9, 16},  0,  0},
    {{ 3,  0}, {19, 19},  0,  0},
    {{ 1,  1}, {16, 18},  0,  0},
    {{ 2,  1}, { 2, 19},  0,  0},
    {{ 4,  1}, { 0, 14},  5,  0},
    {{ 0,  2}, {19, 19},  1,  0},
    {{ 1,  2}, {14, 10},  5,  0},
    {{ 3,  2}, { 9,  2},  0,  0},
    {{ 1,  3}, {10, 12}, 32,  0},
    {{ 3,  3}, { 3, 14},  0,  0},
    {{ 4,  3}, { 5, 18},  0,  0},
    {{ 1,  4}, {10, 19},  0,  0},
    {{ 2,  4}, { 9,  5},  0,  0},
    {{ 3,  4}, {11, 17},  0,  0},
    {{ 4,  4}, { 4,  9},  0,  0},
    {{ 2,  5}, { 2,  4},  0,  0},
    {{ 3,  5}, { 9,  2},  6,  0},
    {{ 4,  1}, { 0, 14}, 32,  0},
    {{ 3,  1}, {18, 16}, 36,  0},
};
StageSpawn sStg06SpawnPoints[2] = {
    {{ 0,  0}, { 8,  7},  0,  0},
    {{ 0,  5}, { 9,  6},  9,  0},
};
StageSpawn sStg07SpawnPoints[5] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 6, 13},  0,  0},
    {{ 1,  0}, {10,  8},  0,  0},
    {{ 2,  0}, {18, 12},  0,  0},
    {{ 3,  0}, {12, 13}, 32,  0},
    {{ 4,  0}, { 5, 10},  0,  0},
};
StageSpawn sStg08SpawnPoints[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 9,  2},  0,  0},
    {{ 0,  1}, {18, 12},  0,  0},
    {{ 0,  2}, {10,  5},  0,  0},
};
StageSpawn sStg09SpawnPoints[4] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 3, 17},  0,  0},
    {{ 0,  1}, {13,  2},  0,  0},
    {{ 0,  1}, {13,  8}, 34,  0},
    {{ 0,  0}, { 4, 12}, 35,  0},
};
StageSpawn sStg10SpawnPoints[5] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 1,  9},  0,  0},
    {{ 1,  0}, { 6,  9},  0,  0},
    {{ 2,  0}, {10,  9},  0,  0},
    {{ 2,  0}, {14, 10}, 34,  0},
    {{ 1,  0}, { 6,  9},  5,  0},
};
StageSpawn sStg11SpawnPoints[4] = {
    /* chunk      tile     adj extra */
    {{ 2,  1}, { 8,  9},  0,  0},
    {{ 1,  1}, { 9,  8},  0,  0},
    {{ 2,  1}, { 8,  9}, 34,  0},
    {{ 3,  1}, { 1,  8},  6,  0},
};
StageSpawn sStg12SpawnPoints[8] = {
    /* chunk      tile     adj extra */
    {{ 1,  1}, { 9,  5},  0,  0},
    {{ 2,  1}, { 4, 10},  6,  0},
    {{ 0,  2}, {16,  6},  6,  0},
    {{ 1,  2}, { 9,  8},  6,  0},
    {{ 2,  2}, { 6,  9},  0,  0},
    {{ 1,  3}, { 4, 10},  6,  0},
    {{ 2,  3}, { 8, 17},  0,  0},
    {{ 1,  2}, { 9,  8}, 10,  0},
};
StageSpawn sStg13SpawnPoints[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 9,  7},  0,  0},
    {{ 0,  1}, {14, 16},  0,  0},
    {{ 1,  1}, { 0,  1},  0,  0},
};
StageSpawn *sStageSpawnPoints[STAGE_COUNT] = {
    sStg00SpawnPoints, sStg01SpawnPoints, sStg02SpawnPoints, sStg03SpawnPoints,
    sStg04SpawnPoints, sStg05SpawnPoints, sStg06SpawnPoints, sStg07SpawnPoints,
    sStg08SpawnPoints, sStg09SpawnPoints, sStg10SpawnPoints, sStg11SpawnPoints,
    sStg12SpawnPoints, sStg13SpawnPoints,
};

/* MATCHING: u8; as s8, GenerateInitialSpawn's loop guard gains a `blez` retail does not have. */
u8 sStageSpawnPointsCount[STAGE_COUNT] = {8, 3, 30, 123, 20, 21, 2, 5, 3, 4, 5, 4, 8, 3};

/* The fixed links, TestForStaticLink's GetStaticSpawn tables: per stage,
 * the spawns, then the triggers (a chunk, a tile or ANY_TILE, and the
 * destination stage and spawn), then their counts. The same shape serves
 * the tunnels, teleporters and staircases below. */
StageSpawn sStg00PermalinkSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  2}, { 4,  9},  2,  0},
    {{ 0,  2}, { 7,  9},  2,  1},
    {{ 0,  2}, { 6,  9},  2,  2},
};
StageSpawn sStg02PermalinkSpawns[2] = {
    {{ 4,  3}, { 8, 14},  0,  3},
    {{ 3,  1}, { 2, 19},  0,  4},
};
StageSpawn sStg03PermalinkSpawns[2] = {
    {{ 6,  4}, { 2,  3},  0,  5},
    {{ 7, 13}, {10,  9}, 22,  6},
};
StageSpawn sStg04PermalinkSpawns[1] = {
    {{ 5,  3}, {11,  9}, 16,  7},
};
StageSpawn sStg05PermalinkSpawns[1] = {
    {{ 1,  3}, {17, 10},  0,  8},
};
StageSpawn sStg06PermalinkSpawns[1] = {
    {{ 0,  0}, { 8,  7},  0,  9},
};
StageSpawn sStg09PermalinkSpawns[1] = {
    {{ 0,  0}, { 3, 17},  0, 10},
};
StageSpawn sStg10PermalinkSpawns[1] = {
    {{ 0,  0}, {11,  9},  0, 11},
};
StageSpawn sStg11PermalinkSpawns[1] = {
    {{ 1,  1}, { 9,  9},  0, 12},
};
StageSpawn sStg12PermalinkSpawns[1] = {
    {{ 1,  2}, {10, 12},  0, 13},
};
StageSpawn *sStagePermalinkSpawns[STAGE_COUNT] = {
    sStg00PermalinkSpawns, NULL, sStg02PermalinkSpawns, sStg03PermalinkSpawns,
    sStg04PermalinkSpawns, sStg05PermalinkSpawns, sStg06PermalinkSpawns, NULL,
    NULL, sStg09PermalinkSpawns, sStg10PermalinkSpawns, sStg11PermalinkSpawns,
    sStg12PermalinkSpawns, NULL,
};
StaticLinkTrigger sStg00PermalinkTriggers[3] = {
    /* chunk     tile         stage                      spawn */
    {{ 0,  2}, {{  4,   9}}, STAGE_LONG_HALLWAY,        0},
    {{ 0,  2}, {{  7,   8}}, STAGE_KYOTO,               0},
    {{ 0,  2}, {{  6,   8}}, STAGE_NATURAL_WORLD,       0},
};
StaticLinkTrigger sStg02PermalinkTriggers[2] = {
    {{ 2,  1}, ANY_TILE,     STAGE_MOONLIGHT_TOWER,     0},
    {{ 5,  3}, {{  2,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 1},
};
StaticLinkTrigger sStg03PermalinkTriggers[1] = {
    {{ 6,  4}, {{  1,   2}}, STAGE_BRIGHT_MOON_COTTAGE, 2},
};
StaticLinkTrigger sStg04PermalinkTriggers[4] = {
    /* chunk     tile         stage                      spawn */
    {{ 2,  1}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
    {{ 3,  1}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
    {{ 4,  1}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
    {{ 4,  2}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
};
StaticLinkTrigger sStg05PermalinkTriggers[2] = {
    {{ 1,  3}, ANY_TILE,     STAGE_CLOCKWORK_MACHINES,  0},
    {{ 2,  3}, ANY_TILE,     STAGE_CLOCKWORK_MACHINES,  0},
};
StaticLinkTrigger sStg06PermalinkTriggers[1] = {
    {{ 0,  5}, {{  5,   5}}, STAGE_KYOTO,               1},
};
StaticLinkTrigger sStg09PermalinkTriggers[1] = {
    {{ 0,  1}, {{ 13,  14}}, STAGE_VIOLENCE_DISTRICT,   0},
};
StaticLinkTrigger sStg10PermalinkTriggers[1] = {
    {{ 0,  0}, {{  0,  10}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
};
StaticLinkTrigger sStg11PermalinkTriggers[1] = {
    {{ 1,  1}, {{  9,   9}}, STAGE_HAPPY_TOWN,          0},
};
StaticLinkTrigger sStg12PermalinkTriggers[1] = {
    {{ 1,  2}, {{ 10,  14}}, STAGE_NATURAL_WORLD,       1},
};
StaticLinkTrigger *sStagePermalinkTriggers[STAGE_COUNT] = {
    sStg00PermalinkTriggers, NULL, sStg02PermalinkTriggers, sStg03PermalinkTriggers,
    sStg04PermalinkTriggers, sStg05PermalinkTriggers, sStg06PermalinkTriggers, NULL,
    NULL, sStg09PermalinkTriggers, sStg10PermalinkTriggers, sStg11PermalinkTriggers,
    sStg12PermalinkTriggers, NULL,
};
s8 sStagePermalinkTriggersCount[STAGE_COUNT] = {3, 0, 2, 1, 4, 2, 1, 0, 0, 1, 1, 1, 1, 0};

/* The four cardinal rotations, yaw 0, 90, 180 and 270 degrees.
 * CheckTunnelHeading and CheckStaircaseHeading store an entry's address in
 * enterRotation / exitRotation, which SetMoveOverride, SpawnAtLink and
 * TryStaircaseLink apply. */
Ratio16 sCardinalRotations[4][3] = {
    {{0, 1}, {0, 1}, {0, 1}},
    {{0, 1}, {90, 1}, {0, 1}},
    {{0, 1}, {180, 1}, {0, 1}},
    {{0, 1}, {270, 1}, {0, 1}},
};

/* The tunnels, TestForTunnelLinks' tables, with a heading per spawn and per
 * trigger (an sCardinalRotations index): the player must face the
 * trigger's to take the tunnel (DreamSys__CheckTunnelHeading) and leaves
 * facing the spawn's. */
StageSpawn sStg00TunnelSpawns[1] = {
    {{ 0,  0}, { 8,  0}, 15, 14},
};
u8 sStg00TunnelExitHeadings[1] = {0};
StageSpawn sStg01TunnelSpawns[2] = {
    {{ 1,  0}, { 8,  0}, 18, 15},
    {{ 1,  1}, {18, 19}, 20, 16},
};
u8 sStg01TunnelExitHeadings[2] = {0, 2};
StageSpawn sStg02TunnelSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 2,  0}, {10,  0}, 19, 17},
    {{ 3,  5}, {18, 17},  0, 18},
    {{ 4,  2}, { 8,  6}, 12, 19},
};
u8 sStg02TunnelExitHeadings[3] = {0, 3, 2};
StageSpawn sStg03TunnelSpawns[5] = {
    /* chunk      tile     adj extra */
    {{ 7,  8}, { 8,  6}, 12, 20},
    {{ 3,  9}, { 8,  6}, 12, 21},
    {{13,  6}, { 9,  6}, 12, 22},
    {{11, 12}, { 8,  6}, 12, 23},
    {{ 3,  4}, { 7,  6}, 12, 24},
};
u8 sStg03TunnelExitHeadings[5] = {2, 2, 2, 2, 2};
StageSpawn sStg04TunnelSpawns[1] = {
    {{ 2,  0}, { 9,  0}, 23, 25},
};
u8 sStg04TunnelExitHeadings[1] = {0};
StageSpawn sStg05TunnelSpawns[1] = {
    {{ 2,  0}, { 8,  0}, 24, 26},
};
u8 sStg05TunnelExitHeadings[1] = {0};
StageSpawn sStg07TunnelSpawns[1] = {
    {{ 0,  0}, { 9,  0}, 23, 27},
};
u8 sStg07TunnelExitHeadings[1] = {0};
StageSpawn sStg08TunnelSpawns[1] = {
    {{ 0,  0}, { 9,  0}, 23, 28},
};
u8 sStg08TunnelExitHeadings[1] = {0};
StageSpawn sStg13TunnelSpawns[1] = {
    {{ 0,  0}, { 8,  0}, 18, 29},
};
u8 sStg13TunnelExitHeadings[1] = {0};
StageSpawn *sTunnelSpawns[STAGE_COUNT] = {
    sStg00TunnelSpawns, sStg01TunnelSpawns, sStg02TunnelSpawns, sStg03TunnelSpawns,
    sStg04TunnelSpawns, sStg05TunnelSpawns, NULL, sStg07TunnelSpawns,
    sStg08TunnelSpawns, NULL, NULL, NULL,
    NULL, sStg13TunnelSpawns,
};
u8 *sTunnelExitHeadings[STAGE_COUNT] = {
    sStg00TunnelExitHeadings, sStg01TunnelExitHeadings, sStg02TunnelExitHeadings, sStg03TunnelExitHeadings,
    sStg04TunnelExitHeadings, sStg05TunnelExitHeadings, NULL, sStg07TunnelExitHeadings,
    sStg08TunnelExitHeadings, NULL, NULL, NULL,
    NULL, sStg13TunnelExitHeadings,
};
StaticLinkTrigger sStg00TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       0},
    {{ 0,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       0},
};
u8 sStg00TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg01TunnelTriggers[4] = {
    /* chunk     tile         stage                      spawn */
    {{ 1,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       1},
    {{ 1,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       1},
    {{ 1,  1}, {{ 18,  17}}, STAGE_FLESH_TUNNELS,       0},
    {{ 1,  1}, {{ 19,  17}}, STAGE_FLESH_TUNNELS,       0},
};
u8 sStg01TunnelEnterHeadings[4] = {2, 2, 0, 0};
StaticLinkTrigger sStg02TunnelTriggers[6] = {
    /* chunk     tile         stage                      spawn */
    {{ 2,  0}, {{ 10,   1}}, STAGE_NATURAL_WORLD,       2},
    {{ 2,  0}, {{ 11,   1}}, STAGE_NATURAL_WORLD,       2},
    {{ 3,  5}, {{ 14,  16}}, STAGE_TEMPLE_DOJO,         0},
    {{ 3,  5}, {{ 14,  17}}, STAGE_TEMPLE_DOJO,         0},
    {{ 4,  2}, {{  8,   8}}, STAGE_MONUMENT_PARK,       0},
    {{ 4,  2}, {{  9,   8}}, STAGE_MONUMENT_PARK,       0},
};
u8 sStg02TunnelEnterHeadings[6] = {2, 2, 1, 1, 0, 0};
StaticLinkTrigger sStg03TunnelTriggers[10] = {
    /* chunk     tile         stage                      spawn */
    {{ 7,  8}, {{  8,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
    {{ 7,  8}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
    {{ 3,  9}, {{  8,   8}}, STAGE_PIT_AND_TEMPLE,      0},
    {{ 3,  9}, {{  9,   8}}, STAGE_PIT_AND_TEMPLE,      0},
    {{13,  6}, {{  9,   8}}, STAGE_KYOTO,               0},
    {{13,  6}, {{ 10,   8}}, STAGE_KYOTO,               0},
    {{11, 12}, {{  8,   8}}, STAGE_HAPPY_TOWN,          0},
    {{11, 12}, {{  9,   8}}, STAGE_HAPPY_TOWN,          0},
    {{ 3,  4}, {{  7,   8}}, STAGE_VIOLENCE_DISTRICT,   0},
    {{ 3,  4}, {{  8,   8}}, STAGE_VIOLENCE_DISTRICT,   0},
};
u8 sStg03TunnelEnterHeadings[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
StaticLinkTrigger sStg04TunnelTriggers[2] = {
    {{ 2,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       3},
    {{ 2,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       3},
};
u8 sStg04TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg05TunnelTriggers[2] = {
    {{ 2,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       4},
    {{ 2,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       4},
};
u8 sStg05TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg07TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   3}}, STAGE_KYOTO,               1},
    {{ 0,  0}, {{  9,   3}}, STAGE_KYOTO,               1},
};
u8 sStg07TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg08TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   1}}, STAGE_PIT_AND_TEMPLE,      1},
    {{ 0,  0}, {{  9,   1}}, STAGE_PIT_AND_TEMPLE,      1},
};
u8 sStg08TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg13TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   1}}, STAGE_KYOTO,               2},
    {{ 0,  0}, {{  9,   1}}, STAGE_KYOTO,               2},
};
u8 sStg13TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger *sTunnelTriggers[STAGE_COUNT] = {
    sStg00TunnelTriggers, sStg01TunnelTriggers, sStg02TunnelTriggers, sStg03TunnelTriggers,
    sStg04TunnelTriggers, sStg05TunnelTriggers, NULL, sStg07TunnelTriggers,
    sStg08TunnelTriggers, NULL, NULL, NULL,
    NULL, sStg13TunnelTriggers,
};
u8 *sTunnelEnterHeadings[STAGE_COUNT] = {
    sStg00TunnelEnterHeadings, sStg01TunnelEnterHeadings, sStg02TunnelEnterHeadings, sStg03TunnelEnterHeadings,
    sStg04TunnelEnterHeadings, sStg05TunnelEnterHeadings, NULL, sStg07TunnelEnterHeadings,
    sStg08TunnelEnterHeadings, NULL, NULL, NULL,
    NULL, sStg13TunnelEnterHeadings,
};
s8 sTunnelTriggersCount[STAGE_COUNT] = {2, 4, 6, 10, 2, 2, 0, 2, 2, 0, 0, 0, 0, 2};

/* The instant teleporters, TestForInstantTeleporters' tables. */
StageSpawn sStg02TeleportSpawns[9] = {
    /* chunk      tile     adj extra */
    {{ 1,  1}, { 4,  8},  0,  0},
    {{ 1,  0}, { 7, 14},  0,  0},
    {{ 4,  1}, { 1,  3},  0,  0},
    {{ 4,  3}, { 3, 10}, 10,  0},
    {{ 3,  4}, {16,  3}, 11,  0},
    {{ 2,  3}, { 2, 16},  0,  0},
    {{ 4,  4}, { 0, 18},  0,  0},
    {{ 2,  1}, { 4, 14},  0,  0},
    {{ 5,  1}, { 1, 14},  0,  0},
};
StageSpawn sStg03TeleportSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 9,  4}, { 9, 18}, 21,  0},
    {{ 9,  4}, { 9,  3}, 21,  0},
    {{ 7,  4}, { 7,  0},  0,  0},
};
StageSpawn sStg05TeleportSpawns[5] = {
    /* chunk      tile     adj extra */
    {{ 4,  1}, { 0, 14},  5,  0},
    {{ 3,  3}, { 8, 10},  0,  0},
    {{ 1,  2}, {13, 14},  0,  0},
    {{ 2,  2}, { 9, 19},  0,  0},
    {{ 1,  2}, {13,  5},  0,  0},
};
StageSpawn sStg08TeleportSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 9,  5}, 17,  0},
    {{ 0,  1}, {11, 12},  0,  0},
    {{ 0,  1}, {17,  3},  0,  0},
};
StageSpawn *sTeleportSpawns[STAGE_COUNT] = {
    NULL, NULL, sStg02TeleportSpawns, sStg03TeleportSpawns,
    NULL, sStg05TeleportSpawns, NULL, NULL,
    sStg08TeleportSpawns, NULL, NULL, NULL,
    NULL, NULL,
};
StaticLinkTrigger sStg02TeleportTriggers[8] = {
    /* chunk     tile         stage                      spawn */
    {{ 1,  1}, {{  4,  10}}, STAGE_KYOTO,               0},
    {{ 0,  0}, {{  7,  17}}, STAGE_KYOTO,               1},
    {{ 4,  1}, {{  7,   0}}, STAGE_KYOTO,               2},
    {{ 4,  3}, {{  4,  18}}, STAGE_KYOTO,               3},
    {{ 1,  2}, {{  1,  14}}, STAGE_KYOTO,               5},
    {{ 2,  1}, {{ 13,  14}}, STAGE_KYOTO,               7},
    {{ 4,  4}, {{ 13,   3}}, STAGE_KYOTO,               6},
    {{ 4,  1}, {{ 11,   0}}, STAGE_KYOTO,               8},
};
StaticLinkTrigger sStg03TeleportTriggers[1] = {
    {{ 7,  4}, {{  0,   5}}, STAGE_NATURAL_WORLD,       2},
};
StaticLinkTrigger sStg05TeleportTriggers[5] = {
    /* chunk     tile         stage                      spawn */
    {{ 3,  1}, {{ 18,  16}}, STAGE_VIOLENCE_DISTRICT,   0},
    {{ 2,  3}, {{ 11,  11}}, STAGE_VIOLENCE_DISTRICT,   1},
    {{ 0,  2}, {{ 18,  10}}, STAGE_VIOLENCE_DISTRICT,   2},
    {{ 2,  2}, {{  9,  16}}, STAGE_VIOLENCE_DISTRICT,   3},
    {{ 1,  2}, {{ 13,  16}}, STAGE_VIOLENCE_DISTRICT,   4},
};
StaticLinkTrigger sStg08TeleportTriggers[3] = {
    /* chunk     tile         stage                      spawn */
    {{ 0,  0}, {{  8,  11}}, STAGE_FLESH_TUNNELS,       0},
    {{ 0,  0}, {{  9,   4}}, STAGE_FLESH_TUNNELS,       1},
    {{ 0,  1}, {{ 17,   8}}, STAGE_FLESH_TUNNELS,       2},
};
StaticLinkTrigger *sTeleportTriggers[STAGE_COUNT] = {
    NULL, NULL, sStg02TeleportTriggers, sStg03TeleportTriggers,
    NULL, sStg05TeleportTriggers, NULL, NULL,
    sStg08TeleportTriggers, NULL, NULL, NULL,
    NULL, NULL,
};
s8 sTeleportTriggersCount[STAGE_COUNT] = {0, 0, 8, 1, 0, 5, 0, 0, 3, 0, 0, 0, 0, 0};

/* The staircases, TestForStaircaseNodes' tables (Bright Moon Cottage only), with
 * headings as for the tunnels (DreamSys__CheckStaircaseHeading). */
StageSpawn sStg00StaircaseSpawns[8] = {
    /* chunk      tile     adj extra */
    {{ 0,  1}, { 8,  7}, 25,  0},
    {{ 0,  2}, { 8,  7}, 26,  0},
    {{ 0,  3}, { 8,  7}, 27,  0},
    {{ 0,  4}, { 9,  7}, 28,  2},
    {{ 0,  0}, { 9,  9},  0,  1},
    {{ 0,  1}, { 9,  9},  1,  1},
    {{ 0,  2}, { 9,  9},  2,  1},
    {{ 0,  3}, { 9,  9},  3,  3},
};
u8 sStg00StaircaseExitHeadings[8] = {0, 0, 0, 3, 3, 3, 3, 3};
StageSpawn *sStaircaseSpawns[STAGE_COUNT] = {
    sStg00StaircaseSpawns, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
u8 *sStaircaseExitHeadings[STAGE_COUNT] = {
    sStg00StaircaseExitHeadings, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
StaticLinkTrigger sStg00StaircaseTriggers[8] = {
    /* chunk     tile         stage                      spawn */
    {{ 0,  0}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
    {{ 0,  1}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 1},
    {{ 0,  2}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 2},
    {{ 0,  3}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 3},
    {{ 0,  1}, {{  8,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 4},
    {{ 0,  2}, {{  8,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 5},
    {{ 0,  3}, {{  8,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 6},
    {{ 0,  4}, {{  8,   7}}, STAGE_BRIGHT_MOON_COTTAGE, 7},
};
u8 sStg00StaircaseEnterHeadings[8] = {2, 2, 2, 2, 2, 2, 2, 1};
StaticLinkTrigger *sStaircaseTriggers[STAGE_COUNT] = {
    sStg00StaircaseTriggers, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
u8 *sStaircaseEnterHeadings[STAGE_COUNT] = {
    sStg00StaircaseEnterHeadings, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
s8 sStaircaseTriggersCount[STAGE_COUNT] = {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/* IsDaySpecial's table of special days. */
s16 sSpecialDays[42] = {2, 15, 43, 81, 120, 126, 202, 259, 267, 284, 308, 328, 358, 7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 77, 84, 91, 98, 105, 112, 119, 126, 134, 141, 148, 155, 162, 169, 176, 183, 190, 197, 204};
/* clang-format on */

/* A `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a` argument
   by DreamSys__TickStaircaseYawPlus45. */
extern struct RelativePos sStaircaseOffset2;

/* The same, for DreamSys__TickStaircaseYawPlus90. */
extern struct RelativePos sStaircaseOffset0;

/* The same, for DreamSys__TickStaircaseYawMinus135. */
extern struct RelativePos sStaircaseOffset1;

/* The same, for DreamSys__TickStaircaseYawMinus90. */
extern struct RelativePos sStaircaseOffset3;

/* The fixed "special day" mood, returned by IsDaySpecial on a match;
   only its address is used. */
extern MoodGraphPoint sSpecialDayMood;

/* With no look command pending, StepLookOffset springs the view height back
 * towards 0 by this much a tick (the size of one sLookOffsetSteps step), and
 * StepLookYaw turns back by this many degrees (one sLookYawSteps step). */
#define LOOK_OFFSET_RETURN_STEP 600
#define LOOK_YAW_RETURN_STEP 45

/* A move command lasts this many ticks (AdvanceMoveCycle); the view bobs
 * down by MOVE_BOB_HEIGHT on the first two and back up on the last two. */
#define MOVE_CYCLE_TICKS 4
#define MOVE_BOB_HEIGHT 50

/* A mood axis runs -9..9; CalcMoodAxis wraps a result past either end round
 * to the other. */
#define MOOD_AXIS_MAX 9

/* Flashback unlock scoring (CalcUnlockScore, CalcNavigationScore): each
 * completed navigation challenge is worth NAV_CHALLENGE_SCORE, completing
 * all of them jumps to UNLOCK_SCORE_MAX (also the cap on the instance
 * score), and every dynamic link costs DYNAMIC_LINK_PENALTY. */
#define NAV_CHALLENGE_SCORE 1000000
#define UNLOCK_SCORE_MAX 50000000
#define DYNAMIC_LINK_PENALTY 11024

/* The volume, vol and endVol, of every tone StartVoice and ExecuteLink play
 * through the VabStreamObj's playTone. */
#define DREAMSYS_TONE_VOLUME 110

/* Defined further down, in ROM order, and called before that. */
s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
s32 TestForTunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
void DreamSys__FlipMoveCommand(DreamSys *self);

DreamSys *New_DreamSys(LinkResource *modelSource, VabStreamObj *soundObj, Viewport *viewport) {
    DreamSys *self;

    self = BMemPMgrAlloc(sizeof(DreamSys));
    if (self != NULL) {
        GetDreamSysMethods()->ctor(self, modelSource, soundObj, viewport);
        return self;
    }
    return NULL;
}

void DreamSys__DreamSys(DreamSys *self, LinkResource *modelSource, VabStreamObj *soundObj,
                        Viewport *viewport) {
    void *model;

    GetActorMethods()->ctor((Actor *)self);
    self->methods = GetDreamSysMethods();
    self->soundObj = soundObj;
    self->viewport = viewport;
    self->etcTim = NULL;
    self->modelSource = modelSource;
    model = modelSource->methods->getModel(modelSource, 0);
    self->methods->addChild(self, model);
    self->methods->getSetDreamTimeLimit(self, -1);
    self->movementBlocked = 1;
    self->moveOverride = MOVE_OVERRIDE_NONE;
    self->newGamePending = 1;
    self->methods->initNewGame(self);
    self->methods->reset(self);
}

void DreamSys__ResetSessionState(DreamSys *self) {
    self->methods->setDisplay(self, 0);
    self->methods->updateRotation(self, 1, sRotationYaw180);
    self->lookCallback = NULL;
    self->moveCallback = NULL;
    self->soundCueSet.tag = 0;
    self->staircaseActive = 0;
    self->staircaseMoveGate = 0;
    self->staircaseTickFn = 0;
    self->unusedFlag78 = 0;
    self->configOption = 0;
}

void DreamSys__SpawnAtLink(DreamSys *self, StageMap *grid) {
    s32 attachPos[4];

    grid->methods->setTargetAndLoadChunks(grid, attachPos, (SceneNode *)self,
                                          (Descriptor10 *)&self->linkCoordinates);
    GetActorMethods()->attachToParent((Actor *)self, (SceneNode *)grid, (LongVec3 *)attachPos);
    self->methods->addChild(self, (BasicClass *)grid);
    if (self->state == DREAMSYS_LINK_FLASHBACK) {
        FlashbackEntry *entry = &self->storedFlashbacks[self->currentFlashbackIndex];
        self->methods->updateRotation(self, 1, &entry->rotation);
        self->methods->getSetDreamTimeLimit(self, entry->timeLimit + 4);
        self->currentFlashbackIndex++;
    }
    if (self->moveOverride != MOVE_OVERRIDE_NONE && self->exitRotation != 0) {
        self->methods->updateRotation(self, 1, (void *)self->exitRotation);
    }
}

void DreamSys__DetachFromParent(DreamSys *self) {
    self->grid->methods->disable(self->grid);
    self->methods->removeChild(self, (BasicClass *)self->grid);
    GetActorMethods()->detachFromParent((Actor *)self);
}

void DreamSys__NotifyLinkAttempt(DreamSys *self, s32 event) {
    s32 voice;

    GetActorMethods()->notifyWithHull((Actor *)self, event);
    switch (event) {
        case ACTOR_EVENT_FLOOR_FOUND:
            voice = self->linkTarget->flags36 & 0x7F;
            self->voiceSelect = voice;
            if (voice >= ARRAY_COUNT(sVoiceBySelect))
                self->voiceSelect = 0;

            if (self->state == DREAMSYS_LINK_TUNNEL && self->voiceSelect == 0)
                self->voiceSelect = 2;

            if (self->currentStage == STAGE_CLOCKWORK_MACHINES) {
                self->methods->tryStageTimerLink(
                    self,
                    (PlayerSpawnPoint *)self->grid->methods->getTargetDescriptor(self->grid, 0, 0));
            }
            break;
        case ACTOR_EVENT_NO_FLOOR:
            if (self->grid->methods
                    ->findSlotForPosition(self->grid, (LongVec3 *)self->coord2->coord.t)
                    ->loader->headerReady == LBDFILE_HEADER_CONSUMED) {
                self->methods->tryStageTimerLink(
                    self,
                    (PlayerSpawnPoint *)self->grid->methods->getTargetDescriptor(self->grid, 0, 0));
            } else {
                self->methods->restoreLinkSnapshot(self);
            }
            break;
    }
}

void DreamSys__OnPadEvent(DreamSys *self, s32 sender, s32 event) {
    if (self->moveOverride != MOVE_OVERRIDE_NONE)
        return;
    if (self->movementBlocked != 0)
        return;
    if (self->staircaseActive != 0)
        return;

    switch (event) {
        case PAD_EVENT_HELD + PAD_BUTTON_LUP:
            self->moveCommand = MOVE_COMMAND_FORWARD;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_LDOWN:
            self->moveCommand = MOVE_COMMAND_BACK;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_LLEFT:
            self->turnCommand = TURN_COMMAND_LEFT;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_LRIGHT:
            self->turnCommand = TURN_COMMAND_RIGHT;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_RUP:
            self->lookOffsetCommand = LOOK_OFFSET_COMMAND_UP;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_RDOWN:
            if (self->moveCommand == MOVE_COMMAND_FORWARD)
                self->methods->changeMoveMode(self, MOVE_MODE_RUN);
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_RLEFT:
            self->lookOffsetCommand = LOOK_OFFSET_COMMAND_DOWN;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_R1:
            self->lookYawCommand = LOOK_YAW_COMMAND_RIGHT;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_R2:
            self->moveCommand = MOVE_COMMAND_RIGHT;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_L1:
            self->lookYawCommand = LOOK_YAW_COMMAND_LEFT;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_L2:
            self->moveCommand = MOVE_COMMAND_LEFT;
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RRIGHT:
            self->linkCommandFlag = 1;
            break;
        case PAD_EVENT_RELEASED + PAD_BUTTON_LUP:
            self->methods->restorePreviousMoveMode(self);
            break;
        case PAD_EVENT_RELEASED + PAD_BUTTON_START: /* MATCHING: empty; retail's jump table runs to it */
            break;
    }
}

void DreamSys__TimerTick(DreamSys *self, s32 sender, s32 event) {
    u32 old;

    if (event != FRAMECLOCK_EVENT_RUNNING)
        return;

    old = self->tick;
    self->tick = old + 1;
    if (old >= self->dreamTimeLimit) {
        if (self->isFlashbackSession) {
            if (self->state != DREAMSYS_NO_LINK || self->methods->loadNextFlashback(self, 0)) {
                /* MATCHING: a barrier; without it this `tick = 0` jumps to the copy after notifyParents */
                __asm__("");
                self->tick = 0;
                return;
            }
        } else {
            self->methods->flashbackSaving(self, 0, 16);
        }
        self->methods->notifyParents(self, DREAMSYS_TIME_UP);
        self->tick = 0;
    } else {
        self->methods->updateTickState(self);
        self->methods->runTickCallbacks(self);
    }
}

void DreamSys__DispatchChunkChange(DreamSys *self, void *sender, s32 event) {
    GetActorMethods()->dispatchLinkCommand((Actor *)self, sender, event);
    if ((((BasicClass *)sender)->methods->header & CLASS_ID_LEVEL3_MASK) == STAGEMAP_CLASS_ID) {
        self->methods->processChunkChange(self, sender, event);
    }
}

void DreamSys__DispatchInstanceEffect(DreamSys *self, void *sender, s32 effect) {
    GetActorMethods()->onActorLinkCommand((Actor *)self, sender, effect);
    if ((((BasicClass *)sender)->methods->header & CLASS_ID_LEVEL5_MASK) == ENTITY_CLASS_ID) {
        self->methods->instanceEffectsOnJournal(self, sender, effect);
    }
}

void DreamSys__WallLink(DreamSys *self, void *sender, int event) {
    GetActorMethods()->onGridCellLinkCommand((Actor *)self, sender, event);
    if (event != SCENENODE_EVENT_LINKED)
        return;
    if (self->state != DREAMSYS_NO_LINK)
        return;
    self->linkCoordinates =
        *(PlayerSpawnPoint *)self->grid->methods->getCurrentCellKey(self->grid, sender);
    if (!self->methods->staticWallLink(self, &self->linkCoordinates) && self->tickBoundary != 0) {
        self->methods->dynamicLink(self);
    }
    self->methods->restoreLinkSnapshot(self);
    self->methods->onLinkUpdate(self);
}

void DreamSys__OnLinkUpdate(void) {}

s32 DreamSys__GetSetFlashbackSession(DreamSys *self, DreamColors *out, s32 value) {
    s32 old;

    old = self->isFlashbackSession;
    if (value < 0) {
        *out = CalcDreamColor(&self->moodPreviousDays[self->currentDay]);
    } else {
        self->isFlashbackSession = value;
    }
    return old;
}

void DreamSys__SetMoveOverride(DreamSys *self, s32 value) {
    self->moveOverride = value;
    if (value != 0) {
        self->methods->getSetMoveMode(self, 1);
        if (self->enterRotation != 0)
            self->methods->updateRotation(self, 1, (void *)self->enterRotation);
    }
}

void DreamSys__ResetLinkState(DreamSys *self, s32 moveMode, s32 tickPeriod) {
    Ratio16 rotation[3];

    self->methods->logChunkMood(self, &self->linkCoordinates);
    self->methods->selectLookCallback(self, LOOK_CALLBACK_STEP_LOOK);
    self->methods->selectMoveCallback(self, MOVE_CALLBACK_TICK_MOVE);
    self->methods->getSetMoveMode(self, moveMode);

    self->voiceIndex = -1;
    self->moveCycleTick = 0;
    self->voiceSelect = 0;
    self->moveCommand = MOVE_COMMAND_NONE;
    self->turnCommand = TURN_COMMAND_NONE;
    self->lookOffsetCommand = LOOK_OFFSET_COMMAND_NONE;
    self->lookYawCommand = LOOK_YAW_COMMAND_NONE;
    self->lookOffset = 0;
    self->lookYaw = 0;
    self->methods->setGateFlags(self, 0, 1, 1, 1);

    self->methods->setTickPeriod(self, tickPeriod);

    self->nextCinematic.entry = -1;
    self->movementBlocked = 0;
    self->state = DREAMSYS_NO_LINK;
    self->linkCommandFlag = 0;
    self->staircaseActive = 0;
    self->staircaseMoveGate = 0;
    self->staircaseTickFn = 0;
    self->unusedFlag78 = 0;
    SceneNode__GetRotationDegrees((SceneNode *)self, rotation);

    rotation[2].num = 0;
    rotation[2].den = 1;
    self->methods->updateRotation(self, 1, rotation);
}

void DreamSys__BlockMovement(DreamSys *self) {
    self->movementBlocked = 1;
}

s32 DreamSys__GetLinkCommandFlag(DreamSys *self) {
    return self->linkCommandFlag;
}

s32 DreamSys__GetSetDreamTimeLimit(DreamSys *self, s32 value) {
    s32 result;

    if (value >= 0)
        value = value * DREAM_TICKS_PER_SECOND;
    result = self->dreamTimeLimit;
    self->dreamTimeLimit = value;
    if (result >= 0)
        result = (u32)result / DREAM_TICKS_PER_SECOND; /* MATCHING: an unsigned divide, as retail does */
    return result;
}

s32 DreamSys__GetDreamTimerScaled(DreamSys *self) {
    return self->tick / DREAM_TICKS_PER_SECOND;
}

void DreamSys__SetSoundObj(DreamSys *self, VabStreamObj *value) {
    self->soundObj = value;
}

void DreamSys__SetViewport(DreamSys *self, Viewport *value) {
    self->viewport = value;
}

void DreamSys__SetEtcTim(DreamSys *self, struct TimImage *value) {
    self->etcTim = value;
}

void DreamSys__UpdateTickState(DreamSys *self) {
    if (self->movementBlocked == 0) {
        self->linkCommandFlag = 0;
        self->tickBoundary = (self->tick % self->tickPeriod) == 0;
    }
}

void DreamSys__RunTickCallbacks(DreamSys *self) {
    if (self->lookCallback != NULL)
        self->lookCallback(self);
    if (self->moveCallback != NULL)
        self->moveCallback(self);
}

/* The y of the line through `from` and `to` in the y/z plane, at z = `at`;
 * the z distance is taken in units of 1024 (1 when it rounds to 0). Defined
 * right after its caller. */
extern s32 InterpolateYAtZ(LongVec3 *from, LongVec3 *to, s32 at);

/* Converts the local offset (0, 0, dist) to a world position, takes its
   height from the viewport's two refView points interpolated at `dist`,
   plus the object's own world y (coord2's workm.t[1]), copies it to `out`,
   and when `reference` is given returns whether it lies within `tolerance`
   of it. */
s32 DreamSys__ProjectPointAtDistance(DreamSys *self, s32 *out, s32 dist, s32 *reference, s32 tolerance) {
    s32 worldPos[3];
    s32 height;
    long *worldTrans;
    s32 *offsetZ;

    offsetZ = &sProjectOffset.z;
    *offsetZ = dist;
    /* MATCHING: the vector's address is formed back from z */
    SceneNode__LocalOffsetToWorldPos((SceneNode *)self, worldPos, offsetZ - 2, 0);

    height = InterpolateYAtZ(&self->viewport->refView.vp, &self->viewport->refView.vr, dist);

    worldTrans = self->parent != 0 ? self->coord2->workm.t : 0;
    worldPos[1] = height + worldTrans[1];

    if (out != NULL) {
        *(LongVec3 *)out = *(LongVec3 *)worldPos;
    }

    if (reference != NULL)
        return IsVec3WithinRange(worldPos, tolerance, reference);
    return 0;
}

s32 InterpolateYAtZ(LongVec3 *from, LongVec3 *to, s32 at) {
    s32 scaledAt;
    s32 dt;
    s32 dv;

    scaledAt = at;
    scaledAt = scaledAt / 1024;
    dt = (to->z - from->z) / 1024;
    if (dt == 0)
        dt = 1;
    dv = to->y - from->y;
    return (dv * scaledAt) / dt + from->y;
}

void DreamSys__ClearUnusedFlag7C(DreamSys *self) {
    self->unusedFlag7C = 0;
}

void DreamSys__ClearUnusedFlag78(DreamSys *self) {
    self->unusedFlag78 = 0;
}

s32 DreamSys__BeforeMoveCommand(DreamSys *self) {
    return 0;
}

void DreamSys__ClearTickCallbacks(DreamSys *self, bool clearLook) {
    self->methods->selectMoveCallback(self, MOVE_CALLBACK_NONE);
    if (clearLook)
        self->methods->selectLookCallback(self, LOOK_CALLBACK_NONE);
}

void DreamSys__SetTickCallbacks(DreamSys *self, s32 moveMode, s32 lookMode) {
    self->methods->selectMoveCallback(self, moveMode);
    self->methods->selectLookCallback(self, lookMode);
}

void DreamSys__SelectLookCallback(DreamSys *self, s32 mode) {
    DreamSysMethods *vt = self->methods;

    self->lookCallbackMode = mode;
    switch (mode) {
        case LOOK_CALLBACK_NONE:
            self->lookCallback = NULL;
            break;
        case LOOK_CALLBACK_STEP_LOOK:
            self->lookCallback = vt->stepLook;
            break;
        case LOOK_CALLBACK_NOOP_2:
            self->lookCallback = vt->noOpLook2;
            break;
        case LOOK_CALLBACK_NOOP_3:
            self->lookCallback = vt->noOpLook3;
            break;
    }
}

void DreamSys__SelectMoveCallback(DreamSys *self, s32 mode) {
    DreamSysMethods *vt = self->methods;

    if (self->moveCallbackMode == MOVE_CALLBACK_TICK_DRIFT)
        vt->stopDrift(self, 0);
    self->moveCallbackMode = mode;
    switch (mode) {
        case MOVE_CALLBACK_NONE:
            self->moveCallback = NULL;
            break;
        case MOVE_CALLBACK_TICK_MOVE:
            self->moveCallback = vt->tickMove;
            break;
        case MOVE_CALLBACK_TICK_DRIFT:
            self->moveCallback = vt->tickDrift;
            self->driftActive = 1;
            self->cueServiceActive = 1;
            InitSoundCueSet(self->soundObj, &self->soundCueSet, 1, self, self->methods->soundCueCallback);
            break;
    }
}

void DreamSys__StepLook(DreamSys *self) {
    self->methods->stepLookOffset(self);
    self->methods->stepLookYaw(self);
}

void DreamSys__StepLookOffset(DreamSys *self) {
    s32 idx;
    s32 delta;
    s32 threshold;
    s32 sum;

    idx = self->lookOffsetCommand;
    if (idx != LOOK_OFFSET_COMMAND_NONE) {
        delta = sLookOffsetSteps[idx];
        threshold = sLookOffsetLimits[idx];
        sum = delta + self->lookOffset;
        /* MATCHING: `~sum + 1`, not -sum, which compiles differently */
        if (sum >= 0 ? sum < threshold : (~sum + 1) < threshold) {
            self->viewport->refView.vr.y += delta;
            self->lookOffset = sum;
        }
        self->lookOffsetCommand = LOOK_OFFSET_COMMAND_NONE;
        return;
    }
    if (self->lookOffset != 0) {
        delta = -LOOK_OFFSET_RETURN_STEP;
        if (self->lookOffset < 0)
            delta = LOOK_OFFSET_RETURN_STEP;
        self->viewport->refView.vr.y += delta;
        self->lookOffset += delta;
    }
}

void DreamSys__StepLookYaw(DreamSys *self) {
    s32 idx;
    s32 delta;
    s32 threshold;
    s32 sum;
    /* MATCHING: a second name for `self`, set on each path before the shared
       tail call; passing `self` straight to it is one word short. */
    DreamSys *flipTarget;

    self->moveCommandLatch = (self->moveCommand == MOVE_COMMAND_FORWARD);
    idx = self->lookYawCommand;
    if (idx != LOOK_YAW_COMMAND_NONE) {
        delta = sLookYawSteps[idx];
        threshold = sLookYawLimits[idx];
        sum = delta + self->lookYaw;
        if ((sum >= 0) ? (sum < threshold) : ((~sum + 1) < threshold)) {
            sTurnRotations[0][1].num = delta;
            self->methods->updateRotation(self, 0, sTurnRotations[0]);
            self->lookYaw = sum;
        }
        self->lookYawCommand = LOOK_YAW_COMMAND_NONE;
        flipTarget = self;
    } else if (self->lookYaw != 0) {
        delta = -LOOK_YAW_RETURN_STEP;
        if (self->lookYaw < 0)
            delta = LOOK_YAW_RETURN_STEP;
        sTurnRotations[0][1].num = delta;
        self->methods->updateRotation(self, 0, sTurnRotations[0]);
        self->lookYaw += delta;
        flipTarget = self;
    } else {
        return;
    }
    DreamSys__FlipMoveCommand(flipTarget);
}

void DreamSys__FlipMoveCommand(DreamSys *self) {
    self->moveCommandLatch = 0;
    if (self->moveCommand != MOVE_COMMAND_NONE) {
        if (self->moveCommand & 1)
            self->moveCommand = self->moveCommand + 1;
        else
            self->moveCommand = self->moveCommand - 1;
    }
}

void DreamSys__NoOpLook2(void) {}

void DreamSys__NoOpLook3(void) {}

void DreamSys__TickMove(DreamSys *self) {
    if (self->moveOverride == MOVE_OVERRIDE_NONE) {
        self->methods->applyPendingTurn(self);
        self->methods->tickMoveFree(self);
    } else if (self->moveOverride == MOVE_OVERRIDE_HELD) {
        self->methods->tickMoveHeld(self);
    } else {
        self->methods->tickMoveForced(self);
    }
}

void DreamSys__TickMoveFree(DreamSys *self) {
    if (self->movementBlocked != 0)
        return;
    self->methods->applyMoveCommand(self, self->methods->advanceMoveCycle(self, 1));
}

void DreamSys__TickMoveForced(DreamSys *self) {
    self->moveCommand = MOVE_COMMAND_FORWARD;
    if (self->movementBlocked == 0)
        self->methods->applyMoveCommand(self, self->methods->advanceMoveCycle(self, 1));
    else
        self->methods->advanceMoveCycle(self, 0);
}

void DreamSys__TickMoveHeld(DreamSys *self) {
    self->moveCommand = MOVE_COMMAND_FORWARD;
}

s32 DreamSys__AdvanceMoveCycle(DreamSys *self, s32 bob) {
    s32 doCallback = 0;
    s32 ret = 0;
    s32 count;
    Viewport *viewport;
    s32 delta;

    if (self->moveCommand != MOVE_COMMAND_NONE) {
        ret = self->moveCommand;
        count = self->moveCycleTick + 1;
        self->moveCycleTick = count;
        if (count < MOVE_CYCLE_TICKS) {
            doCallback = (self->moveMode == MOVE_MODE_RUN) && ((count & 1) == 0);
        } else {
            self->moveCommand = MOVE_COMMAND_NONE;
            doCallback = 1;
        }

        if (doCallback)
            self->methods->startVoice(self);

        viewport = self->viewport;
        if (viewport != NULL && self->screenShakeOn != 0 && bob != 0) {
            delta = -MOVE_BOB_HEIGHT;
            if (self->moveCycleTick >= 3)
                delta = MOVE_BOB_HEIGHT;
            viewport->refView.vp.y += delta;
            viewport->refView.vr.y += delta;
        }

        if (self->moveCommand == MOVE_COMMAND_NONE)
            self->moveCycleTick = 0;
    }

    if (!doCallback)
        self->methods->stopVoice(self);
    return ret;
}

/* MATCHING: the program index read into `scratch`, reused for program 9, and
   the tone passed as a copy `headingArg`; one expression per call compiles differently. */
void DreamSys__StartVoice(DreamSys *self) {
    VabStreamObj *obj;
    s32 idx;
    VabStreamObjMethods *vt;
    s32 heading;
    s32 headingArg;
    s32 scratch;

    obj = self->soundObj;
    vt = obj->methods;
    idx = self->voiceSelect;
    if (idx == 0) {
        return;
    }

    scratch = sVoiceBySelect[idx];
    heading = scratch << 4;
    headingArg = heading;
    vt->setPitchOffset(obj, sVoicePitchBySelect[idx]);
    self->voiceIndex = vt->playTone(obj, headingArg, DREAMSYS_TONE_VOLUME, DREAMSYS_TONE_VOLUME);
    if (self->voiceSelect != 22) {
        self->voiceIndex = -1;
    }

    if (self->voiceSelect == 11) {
        vt->setPitchOffset(obj, 1);
        vt->playTone(obj, headingArg, DREAMSYS_TONE_VOLUME, DREAMSYS_TONE_VOLUME);
        vt->setPitchOffset(obj, 2);
        scratch = 9 << 4; /* program 9, tone 0 */
        vt->playTone(obj, scratch, DREAMSYS_TONE_VOLUME, DREAMSYS_TONE_VOLUME);
    }
}

void DreamSys__StopVoice(DreamSys *self) {
    VabStreamObj *obj;

    if (self->voiceIndex >= 0) {
        obj = self->soundObj;
        obj->methods->stopVoice(obj, self->voiceIndex);
        self->voiceIndex = -1;
    }
}

void DreamSys__ApplyMoveCommand(DreamSys *self, s32 command) {
    s32 delta;
    PlayerSpawnPoint *pos;

    if (command != 0) {
        delta = sMoveCommandSigns[command] * sMoveModeSpeeds[self->moveMode];
        self->methods->beforeMoveCommand(self);
        pos = (PlayerSpawnPoint *)self->grid->methods->getTargetDescriptor(self->grid, 0, 0);
        if (!self->methods->tryStaircaseLink(self, pos) &&
            !self->methods->tryInstantTeleportLink(self, pos) &&
            !self->methods->tryTunnelLink(self, pos)) {
            self->methods->saveLinkSnapshot(self);
            /* MATCHING: staircaseMoveGate is u32 for this unsigned `< 1`; its writers store 0 or 1 */
            sMoveCommandDispatch[command](self, delta, (void *)(self->staircaseMoveGate < 1));
            if (self->currentStage == STAGE_BRIGHT_MOON_COTTAGE &&
                self->coord2->coord.t[1] < -2000 && self->coord2->coord.t[0] >= -499) {
                self->methods->onGridCellLinkCommand(self, self, 4);
            }
        }
        self->coord2->flg = 0;
    }
}

void DreamSys__ApplyPendingTurn(DreamSys *self) {
    s32 idx;

    idx = self->turnCommand;
    if (idx != TURN_COMMAND_NONE) {
        self->methods->updateRotation(self, 0, sTurnRotations[idx]);
        self->turnCommand = TURN_COMMAND_NONE;
    }
}

void DreamSys__TickDrift(DreamSys *self) {
    if (self->driftActive != 0) {
        self->methods->addTranslation(self, &sDriftStep);
        self->viewport->refView.vr.y -= 600;
    }
    if (self->cueServiceActive != 0)
        ServiceSoundCueSet(self->soundObj, &self->soundCueSet);
}

void DreamSys__StopDrift(DreamSys *self, s32 keepCues) {
    self->driftActive = 0;
    self->cueServiceActive = keepCues;
    if (keepCues != 0)
        FlushSoundCueSet(self->soundObj, &self->soundCueSet);
}

s32 DreamSys__GetSetMoveMode(DreamSys *self, s32 value) {
    s32 old;

    old = self->moveMode;
    if (value >= 0) {
        self->moveMode = value;
        self->previousMoveMode = value;
    }
    return old;
}

void DreamSys__ChangeMoveMode(DreamSys *self, s32 value) {
    s32 old;

    old = self->moveMode;
    if (old != value) {
        self->previousMoveMode = old;
        self->moveMode = value;
    }
}

void DreamSys__RestorePreviousMoveMode(DreamSys *self) {
    self->moveMode = self->previousMoveMode;
}

void DreamSys__SetGateFlags(DreamSys *self, s32 a, s32 b, s32 c, s32 d) {
    if (a >= 0)
        self->tickBoundary = a;
    if (b >= 0)
        self->gateFlags[0] = b;
    if (c >= 0)
        self->gateFlags[1] = c;
    if (d >= 0)
        self->gateFlags[2] = d;
}

void DreamSys__SetTickPeriod(DreamSys *self, s32 value) {
    self->tickPeriod = value;
}

void DreamSys__SoundCueCallback(void *owner, SoundCueSet *set) {
    s32 isDivisible;

    if (set->tag == 1) {
        isDivisible = (set->tick % 20) == 0;
        if (isDivisible) {
            set->slots[0].program = 9;
            set->slots[0].octave = -1;
        } else {
            set->slots[1].program = 9;
            set->slots[1].octave = -1;
        }
    }
}

/* The word every save block starts with. */
extern s32 sSaveMagic;

void DreamSys__InitNewGame(DreamSys *self) {
    self->saveMagic = sSaveMagic;
    self->currentYear = 0;
    self->currentDay = 0;
    self->totalFlashbackUnlockScore = 0;
    self->navigationFlashbackUnlockScore = 0;
    self->instanceFlashbackUnlockScore = 0;
    self->amountFlashbacksAvailable = 0;
    self->graphScored = 0;
    self->unk5D8 = 0;
    self->screenShakeOn = 1;
    self->unk67C = 0;
    self->unk680 = 0;
    InitNavChallengesArray(&self->navChallengesArray, &self->amountDynamicLinksDone);
    memset(self->unk684, 0, sizeof(self->unk684));
}

void DreamSys__GetSetScreenShake(DreamSys *self, bool *value) {
    bool old;

    old = self->screenShakeOn;
    self->screenShakeOn = *value;
    *value = old;
}

s32 DreamSys__GetCurrentDayAndYear(DreamSys *self, s32 *outYear) {
    if (outYear != NULL)
        *outYear = self->currentYear;
    return self->currentDay + 1;
}

s32 DreamSys__AdvanceDay(DreamSys *self) {
    self->currentDay++;
    if (self->currentDay >= DAYS_PER_YEAR) {
        self->currentDay = 0;
        self->currentYear++;
    }
    return self->currentDay;
}

void DreamSys__ClearNewGameFlag(DreamSys *self) {
    self->newGamePending = 0;
}

s32 DreamSys__GetNewGameFlag(DreamSys *self) {
    return self->newGamePending;
}

s32 *DreamSys__GetSaveBlock(DreamSys *self, s32 *outSize) {
    if (outSize != NULL)
        *outSize = DREAMSYS_SAVE_SIZE;
    return &self->saveMagic;
}

s32 DreamSys__StartDay(DreamSys *self) {
    s32 oldDay;
    MoodGraphPoint *special;

    oldDay = self->currentDay;
    self->currentFlashbackIndex = 0;
    self->tick = 0;
    self->storedDay = oldDay;
    if (self->isFlashbackSession) {
        self->methods->loadNextFlashback(self, 1);
    } else {
        special = IsDaySpecial(&self->nextCinematic, self->currentDay + 1);
        self->methods->initMoodContributors(self, special);
        if (special != NULL) {
            return -1;
        }
        self->methods->initSpawnLoc(self);
    }
    return self->currentStage;
}

s32 DreamSys__EndDay(DreamSys *self, s32 outcome) {
    self->currentDay = self->storedDay;
    if (!self->isFlashbackSession && outcome == DAY_OUTCOME_ENDED) {
        self->methods->calcUnlockScore(self);
        self->methods->updateDreamChart(self, &self->moodPreviousDays[self->currentDay]);
        self->methods->advanceDay(self);
    } else if (outcome == DAY_OUTCOME_NEW_GAME) {
        self->methods->initNewGame(self);
        self->newGamePending = 1;
    }
    return self->isFlashbackSession;
}

CinematicCall DreamSys__GetCinematic(DreamSys *self) {
    return self->nextCinematic;
}

void DreamSys__InitSpawnLoc(DreamSys *self) {
    MoodGraphPoint mood;
    s32 timeLimit;

    self->methods->getPreviousDayMood(self, &mood, 1);
    self->currentStage =
        GenerateInitialSpawn(&self->linkCoordinates, &timeLimit, &mood, self->currentDay);
    timeLimit = self->methods->getSetDreamTimeLimit(self, timeLimit);
    self->state = DREAMSYS_LINK_DAY_START;
}

void DreamSys__DynamicLink(DreamSys *self) {
    s32 stage;

    if (self->state == DREAMSYS_NO_LINK) {
        stage = GetRandomSpawnFromStage(&self->linkCoordinates, self->currentStage, self->tick);
        ExecuteLink(self, stage, DREAMSYS_LINK_DYNAMIC, 1);
    }
}

bool DreamSys__StaticWallLink(DreamSys *self, PlayerSpawnPoint *currentPos) {
    s32 result;

    if (self->state != DREAMSYS_NO_LINK)
        return false;
    result = TestForStaticLink(&self->linkCoordinates, currentPos, self->currentStage);
    if (result < 0)
        return false;
    ExecuteLink(self, result, DREAMSYS_LINK_WALL, 1);
    return true;
}

bool DreamSys__LoadNextFlashback(DreamSys *self, bool quiet) {
    s32 idx;
    FlashbackEntry *entry;

    idx = self->currentFlashbackIndex;
    if (idx < self->amountFlashbacksAvailable) {
        self->state = DREAMSYS_LINK_FLASHBACK;
        entry = &self->storedFlashbacks[idx];
        if (!quiet) {
            self->methods->notifyParents(self, DREAMSYS_LINK_FLASHBACK);
        }
        self->currentDay = entry->day;
        self->currentStage = entry->stageID;
        self->linkCoordinates = entry->position;
        return true;
    }
    return false;
}

bool DreamSys__TryTunnelLink(DreamSys *self, PlayerSpawnPoint *currentPos) {
    s32 result;
    s32 rotation[4];

    if (self->state != DREAMSYS_NO_LINK)
        return false;
    result = TestForTunnelLinks(&self->linkCoordinates, currentPos, self->currentStage);
    if (result < 0)
        return false;
    SceneNode__GetRotationDegrees((SceneNode *)self, (Ratio16 *)rotation);
    if (!DreamSys__CheckTunnelHeading(&self->exitRotation, &self->enterRotation, rotation))
        return false;
    if (self->moveCommandLatch == 0)
        return false;
    ExecuteLink(self, result, DREAMSYS_LINK_TUNNEL, 0);
    return true;
}

bool DreamSys__TryStageTimerLink(DreamSys *self, PlayerSpawnPoint *currentPos) {
    s32 result;

    if (self->state != DREAMSYS_NO_LINK)
        return false;
    result = TestForStageTransition(&self->linkCoordinates, self->currentStage, currentPos, self->tick);
    if (result < 0)
        return false;
    self->stageLinkAngle = GetStageLinkAngle();
    self->enterRotation = 0;
    self->exitRotation = 0;
    ExecuteLink(self, result, DREAMSYS_LINK_STAGE_TIMER, 0);
    return true;
}

/* MATCHING: the body nests inside `if (result >= 0)`; an early
   `return false` compiles differently. */
bool DreamSys__TryInstantTeleportLink(DreamSys *self, PlayerSpawnPoint *currentPos) {
    s32 result;
    s32 bonus;
    s32 cellPos[4];

    result = TestForInstantTeleporters(&self->linkCoordinates, currentPos, self->currentStage);
    if (result >= 0) {
        bonus = GetTeleportTimeBonus();
        if (ExecuteLink(self, result, DREAMSYS_LINK_TELEPORT, 0)) {
            self->state = DREAMSYS_NO_LINK;
            self->grid->methods->computeCellOffsets(self->grid, cellPos, &self->linkCoordinates);
            self->methods->setTranslation(self, (LongVec3 *)cellPos);
            if (bonus != 0 && !self->isFlashbackSession)
                self->methods->getSetDreamTimeLimit(self, self->methods->getDreamTimerScaled(self) + bonus);
        }
        return true;
    }
    return false;
}

bool ExecuteLink(DreamSys *system, s32 stage, s32 linkType, s32 playSound) {
    VabStreamObj *obj;

    system->state = linkType;
    system->methods->notifyParents(system, linkType);
    if (system->state == DREAMSYS_NO_LINK) {
        return false;
    }
    system->currentStage = stage;
    if (system->isFlashbackSession) {
        system->tick = 0;
    }
    if (playSound != 0) {
        obj = system->soundObj;
        obj->methods->playTone(obj, 9 << 4, DREAMSYS_TONE_VOLUME, DREAMSYS_TONE_VOLUME);
    }
    return true;
}

/* MATCHING: one nested `if` chain, not early returns, and one whole-PlayerSpawnPoint copy */
bool DreamSys__TryStaircaseLink(DreamSys *self, PlayerSpawnPoint *currentPos) {
    s32 rotation[4];

    if (self->state == DREAMSYS_NO_LINK) {
        if (self->staircaseTickFn != 0) {
            if (self->staircaseTickFn(self)) {
                self->staircaseActive = 0;
                self->staircaseTickFn = 0;
                self->staircaseMoveGate = 0;
                if (self->moveMode == MOVE_MODE_RUN) {
                    self->methods->restorePreviousMoveMode(self);
                }
            }
        } else if (TestForStaircaseNodes(&self->linkCoordinates, currentPos, self->currentStage) >= 0) {
            SceneNode__GetRotationDegrees((SceneNode *)self, (Ratio16 *)rotation);
            if (DreamSys__CheckStaircaseHeading(&self->exitRotation, &self->enterRotation, rotation) &&
                self->moveCommandLatch != 0) {
                *(PlayerSpawnPoint *)&self->staircaseGridPos = *currentPos;
                self->staircaseActive = 1;
                self->staircaseMoveGate = 1;
                self->staircaseFrame = 0;
                self->staircaseTickFn = sStaircaseTickFns[GetLastSpawnExtra()];
                self->methods->updateRotation(self, 1, (void *)self->enterRotation);
                self->staircaseTickFn(self);
            }
        }
    }
    return false;
}

s32 DreamSys__TickStaircaseYawPlus90(DreamSys *self) {
    if (self->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(self, &sStaircaseOffset0, &self->staircaseOrigin);
    }
    if (self->moveMode != MOVE_MODE_RUN) {
        if (self->staircaseFrame >= 133)
            return 1;
        if ((self->staircaseFrame >= 43 && self->staircaseFrame < 58) ||
            (self->staircaseFrame >= 75 && self->staircaseFrame < 90)) {
            self->turnCommand = TURN_COMMAND_RIGHT;
        }
    } else {
        if (self->staircaseFrame >= 19)
            return 1;
        if ((self->staircaseFrame >= 8 && self->staircaseFrame < 10) ||
            (self->staircaseFrame >= 13 && self->staircaseFrame < 15)) {
            self->methods->updateRotation(self, 0, sRotationYawPlus45);
        }
    }
    self->moveCommand = MOVE_COMMAND_FORWARD;
    self->staircaseFrame++;
    return 0;
}

s32 DreamSys__TickStaircaseYawMinus135(DreamSys *self) {
    s32 flag;

    if (self->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(self, &sStaircaseOffset1, &self->staircaseOrigin);
    }
    if (self->moveMode != MOVE_MODE_RUN) {
        if (self->staircaseFrame >= 149)
            return 1;
        if ((self->staircaseFrame >= 22 && self->staircaseFrame < 37) ||
            (self->staircaseFrame >= 57 && self->staircaseFrame < 73) ||
            (self->staircaseFrame >= 110 && self->staircaseFrame < 125)) {
            self->turnCommand = TURN_COMMAND_LEFT;
        }
        flag = (self->staircaseFrame >= 57 && self->staircaseFrame < 110);
    } else {
        if (self->staircaseFrame >= 25)
            return 1;
        if ((self->staircaseFrame >= 6 && self->staircaseFrame < 8) ||
            (self->staircaseFrame >= 11 && self->staircaseFrame < 13) ||
            (self->staircaseFrame >= 20 && self->staircaseFrame < 22)) {
            self->methods->updateRotation(self, 0, sRotationYawMinus45);
        }
        flag = (self->staircaseFrame >= 3 && self->staircaseFrame < 17);
    }
    if (flag) {
        self->lookOffsetCommand = LOOK_OFFSET_COMMAND_DOWN;
    }
    self->moveCommand = MOVE_COMMAND_FORWARD;
    self->staircaseFrame++;
    return 0;
}

s32 DreamSys__TickStaircaseYawPlus45(DreamSys *self) {
    if (self->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(self, &sStaircaseOffset2, &self->staircaseOrigin);
    }
    if (self->moveMode != MOVE_MODE_RUN) {
        if (self->staircaseFrame < 101) {
            if (self->staircaseFrame >= 43 && self->staircaseFrame < 58) {
                self->turnCommand = TURN_COMMAND_RIGHT;
            }
        } else {
            return 1;
        }
    } else {
        if (self->staircaseFrame < 15) {
            if (self->staircaseFrame >= 8 && self->staircaseFrame < 10) {
                self->methods->updateRotation(self, 0, sRotationYawPlus45);
            }
        } else {
            return 1;
        }
    }
    self->moveCommand = MOVE_COMMAND_FORWARD;
    self->staircaseFrame++;
    return 0;
}

s32 DreamSys__TickStaircaseYawMinus90(DreamSys *self) {
    s32 flag;

    if (self->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(self, &sStaircaseOffset3, &self->staircaseOrigin);
    }
    if (self->moveMode != MOVE_MODE_RUN) {
        if (self->staircaseFrame >= 113)
            return 1;
        if ((self->staircaseFrame >= 30 && self->staircaseFrame < 45) ||
            (self->staircaseFrame >= 82 && self->staircaseFrame < 97)) {
            self->turnCommand = TURN_COMMAND_LEFT;
        }
        flag = (self->staircaseFrame >= 30 && self->staircaseFrame < 82);
    } else {
        if (self->staircaseFrame >= 19)
            return 1;
        if ((self->staircaseFrame >= 6 && self->staircaseFrame < 8) ||
            (self->staircaseFrame >= 15 && self->staircaseFrame < 17)) {
            self->methods->updateRotation(self, 0, sRotationYawMinus45);
        }
        flag = (self->staircaseFrame >= 0 && self->staircaseFrame < 9);
    }
    if (flag) {
        self->lookOffsetCommand = LOOK_OFFSET_COMMAND_DOWN;
    }
    self->moveCommand = MOVE_COMMAND_FORWARD;
    self->staircaseFrame++;
    return 0;
}

void DreamSys__ApplyRelativeOffset(DreamSys *self, struct RelativePos *a, struct RelativePos *b) {
    LongVec3 diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    diff.y = 0;
    self->methods->addTranslation(self, &diff);
}

s32 DreamSys__GetCurrentStage(DreamSys *self) {
    return self->currentStage;
}

void DreamSys__ProcessChunkChange(DreamSys *self, void *entity, s32 effect) {
    PlayerSpawnPoint *pos;

    if (effect == STAGEMAP_EVENT_CHUNK_CHANGED) {
        pos =
            (PlayerSpawnPoint *)((StageMap *)entity)->methods->getTargetDescriptor((StageMap *)entity, 0, 0);
        self->methods->logChunkMood(self, pos);
    }
}

/* What an Entity's instance effect does to the dream, while no link is
   pending: SCENENODE_EVENT_LINKED is answered with the same event, 9 logs its mood and adds its unlock
   score (and may record a flashback), 10 links to the stage it names, 11
   ends the dream into its event video, 12 ends the dream. 9, 11 and 12 do
   nothing in a flashback session. */
void DreamSys__InstanceEffectsOnJournal(DreamSys *self, void *entity, s32 effect) {
    if (self->state != DREAMSYS_NO_LINK) {
        return;
    }

    switch (effect) {
        case SCENENODE_EVENT_LINKED:
            ((BasicClass *)entity)->methods->onNotify((BasicClass *)entity, self, effect);
            break;
        case ACTOR_EVENT_UNSWEPT:
        case ACTOR_EVENT_MOVED_Z:
        case ACTOR_EVENT_MOVED_X:
        case ACTOR_EVENT_MOVED_Y:
            break;
        case ENTITY_EFFECT_LOG_MOOD:
            if (self->isFlashbackSession != 0) {
                return;
            }
            self->methods->logInstanceMood(
                self, (MoodGraphPoint *)((Entity *)entity)->methods->getMoodEffect(entity));
            self->instanceFlashbackUnlockScore += ((Entity *)entity)->methods->getUnlockEffect(entity);
            self->methods->flashbackSaving(self, 0, 16);
            break;
        case ENTITY_EFFECT_LINK_STAGE: {
            s32 previousStage = self->currentStage;
            self->currentStage = -((Entity *)entity)->methods->getLinkStage(entity);
            self->methods->dynamicLink(self);
            if (self->currentStage < 0) {
                self->currentStage = previousStage;
            }
            break;
        }
        case ENTITY_EFFECT_EVENT_VIDEO:
            if (self->isFlashbackSession != 0) {
                return;
            }
            self->nextCinematic.bank = -1;
            self->tick = self->dreamTimeLimit;
            self->nextCinematic.entry = ((Entity *)entity)->methods->getEventVideo(entity);
            break;
        case ENTITY_EFFECT_END_DREAM:
            if (self->isFlashbackSession != 0) {
                return;
            }
            self->tick = self->dreamTimeLimit;
            break;
    }
}

void DreamSys__GetPreviousDayMood(DreamSys *self, MoodGraphPoint *target, bool lastDayOnly) {
    s32 upper = 0;
    s32 dynamic = 0;

    if (lastDayOnly) {
        if (self->currentYear != 0 || self->currentDay != 0) {
            s32 idx;

            idx = self->currentDay - 1;
            dynamic = self->moodPreviousDays[idx].axis.dynamic;
            upper = self->moodPreviousDays[idx].axis.upper;
        }
    } else {
        s32 count;
        count = DAYS_PER_YEAR;
        if (self->currentYear == 0)
            count = self->currentDay;
        if (count != 0) {
            MoodGraphPoint *p;
            s32 i;

            p = self->moodPreviousDays;
            i = 0;
            if (upper < count) {
                do {
                    i++;
                    dynamic += p->axis.dynamic;
                    upper += p->axis.upper;
                    p++;
                } while (i < count);
            }
            dynamic /= count;
            upper /= count;
        }
    }
    target->axis.dynamic = dynamic;
    target->axis.upper = upper;
}

void DreamSys__InitMoodContributors(DreamSys *self, MoodGraphPoint *special) {
    self->methods->clearMoodGraph(self, &self->areaMoods);
    self->methods->clearMoodGraph(self, &self->entityMoods);
    if (special != NULL) {
        self->methods->logMood(self, &self->areaMoods, special);
        self->methods->logMood(self, &self->entityMoods, special);
    }
}

void DreamSys__LogChunkMood(DreamSys *self, PlayerSpawnPoint *currentPos) {
    MoodGraphPoint *mood;

    mood = GetMoodFromStageChunk(self->currentStage, (StageChunk *)currentPos);
    self->methods->logMood(self, &self->areaMoods, mood);
}

void DreamSys__LogInstanceMood(DreamSys *self, MoodGraphPoint *source) {
    self->methods->logMood(self, &self->entityMoods, source);
}

void DreamSys__UpdateDreamChart(DreamSys *self, MoodGraphPoint *ret) {
    MoodGraphPoint areaAvg;
    MoodGraphPoint entityAvg;

    self->methods->getMoodAverage(self, &self->areaMoods, &areaAvg);
    self->methods->getMoodAverage(self, &self->entityMoods, &entityAvg);
    if (self->entityMoods.amountMoods == 0) {
        entityAvg.value = areaAvg.value;
    }
    ret->axis.dynamic = (areaAvg.axis.dynamic + entityAvg.axis.dynamic) / 2;
    ret->axis.upper = (areaAvg.axis.upper + entityAvg.axis.upper) / 2;
}

DreamColors DreamSys__GetDreamColor(DreamSys *self) {
    MoodGraphPoint local;

    self->methods->updateDreamChart(self, &local);
    return CalcDreamColor(&local);
}

/* Classifies each mood axis as low (< -3), middle or high (>= 4) and looks
   the pair up in the 3x3 sDreamColorTable, [dynamic][upper]. */
/* MATCHING: the lookup goes through a row pointer, not a flat `[d * 3 + u]`. */
DreamColors CalcDreamColor(MoodGraphPoint *mood) {
    MoodGraphPoint local;
    s8 *p;
    s32 i;
    s8 val;
    s8(*table)[3];

    local.value = mood->value;
    p = &local.axis.dynamic;
    for (i = 0; i < 2; i++, p++) {
        val = *p;
        if (val >= 4) {
            *p = 2;
        } else if (val < -3) {
            *p = 0;
        } else {
            *p = 1;
        }
    }
    table = (s8(*)[3])sDreamColorTable;
    return table[local.axis.dynamic][local.axis.upper];
}

void DreamSys__ClearMoodGraph(DreamSys *self, MoodGraphContributor *contributor) {
    contributor->lastMood.value = 0;
    contributor->sumMoods.upper = 0;
    contributor->sumMoods.dynamic = 0;
    contributor->amountMoods = 0;
}

void DreamSys__LogMood(DreamSys *self, MoodGraphContributor *layer, MoodGraphPoint *mood) {
    layer->lastMood.value = mood->value;
    layer->sumMoods.dynamic = mood->axis.dynamic + layer->sumMoods.dynamic;
    layer->sumMoods.upper = mood->axis.upper + layer->sumMoods.upper;
    layer->amountMoods = layer->amountMoods + 1;
}

void DreamSys__GetMoodAverage(DreamSys *self, MoodGraphContributor *layer, MoodGraphPoint *ret) {
    if (layer->amountMoods != 0) {
        ret->axis.dynamic =
            CalcMoodAxis(layer->lastMood.axis.dynamic, layer->sumMoods.dynamic, layer->amountMoods);
        ret->axis.upper =
            CalcMoodAxis(layer->lastMood.axis.upper, layer->sumMoods.upper, layer->amountMoods);
    } else {
        ret->value = layer->lastMood.value;
    }
}

s32 CalcMoodAxis(s32 last, s32 sum, s32 amount) {
    s32 result;

    result = sum / amount;
    result += last / 3;
    if (result > MOOD_AXIS_MAX)
        result = -MOOD_AXIS_MAX;
    else if (result < -MOOD_AXIS_MAX)
        result = MOOD_AXIS_MAX;
    return result;
}

void DreamSys__CalcUnlockScore(DreamSys *self) {
    self->navigationFlashbackUnlockScore = CalcNavigationScore();
    if (self->instanceFlashbackUnlockScore < 0) {
        self->instanceFlashbackUnlockScore = 0;
    } else if (self->instanceFlashbackUnlockScore > UNLOCK_SCORE_MAX) {
        self->instanceFlashbackUnlockScore = UNLOCK_SCORE_MAX;
    }
    self->totalFlashbackUnlockScore =
        self->navigationFlashbackUnlockScore + self->instanceFlashbackUnlockScore;
}

void DreamSys__AddFlashback(DreamSys *self, s32 stage, PlayerSpawnPoint *pos, s32 *angles,
                            s32 unknown, s32 time, s32 day) {
    FlashbackEntry *entry;

    entry = self->storedFlashbacks;
    if (self->amountFlashbacksAvailable < ARRAY_COUNT(self->storedFlashbacks)) {
        entry += self->amountFlashbacksAvailable++;
    } else {
        entry += self->tick % 9;
    }
    entry->stageID = stage;
    entry->position = *pos;
    entry->rotation =
        *(FlashbackRotation *)angles; /* MATCHING: one struct assignment: every load before every store */
    entry->unk1C = unknown;
    entry->timeLimit = time;
    entry->day = day;
}

void DreamSys__FlashbackSaving(DreamSys *self, s32 unknown, s32 timeLimit) {
    PlayerSpawnPoint *pos;
    s32 rotation[4];

    if (self->grid != NULL && rand() % 3 == 0) {
        pos = (PlayerSpawnPoint *)self->grid->methods->getTargetDescriptor(self->grid, 0, 0);
        SceneNode__GetRotationDegrees((SceneNode *)self, (Ratio16 *)rotation);
        self->methods->addFlashback(self, self->currentStage, pos, rotation, unknown, timeLimit,
                                    self->currentDay);
    }
}

void DreamSys__ResetFlashbackList(DreamSys *self) {
    self->amountFlashbacksAvailable = 0;
}

void DreamSys__SaveLinkSnapshot(DreamSys *self) {
    GsCOORDINATE2 *p = self->coord2;

    self->coord2Snapshot = *p;
    self->coord2ParamSnapshot = *p->param;
}

void DreamSys__RestoreLinkSnapshot(DreamSys *self) {
    GsCOORDINATE2 *p = self->coord2;

    *p = self->coord2Snapshot;
    *p->param = self->coord2ParamSnapshot;
    p->flg = 0;
}

s32 DreamSys__GetSetConfigOption(DreamSys *self, s32 value) {
    s32 old;

    if (value >= 0) {
        old = self->configOption;
        self->configOption = value;
    } else {
        old = self->configOption;
    }
    return old;
}

DreamSysMethods *GetDreamSysMethods(void) {
    return &gDreamSysMethods;
}

void InitNavChallengesArray(s8 (*arrayMem)[NAV_CHALLENGE_COUNT], s32 *linkCounter) {
    s32 i;

    for (i = NAV_CHALLENGE_COUNT - 1; i >= 0; i--)
        (*arrayMem)[i] = 0;
    gpNavChallengesComplete = arrayMem;
    *linkCounter = 0;
    gpDinamicLinkPenalty = linkCounter;
}

s32 CalcNavigationScore(void) {
    s32 sum;
    s8 *p;
    s32 i;

    sum = 0;
    p = *gpNavChallengesComplete;
    for (i = 0; i < NAV_CHALLENGE_COUNT; i++) {
        if (p[i] != 0)
            sum += NAV_CHALLENGE_SCORE;
    }
    if (sum >= NAV_CHALLENGE_COUNT * NAV_CHALLENGE_SCORE)
        sum = UNLOCK_SCORE_MAX;
    sum -= *gpDinamicLinkPenalty * DYNAMIC_LINK_PENALTY;
    if (sum < 0)
        sum = 0;
    return sum;
}

s32 GetStageTimeLimit(s32 stage) {
    return sStageTimeLimits[stage];
}

/* A random link lands on one of the first six stages, Bright Moon Cottage to
 * Violence District; the others are reached only by fixed links. */
#define RANDOM_LINK_STAGE_COUNT (STAGE_VIOLENCE_DISTRICT + 1)

/* A random spawn point: on a random one of the first six stages other than
 * fromStage, or, for a negative fromStage, on stage -fromStage. */
s32 GetRandomSpawnFromStage(PlayerSpawnPoint *target, s32 fromStage, s32 unused) {
    s32 stage;
    s32 index;
    StageSpawn *entry;
    s32 stageCount;

    stageCount = RANDOM_LINK_STAGE_COUNT; /* MATCHING: a local; retail loads the 6 into a saved register before the branch */
    if (fromStage >= 0) {
        stage = rand() % stageCount;
        if (stage == fromStage) {
            stage++;
            if (stage >= RANDOM_LINK_STAGE_COUNT)
                stage = STAGE_BRIGHT_MOON_COTTAGE;
        }
    } else {
        stage = -fromStage;
    }

    index = rand() % sStageSpawnPointsCount[stage];
    entry = &sStageSpawnPoints[stage][index];
    *(PlayerSpawnGridPos *)target = *(PlayerSpawnGridPos *)entry;
    target->position = sSpawnPosAdjust[entry->adjustment];
    (*gpDinamicLinkPenalty)++;
    return stage;
}

s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    return GetStaticSpawn(target, currentPos, stage, sStagePermalinkTriggersCount,
                          sStagePermalinkTriggers, sStagePermalinkSpawns, 1);
}

s32 TestForTunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    return GetStaticSpawn(target, currentPos, stage, sTunnelTriggersCount, sTunnelTriggers,
                          sTunnelSpawns, 1);
}

/* Defined below, in ROM order. */
extern s32 IsHeadingAligned(Ratio16 *rotation, u8 heading);

s32 DreamSys__CheckTunnelHeading(s32 *outExit, s32 *outEnter, void *rotation) {
    u8 heading;
    s32 idx;
    s32 result;

    heading = sTunnelEnterHeadings[sLinkSrcStage][sLinkTriggerIndex];
    if (IsHeadingAligned(rotation, heading)) {
        if (outEnter != NULL)
            *outEnter = (s32)sCardinalRotations[heading];

        if (outExit != NULL) {
            idx = sTunnelExitHeadings[sLinkDstStage][sLinkSpawnIndex];
            *outExit = (s32)sCardinalRotations[idx];
        }
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

/* Whether the yaw of `rotation` (SceneNode__GetRotationDegrees' form) is
 * within 44 degrees of cardinal direction `heading`. */
s32 IsHeadingAligned(Ratio16 *rotation, u8 heading) {
    s16 diff;

    diff = rotation[1].num - sCardinalRotations[heading][1].num;
    if (diff >= 181) {
        diff -= 360;
    } else if (diff < -180) {
        diff += 360;
    }
    return (u16)(diff + 44) < 89;
}

/* Compared against the leading 4 bytes (chunk+tile) of `currentPos` as a
   raw word; only ever compared here, never dereferenced field-by-field. */
extern s32 sStage5TriggerGridPos;

/* The stage links that TryStageTimerLink takes: only Pit & Temple, The
 * Natural World, Violence District, Clockwork Machines and Black Space have
 * one. Violence District links below y -4095 or at the one grid position
 * sStage5TriggerGridPos, Clockwork Machines at y 2048 and up, the others
 * anywhere. On an odd timer the link lands on Black Space
 * (GetRandomSpawnFromStage's negative form), otherwise away from the current
 * stage. Returns the destination stage
 * (also sLinkDstStage), or -1 for no link. */
s32 TestForStageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer) {
    s32 result;

    if (stage != STAGE_NATURAL_WORLD && stage != STAGE_PIT_AND_TEMPLE && stage != STAGE_VIOLENCE_DISTRICT &&
        stage != STAGE_CLOCKWORK_MACHINES && stage != STAGE_BLACK_SPACE)
        return -1;
    if (stage == STAGE_VIOLENCE_DISTRICT) {
        if (currentPos->position.y >= -4095 && *(s32 *)currentPos != sStage5TriggerGridPos)
            return -1;
    } else if (stage == STAGE_CLOCKWORK_MACHINES) {
        if (currentPos->position.y < 2048)
            return -1;
    }
    if (timer & 1)
        stage = -STAGE_BLACK_SPACE;
    result = GetRandomSpawnFromStage(target, stage, timer);
    sLinkDstStage = result;
    return result;
}

/* The rotation TryStageTimerLink stores in stageLinkAngle for every
   destination but Black Space. */
extern s32 sLinkAngle180;

s32 GetStageLinkAngle(void) {
    s32 result;

    result = 0;
    if (sLinkDstStage != STAGE_BLACK_SPACE)
        result = (s32)&sLinkAngle180;
    return result;
}

/* Set by SetInstantTeleportersEnabled (dream_aux.c calls it), tested by
   TestForInstantTeleporters. */
extern s32 sInstantTeleportersEnabled;

void SetInstantTeleportersEnabled(bool value) {
    sInstantTeleportersEnabled = value;
}

s32 TestForInstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    s32 result;

    if (sInstantTeleportersEnabled == 0) {
        result = -1;
    } else {
        result = GetStaticSpawn(target, currentPos, stage, sTeleportTriggersCount,
                                sTeleportTriggers, sTeleportSpawns, 0);
    }
    return result;
}

s32 GetTeleportTimeBonus(void) {
    return (sLinkSrcStage == STAGE_BRIGHT_MOON_COTTAGE) ? 10 : 0;
}

s32 TestForStaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    if (stage == 0)
        return GetStaticSpawn(target, currentPos, 0, sStaircaseTriggersCount, sStaircaseTriggers,
                              sStaircaseSpawns, 0);
    return -1;
}

s32 DreamSys__CheckStaircaseHeading(s32 *outExit, s32 *outEnter, void *rotation) {
    u8 heading;
    s32 idx;
    s32 result;

    heading = sStaircaseEnterHeadings[sLinkSrcStage][sLinkTriggerIndex];
    if (IsHeadingAligned(rotation, heading)) {
        if (outEnter != NULL)
            *outEnter = (s32)sCardinalRotations[heading];

        if (outExit != NULL) {
            idx = sStaircaseExitHeadings[sLinkDstStage][sLinkSpawnIndex];
            *outExit = (s32)sCardinalRotations[idx];
        }
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

s32 GetLastSpawnExtra(void) {
    return sStaircaseSpawns[sLinkDstStage][sLinkSpawnIndex].extra;
}

s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                   s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag) {
    s32 count;
    StaticLinkTrigger *trig;
    s32 i;
    StageSpawn *entry;
    s32 triggerStage;
    u32 spawnIndex;

    count = (u8)triggerLens[stage];
    if (count == 0)
        return -1;

    trig = triggers[stage];
    for (i = 0; i < count; i++, trig++) {
        if (*(s16 *)&currentPos->chunk != *(s16 *)&trig->chunk)
            continue;
        if (*(s16 *)&currentPos->tile != trig->tile.value && trig->tile.value >= 0)
            continue;

        sLinkSrcStage = stage;
        sLinkTriggerIndex = i;
        triggerStage = trig->stage;
        sLinkDstStage = triggerStage;
        spawnIndex = (u8)trig->spawnpointIndex;
        entry = &spawns[triggerStage][spawnIndex];
        sLinkSpawnIndex = spawnIndex;
        *(PlayerSpawnGridPos *)target = *(PlayerSpawnGridPos *)entry;
        target->position = sSpawnPosAdjust[entry->adjustment];
        if (flag != 0)
            (*gpNavChallengesComplete)[entry->extra] = 1;
        return sLinkDstStage;
    }
    return -1;
}

s32 GenerateInitialSpawn(PlayerSpawnPoint *dest, s32 *timeLimit, MoodGraphPoint *mood, s32 day) {
    StageChunk chunk;
    s32 stage;
    s32 count;
    s32 i;
    StageSpawn *entry;

    stage = GetStageChunkFromMood(&chunk, mood);
    if (stage >= 0) {
        *timeLimit = sStageTimeLimits[stage];

        count = sStageSpawnPointsCount[stage];
        entry = sStageSpawnPoints[stage];
        for (i = 0; i < count; i++, entry++) {
            if (*(s16 *)&chunk == *(s16 *)&entry->chunk)
                goto found; /* MATCHING: a break and a test after the loop compiles an extra compare */
        }
        entry = &sStageSpawnPoints[stage][*(s16 *)&chunk % count];

    found:
        *(PlayerSpawnGridPos *)dest = *(PlayerSpawnGridPos *)entry;
        dest->position = sSpawnPosAdjust[entry->adjustment];
        return stage;
    }

    stage = GetRandomSpawnFromStage(dest, stage, day);
    *timeLimit = sStageTimeLimits[stage];
    return stage;
}

MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day) {
    s32 i;

    /* MATCHING: tested unsigned, as retail does; `i` stays signed, since a u32 changes the `% 12` */
    for (i = 0; (u32)i < ARRAY_COUNT(sSpecialDays); i++) {
        if (day == sSpecialDays[i]) {
            /* One of the special day's records, and one of twelve
             * special-day banks. */
            cinematic->entry = rand() % SPECIAL_DAY_RECORD_COUNT;
            cinematic->bank = i % 12;
            return &sSpecialDayMood;
        }
    }
    return NULL;
}
