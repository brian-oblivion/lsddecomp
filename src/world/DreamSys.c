/* DreamSys -- the class that runs a dream in progress (include/DreamSys.h
 * has the class as a whole). Every DreamSys method is here, in four groups,
 * followed by the free functions the link tests are built from.
 *
 * 1. Construction and reset: New_DreamSys, DreamSys__DreamSys,
 *    ResetSessionState (no tick callbacks, sound cue set freed, staircase
 *    walk cleared, base orientation applied), ResetLinkState, SpawnAtLink.
 *
 * 2. The per-tick chain. TimerTick advances the dream clock (Actor's `tick`)
 *    and, at dreamTimeLimit, loads the next flashback or ends the dream;
 *    below the limit it runs UpdateTickState and RunTickCallbacks, which call
 *    the look and move callbacks SelectLookCallback/SelectMoveCallback install. The look
 *    callback is StepLook: two spring-back accumulators, StepLookOffset (the
 *    view height) and StepLookYaw (45 degrees a tick, up to 181, and back).
 *    The move callback is TickDrift or TickMove, the movement state machine
 *    (free, forced or held, by moveOverride) around AdvanceMoveCycle's
 *    four-tick step: a voice through the VabStreamObj in soundObj
 *    (StartVoice/StopVoice), a view bob, and ApplyMoveCommand, which tries
 *    the link tests before it lets the step happen.
 *
 * 3. Day, mood and flashback bookkeeping: StartDay/EndDay, the mood graph
 *    (two MoodGraphContributor accumulators that UpdateDreamChart averages
 *    and CalcDreamColor turns into a DreamColors value),
 *    AddFlashback/FlashbackSaving, CalcUnlockScore, and GetSaveBlock, which
 *    hands out the DREAMSYS_SAVE_SIZE bytes from saveMagic that InitNewGame
 *    initializes.
 *
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
#include "DreamSys.h"
#include "Entity.h"
#include "LinkResource.h"
#include "StageMap.h"
#include "LbdFile.h"
#include "VabStreamObj.h"
#include "Viewport.h"
#include "BMemPMgr.h"

extern s32 sLinkSrcStage;
extern s32 sLinkTriggerIndex;
extern s32 sLinkDstStage;
extern s32 sLinkSpawnIndex;

/* sProjectOffsetZ is the LAST word of a 3-word (LongVec3-shaped) scratch
   vector whose first two words have no symbol of their own: they are the 8
   zero bytes after sVoicePitchBySelect's 24, which voiceSelect (bounded to
   [0, 0x18) by DreamSys__NotifyLinkAttempt) never reaches.
   DreamSys__ProjectPointAtDistance writes `dist` into this word and passes
   `&sProjectOffsetZ - 2` to SceneNode__LocalOffsetToWorldPos as its 3-word
   `src` vector: the local offset (0, 0, dist). */
extern s32 sProjectOffsetZ;

/* Delta/threshold table pairs consumed by DreamSys__StepLookOffset (sLookOffsetSteps /
   sLookOffsetLimits, indexed by DreamSys::lookOffsetCommand) and DreamSys__StepLookYaw (sLookYawSteps /
   sLookYawLimits, indexed by DreamSys::lookYawCommand). Index 0 is unused/zero in both
   pairs; indices 1 and 2 are the negative/positive delta and its matching
   threshold. */
extern s32 sLookOffsetSteps[3];
extern s32 sLookOffsetLimits[3];
extern s32 sLookYawSteps[3];
extern s32 sLookYawLimits[3];

/* Consumed by DreamSys__ApplyMoveCommand, both indexed by that
   function's own `arg1` (a mood/day-type selector, range implied by the
   table sizes below): `sMoveCommandSigns[arg1] * sMoveModeSpeeds[self->moveMode]` forms
   a signed delta, then `sMoveCommandDispatch[arg1]` is called with it. Index 0 is
   unused/null in sMoveCommandDispatch (arg1 == 0 returns before reaching any of
   these, per that function's own guard) -- consistent with sMoveCommandSigns[0]
   being 0 too. sMoveModeSpeeds is indexed separately by DreamSys::moveMode (its
   own "Current" value, see that field), not by arg1. */
extern s32 sMoveModeSpeeds[5];
extern s8 sMoveCommandSigns[8];

/* One table of Ratio16[3] entries under two labels:
   DreamSys__StepLookYaw references its SECOND word (entry 0's yaw
   numerator, which it overwrites with its own per-tick delta) while
   DreamSys__ApplyPendingTurn address-takes whole entries. Entry 0 is
   (0 deg, 45 deg, 0 deg), the 45 being the +-0x2D DreamSys__StepLookYaw
   writes; entries 1, (0, -6, 0), and 2, (0, +6, 0), are
   DreamSys::turnCommand's values 1 and 2. */
extern Ratio16 sTurnRotationYaw[]; /* == &sTurnRotations[0][1] */
extern Ratio16 sTurnRotations[][3];

/* (0 deg, 180 deg, 0 deg). Address-of only, forwarded as SceneNode__UpdateRotation's
   arg2 with flag 1 (absolute) by DreamSys__ResetSessionState. */
extern Ratio16 sRotationYaw180[3];

/* DreamSys__TickDrift's per-tick addTranslation (+0x0BC) step. */
extern LongVec3 sDriftStep;

/* A `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a` argument
   by DreamSys__TickStaircaseYawPlus45. */
extern struct RelativePos sStaircaseOffset2;

/* The same, for DreamSys__TickStaircaseYawPlus90. */
extern struct RelativePos sStaircaseOffset0;

/* The same, for DreamSys__TickStaircaseYawMinus135. */
extern struct RelativePos sStaircaseOffset1;

/* The same, for DreamSys__TickStaircaseYawMinus90. */
extern struct RelativePos sStaircaseOffset3;

/* (0 deg, +45 deg, 0 deg), forwarded as vtable slot +0x044's (SceneNode__UpdateRotation)
   arg2 with flag 0 (relative) by DreamSys__TickStaircaseYawPlus90 and
   DreamSys__TickStaircaseYawPlus45. Its three {num, den} Ratio16s
   are {0,1} {0x2D,1} {0,1}, the same form as sRotationYaw180 and every
   sCardinalRotations entry. */
extern Ratio16 sRotationYawPlus45[3];

/* (0 deg, -45 deg, 0 deg) -- the mirror of sRotationYawPlus45 above
   ({0,1} {0xFFD3,1} {0,1}), used the same way by
   DreamSys__TickStaircaseYawMinus135 and DreamSys__TickStaircaseYawMinus90. */
extern Ratio16 sRotationYawMinus45[3];

/* 3x3 lookup table indexed by [dynamicClass][upperClass], each axis
   classified into {0,1,2} by CalcDreamColor first. */
extern s8 sDreamColorTable[9];

/* Byte tables indexed by DreamSys::voiceSelect (bounded to [0,0x18) at
   the write site -- see that field's own comment). DreamSys__StartVoice
   reads both: sVoiceBySelect[voiceSelect] (values 0..0x1E) feeds
   VabStreamObj playTone's `index` argument (program << 4, tone 0);
   sVoicePitchBySelect[voiceSelect] (values include -2..2, hence `s8` not `u8`) feeds
   setPitchOffset's `octave` argument directly. sVoicePitchBySelect is these
   24 bytes; the zero bytes after it are the sProjectOffsetZ vector above. */
extern const s8 sVoiceBySelect[0x18];
extern const s8 sVoicePitchBySelect[0x18];

/* Dispatch table indexed by DreamSys__ApplyMoveCommand's `arg1`; see that table's own
   comment near sMoveModeSpeeds/sMoveCommandSigns above. Same element signature as
   Actor__MoveLocalZOrFindLink/Actor__MoveLocalXOrFindLink (Actor +0x0D0/+0x0D4). */
extern void (*sMoveCommandDispatch[5])(DreamSys *self, s32 val, void *extra);

/* 4-entry table of `s32 (DreamSys *self)` functions (DreamSys__TickStaircaseYawPlus90,
   DreamSys__TickStaircaseYawMinus135, DreamSys__TickStaircaseYawPlus45, DreamSys__TickStaircaseYawMinus90),
   indexed by GetLastSpawnExtra()'s return value and
   stashed into DreamSys::staircaseTickFn by DreamSys__TryStaircaseLink. */
extern s32 (*sStaircaseTickFns[4])(DreamSys *self);

extern s16 sStageTimeLimits[];

extern struct RelativePos sSpawnPosAdjust[];

extern StageSpawn *sStageSpawnPoints[];

/* MATCHING: u8; as s8, GenerateInitialSpawn's loop guard gains a `blez` retail does not have. */
extern u8 sStageSpawnPointsCount[];

extern StageSpawn *sStagePermalinkSpawns[];
extern StaticLinkTrigger *sStagePermalinkTriggers[];
extern s8 sStagePermalinkTriggersCount[];

extern s16 sSpecialDays[];

/* The fixed "special day" mood, returned by IsDaySpecial on a match;
   only its address is used. */
extern MoodGraphPoint sSpecialDayMood;

/* Table triple for TestForTunnelLinks, same roles as the
   STAGE_PERMALINK_* triple above but for tunnel links specifically. */
extern s8 sTunnelTriggersCount[];
extern StaticLinkTrigger *sTunnelTriggers[];
extern StageSpawn *sTunnelSpawns[];

/* Table triple for TestForStaircaseNodes. */
extern s8 sStaircaseTriggersCount[];
extern StaticLinkTrigger *sStaircaseTriggers[];
extern StageSpawn *sStaircaseSpawns[];

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

/* Defined further down, in ROM order, and called before that. */
s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
s32 TestForTunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
void DreamSys__FlipMoveCommand(DreamSys *self);

DreamSys *New_DreamSys(LinkResource *modelSource, s32 soundObj, s32 viewport) {
    DreamSys *self;

    self = BMemPMgrAlloc(sizeof(DreamSys));
    if (self != NULL) {
        GetDreamSysMethods()->ctor(self, modelSource, soundObj, viewport);
        return self;
    }
    return NULL;
}

DreamSys *DreamSys__DreamSys(DreamSys *self, LinkResource *modelSource, s32 soundObj, s32 viewport) {
    void *model;

    GetActorMethods()->ctor((Actor *)self);
    self->methods = GetDreamSysMethods();
    self->soundObj = soundObj;
    self->viewport = (Viewport *)viewport;
    self->etcTim = 0;
    self->modelSource = modelSource;
    model = modelSource->methods->getModel(modelSource, 0);
    self->methods->addChild(self, model);
    self->methods->getSetDreamTimeLimit(self, -1);
    self->movementBlocked = 1;
    self->moveOverride = 0;
    self->newGamePending = 1;
    self->methods->initNewGame(self);
    return ((DreamSysResetRetFn)self->methods->reset)(self);
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
    self->unk78 = 0;
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
    if (self->moveOverride != 0 && self->exitRotation != 0) {
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
    if (event == -2)
        goto handle_neg2;
    if (event != -1)
        return;

    voice = self->linkTarget->flags36 & 0x7F;
    self->voiceSelect = voice;
    if (voice >= ARRAY_COUNT(sVoiceBySelect))
        self->voiceSelect = 0;

    if (self->state == DREAMSYS_LINK_TUNNEL && self->voiceSelect == 0)
        self->voiceSelect = 2;

    if (self->currentStage != 9)
        return;
    goto shared_tail;

handle_neg2:
    if (self->grid->methods->findSlotForPosition(self->grid, (LongVec3 *)self->coord2->coord.t)
            ->loader->headerReady != 2)
        goto neg2_mismatch;

shared_tail:
    self->methods->tryStageTimerLink(
        self, (PlayerSpawnPoint *)self->grid->methods->getTargetDescriptor(self->grid, 0, 0));
    return;

neg2_mismatch:
    self->methods->restoreLinkSnapshot(self);
}

void DreamSys__OnPadEvent(DreamSys *self, s32 sender, s32 event) {
    if (self->moveOverride != 0)
        return;
    if (self->movementBlocked != 0)
        return;
    if (self->staircaseActive != 0)
        return;

    switch (event - 2) {
        case 0:
            self->moveCommand = MOVE_COMMAND_FORWARD;
            break;
        case 1:
            self->moveCommand = MOVE_COMMAND_BACK;
            break;
        case 2:
            self->turnCommand = 1;
            break;
        case 3:
            self->turnCommand = 2;
            break;
        case 4:
            self->lookOffsetCommand = 1;
            break;
        case 5:
            if (self->moveCommand == MOVE_COMMAND_FORWARD)
                self->methods->changeMoveMode(self, MOVE_MODE_RUN);
            break;
        case 6:
            self->lookOffsetCommand = 2;
            break;
        case 11:
            self->lookYawCommand = 2;
            break;
        case 12:
            self->moveCommand = MOVE_COMMAND_RIGHT;
            break;
        case 13:
            self->lookYawCommand = 1;
            break;
        case 14:
            self->moveCommand = MOVE_COMMAND_LEFT;
            break;
        case 23:
            self->linkCommandFlag = 1;
            break;
        case 32:
            self->methods->restorePreviousMoveMode(self);
            break;
        case 47:
            break;
    }
}

void DreamSys__TimerTick(DreamSys *self, s32 sender, s32 event) {
    s32 old;

    if (event != 2)
        return;

    old = self->tick;
    self->tick = old + 1;
    if ((u32)old < (u32)self->dreamTimeLimit)
        goto tick_only;

    if (self->isFlashbackSession) {
        if (self->state != DREAMSYS_NO_LINK || self->methods->loadNextFlashback(self, 0)) {
            /* MATCHING: keeps GCC from cross-jumping this branch to the identical
               `tick = 0; return;` tail after notifyParents. */
            __asm__("");
            self->tick = 0;
            return;
        }
    } else {
        self->methods->flashbackSaving(self, 0, 16);
    }
    self->methods->notifyParents(self, DREAMSYS_TIME_UP);
    self->tick = 0;
    return;

tick_only:
    self->methods->updateTickState(self);
    self->methods->runTickCallbacks(self);
}

void DreamSys__DispatchChunkChange(DreamSys *self, void *sender, s32 event) {
    GetActorMethods()->dispatchLinkCommand((Actor *)self, sender, event);
    if ((((BasicClass *)sender)->methods->header & 0xFFF) == STAGEMAP_CLASS_ID) {
        self->methods->processChunkChange(self, sender, event);
    }
}

void DreamSys__DispatchInstanceEffect(DreamSys *self, void *sender, s32 effect) {
    GetActorMethods()->onActorLinkCommand((Actor *)self, sender, effect);
    if ((((BasicClass *)sender)->methods->header & 0xFFFFF) == ENTITY_CLASS_ID) {
        self->methods->instanceEffectsOnJournal(self, sender, effect);
    }
}

void DreamSys__WallLink(DreamSys *self, void *sender, int event) {
    GetActorMethods()->onGridCellLinkCommand((Actor *)self, sender, event);
    if (event != 4)
        return;
    if (self->state != DREAMSYS_NO_LINK)
        return;
    self->linkCoordinates =
        *(PlayerSpawnPoint *)self->grid->methods->getCurrentCellKey(self->grid, sender);
    if (!self->methods->staticWallLink(self, &self->linkCoordinates) && self->tickBoundary != 0) {
        self->methods->dynamicLink(self);
    }
    self->methods->restoreLinkSnapshot(self);
    self->methods->slotE8(self);
}

void DreamSys__NoOpSlotE8Default(void) {}

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
    self->turnCommand = 0;
    self->lookOffsetCommand = 0;
    self->lookYawCommand = 0;
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
    self->unk78 = 0;
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
        result = (u32)result / DREAM_TICKS_PER_SECOND;
    return result;
}

s32 DreamSys__GetDreamTimerScaled(DreamSys *self) {
    return (u32)self->tick / DREAM_TICKS_PER_SECOND;
}

void DreamSys__SetSoundObj(DreamSys *self, s32 value) {
    self->soundObj = value;
}

void DreamSys__SetViewport(DreamSys *self, Viewport *value) {
    self->viewport = value;
}

void DreamSys__SetEtcTim(DreamSys *self, s32 value) {
    self->etcTim = value;
}

void DreamSys__UpdateTickState(DreamSys *self) {
    if (self->movementBlocked == 0) {
        self->linkCommandFlag = 0;
        self->tickBoundary = ((u32)self->tick % (u32)self->tickPeriod) == 0;
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

    offsetZ = &sProjectOffsetZ;
    *offsetZ = dist;
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

void DreamSys__func_59590(DreamSys *self) {
    self->unk7C = 0;
}

void DreamSys__func_59598(DreamSys *self) {
    self->unk78 = 0;
}

s32 DreamSys__NoOpSlot12C(DreamSys *self) {
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
        case LOOK_CALLBACK_SLOT14C:
            self->lookCallback = vt->slot14C;
            break;
        case LOOK_CALLBACK_SLOT150:
            self->lookCallback = vt->slot150;
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
            self->moveCallback = (void (*)(DreamSys *))vt->tickMove;
            break;
        case MOVE_CALLBACK_TICK_DRIFT:
            self->moveCallback = vt->tickDrift;
            self->driftActive = 1;
            self->cueServiceActive = 1;
            InitSoundCueSet((VabStreamObj *)self->soundObj, &self->soundCueSet, 1, self,
                            self->methods->soundCueCallback);
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
    if (idx != 0) {
        delta = sLookOffsetSteps[idx];
        threshold = sLookOffsetLimits[idx];
        sum = delta + self->lookOffset;
        if (sum >= 0) {
            if (sum < threshold)
                goto apply;
            self->lookOffsetCommand = 0;
            return;
        }
        if ((~sum + 1) >= threshold) {
            self->lookOffsetCommand = 0;
            return;
        }
    apply:
        self->viewport->refView.vr.y += delta;
        self->lookOffset = sum;
        self->lookOffsetCommand = 0;
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
    if (idx != 0) {
        delta = sLookYawSteps[idx];
        threshold = sLookYawLimits[idx];
        sum = delta + self->lookYaw;
        if ((sum >= 0) ? (sum < threshold) : ((~sum + 1) < threshold)) {
            sTurnRotationYaw[0].num = delta;
            self->methods->updateRotation(self, 0, &sTurnRotationYaw[-1]);
            self->lookYaw = sum;
        }
        self->lookYawCommand = 0;
        flipTarget = self;
    } else if (self->lookYaw != 0) {
        delta = -LOOK_YAW_RETURN_STEP;
        if (self->lookYaw < 0)
            delta = LOOK_YAW_RETURN_STEP;
        sTurnRotationYaw[0].num = delta;
        self->methods->updateRotation(self, 0, &sTurnRotationYaw[-1]);
        self->lookYaw += delta;
        flipTarget = self;
    } else {
        return;
    }
    DreamSys__FlipMoveCommand(flipTarget);
}

void DreamSys__FlipMoveCommand(DreamSys *self) {
    self->moveCommandLatch = 0;
    if (self->moveCommand != 0) {
        if (self->moveCommand & 1)
            self->moveCommand = self->moveCommand + 1;
        else
            self->moveCommand = self->moveCommand - 1;
    }
}

void DreamSys__NoOpSlot14C(void) {}

void DreamSys__NoOpSlot150(void) {}

s32 DreamSys__TickMove(DreamSys *self) {
    if (self->moveOverride == 0) {
        self->methods->applyPendingTurn(self);
        return self->methods->tickMoveFree(self);
    } else if (self->moveOverride != 2) {
        return self->methods->tickMoveForced(self);
    } else {
        return self->methods->tickMoveHeld(self);
    }
}

s32 DreamSys__TickMoveFree(DreamSys *self) {
    if (self->movementBlocked != 0)
        return self->movementBlocked;
    return self->methods->applyMoveCommand(self, self->methods->advanceMoveCycle(self, 1));
}

s32 DreamSys__TickMoveForced(DreamSys *self) {
    self->moveCommand = MOVE_COMMAND_FORWARD;
    if (self->movementBlocked != 0)
        return self->methods->advanceMoveCycle(self, 0);
    return self->methods->applyMoveCommand(self, self->methods->advanceMoveCycle(self, 1));
}

s32 DreamSys__TickMoveHeld(DreamSys *self) {
    return self->moveCommand = MOVE_COMMAND_FORWARD;
}

s32 DreamSys__AdvanceMoveCycle(DreamSys *self, s32 bob) {
    s32 doCallback = 0;
    s32 ret = 0;
    s32 count;
    Viewport *viewport;
    s32 delta;

    if (self->moveCommand != 0) {
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

        if (self->moveCommand == 0)
            self->moveCycleTick = 0;
    }

    if (!doCallback)
        self->methods->stopVoice(self);
    return ret;
}

/* MATCHING: `headingArg` and `scratch` give their values the live ranges
   retail's register allocation needs; the plain form does not match. */
void DreamSys__StartVoice(DreamSys *self) {
    VabStreamObj *obj;
    s32 idx;
    VabStreamObjMethods *vt;
    s32 heading;
    s32 headingArg;
    s32 scratch;

    obj = (VabStreamObj *)self->soundObj;
    vt = obj->methods;
    idx = self->voiceSelect;
    if (idx == 0) {
        return;
    }

    scratch = sVoiceBySelect[idx];
    heading = scratch << 4;
    headingArg = heading;
    vt->setPitchOffset(obj, sVoicePitchBySelect[idx]);
    self->voiceIndex = vt->playTone(obj, headingArg, 110, 110);
    if (self->voiceSelect != 22) {
        self->voiceIndex = -1;
    }

    if (self->voiceSelect == 11) {
        vt->setPitchOffset(obj, 1);
        vt->playTone(obj, headingArg, 110, 110);
        vt->setPitchOffset(obj, 2);
        scratch = 9 << 4; /* program 9, tone 0 */
        vt->playTone(obj, scratch, 110, 110);
    }
}

void DreamSys__StopVoice(DreamSys *self) {
    VabStreamObj *obj;

    if (self->voiceIndex >= 0) {
        obj = (VabStreamObj *)self->soundObj;
        obj->methods->stopVoice(obj, self->voiceIndex);
        self->voiceIndex = -1;
    }
}

s32 DreamSys__ApplyMoveCommand(DreamSys *self, s32 command) {
    s32 delta;
    PlayerSpawnPoint *pos;

    if (command != 0) {
        delta = sMoveCommandSigns[command] * sMoveModeSpeeds[self->moveMode];
        self->methods->slot12C(self);
        pos = (PlayerSpawnPoint *)self->grid->methods->getTargetDescriptor(self->grid, 0, 0);
        if (!self->methods->tryStaircaseLink(self, pos) &&
            !self->methods->tryInstantTeleportLink(self, pos) &&
            !self->methods->tryTunnelLink(self, pos)) {
            self->methods->saveLinkSnapshot(self);
            sMoveCommandDispatch[command](self, delta, (void *)(self->staircaseMoveGate < 1));
            if (self->currentStage == 0 && self->coord2->coord.t[1] < -2000 &&
                self->coord2->coord.t[0] >= -499) {
                self->methods->onGridCellLinkCommand(self, self, 4);
            }
        }
        self->coord2->flg = 0;
    }
}

void DreamSys__ApplyPendingTurn(DreamSys *self) {
    s32 idx;

    idx = self->turnCommand;
    if (idx != 0) {
        self->methods->updateRotation(self, 0, sTurnRotations[idx]);
        self->turnCommand = 0;
    }
}

void DreamSys__TickDrift(DreamSys *self) {
    if (self->driftActive != 0) {
        self->methods->addTranslation(self, &sDriftStep);
        self->viewport->refView.vr.y -= 600;
    }
    if (self->cueServiceActive != 0)
        ServiceSoundCueSet((VabStreamObj *)self->soundObj, &self->soundCueSet);
}

void DreamSys__StopDrift(DreamSys *self, s32 keepCues) {
    self->driftActive = 0;
    self->cueServiceActive = keepCues;
    if (keepCues != 0)
        FlushSoundCueSet((VabStreamObj *)self->soundObj, &self->soundCueSet);
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
    if (!self->isFlashbackSession && outcome == 0) {
        self->methods->calcUnlockScore(self);
        self->methods->updateDreamChart(self, &self->moodPreviousDays[self->currentDay]);
        self->methods->advanceDay(self);
    } else if (outcome == 2) {
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
    if (idx >= self->amountFlashbacksAvailable) {
        goto fail;
    }
    self->state = DREAMSYS_LINK_FLASHBACK;
    entry = &self->storedFlashbacks[idx];
    if (!quiet) {
        self->methods->notifyParents(self, DREAMSYS_LINK_FLASHBACK);
    }
    self->currentDay = entry->day;
    self->currentStage = entry->stageID;
    self->linkCoordinates = entry->position;
    return true;
fail:
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
   `return false` fills two delay slots differently. */
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
        obj = (VabStreamObj *)system->soundObj;
        obj->methods->playTone(obj, 9 << 4, 110, 110);
    }
    return true;
}

/* MATCHING: one nested `if` chain, not early returns (a label after an
   early return keeps a redundant `move a0,s0`). The copy into
   staircaseGridPos/staircaseOrigin is one whole-PlayerSpawnPoint copy. */
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
        if ((u32)(self->staircaseFrame - 43) < 15 || (u32)(self->staircaseFrame - 75) < 15) {
            self->turnCommand = 2;
        }
    } else {
        if (self->staircaseFrame >= 19)
            return 1;
        if ((u32)(self->staircaseFrame - 8) < 2 || (u32)(self->staircaseFrame - 13) < 2) {
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
        if ((u32)(self->staircaseFrame - 22) < 15 || (u32)(self->staircaseFrame - 57) < 16 ||
            (u32)(self->staircaseFrame - 110) < 15) {
            self->turnCommand = 1;
        }
        flag = (u32)(self->staircaseFrame - 57) < 53;
    } else {
        if (self->staircaseFrame >= 25)
            return 1;
        if ((u32)(self->staircaseFrame - 6) < 2 || (u32)(self->staircaseFrame - 11) < 2 ||
            (u32)(self->staircaseFrame - 20) < 2) {
            self->methods->updateRotation(self, 0, sRotationYawMinus45);
        }
        flag = (u32)(self->staircaseFrame - 3) < 14;
    }
    if (flag) {
        self->lookOffsetCommand = 2;
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
            if ((u32)(self->staircaseFrame - 43) < 15) {
                self->turnCommand = 2;
            }
        } else {
            return 1;
        }
    } else {
        if (self->staircaseFrame < 15) {
            if ((u32)(self->staircaseFrame - 8) < 2) {
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
        if ((u32)(self->staircaseFrame - 30) < 15 || (u32)(self->staircaseFrame - 82) < 15) {
            self->turnCommand = 1;
        }
        flag = (u32)(self->staircaseFrame - 30) < 52;
    } else {
        if (self->staircaseFrame >= 19)
            return 1;
        if ((u32)(self->staircaseFrame - 6) < 2 || (u32)(self->staircaseFrame - 15) < 2) {
            self->methods->updateRotation(self, 0, sRotationYawMinus45);
        }
        flag = (u32)self->staircaseFrame < 9;
    }
    if (flag) {
        self->lookOffsetCommand = 2;
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
   pending: 4 notifies the entity back, 9 logs its mood and adds its unlock
   score (and may record a flashback), 10 links to the stage it names, 11
   ends the dream into its event video, 12 ends the dream. 9, 11 and 12 do
   nothing in a flashback session. */
void DreamSys__InstanceEffectsOnJournal(DreamSys *self, void *entity, s32 effect) {
    if (self->state != DREAMSYS_NO_LINK) {
        return;
    }

    switch (effect) {
        case 4:
            ((BasicClass *)entity)->methods->onNotify((BasicClass *)entity, self, effect);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
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
   the pair up in the 3x3 sDreamColorTable, [dynamic][upper]. MATCHING: the
   lookup goes through a row pointer; a flat `[d * 3 + u]` swaps two
   registers. */
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
        entry += (u32)self->tick % 9;
    }
    entry->stageID = stage;
    entry->position = *pos;
    entry->rotation = *(FlashbackRotation *)angles;
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
    i = 0;
    do {
        if (p[i] != 0)
            sum += NAV_CHALLENGE_SCORE;
        i++;
    } while (i < NAV_CHALLENGE_COUNT);
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

s32 GetRandomSpawnFromStage(PlayerSpawnPoint *target, s32 fromStage, s32 unused) {
    s32 stage;
    s32 index;
    StageSpawn *entry;
    s32 six;

    six = 6;
    if (fromStage >= 0) {
        stage = rand() % six;
        if (stage == fromStage) {
            stage++;
            if (stage >= 6)
                stage = 0;
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

/* Cardinal-direction indices, per stage: the player must face
   sTunnelEnterHeadings[sLinkSrcStage][sLinkTriggerIndex] to take the
   tunnel GetStaticSpawn matched, and leaves facing
   sTunnelExitHeadings[sLinkDstStage][sLinkSpawnIndex]. */
extern u8 *sTunnelEnterHeadings[];
extern u8 *sTunnelExitHeadings[];

/* The four cardinal rotations, yaw 0, 90, 180 and 270 degrees, in
   SceneNode__UpdateRotation's form. CheckTunnelHeading and
   CheckStaircaseHeading store an entry's address in enterRotation /
   exitRotation, which SetMoveOverride, SpawnAtLink and TryStaircaseLink
   apply. */
extern Ratio16 sCardinalRotations[][3];

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

/* The stage links that TryStageTimerLink takes: only stages 1, 3, 5, 9 and
 * 12 have one. Stage 5 links below y -4095 or at the one grid position
 * sStage5TriggerGridPos, stage 9 at y 2048 and up, the others anywhere. On an
 * odd timer the link lands on stage 12 (GetRandomSpawnFromStage's negative
 * form), otherwise away from the current stage. Returns the destination stage
 * (also sLinkDstStage), or -1 for no link. */
s32 TestForStageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer) {
    s32 result;

    if (stage != 3 && stage != 1 && stage != 5 && stage != 9 && stage != 12)
        return -1;
    if (stage == 5) {
        if (currentPos->position.y >= -4095 && *(s32 *)currentPos != sStage5TriggerGridPos)
            return -1;
    } else if (stage == 9) {
        if (currentPos->position.y < 2048)
            return -1;
    }
    if (timer & 1)
        stage = -12;
    result = GetRandomSpawnFromStage(target, stage, timer);
    sLinkDstStage = result;
    return result;
}

/* The rotation TryStageTimerLink stores in stageLinkAngle for every
   destination but stage 12. */
extern s32 sLinkAngle180;

s32 GetStageLinkAngle(void) {
    s32 result;

    result = 0;
    if (sLinkDstStage != 12)
        result = (s32)&sLinkAngle180;
    return result;
}

/* Set by SetInstantTeleportersEnabled (DreamAux.c calls it), tested by
   TestForInstantTeleporters. */
extern s32 sInstantTeleportersEnabled;

void SetInstantTeleportersEnabled(bool value) {
    sInstantTeleportersEnabled = value;
}

/* TestForInstantTeleporters' GetStaticSpawn tables: trigger counts,
   triggers and spawns per stage, as for tunnels and staircases. */
extern s8 sTeleportTriggersCount[];
extern StaticLinkTrigger *sTeleportTriggers[];
extern StageSpawn *sTeleportSpawns[];

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
    return (sLinkSrcStage == 0) ? 10 : 0;
}

s32 TestForStaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    if (stage == 0)
        return GetStaticSpawn(target, currentPos, 0, sStaircaseTriggersCount, sStaircaseTriggers,
                              sStaircaseSpawns, 0);
    return -1;
}

/* CheckStaircaseHeading's pair of heading tables, indexed as
   sTunnelEnterHeadings / sTunnelExitHeadings are. */
extern u8 *sStaircaseEnterHeadings[];
extern u8 *sStaircaseExitHeadings[];

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
                goto found;
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

    for (i = 0; (u32)i < 42; i++) {
        if (day == sSpecialDays[i]) {
            cinematic->entry = rand() % 6;
            cinematic->bank = i % 12;
            return &sSpecialDayMood;
        }
    }
    return NULL;
}
