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
 *    the look and move callbacks SelectCallback80/98 install. The look
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
 *    Test4.../GetStaticSpawn testers and the stage spawn tables. ExecuteLink
 *    records the link (enum DreamSysLinkCode) in Actor's `state` and tells
 *    the parents.
 *
 * What decided its edges (python3 tools/tuboundary.py): both are kept, and
 * the binary forces a file boundary in each one's stretch. Before it, the
 * jump tables of ObjM__OnDreamSysNotify (0x8001174c, ObjMStyleActor.c) and
 * DreamSys__OnPadEvent (0x80011788) differ in parity ("a forced boundary
 * lies in this stretch: tables 0x8001174c / 0x80011788"), and the edge
 * after GetGraphRoomMethods is the only gap left there outside this class.
 * After it, DreamSys__InstanceEffectsOnJournal (0x80011848) and
 * CheckDreamAuxTriggerCondition (0x8001188c, DreamAux.c) do the same
 * ("tables 0x80011848 / 0x8001188c"), and content puts that boundary at
 * the edge: DreamAux is its own subsystem, with its own .rodata line. */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <memory.h>
#include <rand.h>
#include "DreamSys.h"
#include "LinkResource.h"
#include "StageMap.h"
#include "LbdFile.h"
#include "VabStreamObj.h"
#include "Viewport.h"

/* With no look command pending, StepLookOffset springs the view height back
 * towards 0 by this much a tick (the size of one LOOK_OFFSET_STEPS step), and
 * StepLookYaw turns back by this many degrees (one LOOK_YAW_STEPS step). */
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

/* code_179d8's SoundCueSet service pair, as TickDrift and StopDrift call
   them: (soundObj, soundCueSet). Local, because Entity.h declares the same
   two functions with other parameter types. */
extern void FlushSoundCueSet(s32 arg0, void *arg1);
extern void ServiceSoundCueSet(s32 arg0, void *arg1);

/* Defined further down, in ROM order, and called before that. */
s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
void DreamSys__FlipMoveCommand(DreamSys *this);

DreamSys *New_DreamSys(LinkResource *modelSource, s32 soundObj, s32 viewport) {
    DreamSys *this;

    this = BMemPMgrAlloc(sizeof(DreamSys));
    if (this != NULL) {
        Get_vtable_DreamSys()->ctor(this, modelSource, soundObj, viewport);
        return this;
    }
    return NULL;
}

DreamSys *DreamSys__DreamSys(DreamSys *this, LinkResource *modelSource, s32 soundObj, s32 viewport) {
    void *model;

    GetActorMethods()->ctor((Actor *)this);
    this->methods = Get_vtable_DreamSys();
    this->soundObj = soundObj;
    this->viewport = (Viewport *)viewport;
    this->etcTim = 0;
    this->modelSource = modelSource;
    model = modelSource->methods->getModel(modelSource, 0);
    this->methods->addChild(this, model);
    this->methods->getSetDreamTimeLimit(this, -1);
    this->movementBlocked = 1;
    this->moveOverride = 0;
    this->newGamePending = 1;
    this->methods->initNewGame(this);
    return ((DreamSysResetRetFn)this->methods->reset)(this);
}

void DreamSys__ResetSessionState(DreamSys *this) {
    this->methods->setDisplay(this, 0);
    this->methods->updateRotation(this, 1, &sRotationYaw180);
    this->lookCallback = NULL;
    this->moveCallback = NULL;
    this->soundCueSet.tag = 0;
    this->staircaseActive = 0;
    this->staircaseMoveGate = 0;
    this->staircaseTickFn = 0;
    this->unk_0x78 = 0;
    this->unk_0x924 = 0;
}

void DreamSys__SpawnAtLink(DreamSys *this, StageMap *grid) {
    s32 attachPos[4];

    grid->methods->setTargetAndLoadChunks(grid, attachPos, (SceneNode *)this,
                                          (Descriptor10 *)&this->linkCoordinates);
    GetActorMethods()->attachToParent((Actor *)this, (SceneNode *)grid, (LongVec3 *)attachPos);
    this->methods->addChild(this, (BasicClass *)grid);
    if (this->state == DREAMSYS_LINK_FLASHBACK) {
        FlashbackEntry *entry = &this->storedFlasbacks[this->currentFlashbackIndex];
        this->methods->updateRotation(this, 1, &entry->rotation);
        this->methods->getSetDreamTimeLimit(this, entry->timeLimit + 4);
        this->currentFlashbackIndex++;
    }
    if (this->moveOverride != 0 && this->exitRotation != 0) {
        this->methods->updateRotation(this, 1, (void *)this->exitRotation);
    }
}

void DreamSys__DetachFromParent(DreamSys *this) {
    this->grid->methods->disable(this->grid);
    this->methods->removeChild(this, (BasicClass *)this->grid);
    GetActorMethods()->detachFromParent((Actor *)this);
}

void DreamSys__NotifyLinkAttempt(DreamSys *this, s32 event) {
    s32 voice;

    GetActorMethods()->notifyWithHull((Actor *)this, event);
    if (event == -2)
        goto handle_neg2;
    if (event != -1)
        return;

    voice = this->linkTarget->flags36 & 0x7F;
    this->voiceSelect = voice;
    if (voice >= ARRAY_COUNT(VOICE_BY_SELECT))
        this->voiceSelect = 0;

    if (this->state == DREAMSYS_LINK_TUNNEL && this->voiceSelect == 0)
        this->voiceSelect = 2;

    if (this->currentStage != 9)
        return;
    goto shared_tail;

handle_neg2:
    if (this->grid->methods->findSlotForPosition(this->grid, (LongVec3 *)this->coord2->coord.t)
            ->loader->headerReady != 2)
        goto neg2_mismatch;

shared_tail:
    this->methods->tryStageTimerLink(
        this, (PlayerSpawnPoint *)this->grid->methods->getTargetDescriptor(this->grid, 0, 0));
    return;

neg2_mismatch:
    this->methods->restoreLinkSnapshot(this);
}

void DreamSys__OnPadEvent(DreamSys *this, s32 sender, s32 event) {
    if (this->moveOverride != 0)
        return;
    if (this->movementBlocked != 0)
        return;
    if (this->staircaseActive != 0)
        return;

    switch (event - 2) {
        case 0:
            this->moveCommand = MOVE_COMMAND_FORWARD;
            break;
        case 1:
            this->moveCommand = MOVE_COMMAND_BACK;
            break;
        case 2:
            this->turnCommand = 1;
            break;
        case 3:
            this->turnCommand = 2;
            break;
        case 4:
            this->lookOffsetCommand = 1;
            break;
        case 5:
            if (this->moveCommand == MOVE_COMMAND_FORWARD)
                this->methods->changeMoveMode(this, MOVE_MODE_RUN);
            break;
        case 6:
            this->lookOffsetCommand = 2;
            break;
        case 11:
            this->lookYawCommand = 2;
            break;
        case 12:
            this->moveCommand = MOVE_COMMAND_RIGHT;
            break;
        case 13:
            this->lookYawCommand = 1;
            break;
        case 14:
            this->moveCommand = MOVE_COMMAND_LEFT;
            break;
        case 23:
            this->linkCommandFlag = 1;
            break;
        case 32:
            this->methods->restorePreviousMoveMode(this);
            break;
        case 47:
            break;
    }
}

void DreamSys__TimerTick(DreamSys *this, s32 sender, s32 event) {
    s32 old;

    if (event != 2)
        return;

    old = this->tick;
    this->tick = old + 1;
    if ((u32)old < (u32)this->dreamTimeLimit)
        goto tick_only;

    if (this->isFlashbackSession) {
        if (this->state != DREAMSYS_NO_LINK || this->methods->loadNextFlashback(this, 0)) {
            /* MATCHING: keeps GCC from cross-jumping this branch to the identical
               `tick = 0; return;` tail after notifyParents. */
            __asm__("");
            this->tick = 0;
            return;
        }
    } else {
        this->methods->flashbackSaving(this, 0, 16);
    }
    this->methods->notifyParents(this, DREAMSYS_TIME_UP);
    this->tick = 0;
    return;

tick_only:
    this->methods->updateTickState(this);
    this->methods->runTickCallbacks(this);
}

void DreamSys__DispatchChunkChange(DreamSys *this, void *sender, s32 event) {
    GetActorMethods()->dispatchLinkCommand((Actor *)this, sender, event);
    if ((((BasicClass *)sender)->methods->header & 0xFFF) == STAGEMAP_CLASS_ID) {
        this->methods->processChunkChange(this, sender, event);
    }
}

void DreamSys__DispatchInstanceEffect(DreamSys *this, void *sender, s32 effect) {
    GetActorMethods()->onActorLinkCommand((Actor *)this, sender, effect);
    if ((((BasicClass *)sender)->methods->header & 0xFFFFF) == 0x1F234) {
        this->methods->instanceEffectsOnJournal(this, sender, effect);
    }
}

void DreamSys__WallLink(DreamSys *this, void *sender, int event) {
    GetActorMethods()->onGridCellLinkCommand((Actor *)this, sender, event);
    if (event != 4)
        return;
    if (this->state != DREAMSYS_NO_LINK)
        return;
    this->linkCoordinates =
        *(PlayerSpawnPoint *)this->grid->methods->getCurrentCellKey(this->grid, sender);
    if (!this->methods->staticWallLink(this, &this->linkCoordinates) && this->tickBoundary != 0) {
        this->methods->dynamicLink(this);
    }
    this->methods->restoreLinkSnapshot(this);
    this->methods->slotE8(this);
}

void DreamSys__NoOpSlotE8Default(void) {}

s32 DreamSys__GetSetFlashbackSession(DreamSys *this, DreamColors *out, s32 value) {
    s32 old;

    old = this->isFlashbackSession;
    if (value < 0) {
        *out = CalcDreamColor(&this->moodPreviousDays[this->currentDay]);
    } else {
        this->isFlashbackSession = value;
    }
    return old;
}

void DreamSys__SetMoveOverride(DreamSys *this, s32 value) {
    this->moveOverride = value;
    if (value != 0) {
        this->methods->getSetMoveMode(this, 1);
        if (this->enterRotation != 0)
            this->methods->updateRotation(this, 1, (void *)this->enterRotation);
    }
}

void DreamSys__ResetLinkState(DreamSys *this, s32 moveMode, s32 tickPeriod) {
    RotationRatios rotation;

    this->methods->logChunkMood(this, &this->linkCoordinates);
    this->methods->selectCallback80(this, LOOK_CALLBACK_STEP_LOOK);
    this->methods->selectCallback98(this, MOVE_CALLBACK_TICK_MOVE);
    this->methods->getSetMoveMode(this, moveMode);

    this->voiceIndex = -1;
    this->moveCycleTick = 0;
    this->voiceSelect = 0;
    this->moveCommand = MOVE_COMMAND_NONE;
    this->turnCommand = 0;
    this->lookOffsetCommand = 0;
    this->lookYawCommand = 0;
    this->lookOffset = 0;
    this->lookYaw = 0;
    this->methods->setGateFlags(this, 0, 1, 1, 1);

    this->methods->setTickPeriod(this, tickPeriod);

    this->nextCinematic.entry = -1;
    this->movementBlocked = 0;
    this->state = DREAMSYS_NO_LINK;
    this->linkCommandFlag = 0;
    this->staircaseActive = 0;
    this->staircaseMoveGate = 0;
    this->staircaseTickFn = 0;
    this->unk_0x78 = 0;
    SceneNode__GetRotationDegrees((SceneNode *)this, (Ratio16 *)&rotation);

    rotation.z.numerator = 0;
    rotation.z.denominator = 1;
    this->methods->updateRotation(this, 1, &rotation);
}

void DreamSys__BlockMovement(DreamSys *this) {
    this->movementBlocked = 1;
}

s32 DreamSys__GetLinkCommandFlag(DreamSys *this) {
    return this->linkCommandFlag;
}

s32 DreamSys__GetSetDreamTimeLimit(DreamSys *this, s32 value) {
    s32 result;

    if (value >= 0)
        value = value * DREAM_TICKS_PER_SECOND;
    result = this->dreamTimeLimit;
    this->dreamTimeLimit = value;
    if (result >= 0)
        result = (u32)result / DREAM_TICKS_PER_SECOND;
    return result;
}

s32 DreamSys__GetDreamTimerScaled(DreamSys *this) {
    return (u32)this->tick / DREAM_TICKS_PER_SECOND;
}

void DreamSys__SetSoundObj(DreamSys *this, s32 value) {
    this->soundObj = value;
}

void DreamSys__SetViewport(DreamSys *this, Viewport *value) {
    this->viewport = value;
}

void DreamSys__SetEtcTim(DreamSys *this, s32 value) {
    this->etcTim = value;
}

void DreamSys__UpdateTickState(DreamSys *this) {
    if (this->movementBlocked == 0) {
        this->linkCommandFlag = 0;
        this->tickBoundary = ((u32)this->tick % (u32)this->tickPeriod) == 0;
    }
}

void DreamSys__RunTickCallbacks(DreamSys *this) {
    if (this->lookCallback != NULL)
        this->lookCallback(this);
    if (this->moveCallback != NULL)
        this->moveCallback(this);
}

/* InterpolateKeyframeValue is defined right after its caller;
   IsVec3WithinRange is another unit's. */
extern s32 InterpolateKeyframeValue(DreamSysInterpPoint *from, DreamSysInterpPoint *to, s32 at);
extern s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b);

/* Converts the local offset (0, 0, dist) to a world position, takes its
   height from the viewport's two refView points interpolated at `dist`,
   plus the object's own world y (coord2's workm.t[1]), copies it to `out`,
   and when `reference` is given returns whether it lies within `tolerance`
   of it. */
s32 DreamSys__ProjectPointAtDistance(DreamSys *this, s32 *out, s32 dist, s32 *reference, s32 tolerance) {
    s32 worldPos[3];
    s32 height;
    long *worldTrans;
    s32 *offsetZ;

    offsetZ = &gProjectOffsetZ;
    *offsetZ = dist;
    SceneNode__LocalOffsetToWorldPos((SceneNode *)this, worldPos, offsetZ - 2, 0);

    height = InterpolateKeyframeValue((void *)&this->viewport->refView.vp,
                                      (void *)&this->viewport->refView.vr, dist);

    worldTrans = this->parent != 0 ? this->coord2->workm.t : 0;
    worldPos[1] = height + worldTrans[1];

    if (out != NULL) {
        *(LongVec3 *)out = *(LongVec3 *)worldPos;
    }

    if (reference != NULL)
        return IsVec3WithinRange(worldPos, tolerance, reference);
    return 0;
}

s32 InterpolateKeyframeValue(DreamSysInterpPoint *from, DreamSysInterpPoint *to, s32 at) {
    s32 scaledAt;
    s32 dt;
    s32 dv;

    scaledAt = at;
    scaledAt = scaledAt / 1024;
    dt = (to->position - from->position) / 1024;
    if (dt == 0)
        dt = 1;
    dv = to->value - from->value;
    return (dv * scaledAt) / dt + from->value;
}

void DreamSys__func_59590(DreamSys *this) {
    this->unk_0x7C = 0;
}

void DreamSys__func_59598(DreamSys *this) {
    this->unk_0x78 = 0;
}

s32 DreamSys__NoOpSlot12C(DreamSys *this) {
    return 0;
}

void DreamSys__ClearTickCallbacks(DreamSys *this, bool clearLook) {
    this->methods->selectCallback98(this, MOVE_CALLBACK_NONE);
    if (clearLook)
        this->methods->selectCallback80(this, LOOK_CALLBACK_NONE);
}

void DreamSys__SetTickCallbacks(DreamSys *this, s32 mode98, s32 mode80) {
    this->methods->selectCallback98(this, mode98);
    this->methods->selectCallback80(this, mode80);
}

void DreamSys__SelectCallback80(DreamSys *this, s32 mode) {
    DreamSysMethods *vt = this->methods;

    this->lookCallbackMode = mode;
    switch (mode) {
        case LOOK_CALLBACK_NONE:
            this->lookCallback = NULL;
            break;
        case LOOK_CALLBACK_STEP_LOOK:
            this->lookCallback = vt->stepLook;
            break;
        case LOOK_CALLBACK_SLOT14C:
            this->lookCallback = vt->slot14C;
            break;
        case LOOK_CALLBACK_SLOT150:
            this->lookCallback = vt->slot150;
            break;
    }
}

extern void InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, DreamSys *arg3, void *arg4);

void DreamSys__SelectCallback98(DreamSys *this, s32 mode) {
    DreamSysMethods *vt = this->methods;

    if (this->moveCallbackMode == MOVE_CALLBACK_TICK_DRIFT)
        vt->stopDrift(this, 0);
    this->moveCallbackMode = mode;
    switch (mode) {
        case MOVE_CALLBACK_NONE:
            this->moveCallback = NULL;
            break;
        case MOVE_CALLBACK_TICK_MOVE:
            this->moveCallback = (void (*)(DreamSys *))vt->tickMove;
            break;
        case MOVE_CALLBACK_TICK_DRIFT:
            this->moveCallback = vt->tickDrift;
            this->driftActive = 1;
            this->cueServiceActive = 1;
            InitSoundCueSet(this->soundObj, &this->soundCueSet, 1, this, this->methods->soundCueCallback);
            break;
    }
}

void DreamSys__StepLook(DreamSys *this) {
    this->methods->stepLookOffset(this);
    this->methods->stepLookYaw(this);
}

void DreamSys__StepLookOffset(DreamSys *this) {
    s32 idx;
    s32 delta;
    s32 threshold;
    s32 sum;

    idx = this->lookOffsetCommand;
    if (idx != 0) {
        delta = LOOK_OFFSET_STEPS[idx];
        threshold = LOOK_OFFSET_LIMITS[idx];
        sum = delta + this->lookOffset;
        if (sum >= 0) {
            if (sum < threshold)
                goto apply;
            this->lookOffsetCommand = 0;
            return;
        }
        if ((~sum + 1) >= threshold) {
            this->lookOffsetCommand = 0;
            return;
        }
    apply:
        this->viewport->refView.vr.y += delta;
        this->lookOffset = sum;
        this->lookOffsetCommand = 0;
        return;
    }
    if (this->lookOffset != 0) {
        delta = -LOOK_OFFSET_RETURN_STEP;
        if (this->lookOffset < 0)
            delta = LOOK_OFFSET_RETURN_STEP;
        this->viewport->refView.vr.y += delta;
        this->lookOffset += delta;
    }
}

void DreamSys__StepLookYaw(DreamSys *this) {
    s32 idx;
    s32 delta;
    s32 threshold;
    s32 sum;
    /* MATCHING: a second name for `this`, set on each path before the shared
       tail call; passing `this` straight to it is one word short. */
    DreamSys *flipTarget;

    this->moveCommandLatch = (this->moveCommand == MOVE_COMMAND_FORWARD);
    idx = this->lookYawCommand;
    if (idx != 0) {
        delta = LOOK_YAW_STEPS[idx];
        threshold = LOOK_YAW_LIMITS[idx];
        sum = delta + this->lookYaw;
        if ((sum >= 0) ? (sum < threshold) : ((~sum + 1) < threshold)) {
            TURN_ROTATION_YAW[0].numerator = delta;
            this->methods->updateRotation(this, 0, &TURN_ROTATION_YAW[-1]);
            this->lookYaw = sum;
        }
        this->lookYawCommand = 0;
        flipTarget = this;
    } else if (this->lookYaw != 0) {
        delta = -LOOK_YAW_RETURN_STEP;
        if (this->lookYaw < 0)
            delta = LOOK_YAW_RETURN_STEP;
        TURN_ROTATION_YAW[0].numerator = delta;
        this->methods->updateRotation(this, 0, &TURN_ROTATION_YAW[-1]);
        this->lookYaw += delta;
        flipTarget = this;
    } else {
        return;
    }
    DreamSys__FlipMoveCommand(flipTarget);
}

void DreamSys__FlipMoveCommand(DreamSys *this) {
    this->moveCommandLatch = 0;
    if (this->moveCommand != 0) {
        if (this->moveCommand & 1)
            this->moveCommand = this->moveCommand + 1;
        else
            this->moveCommand = this->moveCommand - 1;
    }
}

void DreamSys__NoOpSlot14C(void) {}

void DreamSys__NoOpSlot150(void) {}

s32 DreamSys__TickMove(DreamSys *this) {
    if (this->moveOverride == 0) {
        this->methods->applyPendingTurn(this);
        return this->methods->tickMoveFree(this);
    } else if (this->moveOverride != 2) {
        return this->methods->tickMoveForced(this);
    } else {
        return this->methods->tickMoveHeld(this);
    }
}

s32 DreamSys__TickMoveFree(DreamSys *this) {
    if (this->movementBlocked != 0)
        return this->movementBlocked;
    return this->methods->applyMoveCommand(this, this->methods->advanceMoveCycle(this, 1));
}

s32 DreamSys__TickMoveForced(DreamSys *this) {
    this->moveCommand = MOVE_COMMAND_FORWARD;
    if (this->movementBlocked != 0)
        return this->methods->advanceMoveCycle(this, 0);
    return this->methods->applyMoveCommand(this, this->methods->advanceMoveCycle(this, 1));
}

s32 DreamSys__TickMoveHeld(DreamSys *this) {
    return this->moveCommand = MOVE_COMMAND_FORWARD;
}

s32 DreamSys__AdvanceMoveCycle(DreamSys *this, s32 bob) {
    s32 doCallback = 0;
    s32 ret = 0;
    s32 count;
    Viewport *viewport;
    s32 delta;

    if (this->moveCommand != 0) {
        ret = this->moveCommand;
        count = this->moveCycleTick + 1;
        this->moveCycleTick = count;
        if (count < MOVE_CYCLE_TICKS) {
            doCallback = (this->moveMode == MOVE_MODE_RUN) && ((count & 1) == 0);
        } else {
            this->moveCommand = MOVE_COMMAND_NONE;
            doCallback = 1;
        }

        if (doCallback)
            this->methods->startVoice(this);

        viewport = this->viewport;
        if (viewport != NULL && this->screenShakeOn != 0 && bob != 0) {
            delta = -MOVE_BOB_HEIGHT;
            if (this->moveCycleTick >= 3)
                delta = MOVE_BOB_HEIGHT;
            viewport->refView.vp.y += delta;
            viewport->refView.vr.y += delta;
        }

        if (this->moveCommand == 0)
            this->moveCycleTick = 0;
    }

    if (!doCallback)
        this->methods->stopVoice(this);
    return ret;
}

/* MATCHING: `headingArg` and `scratch` give their values the live ranges
   retail's register allocation needs; the plain form does not match. */
void DreamSys__StartVoice(DreamSys *this) {
    VabStreamObj *obj;
    s32 idx;
    VabStreamObjMethods *vt;
    s32 heading;
    s32 headingArg;
    s32 scratch;

    obj = (VabStreamObj *)this->soundObj;
    vt = obj->methods;
    idx = this->voiceSelect;
    if (idx == 0) {
        return;
    }

    scratch = VOICE_BY_SELECT[idx];
    heading = scratch << 4;
    headingArg = heading;
    vt->setPitchOffset(obj, VOICE_PITCH_BY_SELECT[idx]);
    this->voiceIndex = vt->playTone(obj, headingArg, 110, 110);
    if (this->voiceSelect != 22) {
        this->voiceIndex = -1;
    }

    if (this->voiceSelect == 11) {
        vt->setPitchOffset(obj, 1);
        vt->playTone(obj, headingArg, 110, 110);
        vt->setPitchOffset(obj, 2);
        scratch = 9 << 4; /* program 9, tone 0 */
        vt->playTone(obj, scratch, 110, 110);
    }
}

void DreamSys__StopVoice(DreamSys *this) {
    VabStreamObj *obj;

    if (this->voiceIndex >= 0) {
        obj = (VabStreamObj *)this->soundObj;
        obj->methods->stopVoice(obj, this->voiceIndex);
        this->voiceIndex = -1;
    }
}

s32 DreamSys__ApplyMoveCommand(DreamSys *this, s32 command) {
    s32 delta;
    PlayerSpawnPoint *pos;

    if (command != 0) {
        delta = sMoveCommandSigns[command] * sMoveModeSpeeds[this->moveMode];
        this->methods->slot12C(this);
        pos = (PlayerSpawnPoint *)this->grid->methods->getTargetDescriptor(this->grid, 0, 0);
        if (!this->methods->tryStaircaseLink(this, pos) &&
            !this->methods->tryInstantTeleportLink(this, pos) &&
            !this->methods->tryTunnelLink(this, pos)) {
            this->methods->saveLinkSnapshot(this);
            sMoveCommandDispatch[command](this, delta, (void *)(this->staircaseMoveGate < 1));
            if (this->currentStage == 0 && this->coord2->coord.t[1] < -2000 &&
                this->coord2->coord.t[0] >= -499) {
                this->methods->onGridCellLinkCommand(this, this, 4);
            }
        }
        this->coord2->flg = 0;
    }
}

void DreamSys__ApplyPendingTurn(DreamSys *this) {
    s32 idx;

    idx = this->turnCommand;
    if (idx != 0) {
        this->methods->updateRotation(this, 0, &TURN_ROTATIONS[idx]);
        this->turnCommand = 0;
    }
}

void DreamSys__TickDrift(DreamSys *this) {
    if (this->driftActive != 0) {
        this->methods->addTranslation(this, &DRIFT_STEP);
        this->viewport->refView.vr.y -= 600;
    }
    if (this->cueServiceActive != 0)
        ServiceSoundCueSet(this->soundObj, &this->soundCueSet);
}

void DreamSys__StopDrift(DreamSys *this, s32 keepCues) {
    this->driftActive = 0;
    this->cueServiceActive = keepCues;
    if (keepCues != 0)
        FlushSoundCueSet(this->soundObj, &this->soundCueSet);
}

s32 DreamSys__GetSetMoveMode(DreamSys *this, s32 value) {
    s32 old;

    old = this->moveMode;
    if (value >= 0) {
        this->moveMode = value;
        this->previousMoveMode = value;
    }
    return old;
}

void DreamSys__ChangeMoveMode(DreamSys *this, s32 value) {
    s32 old;

    old = this->moveMode;
    if (old != value) {
        this->previousMoveMode = old;
        this->moveMode = value;
    }
}

void DreamSys__RestorePreviousMoveMode(DreamSys *this) {
    this->moveMode = this->previousMoveMode;
}

void DreamSys__SetGateFlags(DreamSys *this, s32 a, s32 b, s32 c, s32 d) {
    if (a >= 0)
        this->tickBoundary = a;
    if (b >= 0)
        this->unk_0x128 = b;
    if (c >= 0)
        this->unk_0x12C = c;
    if (d >= 0)
        this->unk_0x130 = d;
}

void DreamSys__SetTickPeriod(DreamSys *this, s32 value) {
    this->tickPeriod = value;
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
extern s32 SAVE_MAGIC;

void DreamSys__InitNewGame(DreamSys *this) {
    this->saveMagic = SAVE_MAGIC;
    this->currentYear = 0;
    this->currentDay = 0;
    this->totalFlasbackUnlockScore = 0;
    this->navigationFlasbackUnlockScore = 0;
    this->instanceFlasbackUnlockScore = 0;
    this->amountFlashbacksAvailable = 0;
    this->unknown_values_0x5d8[7] = 0;
    this->unknown_values_0x5d8[0] = 0;
    this->screenShakeOn = 1;
    this->unknown_word_0x67c = 0;
    this->unknown_word_0x680 = 0;
    InitNavChallengesArray(&this->navChallengesArray, &this->amountDynamicLinksDone);
    memset(this->unknown_values_0x684, 0, sizeof(this->unknown_values_0x684));
}

void DreamSys__GetSetScreenShake(DreamSys *this, bool *value) {
    bool old;

    old = this->screenShakeOn;
    this->screenShakeOn = *value;
    *value = old;
}

s32 DreamSys__GetCurrentDayAndYear(DreamSys *this, s32 *outYear) {
    if (outYear != NULL)
        *outYear = this->currentYear;
    return this->currentDay + 1;
}

s32 DreamSys__AdvanceDay(DreamSys *this) {
    this->currentDay++;
    if (this->currentDay >= DAYS_PER_YEAR) {
        this->currentDay = 0;
        this->currentYear++;
    }
    return this->currentDay;
}

void DreamSys__ClearNewGameFlag(DreamSys *this) {
    this->newGamePending = 0;
}

s32 DreamSys__GetNewGameFlag(DreamSys *this) {
    return this->newGamePending;
}

s32 *DreamSys__GetSaveBlock(DreamSys *this, s32 *outSize) {
    if (outSize != NULL)
        *outSize = DREAMSYS_SAVE_SIZE;
    return &this->saveMagic;
}

s32 DreamSys__StartDay(DreamSys *this) {
    s32 oldDay;
    MoodGraphPoint *special;

    oldDay = this->currentDay;
    this->currentFlashbackIndex = 0;
    this->tick = 0;
    this->storedDay = oldDay;
    if (this->isFlashbackSession) {
        this->methods->loadNextFlashback(this, 1);
    } else {
        special = IsDaySpecial(&this->nextCinematic, this->currentDay + 1);
        this->methods->initMoodContributors(this, special);
        if (special != NULL) {
            return -1;
        }
        this->methods->initSpawnLoc(this);
    }
    return this->currentStage;
}

s32 DreamSys__EndDay(DreamSys *this, s32 outcome) {
    this->currentDay = this->storedDay;
    if (!this->isFlashbackSession && outcome == 0) {
        this->methods->calcUnlockScore(this);
        this->methods->updateDreamChart(this, &this->moodPreviousDays[this->currentDay]);
        this->methods->advanceDay(this);
    } else if (outcome == 2) {
        this->methods->initNewGame(this);
        this->newGamePending = 1;
    }
    return this->isFlashbackSession;
}

CinematicCall DreamSys__GetCinematic(DreamSys *this) {
    return this->nextCinematic;
}

void DreamSys__InitSpawnLoc(DreamSys *this) {
    MoodGraphPoint mood;
    s32 timeLimit;

    this->methods->getPreviousDayMood(this, &mood, 1);
    this->currentStage =
        GenerateInitialSpawn(&this->linkCoordinates, &timeLimit, &mood, this->currentDay);
    timeLimit = this->methods->getSetDreamTimeLimit(this, timeLimit);
    this->state = DREAMSYS_LINK_DAY_START;
}

void DreamSys__DynamicLink(DreamSys *this) {
    s32 stage;

    if (this->state == DREAMSYS_NO_LINK) {
        stage = GetRandomSpawnFromStage(&this->linkCoordinates, this->currentStage, this->tick);
        ExecuteLink(this, stage, DREAMSYS_LINK_DYNAMIC, 1);
    }
}

bool DreamSys__StaticWallLink(DreamSys *this, PlayerSpawnPoint *currentPos) {
    s32 result;

    if (this->state != DREAMSYS_NO_LINK)
        return false;
    result = TestForStaticLink(&this->linkCoordinates, currentPos, this->currentStage);
    if (result < 0)
        return false;
    ExecuteLink(this, result, DREAMSYS_LINK_WALL, 1);
    return true;
}

bool DreamSys__LoadNextFlashback(DreamSys *this, bool quiet) {
    s32 idx;
    FlashbackEntry *entry;

    idx = this->currentFlashbackIndex;
    if (idx >= this->amountFlashbacksAvailable) {
        goto fail;
    }
    this->state = DREAMSYS_LINK_FLASHBACK;
    entry = &this->storedFlasbacks[idx];
    if (!quiet) {
        this->methods->notifyParents(this, DREAMSYS_LINK_FLASHBACK);
    }
    this->currentDay = entry->day;
    this->currentStage = entry->stageID;
    this->linkCoordinates = entry->position;
    return true;
fail:
    return false;
}

bool DreamSys__TryTunnelLink(DreamSys *this, PlayerSpawnPoint *currentPos) {
    s32 result;
    s32 rotation[4];

    if (this->state != DREAMSYS_NO_LINK)
        return false;
    result = Test4TunnelLinks(&this->linkCoordinates, currentPos, this->currentStage);
    if (result < 0)
        return false;
    SceneNode__GetRotationDegrees((SceneNode *)this, (Ratio16 *)rotation);
    if (!DreamSys__CheckTunnelHeading(&this->exitRotation, &this->enterRotation, rotation))
        return false;
    if (this->moveCommandLatch == 0)
        return false;
    ExecuteLink(this, result, DREAMSYS_LINK_TUNNEL, 0);
    return true;
}

bool DreamSys__TryStageTimerLink(DreamSys *this, PlayerSpawnPoint *currentPos) {
    s32 result;

    if (this->state != DREAMSYS_NO_LINK)
        return false;
    result = Test4StageTransition(&this->linkCoordinates, this->currentStage, currentPos, this->tick);
    if (result < 0)
        return false;
    this->stageLinkAngle = GetStageLinkAngle();
    this->enterRotation = 0;
    this->exitRotation = 0;
    ExecuteLink(this, result, DREAMSYS_LINK_STAGE_TIMER, 0);
    return true;
}

/* MATCHING: the body nests inside `if (result >= 0)`; an early
   `return false` fills two delay slots differently. */
bool DreamSys__TryInstantTeleportLink(DreamSys *this, PlayerSpawnPoint *currentPos) {
    s32 result;
    s32 bonus;
    s32 cellPos[4];

    result = Test4InstantTeleporters(&this->linkCoordinates, currentPos, this->currentStage);
    if (result >= 0) {
        bonus = GetTeleportTimeBonus();
        if (ExecuteLink(this, result, DREAMSYS_LINK_TELEPORT, 0)) {
            this->state = DREAMSYS_NO_LINK;
            this->grid->methods->computeCellOffsets(this->grid, cellPos, &this->linkCoordinates);
            this->methods->setTranslation(this, (LongVec3 *)cellPos);
            if (bonus != 0 && !this->isFlashbackSession)
                this->methods->getSetDreamTimeLimit(this, this->methods->getDreamTimerScaled(this) + bonus);
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
bool DreamSys__TryStaircaseLink(DreamSys *this, PlayerSpawnPoint *currentPos) {
    s32 rotation[4];

    if (this->state == DREAMSYS_NO_LINK) {
        if (this->staircaseTickFn != 0) {
            if (this->staircaseTickFn(this)) {
                this->staircaseActive = 0;
                this->staircaseTickFn = 0;
                this->staircaseMoveGate = 0;
                if (this->moveMode == MOVE_MODE_RUN) {
                    this->methods->restorePreviousMoveMode(this);
                }
            }
        } else if (Test4StaircaseNodes(&this->linkCoordinates, currentPos, this->currentStage) >= 0) {
            SceneNode__GetRotationDegrees((SceneNode *)this, (Ratio16 *)rotation);
            if (DreamSys__CheckStaircaseHeading(&this->exitRotation, &this->enterRotation, rotation) &&
                this->moveCommandLatch != 0) {
                *(PlayerSpawnPoint *)&this->staircaseGridPos = *currentPos;
                this->staircaseActive = 1;
                this->staircaseMoveGate = 1;
                this->staircaseFrame = 0;
                this->staircaseTickFn = STAIRCASE_TICK_FNS[GetLastSpawnExtra()];
                this->methods->updateRotation(this, 1, (void *)this->enterRotation);
                this->staircaseTickFn(this);
            }
        }
    }
    return false;
}

s32 DreamSys__TickStaircaseCase0(DreamSys *this) {
    if (this->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_0, &this->staircaseOrigin);
    }
    if (this->moveMode != MOVE_MODE_RUN) {
        if (this->staircaseFrame >= 133)
            return 1;
        if ((u32)(this->staircaseFrame - 43) < 15 || (u32)(this->staircaseFrame - 75) < 15) {
            this->turnCommand = 2;
        }
    } else {
        if (this->staircaseFrame >= 19)
            return 1;
        if ((u32)(this->staircaseFrame - 8) < 2 || (u32)(this->staircaseFrame - 13) < 2) {
            this->methods->updateRotation(this, 0, &ROTATION_YAW_PLUS45);
        }
    }
    this->moveCommand = MOVE_COMMAND_FORWARD;
    this->staircaseFrame++;
    return 0;
}

s32 DreamSys__TickStaircaseCase1(DreamSys *this) {
    s32 flag;

    if (this->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_1, &this->staircaseOrigin);
    }
    if (this->moveMode != MOVE_MODE_RUN) {
        if (this->staircaseFrame >= 149)
            return 1;
        if ((u32)(this->staircaseFrame - 22) < 15 || (u32)(this->staircaseFrame - 57) < 16 ||
            (u32)(this->staircaseFrame - 110) < 15) {
            this->turnCommand = 1;
        }
        flag = (u32)(this->staircaseFrame - 57) < 53;
    } else {
        if (this->staircaseFrame >= 25)
            return 1;
        if ((u32)(this->staircaseFrame - 6) < 2 || (u32)(this->staircaseFrame - 11) < 2 ||
            (u32)(this->staircaseFrame - 20) < 2) {
            this->methods->updateRotation(this, 0, &ROTATION_YAW_MINUS45);
        }
        flag = (u32)(this->staircaseFrame - 3) < 14;
    }
    if (flag) {
        this->lookOffsetCommand = 2;
    }
    this->moveCommand = MOVE_COMMAND_FORWARD;
    this->staircaseFrame++;
    return 0;
}

s32 DreamSys__TickStaircaseCase2(DreamSys *this) {
    if (this->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_2, &this->staircaseOrigin);
    }
    if (this->moveMode != MOVE_MODE_RUN) {
        if (this->staircaseFrame < 101) {
            if ((u32)(this->staircaseFrame - 43) < 15) {
                this->turnCommand = 2;
            }
        } else {
            return 1;
        }
    } else {
        if (this->staircaseFrame < 15) {
            if ((u32)(this->staircaseFrame - 8) < 2) {
                this->methods->updateRotation(this, 0, &ROTATION_YAW_PLUS45);
            }
        } else {
            return 1;
        }
    }
    this->moveCommand = MOVE_COMMAND_FORWARD;
    this->staircaseFrame++;
    return 0;
}

s32 DreamSys__TickStaircaseCase3(DreamSys *this) {
    s32 flag;

    if (this->staircaseFrame == 0) {
        DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_3, &this->staircaseOrigin);
    }
    if (this->moveMode != MOVE_MODE_RUN) {
        if (this->staircaseFrame >= 113)
            return 1;
        if ((u32)(this->staircaseFrame - 30) < 15 || (u32)(this->staircaseFrame - 82) < 15) {
            this->turnCommand = 1;
        }
        flag = (u32)(this->staircaseFrame - 30) < 52;
    } else {
        if (this->staircaseFrame >= 19)
            return 1;
        if ((u32)(this->staircaseFrame - 6) < 2 || (u32)(this->staircaseFrame - 15) < 2) {
            this->methods->updateRotation(this, 0, &ROTATION_YAW_MINUS45);
        }
        flag = (u32)this->staircaseFrame < 9;
    }
    if (flag) {
        this->lookOffsetCommand = 2;
    }
    this->moveCommand = MOVE_COMMAND_FORWARD;
    this->staircaseFrame++;
    return 0;
}

void DreamSys__ApplyRelativeOffset(DreamSys *this, struct RelativePos *a, struct RelativePos *b) {
    LongVec3 diff;

    diff.x = a->x - b->x;
    diff.y = a->y - b->y;
    diff.z = a->z - b->z;
    diff.y = 0;
    this->methods->addTranslation(this, &diff);
}

s32 DreamSys__GetCurrentStage(DreamSys *this) {
    return this->currentStage;
}

void DreamSys__ProcessChunkChange(DreamSys *this, void *entity, s32 effect) {
    PlayerSpawnPoint *pos;

    if (effect == 5) {
        pos =
            (PlayerSpawnPoint *)((StageMap *)entity)->methods->getTargetDescriptor((StageMap *)entity, 0, 0);
        this->methods->logChunkMood(this, pos);
    }
}

/* What an Entity's instance effect does to the dream, while no link is
   pending: 4 notifies the entity back, 9 logs its mood and adds its unlock
   score (and may record a flashback), 10 links to the stage it names, 11
   ends the dream into its event video, 12 ends the dream. 9, 11 and 12 do
   nothing in a flashback session. */
void DreamSys__InstanceEffectsOnJournal(DreamSys *this, void *entity, s32 effect) {
    if (this->state != DREAMSYS_NO_LINK) {
        return;
    }

    switch (effect) {
        case 4:
            ((BasicClass *)entity)->methods->onNotify((BasicClass *)entity, this, effect);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            break;
        case 9:
            if (this->isFlashbackSession != 0) {
                return;
            }
            this->methods->logInstanceMood(
                this, ((DreamSysEntityObj *)entity)->methods->getMoodEffect(entity));
            this->instanceFlasbackUnlockScore +=
                ((DreamSysEntityObj *)entity)->methods->getUnlockEffect(entity);
            this->methods->flashbackSaving(this, 0, 16);
            break;
        case 10: {
            s32 previousStage = this->currentStage;
            this->currentStage = -((DreamSysEntityObj *)entity)->methods->getLinkStage(entity);
            this->methods->dynamicLink(this);
            if (this->currentStage < 0) {
                this->currentStage = previousStage;
            }
            break;
        }
        case 11:
            if (this->isFlashbackSession != 0) {
                return;
            }
            this->nextCinematic.bank = -1;
            this->tick = this->dreamTimeLimit;
            this->nextCinematic.entry = ((DreamSysEntityObj *)entity)->methods->getEventVideo(entity);
            break;
        case 12:
            if (this->isFlashbackSession != 0) {
                return;
            }
            this->tick = this->dreamTimeLimit;
            break;
    }
}

void DreamSys__GetPreviousDayMood(DreamSys *this, MoodGraphPoint *target, bool lastDayOnly) {
    s32 upper = 0;
    s32 dynamic = 0;

    if (lastDayOnly) {
        if (this->currentYear != 0 || this->currentDay != 0) {
            s32 idx;

            idx = this->currentDay - 1;
            dynamic = this->moodPreviousDays[idx].axis.dynamic;
            upper = this->moodPreviousDays[idx].axis.upper;
        }
    } else {
        s32 count;
        count = DAYS_PER_YEAR;
        if (this->currentYear == 0)
            count = this->currentDay;
        if (count != 0) {
            MoodGraphPoint *p;
            s32 i;

            p = this->moodPreviousDays;
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

void DreamSys__InitMoodContibutors(DreamSys *this, MoodGraphPoint *special) {
    this->methods->clearMoodGraph(this, &this->areaMoods);
    this->methods->clearMoodGraph(this, &this->entityMoods);
    if (special != NULL) {
        this->methods->logMood(this, &this->areaMoods, special);
        this->methods->logMood(this, &this->entityMoods, special);
    }
}

void DreamSys__LogChunkMood(DreamSys *this, PlayerSpawnPoint *currentPos) {
    MoodGraphPoint *mood;

    mood = GetMoodFromStageChunk(this->currentStage, (StageChunk *)currentPos);
    this->methods->logMood(this, &this->areaMoods, mood);
}

void DreamSys__LogInstanceMood(DreamSys *this, MoodGraphPoint *source) {
    this->methods->logMood(this, &this->entityMoods, source);
}

void DreamSys__UpdateDreamChart(DreamSys *this, MoodGraphPoint *ret) {
    MoodGraphPoint areaAvg;
    MoodGraphPoint entityAvg;

    this->methods->getMoodAverage(this, &this->areaMoods, &areaAvg);
    this->methods->getMoodAverage(this, &this->entityMoods, &entityAvg);
    if (this->entityMoods.amountMoods == 0) {
        entityAvg.value = areaAvg.value;
    }
    ret->axis.dynamic = (areaAvg.axis.dynamic + entityAvg.axis.dynamic) / 2;
    ret->axis.upper = (areaAvg.axis.upper + entityAvg.axis.upper) / 2;
}

DreamColors DreamSys__GetDreamColor(DreamSys *this) {
    MoodGraphPoint local;

    this->methods->updateDreamChart(this, &local);
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

void DreamSys__ClearMoodGraph(DreamSys *this, MoodGraphContributor *contributor) {
    contributor->lastMood.value = 0;
    contributor->sumMoods.upper = 0;
    contributor->sumMoods.dynamic = 0;
    contributor->amountMoods = 0;
}

void DreamSys__LogMood(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *mood) {
    layer->lastMood.value = mood->value;
    layer->sumMoods.dynamic = mood->axis.dynamic + layer->sumMoods.dynamic;
    layer->sumMoods.upper = mood->axis.upper + layer->sumMoods.upper;
    layer->amountMoods = layer->amountMoods + 1;
}

void DreamSys__GetMoodAverage(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *ret) {
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

void DreamSys__CalcUnlockScore(DreamSys *this) {
    this->navigationFlasbackUnlockScore = CalcNavigationScore();
    if (this->instanceFlasbackUnlockScore < 0) {
        this->instanceFlasbackUnlockScore = 0;
    } else if (this->instanceFlasbackUnlockScore > UNLOCK_SCORE_MAX) {
        this->instanceFlasbackUnlockScore = UNLOCK_SCORE_MAX;
    }
    this->totalFlasbackUnlockScore =
        this->navigationFlasbackUnlockScore + this->instanceFlasbackUnlockScore;
}

void DreamSys__AddFlashback(DreamSys *this, s32 stage, PlayerSpawnPoint *pos, s32 *angles,
                            s32 unknown, s32 time, s32 day) {
    FlashbackEntry *entry;

    entry = this->storedFlasbacks;
    if (this->amountFlashbacksAvailable < ARRAY_COUNT(this->storedFlasbacks)) {
        entry += this->amountFlashbacksAvailable++;
    } else {
        entry += (u32)this->tick % 9;
    }
    entry->stageID = stage;
    entry->position = *pos;
    entry->rotation = *(FlashbackRotation *)angles;
    entry->unknown_value_0x1c = unknown;
    entry->timeLimit = time;
    entry->day = day;
}

void DreamSys__FlashbackSaving(DreamSys *this, s32 unknown, s32 timeLimit) {
    PlayerSpawnPoint *pos;
    s32 rotation[4];

    if (this->grid != NULL && rand() % 3 == 0) {
        pos = (PlayerSpawnPoint *)this->grid->methods->getTargetDescriptor(this->grid, 0, 0);
        SceneNode__GetRotationDegrees((SceneNode *)this, (Ratio16 *)rotation);
        this->methods->addFlashback(this, this->currentStage, pos, rotation, unknown, timeLimit,
                                    this->currentDay);
    }
}

void DreamSys__ResetFlashbackList(DreamSys *this) {
    this->amountFlashbacksAvailable = 0;
}

void DreamSys__SaveLinkSnapshot(DreamSys *this) {
    GsCOORDINATE2 *p = this->coord2;

    this->coord2Snapshot = *p;
    this->coord2ParamSnapshot = *p->param;
}

void DreamSys__RestoreLinkSnapshot(DreamSys *this) {
    GsCOORDINATE2 *p = this->coord2;

    *p = this->coord2Snapshot;
    *p->param = this->coord2ParamSnapshot;
    p->flg = 0;
}

s32 DreamSys__func_5ba20(DreamSys *this, s32 value) {
    s32 old;

    if (value >= 0) {
        old = this->unk_0x924;
        this->unk_0x924 = value;
    } else {
        old = this->unk_0x924;
    }
    return old;
}

DreamSysMethods *Get_vtable_DreamSys(void) {
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

s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    return GetStaticSpawn(target, currentPos, stage, LEN_TUNNEL_TRIGGERS, TUNNEL_TRIGGERS,
                          TUNNEL_SPAWNS, 1);
}

/* The rotation SceneNode__GetRotationDegrees writes, as IsHeadingAligned
   reads it: +4 is the yaw in whole degrees, read unsigned. A local view,
   because Ratio16 reads the same bytes signed. */
typedef struct DirectionCheckArg {
    u8 pad0[4];
    u16 heading;
} DirectionCheckArg;

/* CARDINAL_ROTATIONS seen from its yaw: this label starts 4 bytes into that
   table, so `angle` is CARDINAL_ROTATIONS[i].y.numerator (0, 90, 180, 270
   degrees). A separate view, because this one reads the angle as a number
   and the other is only handed to SceneNode__UpdateRotation. */
typedef struct DirectionTableEntry {
    u16 angle;
    u16 pad2[5];
} DirectionTableEntry;

extern DirectionTableEntry CARDINAL_ANGLES[];

/* Defined below, in ROM order. */
extern s32 IsHeadingAligned(DirectionCheckArg *rotation, u8 heading);

/* Cardinal-direction indices, per stage: the player must face
   TUNNEL_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex] to take the
   tunnel GetStaticSpawn matched, and leaves facing
   TUNNEL_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex]. */
extern u8 *TUNNEL_ENTER_HEADINGS[];
extern u8 *TUNNEL_EXIT_HEADINGS[];

/* The four cardinal rotations, yaw 0, 90, 180 and 270 degrees, in
   SceneNode__UpdateRotation's form. CheckTunnelHeading and
   CheckStaircaseHeading store an entry's address in enterRotation /
   exitRotation, which SetMoveOverride, SpawnAtLink and TryStaircaseLink
   apply. */
extern RotationRatios CARDINAL_ROTATIONS[];

s32 DreamSys__CheckTunnelHeading(s32 *outExit, s32 *outEnter, void *rotation) {
    u8 heading;
    s32 idx;
    s32 result;

    heading = TUNNEL_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex];
    if (IsHeadingAligned((DirectionCheckArg *)rotation, heading)) {
        if (outEnter != NULL)
            *outEnter = (s32)&CARDINAL_ROTATIONS[heading];

        if (outExit != NULL) {
            idx = TUNNEL_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex];
            *outExit = (s32)&CARDINAL_ROTATIONS[idx];
        }
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

s32 IsHeadingAligned(DirectionCheckArg *rotation, u8 heading) {
    s16 diff;

    diff = rotation->heading - CARDINAL_ANGLES[heading].angle;
    if (diff >= 181) {
        diff -= 360;
    } else if (diff < -180) {
        diff += 360;
    }
    return (u16)(diff + 44) < 89;
}

/* Compared against the leading 4 bytes (chunk+tile) of `currentPos` as a
   raw word; only ever compared here, never dereferenced field-by-field. */
extern s32 STAGE5_TRIGGER_GRIDPOS;

s32 Test4StageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer) {
    s32 result;

    if (stage == 3)
        goto shared;
    if (stage == 1)
        goto shared;
    if (stage == 5)
        goto case5;
    if (stage == 9)
        goto shared;
    if (stage != 12)
        return -1;

shared:
    if (stage != 5)
        goto case9check;
case5:
    if (currentPos->position.y < -4095)
        goto merge;
    if (*(s32 *)currentPos == STAGE5_TRIGGER_GRIDPOS)
        goto merge;
    return -1;

case9check:
    if (stage != 9)
        goto merge;
    if (currentPos->position.y < 2048)
        return -1;

merge:
    if (timer & 1)
        stage = -12;
    result = GetRandomSpawnFromStage(target, stage, timer);
    gLinkDstStage = result;
    return result;
}

/* The rotation TryStageTimerLink stores in stageLinkAngle for every
   destination but stage 12. */
extern s32 LINK_ANGLE_180;

s32 GetStageLinkAngle(void) {
    s32 result;

    result = 0;
    if (gLinkDstStage != 12)
        result = (s32)&LINK_ANGLE_180;
    return result;
}

/* Set by SetInstantTeleportersEnabled (DreamAux.c calls it), tested by
   Test4InstantTeleporters. */
extern s32 gInstantTeleportersEnabled;

void SetInstantTeleportersEnabled(bool value) {
    gInstantTeleportersEnabled = value;
}

/* Test4InstantTeleporters' GetStaticSpawn tables: trigger counts,
   triggers and spawns per stage, as for tunnels and staircases. */
extern s8 LEN_TELEPORT_TRIGGERS[];
extern StaticLinkTrigger *TELEPORT_TRIGGERS[];
extern StageSpawn *TELEPORT_SPAWNS[];

s32 Test4InstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    s32 result;

    if (gInstantTeleportersEnabled == 0) {
        result = -1;
    } else {
        result = GetStaticSpawn(target, currentPos, stage, LEN_TELEPORT_TRIGGERS, TELEPORT_TRIGGERS,
                                TELEPORT_SPAWNS, 0);
    }
    return result;
}

s32 GetTeleportTimeBonus(void) {
    return (gLinkSrcStage == 0) ? 10 : 0;
}

s32 Test4StaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage) {
    if (stage == 0)
        return GetStaticSpawn(target, currentPos, 0, LEN_STAIRCASE_TRIGGERS, STAIRCASE_TRIGGERS,
                              STAIRCASE_SPAWNS, 0);
    return -1;
}

/* CheckStaircaseHeading's pair of heading tables, indexed as
   TUNNEL_ENTER_HEADINGS / TUNNEL_EXIT_HEADINGS are. */
extern u8 *STAIRCASE_ENTER_HEADINGS[];
extern u8 *STAIRCASE_EXIT_HEADINGS[];

s32 DreamSys__CheckStaircaseHeading(s32 *outExit, s32 *outEnter, void *rotation) {
    u8 heading;
    s32 idx;
    s32 result;

    heading = STAIRCASE_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex];
    if (IsHeadingAligned((DirectionCheckArg *)rotation, heading)) {
        if (outEnter != NULL)
            *outEnter = (s32)&CARDINAL_ROTATIONS[heading];

        if (outExit != NULL) {
            idx = STAIRCASE_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex];
            *outExit = (s32)&CARDINAL_ROTATIONS[idx];
        }
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

s32 GetLastSpawnExtra(void) {
    return STAIRCASE_SPAWNS[gLinkDstStage][gLinkSpawnIndex].extra;
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

        gLinkSrcStage = stage;
        gLinkTriggerIndex = i;
        triggerStage = trig->stage;
        gLinkDstStage = triggerStage;
        spawnIndex = (u8)trig->spawnpointIndex;
        entry = &spawns[triggerStage][spawnIndex];
        gLinkSpawnIndex = spawnIndex;
        *(PlayerSpawnGridPos *)target = *(PlayerSpawnGridPos *)entry;
        target->position = sSpawnPosAdjust[entry->adjustment];
        if (flag != 0)
            (*gpNavChallengesComplete)[entry->extra] = 1;
        return gLinkDstStage;
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
            return &SPECIAL_DAY_MOOD;
        }
    }
    return NULL;
}
