/* DreamSys -- the object that IS a dream in progress: it owns the dream
 * clock, the player-ish body that walks around the stage, the mood record
 * that decides the next day's dream, and the "link" (teleport) machinery
 * that ends one stage and starts another. See include/DreamSys.h for the
 * class as a whole; several of its methods live in sibling units
 * (class_3bb8c_p/t/r/o) because the class spans more than one segment.
 *
 * Four groups of functions live here.
 *
 * 1. Construction and reset. New_DreamSys/DreamSys__DreamSys,
 *    DreamSys__ResetSessionState (the constructor's last step: no tick
 *    callbacks, sound cue set freed, staircase-walk state cleared, base
 *    orientation applied), DreamSys__ResetLinkState, DreamSys__SpawnAtLink.
 *
 * 2. The per-tick chain. DreamSys__TimerTick advances dreamTimer and, at
 *    dreamTimeLimit, either loads the next flashback or ends the dream;
 *    below the limit it runs DreamSys__UpdateTickState and
 *    DreamSys__RunTickCallbacks, which call the two callback slots
 *    DreamSys__SelectCallback80/98 install. Slot 80's mode 1 is
 *    DreamSys__StepLook (the two spring-with-decay "look" accumulators:
 *    DreamSys__StepLookOffset moves the height curve, DreamSys__StepLookYaw
 *    turns the object +-45 degrees a tick up to +-181 and springs back).
 *    Slot 98's mode 1 is DreamSys__TickMove, the movement state machine
 *    (Free/Forced/Held by moveOverride) that runs a four-tick step cycle in
 *    DreamSys__AdvanceMoveCycle -- a sound voice at the end of each cycle
 *    (DreamSys__StartVoice / DreamSys__StopVoice, through a VabStreamObj in
 *    DreamSys::soundObj), a +-50 view bob, and finally
 *    DreamSys__ApplyMoveCommand, which tries the three link tests before
 *    letting the move happen. Slot 98's mode 2 is DreamSys__TickDrift.
 *
 * 3. Day, mood and flashback bookkeeping. DreamSys__StartDay/EndDay,
 *    DreamSys__UpdateDreamChart and the MoodGraphContributor helpers,
 *    DreamSys__AddFlashback/FlashbackSaving, DreamSys__CalcUnlockScore, and
 *    DreamSys__GetSaveBlock, which hands out the 0x700 bytes of the object
 *    that DreamSys__InitNewGame initializes and that start with SAVE_MAGIC.
 *
 * 4. Linking. DreamSys__WallLink/DynamicLink, the "Try...Link" family and
 *    the free "Test4..."/GetStaticSpawn static-link testers underneath them;
 *    ExecuteLink writes the link type into DreamSys::pendingLinkType.
 *
 * Naming pass: round 66 (Opus). Everything above is measured, not guessed --
 * the evidence for each name is in that function's match report under
 * `## Naming`. Four functions are deliberately still `func_`-shaped
 * (DreamSys__func_5938c/59590/59598/5ba20 and the free func_8005BFC4): each
 * touches exactly one field or global that nothing in any carved unit ever
 * reads back, so there is nothing to name them after. */
#include "common.h"
#include "DreamSys.h"

/* Forward declarations for two of this unit's OWN functions, both called
   around line 450 but not defined until ~200 lines later, in ROM order.
   Without these, C89 implicitly declares them as `int ()` at the call site
   and cpp emits "implicit declaration of function". The implicit type
   happens to agree with the real one here, so nothing miscompiled -- but an
   implicit declaration also disables argument checking, which is precisely
   what caught Entity__IsNearTarget's over-narrow `s8` parameters in include/Entity.h
   this round. A declaration is not a definition, so this does NOT affect the
   strict ROM-address ordering of the definitions below. */
s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
/* DreamSys__StepLookYaw (round 2026-09-08) calls this unit's own DreamSys__FlipMoveCommand,
   defined immediately after it in ROM order -- same forward-declaration
   need as the two above. */
void DreamSys__FlipMoveCommand(DreamSys *this);

DreamSys *New_DreamSys(void *arg0, s32 arg1, s32 arg2)
{
	DreamSys *this;

	this = func_80017B34(sizeof(DreamSys));
	if (this != NULL) {
		Get_vtable_DreamSys()->Constructor(this, arg0, arg1, arg2);
		return this;
	}
	return NULL;
}

DreamSys *DreamSys__DreamSys(DreamSys *this, void *arg1, s32 arg2, s32 arg3)
{
	void *val;

	DreamSys__GetBaseMethods()->ctor(this);
	this->vt = Get_vtable_DreamSys();
	this->soundObj = arg2;
	this->heightCurve = (DreamSysUnk5C *)arg3;
	this->unk_0x64 = 0;
	this->unk_0x60 = arg1;
	val = ((DreamSysCtorArgObj *)arg1)->methods->slot0x80(arg1, 0);
	this->vt->slot10(this, val);
	this->vt->GetSetDreamTimeLimit(this, -1);
	this->movementBlocked = 1;
	this->moveOverride = 0;
	this->newGamePending = 1;
	this->vt->InitNewGame(this);
	return this->vt->DreamSys__ResetSessionState(this);
}

void DreamSys__ResetSessionState(DreamSys *this)
{
	this->vt->func_8001D344(this, 0);
	this->vt->func_8001CEB4(this, 1, &ROTATION_YAW_180);
	this->callback_0x80 = NULL;
	this->callback_0x98 = NULL;
	*(s32 *)this->soundCueSet = 0;
	this->staircaseActive = 0;
	this->staircaseMoveGate = 0;
	this->staircaseTickFn = 0;
	this->unk_0x78 = 0;
	this->unk_0x924 = 0;
}

void DreamSys__SpawnAtLink(DreamSys *this, DreamSysSpawnArgObj *arg1)
{
	s32 local[4];

	arg1->methods->slot0xE4(arg1, local, this, &this->linkCoordinates);
	DreamSys__GetBaseMethods()->slot4C(this, arg1, local);
	this->vt->slot10(this, arg1);
	if (this->pendingLinkType == 0xE) {
		FlashbackEntry *entry = &this->storedFlasbacks[this->currentFlashbackIndex];
		this->vt->func_8001CEB4(this, 1, &entry->rotation);
		this->vt->GetSetDreamTimeLimit(this, entry->timeLimit + 4);
		this->currentFlashbackIndex++;
	}
	if (this->moveOverride != 0 && this->exitRotation != 0) {
		this->vt->func_8001CEB4(this, 1, (void *)this->exitRotation);
	}
}

void DreamSys__UnlinkLinkMgr(DreamSys *this)
{
	this->linkMgr->methods->slot0xF0(this->linkMgr);
	this->vt->BaseObjO__UnlinkCompanion(this, this->linkMgr);
	DreamSys__GetBaseMethods()->slot0x50(this);
}

void DreamSys__NotifyLinkAttempt(DreamSys *this, s32 arg1)
{
	s32 v;

	DreamSys__GetBaseMethods()->slot0x88(this, arg1);
	if (arg1 == -2)
		goto handle_neg2;
	if (arg1 != -1)
		return;

	v = this->linkTarget->unk_0x36 & 0x7F;
	this->voiceSelect = v;
	if (v >= 0x18)
		this->voiceSelect = 0;

	if (this->pendingLinkType == 15 && this->voiceSelect == 0)
		this->voiceSelect = 2;

	if (this->currentStage != 9)
		return;
	goto shared_tail;

handle_neg2:
	if (this->linkMgr->methods->slot0x11C(this->linkMgr, (u8 *)this->unk_0x14 + 0x18)->unk_0x4->unk_0x2C != 2)
		goto neg2_mismatch;

shared_tail:
	this->vt->DreamSys__TryStageTimerLink(this, this->linkMgr->methods->slot0x10C(this->linkMgr, 0, 0));
	return;

neg2_mismatch:
	this->vt->DreamSys__RestoreLinkSnapshot(this);
}

void DreamSys__ApplyLinkCommand(DreamSys *this, s32 arg1, s32 mode)
{
	if (this->moveOverride != 0)
		return;
	if (this->movementBlocked != 0)
		return;
	if (this->staircaseActive != 0)
		return;

	switch (mode - 2) {
	case 0:
		this->moveCommand = 1;
		break;
	case 1:
		this->moveCommand = 2;
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
		if (this->moveCommand == 1)
			this->vt->DreamSys__ChangeMoveMode(this, 4);
		break;
	case 6:
		this->lookOffsetCommand = 2;
		break;
	case 11:
		this->lookYawCommand = 2;
		break;
	case 12:
		this->moveCommand = 4;
		break;
	case 13:
		this->lookYawCommand = 1;
		break;
	case 14:
		this->moveCommand = 3;
		break;
	case 23:
		this->linkCommandFlag = 1;
		break;
	case 32:
		this->vt->DreamSys__RestorePreviousMoveMode(this);
		break;
	case 47:
		break;
	}
}

void DreamSys__TimerTick(DreamSys *this, s32 arg1, s32 arg2)
{
	s32 old;

	if (arg2 != 2)
		return;

	old = this->dreamTimer;
	this->dreamTimer = old + 1;
	if ((u32)old < (u32)this->dreamTimeLimit)
		goto tick_only;

	if (this->isFlashbackSession) {
		if (this->pendingLinkType != 0 || this->vt->LoadNextFlashback(this, 0)) {
			__asm__("");
			this->dreamTimer = 0;
			return;
		}
	} else {
		this->vt->FlashbackSaving(this, 0, 0x10);
	}
	this->vt->slot30(this, 0xA);
	this->dreamTimer = 0;
	return;

tick_only:
	this->vt->DreamSys__UpdateTickState(this);
	this->vt->DreamSys__RunTickCallbacks(this);
}

void DreamSys__DispatchChunkChange(DreamSys *this, void *arg1, s32 arg2)
{
	DreamSys__GetBaseMethods()->slot0x9C(this, arg1, arg2);
	if ((*(s32 *)(*(void **)arg1) & 0xFFF) == 0x114) {
		this->vt->ProcessChunkChange(this, arg1, arg2);
	}
}

void DreamSys__DispatchInstanceEffect(DreamSys *this, void *arg1, s32 arg2)
{
	DreamSys__GetBaseMethods()->slot0xDC(this, arg1, arg2);
	if ((*(s32 *)(*(void **)arg1) & 0xFFFFF) == 0x1F234) {
		this->vt->InstanceEffectsOnJournal(this, arg1, arg2);
	}
}

void DreamSys__WallLink(DreamSys *this, void* unk_class_86aa0, int arg2)
{
	DreamSys__GetBaseMethods()->slot0xE0(this, unk_class_86aa0, arg2);
	if (arg2 != 4)
		return;
	if (this->pendingLinkType != 0)
		return;
	this->linkCoordinates = *this->linkMgr->methods->slot0xD4(this->linkMgr, unk_class_86aa0);
	if (!this->vt->StaticWallLink(this, &this->linkCoordinates) && this->tickBoundary != 0) {
		this->vt->DynamicLink(this);
	}
	this->vt->DreamSys__RestoreLinkSnapshot(this);
	this->vt->DreamSys__NoOpSlotE8Default(this);
}

void DreamSys__NoOpSlotE8Default(void) {
}

s32 DreamSys__GetSetFlashbackSession(DreamSys *this, DreamColors *out, s32 value)
{
	s32 old;

	old = this->isFlashbackSession;
	if (value < 0) {
		*out = CalcDreamColor(&this->moodPreviousDays[this->currentDay]);
	} else {
		this->isFlashbackSession = value;
	}
	return old;
}

void DreamSys__SetMoveOverride(DreamSys *this, s32 value)
{
	this->moveOverride = value;
	if (value != 0) {
		this->vt->DreamSys__GetSetMoveMode(this, 1);
		if (this->enterRotation != 0)
			this->vt->func_8001CEB4(this, 1, (void *)this->enterRotation);
	}
}

void DreamSys__ResetLinkState(DreamSys *this, s32 arg1, s32 arg2)
{
	struct {
		s8 unknown_values_0x0[8];
		s16 field_0x8;
		s16 field_0xA;
	} local;

	this->vt->LogChunkMood(this, &this->linkCoordinates);
	this->vt->DreamSys__SelectCallback80(this, 1);
	this->vt->DreamSys__SelectCallback98(this, 1);
	this->vt->DreamSys__GetSetMoveMode(this, arg1);

	this->voiceIndex = -1;
	this->moveCycleTick = 0;
	this->voiceSelect = 0;
	this->moveCommand = 0;
	this->turnCommand = 0;
	this->lookOffsetCommand = 0;
	this->lookYawCommand = 0;
	this->lookOffset = 0;
	this->lookYaw = 0;
	this->vt->DreamSys__SetGateFlags(this, 0, 1, 1, 1);

	this->vt->DreamSys__SetTickPeriod(this, arg2);

	this->nextCinematic.entry = -1;
	this->movementBlocked = 0;
	this->pendingLinkType = 0;
	this->linkCommandFlag = 0;
	this->staircaseActive = 0;
	this->staircaseMoveGate = 0;
	this->staircaseTickFn = 0;
	this->unk_0x78 = 0;
	Class6B5CC__GetRotationDegrees(this, &local);

	local.field_0x8 = 0;
	local.field_0xA = 1;
	this->vt->func_8001CEB4(this, 1, &local);
}

void DreamSys__BlockMovement(DreamSys *this)
{
	this->movementBlocked = 1;
}
s32 DreamSys__GetLinkCommandFlag(DreamSys *this)
{
	return this->linkCommandFlag;
}
s32 DreamSys__GetSetDreamTimeLimit(DreamSys *this, s32 value)
{
	s32 result;

	if (value >= 0)
		value = value * 15;
	result = this->dreamTimeLimit;
	this->dreamTimeLimit = value;
	if (result >= 0)
		result = (u32)result / 15;
	return result;
}
s32 DreamSys__GetDreamTimerScaled(DreamSys *this)
{
	return (u32)this->dreamTimer / 15;
}
void DreamSys__SetSoundObj(DreamSys *this, s32 value)
{
	this->soundObj = value;
}
void DreamSys__SetHeightCurve(DreamSys *this, void *value)
{
	this->heightCurve = value;
}
void DreamSys__func_5938c(DreamSys *this, s32 value)
{
	this->unk_0x64 = value;
}
void DreamSys__UpdateTickState(DreamSys *this)
{
	if (this->movementBlocked == 0) {
		this->linkCommandFlag = 0;
		this->tickBoundary = ((u32)this->dreamTimer % (u32)this->tickPeriod) == 0;
	}
}
void DreamSys__RunTickCallbacks(DreamSys *this)
{
	if (this->callback_0x80 != NULL)
		this->callback_0x80(this);
	if (this->callback_0x98 != NULL)
		this->callback_0x98(this);
}
/* Local prototypes, own local view (Class6B5CC__LocalOffsetToWorldPos is a different unit's
 * already-matched function taking an unrelated class as arg0; InterpolateKeyframeValue
 * is this unit's own next-in-queue function, forward-declared per
 * CLAUDE.md's convention for calling into a not-yet-preceding definition).
 * arg4 on Class6B5CC__LocalOffsetToWorldPos is unused by its own body but IS set (to 0) by this
 * call site's own disassembly, so it is declared here to reproduce that. */
extern void Class6B5CC__LocalOffsetToWorldPos(void *self, s32 *dst, s32 *src, s32 arg4); /* arity-ok: definition is 3-parameter, but arg4 is byte-load-bearing HERE -- retail emits `move a3,zero` at 0x80059460 */
extern s32 InterpolateKeyframeValue(DreamSysInterpPoint *a, DreamSysInterpPoint *b, s32 at);
extern s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b);

/* `dist` was called `day` until round 66, which was a transcription of the
   caller-less m2c signature and is wrong: it is written into the z word of
   the global scratch vector that Class6B5CC__LocalOffsetToWorldPos converts
   from a LOCAL OFFSET to a world position, and it is also the abscissa
   InterpolateKeyframeValue evaluates heightCurve's two keyframes at -- whose
   own `position` fields are what it is compared against. It is a distance
   along the local axis, not a day index. */
s32 DreamSys__ProjectPointAtDistance(DreamSys *this, s32 *out, s32 dist, s32 *reference, s32 tolerance)
{
	s32 local[3];
	s32 ret;
	s32 *vec;
	s32 *p;

	p = &gProjectOffsetZ;
	*p = dist;
	Class6B5CC__LocalOffsetToWorldPos(this, local, p - 2, 0);

	ret = InterpolateKeyframeValue((void *)((u8 *)this->heightCurve + 0x14),
	                     (void *)((u8 *)this->heightCurve + 0x20), dist);

	vec = this->unk_0xC != 0 ? (s32 *)((u8 *)this->unk_0x14 + 0x38) : 0;
	local[1] = ret + vec[1];

	if (out != NULL) {
		*(DreamSysVec3 *)out = *(DreamSysVec3 *)local;
	}

	if (reference != NULL)
		return IsVec3WithinRange(local, tolerance, reference);
	return 0;
}

s32 InterpolateKeyframeValue(DreamSysInterpPoint *a, DreamSysInterpPoint *b, s32 arg2)
{
	s32 scaledArg2;
	s32 dt;
	s32 dv;

	scaledArg2 = arg2;
	scaledArg2 = scaledArg2 / 0x400;
	dt = (b->position - a->position) / 0x400;
	if (dt == 0)
		dt = 1;
	dv = b->value - a->value;
	return (dv * scaledArg2) / dt + a->value;
}

void DreamSys__func_59590(DreamSys *this)
{
	this->unk_0x7C = 0;
}
void DreamSys__func_59598(DreamSys *this)
{
	this->unk_0x78 = 0;
}
s32 DreamSys__NoOpSlot12C(DreamSys *this)
{
	return 0;
}
void DreamSys__ClearTickCallbacks(DreamSys *this, bool arg1)
{
	this->vt->DreamSys__SelectCallback98(this, 0);
	if (arg1)
		this->vt->DreamSys__SelectCallback80(this, 0);
}
void DreamSys__SetTickCallbacks(DreamSys *this, s32 arg1, s32 arg2)
{
	this->vt->DreamSys__SelectCallback98(this, arg1);
	this->vt->DreamSys__SelectCallback80(this, arg2);
}

void DreamSys__SelectCallback80(DreamSys *this, s32 arg1)
{
	struct vtable_DreamSys *vt = this->vt;

	this->callback80Mode = arg1;
	switch (arg1) {
	case 0:
		this->callback_0x80 = NULL;
		break;
	case 1:
		this->callback_0x80 = vt->DreamSys__StepLook;
		break;
	case 2:
		this->callback_0x80 = vt->DreamSys__NoOpSlot14C;
		break;
	case 3:
		this->callback_0x80 = vt->DreamSys__NoOpSlot150;
		break;
	}
}

extern void InitSoundCueSet(s32 arg0, void *arg1, s32 arg2, DreamSys *arg3, void *arg4);

void DreamSys__SelectCallback98(DreamSys *this, s32 arg1)
{
	struct vtable_DreamSys *vt = this->vt;

	if (this->callback98Mode == 2)
		vt->DreamSys__StopDrift(this, 0);
	this->callback98Mode = arg1;
	switch (arg1) {
	case 0:
		this->callback_0x98 = NULL;
		break;
	case 1:
		this->callback_0x98 = (void (*)(DreamSys *))vt->DreamSys__TickMove;
		break;
	case 2:
		this->callback_0x98 = vt->DreamSys__TickDrift;
		this->driftActive = 1;
		this->cueServiceActive = 1;
		InitSoundCueSet(this->soundObj, this->soundCueSet, 1, this, this->vt->DreamSys__SoundCueCallback);
		break;
	}
}

void DreamSys__StepLook(DreamSys *this)
{
	this->vt->DreamSys__StepLookOffset(this);
	this->vt->DreamSys__StepLookYaw(this);
}

void DreamSys__StepLookOffset(DreamSys *this)
{
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
		this->heightCurve->endValue += delta;
		this->lookOffset = sum;
		this->lookOffsetCommand = 0;
		return;
	}
	if (this->lookOffset != 0) {
		delta = -0x258;
		if (this->lookOffset < 0)
			delta = 0x258;
		this->heightCurve->endValue += delta;
		this->lookOffset += delta;
	}
}

#if 0
/* best-reached body, round 32 (2026-09-12, runner alpha2): the delta/step
   register-coalescing fix that closed DreamSys__StepLookOffset (reusing the SAME `delta`
   local across both mutually exclusive branches instead of a separate `step`)
   applies here too and closes ONE of the two previously-missing words: 76/77
   words, 1 word SHORT (was 75/77, 2 words short). See
   docs/match-reports/DreamSys__StepLookYaw.md for the residue that's left -- it is a
   different, deeper mechanism (opportunistic delay-slot placement of the
   `this` register setup across multiple converging paths into the shared
   `DreamSys__FlipMoveCommand(this)` tail call), not reachable by the rename lever nor by
   a scheduling barrier at the merge point (tried, no effect). Re-verified
   fresh round 37 (2026-09-12, runner charlie); a permuter search was run
   this round and found no improvement -- see the report's round 37
   addendum. Re-verified fresh again round 39 (2026-09-14, runner echo); one
   new reshape tried (duplicating the tail call at just the decay branch's
   own exit, rather than at all three converging paths as round 32 already
   tried and rejected) -- this DOES fix the a0-vs-s0 register mismatch at the
   store immediately before the call, but GCC does not cross-jump-merge the
   duplicated call site back down, so the function grows to 78 words (1 word
   LONG) instead of 76 (1 word SHORT): trades one residue for a different,
   equally-real one rather than closing it. Reverted immediately. */
void DreamSys__StepLookYaw(DreamSys *this)
{
	s32 idx;
	s32 delta;
	s32 threshold;
	s32 sum;

	this->moveCommandLatch = (this->moveCommand == 1);
	idx = this->lookYawCommand;
	if (idx != 0) {
		delta = LOOK_YAW_STEPS[idx];
		threshold = LOOK_YAW_LIMITS[idx];
		sum = delta + this->lookYaw;
		if (sum >= 0) {
			if (sum < threshold)
				goto apply;
			this->lookYawCommand = 0;
			goto call_tail;
		}
		if ((~sum + 1) >= threshold) {
			this->lookYawCommand = 0;
			goto call_tail;
		}
	apply:
		TURN_ROTATION_YAW[0].numerator = delta;
		this->vt->func_8001CEB4(this, 0, &TURN_ROTATION_YAW[-1]);
		this->lookYaw = sum;
		this->lookYawCommand = 0;
	} else if (this->lookYaw != 0) {
		delta = -0x2D;
		if (this->lookYaw < 0)
			delta = 0x2D;
		TURN_ROTATION_YAW[0].numerator = delta;
		this->vt->func_8001CEB4(this, 0, &TURN_ROTATION_YAW[-1]);
		this->lookYaw += delta;
	} else {
		return;
	}
call_tail:
	DreamSys__FlipMoveCommand(this);
}
#endif
INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__StepLookYaw);

void DreamSys__FlipMoveCommand(DreamSys *this)
{
	this->moveCommandLatch = 0;
	if (this->moveCommand != 0) {
		if (this->moveCommand & 1)
			this->moveCommand = this->moveCommand + 1;
		else
			this->moveCommand = this->moveCommand - 1;
	}
}

void DreamSys__NoOpSlot14C(void) {
}

void DreamSys__NoOpSlot150(void) {
}

s32 DreamSys__TickMove(DreamSys *this)
{
	if (this->moveOverride == 0) {
		this->vt->DreamSys__ApplyPendingTurn(this);
		return this->vt->DreamSys__TickMoveFree(this);
	} else if (this->moveOverride != 2) {
		return this->vt->DreamSys__TickMoveForced(this);
	} else {
		return this->vt->DreamSys__TickMoveHeld(this);
	}
}

s32 DreamSys__TickMoveFree(DreamSys *this)
{
	if (this->movementBlocked != 0)
		return this->movementBlocked;
	return this->vt->DreamSys__ApplyMoveCommand(this, this->vt->DreamSys__AdvanceMoveCycle(this, 1));
}

s32 DreamSys__TickMoveForced(DreamSys *this)
{
	this->moveCommand = 1;
	if (this->movementBlocked != 0)
		return this->vt->DreamSys__AdvanceMoveCycle(this, 0);
	return this->vt->DreamSys__ApplyMoveCommand(this, this->vt->DreamSys__AdvanceMoveCycle(this, 1));
}

s32 DreamSys__TickMoveHeld(DreamSys *this)
{
	return this->moveCommand = 1;
}

s32 DreamSys__AdvanceMoveCycle(DreamSys *this, s32 arg1)
{
	s32 doCallback = 0;
	s32 ret = 0;
	s32 count;
	DreamSysUnk5C *p;
	s32 delta;

	if (this->moveCommand != 0) {
		ret = this->moveCommand;
		count = this->moveCycleTick + 1;
		this->moveCycleTick = count;
		if (count < 4) {
			doCallback = (this->moveMode == 4) && ((count & 1) == 0);
		} else {
			this->moveCommand = 0;
			doCallback = 1;
		}

		if (doCallback)
			this->vt->DreamSys__StartVoice(this);

		p = this->heightCurve;
		if (p != NULL && this->screenShakeOn != 0 && arg1 != 0) {
			delta = -50;
			if (this->moveCycleTick >= 3)
				delta = 50;
			p->startValue += delta;
			p->endValue += delta;
		}

		if (this->moveCommand == 0)
			this->moveCycleTick = 0;
	}

	if (!doCallback)
		this->vt->DreamSys__StopVoice(this);
	return ret;
}

/* `headingArg` and `scratch` are not superfluous: they were found by a
   permuter search (round 37, 2026-09-12, runner charlie) after 20+ hand
   attempts across three rounds failed to reproduce retail's whole-function
   this/obj/vt/heading register allocation. Both are ordinary, valid C89 --
   `headingArg` is a second copy of `heading` used at its two call sites,
   giving the two logical uses disjoint live ranges so GCC 2.6.3's
   allocator lands them in the SAME register retail does; `scratch` plays
   the same role for the raw `VOICE_BY_SELECT[idx]` read and, independently, for
   the literal `0x90` argument at the very end. Removing either variable
   (rebuilding the "obvious" simpler form) reproduces a real, measured
   regression -- see docs/match-reports/DreamSys__StartVoice.md. */
void DreamSys__StartVoice(DreamSys *this)
{
	DreamSysUnk58 *obj;
	s32 idx;
	DreamSysUnk58Vtable *vt;
	s32 heading;
	s32 headingArg;
	s32 scratch;

	obj = (DreamSysUnk58 *)this->soundObj;
	vt = obj->vt;
	idx = this->voiceSelect;
	if (idx == 0) {
		return;
	}

	scratch = VOICE_BY_SELECT[idx];
	heading = scratch << 4;
	headingArg = heading;
	vt->slot0x9C(obj, VOICE_PITCH_BY_SELECT[idx]);
	this->voiceIndex = vt->slot0x80(obj, headingArg, 0x6E, 0x6E);
	if (this->voiceSelect != 0x16) {
		this->voiceIndex = -1;
	}

	if (this->voiceSelect == 0xB) {
		vt->slot0x9C(obj, 1);
		vt->slot0x80(obj, headingArg, 0x6E, 0x6E);
		vt->slot0x9C(obj, 2);
		scratch = 0x90;
		vt->slot0x80(obj, scratch, 0x6E, 0x6E);
	}
}

void DreamSys__StopVoice(DreamSys *this)
{
	DreamSysUnk58 *obj;

	if (this->voiceIndex >= 0) {
		obj = (DreamSysUnk58 *)this->soundObj;
		obj->vt->slot0x84(obj, this->voiceIndex);
		this->voiceIndex = -1;
	}
}

s32 DreamSys__ApplyMoveCommand(DreamSys *this, s32 arg1)
{
	s32 delta;
	PlayerSpawnPoint *pos;

	if (arg1 != 0) {
		delta = MOVE_COMMAND_SIGNS[arg1] * MOVE_MODE_SPEEDS[this->moveMode];
		this->vt->DreamSys__NoOpSlot12C(this);
		pos = this->linkMgr->methods->slot0x10C(this->linkMgr, 0, 0);
		if (!this->vt->DreamSys__TryStaircaseLink(this, pos)
		 && !this->vt->DreamSys__TryInstantTeleportLink(this, pos)
		 && !this->vt->DreamSys__TryTunnelLink(this, pos)) {
			this->vt->DreamSys__SaveLinkSnapshot(this);
			MOVE_COMMAND_DISPATCH[arg1](this, delta, (void *)(this->staircaseMoveGate < 1));
			if (this->currentStage == 0
			 && this->unk_0x14->unk_0x1C < -0x7D0
			 && this->unk_0x14->unk_0x18 >= -0x1F3) {
				this->vt->LinkWall(this, this, 4);
			}
		}
		this->unk_0x14->unk_0x0 = 0;
	}
}

void DreamSys__ApplyPendingTurn(DreamSys *this)
{
	s32 idx;

	idx = this->turnCommand;
	if (idx != 0) {
		this->vt->func_8001CEB4(this, 0, &TURN_ROTATIONS[idx]);
		this->turnCommand = 0;
	}
}

void DreamSys__TickDrift(DreamSys *this)
{
	if (this->driftActive != 0) {
		this->vt->BaseObjO__AddVec14(this, &DRIFT_STEP);
		this->heightCurve->endValue -= 0x258;
	}
	if (this->cueServiceActive != 0)
		func_8002CD08(this->soundObj, this->soundCueSet);
}

void DreamSys__StopDrift(DreamSys *this, s32 arg1)
{
	this->driftActive = 0;
	this->cueServiceActive = arg1;
	if (arg1 != 0)
		FlushSoundCueSet(this->soundObj, this->soundCueSet);
}

s32 DreamSys__GetSetMoveMode(DreamSys *this, s32 value)
{
	s32 old;

	old = this->moveMode;
	if (value >= 0) {
		this->moveMode = value;
		this->previousMoveMode = value;
	}
	return old;
}

void DreamSys__ChangeMoveMode(DreamSys *this, s32 value)
{
	s32 old;

	old = this->moveMode;
	if (old != value) {
		this->previousMoveMode = old;
		this->moveMode = value;
	}
}

void DreamSys__RestorePreviousMoveMode(DreamSys *this)
{
	this->moveMode = this->previousMoveMode;
}

void DreamSys__SetGateFlags(DreamSys *this, s32 a, s32 b, s32 c, s32 d)
{
	if (a >= 0)
		this->tickBoundary = a;
	if (b >= 0)
		this->unk_0x128 = b;
	if (c >= 0)
		this->unk_0x12C = c;
	if (d >= 0)
		this->unk_0x130 = d;
}

void DreamSys__SetTickPeriod(DreamSys *this, s32 value)
{
	this->tickPeriod = value;
}

void DreamSys__SoundCueCallback(void *arg0, SoundCueCallbackArg *arg1)
{
	s32 isDivisible;

	if (arg1->mode == 1) {
		isDivisible = (arg1->value % 20) == 0;
		if (isDivisible) {
			arg1->field_0x1C = 9;
			arg1->field_0x20 = -1;
		} else {
			arg1->field_0x30 = 9;
			arg1->field_0x34 = -1;
		}
	}
}

/* Also declared in code_2cc8c_f.c with the same signature (that unit's own
   local view of the same libc-style function). */
extern void *memset(unsigned char *dst, unsigned char c, int n);

/* A constant read out of .sdata and copied whole into
   this->saveMagic -- naming convention matches (the field's own
   name already flags it as sdata-sourced). Not dereferenced by this
   function or any other in this unit's queue. */
extern s32 SAVE_MAGIC;

void DreamSys__InitNewGame(DreamSys *this)
{
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
	memset((unsigned char *)&this->unknown_values_0x684, 0, 0x1F4);
}

void DreamSys__GetSetScreenShake(DreamSys *this, bool *value)
{
	bool old;

	old = this->screenShakeOn;
	this->screenShakeOn = *value;
	*value = old;
}

s32 DreamSys__GetCurrentDayAndYear(DreamSys *this, s32 *arg1)
{
	if (arg1 != NULL)
		*arg1 = this->currentYear;
	return this->currentDay + 1;
}

s32 DreamSys__AdvanceDay(DreamSys *this)
{
	this->currentDay++;
	if (this->currentDay >= 0x16D) {
		this->currentDay = 0;
		this->currentYear++;
	}
	return this->currentDay;
}

void DreamSys__ClearNewGameFlag(DreamSys *this)
{
	this->newGamePending = 0;
}

s32 DreamSys__GetNewGameFlag(DreamSys *this)
{
	return this->newGamePending;
}

s32 *DreamSys__GetSaveBlock(DreamSys *this, s32 *arg1)
{
	if (arg1 != NULL)
		*arg1 = 0x700;
	return &this->saveMagic;
}

s32 DreamSys__StartDay(DreamSys *this)
{
	s32 oldDay;
	MoodGraphPoint *special;

	oldDay = this->currentDay;
	this->currentFlashbackIndex = 0;
	this->dreamTimer = 0;
	this->storedDay = oldDay;
	if (this->isFlashbackSession) {
		this->vt->LoadNextFlashback(this, 1);
	} else {
		special = IsDaySpecial(&this->nextCinematic, this->currentDay + 1);
		this->vt->InitMoodContibutors(this, special);
		if (special != NULL) {
			return -1;
		}
		this->vt->InitSpawnLoc(this);
	}
	return this->currentStage;
}

s32 DreamSys__EndDay(DreamSys *this, s32 arg1)
{
	this->currentDay = this->storedDay;
	if (!this->isFlashbackSession && arg1 == 0) {
		this->vt->CalcUnlockScore(this);
		this->vt->UpdateDreamChart(this, &this->moodPreviousDays[this->currentDay]);
		this->vt->AdvanceDay(this);
	} else if (arg1 == 2) {
		this->vt->InitNewGame(this);
		this->newGamePending = 1;
	}
	return this->isFlashbackSession;
}

CinematicCall DreamSys__GetCinematic(DreamSys *this)
{
	return this->nextCinematic;
}

void DreamSys__InitSpawnLoc(DreamSys *this)
{
	MoodGraphPoint mood;
	s32 timeLimit;

	this->vt->GetPreviousDayMood(this, &mood, 1);
	this->currentStage = GenerateInitialSpawn(&this->linkCoordinates, &timeLimit, &mood, this->currentDay);
	timeLimit = this->vt->GetSetDreamTimeLimit(this, timeLimit);
	this->pendingLinkType = 0xB;
}

void DreamSys__DynamicLink(DreamSys *this)
{
	s32 stage;

	if (this->pendingLinkType == 0) {
		stage = GetRandomSpawnFromStage(&this->linkCoordinates, this->currentStage, this->dreamTimer);
		ExecuteLink(this, stage, 0xC, 1);
	}
}

bool DreamSys__StaticWallLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;

	if (this->pendingLinkType != 0)
		return false;
	result = TestForStaticLink(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	ExecuteLink(this, result, 0xD, 1);
	return true;
}

bool DreamSys__LoadNextFlashback(DreamSys *this, bool unknown)
{
	s32 idx;
	FlashbackEntry *entry;

	idx = this->currentFlashbackIndex;
	if (idx >= this->amountFlashbacksAvailable) {
		goto fail;
	}
	this->pendingLinkType = 0xE;
	entry = &this->storedFlasbacks[idx];
	if (!unknown) {
		this->vt->slot30(this, 0xE);
	}
	this->currentDay = entry->day;
	this->currentStage = entry->stageID;
	this->linkCoordinates = entry->position;
	return true;
fail:
	return false;
}

bool DreamSys__TryTunnelLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 local[4];

	if (this->pendingLinkType != 0)
		return false;
	result = Test4TunnelLinks(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	Class6B5CC__GetRotationDegrees(this, local);
	if (!DreamSys__CheckTunnelHeading(&this->exitRotation, &this->enterRotation, local))
		return false;
	if (this->moveCommandLatch == 0)
		return false;
	ExecuteLink(this, result, 0xF, 0);
	return true;
}

bool DreamSys__TryStageTimerLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;

	if (this->pendingLinkType != 0)
		return false;
	result = Test4StageTransition(&this->linkCoordinates, this->currentStage, currentPos, this->dreamTimer);
	if (result < 0)
		return false;
	this->stageLinkAngle = GetStageLinkAngle();
	this->enterRotation = 0;
	this->exitRotation = 0;
	ExecuteLink(this, result, 0x10, 0);
	return true;
}

#if 0
/* Best-reached body, 58/63 words, exact length (zero address drift) -- see
   docs/match-reports/DreamSys__TryInstantTeleportLink.md for the residue analysis (delay-slot
   fillers around the constant "return true" materialization;
   PERMUTER-EXHAUSTED, ~49300 iterations). Restored to INCLUDE_ASM below per
   project rule (no score short of byte-exact stays in src/). Re-verified
   fresh round 39 (2026-09-14, runner echo); two new reshapes tried, neither
   moved it -- see the round 39 note in the report. */
bool DreamSys__TryInstantTeleportLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 saved;
	s32 local[4];

	result = Test4InstantTeleporters(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	saved = func_8005BFC4();
	if (!ExecuteLink(this, result, 0x11, 0))
		return true;
	this->pendingLinkType = 0;
	this->linkMgr->methods->slot0xE8(this->linkMgr, local, &this->linkCoordinates);
	this->vt->BaseObjO__SetVec14(this, local);
	if (saved == 0)
		return true;
	if (this->isFlashbackSession)
		return true;
	this->vt->GetSetDreamTimeLimit(this, this->vt->DreamSys__GetDreamTimerScaled(this) + saved);
	return true;
}
#endif
INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__TryInstantTeleportLink);

bool ExecuteLink(DreamSys *system, s32 stage, s32 unk1, s32 unk2)
{
	DreamSysUnk58 *obj;

	system->pendingLinkType = unk1;
	system->vt->slot30(system, unk1);
	if (system->pendingLinkType == 0) {
		return false;
	}
	system->currentStage = stage;
	if (system->isFlashbackSession) {
		system->dreamTimer = 0;
	}
	if (unk2 != 0) {
		obj = (DreamSysUnk58 *)system->soundObj;
		obj->vt->slot0x80(obj, 0x90, 0x6E, 0x6E);
	}
	return true;
}

#if 0
/* Best-reached body, 57/88 words, no address drift -- see
   docs/match-reports/DreamSys__TryStaircaseLink.md for the residue analysis. Restored to
   INCLUDE_ASM below per project rule (no score short of byte-exact stays in
   src/). Re-verified fresh round 39 (2026-09-14, runner echo); one new
   reshape tried (hoisting `&this->linkCoordinates` into a function-top local
   named `coords`, on the theory that computing it once outside both branches
   might suppress the `fill_eager_delay_slots` duplication into the branch
   target) -- regressed hard (18/88, 135807 bytes of whole-image drift, an
   extra callee-saved register), confirming the same "cast/hoist to a
   function-scope local costs a register" class already documented for
   `DreamSys__InstanceEffectsOnJournal`. Reverted immediately. */
bool DreamSys__TryStaircaseLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 local[4];

	if (this->pendingLinkType != 0) {
		return false;
	}

	if (this->staircaseTickFn == 0) {
		goto staircase;
	}
	if (!this->staircaseTickFn(this)) {
		return false;
	}
	this->staircaseActive = 0;
	this->staircaseTickFn = 0;
	this->staircaseMoveGate = 0;
	if (this->moveMode != 4) {
		return false;
	}
	this->vt->DreamSys__RestorePreviousMoveMode(this);
	return false;

staircase:
	result = Test4StaircaseNodes(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0) {
		return false;
	}
	Class6B5CC__GetRotationDegrees(this, local);
	if (!DreamSys__CheckStaircaseHeading(&this->exitRotation, &this->enterRotation, local)) {
		return false;
	}
	if (this->moveCommandLatch == 0) {
		return false;
	}

	this->staircaseGridPos = *(PlayerSpawnGridPos *)currentPos;
	this->staircaseOrigin = currentPos->position;
	this->staircaseActive = 1;
	this->staircaseMoveGate = 1;
	this->staircaseFrame = 0;
	this->staircaseTickFn = STAIRCASE_TICK_FNS[GetLastSpawnExtra()];
	this->vt->func_8001CEB4(this, 1, (void *)this->enterRotation);
	this->staircaseTickFn(this);
	return false;
}
#endif
INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__TryStaircaseLink);

s32 DreamSys__TickStaircaseCase0(DreamSys *this)
{
	if (this->staircaseFrame == 0) {
		DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_0, &this->staircaseOrigin);
	}
	if (this->moveMode != 4) {
		if (this->staircaseFrame >= 0x85)
			return 1;
		if ((u32)(this->staircaseFrame - 0x2B) < 0xF || (u32)(this->staircaseFrame - 0x4B) < 0xF) {
			this->turnCommand = 2;
		}
	} else {
		if (this->staircaseFrame >= 0x13)
			return 1;
		if ((u32)(this->staircaseFrame - 8) < 2 || (u32)(this->staircaseFrame - 0xD) < 2) {
			this->vt->func_8001CEB4(this, 0, &ROTATION_YAW_PLUS45);
		}
	}
	this->moveCommand = 1;
	this->staircaseFrame++;
	return 0;
}

s32 DreamSys__TickStaircaseCase1(DreamSys *this)
{
	s32 flag;

	if (this->staircaseFrame == 0) {
		DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_1, &this->staircaseOrigin);
	}
	if (this->moveMode != 4) {
		if (this->staircaseFrame >= 0x95)
			return 1;
		if ((u32)(this->staircaseFrame - 0x16) < 0xF || (u32)(this->staircaseFrame - 0x39) < 0x10 || (u32)(this->staircaseFrame - 0x6E) < 0xF) {
			this->turnCommand = 1;
		}
		flag = (u32)(this->staircaseFrame - 0x39) < 0x35;
	} else {
		if (this->staircaseFrame >= 0x19)
			return 1;
		if ((u32)(this->staircaseFrame - 6) < 2 || (u32)(this->staircaseFrame - 0xB) < 2 || (u32)(this->staircaseFrame - 0x14) < 2) {
			this->vt->func_8001CEB4(this, 0, &ROTATION_YAW_MINUS45);
		}
		flag = (u32)(this->staircaseFrame - 3) < 0xE;
	}
	if (flag) {
		this->lookOffsetCommand = 2;
	}
	this->moveCommand = 1;
	this->staircaseFrame++;
	return 0;
}

s32 DreamSys__TickStaircaseCase2(DreamSys *this)
{
	if (this->staircaseFrame == 0) {
		DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_2, &this->staircaseOrigin);
	}
	if (this->moveMode != 4) {
		if (this->staircaseFrame < 0x65) {
			if ((u32)(this->staircaseFrame - 0x2B) < 0xF) {
				this->turnCommand = 2;
			}
		} else {
			return 1;
		}
	} else {
		if (this->staircaseFrame < 15) {
			if ((u32)(this->staircaseFrame - 8) < 2) {
				this->vt->func_8001CEB4(this, 0, &ROTATION_YAW_PLUS45);
			}
		} else {
			return 1;
		}
	}
	this->moveCommand = 1;
	this->staircaseFrame++;
	return 0;
}

s32 DreamSys__TickStaircaseCase3(DreamSys *this)
{
	s32 flag;

	if (this->staircaseFrame == 0) {
		DreamSys__ApplyRelativeOffset(this, &STAIRCASE_OFFSET_3, &this->staircaseOrigin);
	}
	if (this->moveMode != 4) {
		if (this->staircaseFrame >= 0x71)
			return 1;
		if ((u32)(this->staircaseFrame - 0x1E) < 0xF || (u32)(this->staircaseFrame - 0x52) < 0xF) {
			this->turnCommand = 1;
		}
		flag = (u32)(this->staircaseFrame - 0x1E) < 0x34;
	} else {
		if (this->staircaseFrame >= 0x13)
			return 1;
		if ((u32)(this->staircaseFrame - 6) < 2 || (u32)(this->staircaseFrame - 0xF) < 2) {
			this->vt->func_8001CEB4(this, 0, &ROTATION_YAW_MINUS45);
		}
		flag = (u32)this->staircaseFrame < 9;
	}
	if (flag) {
		this->lookOffsetCommand = 2;
	}
	this->moveCommand = 1;
	this->staircaseFrame++;
	return 0;
}

void DreamSys__ApplyRelativeOffset(DreamSys *this, struct RelativePos *a, struct RelativePos *b)
{
	DreamSysVec3 diff;

	diff.x = a->x - b->x;
	diff.y = a->y - b->y;
	diff.z = a->z - b->z;
	diff.y = 0;
	this->vt->BaseObjO__AddVec14(this, &diff);
}

s32 DreamSys__GetCurrentStage(DreamSys *this)
{
	return this->currentStage;
}

void DreamSys__ProcessChunkChange(DreamSys *this, void *entity, s32 effect)
{
	PlayerSpawnPoint *pos;

	if (effect == 5) {
		pos = ((DreamSysEntityObj *)entity)->methods->slot0x10C(entity, 0, 0);
		this->vt->LogChunkMood(this, pos);
	}
}

#if 0
/* Best-reached body, 1 word short (109/110 instructions), 106/110 words
   truly correct after asm-differ realignment (see the match report for why
   funcdiff's own raw count reads far lower) -- see
   docs/match-reports/DreamSys__InstanceEffectsOnJournal.md for the residue
   analysis. Restored to INCLUDE_ASM below per project rule (no score short
   of byte-exact stays in src/). Re-verified fresh round 37 (2026-09-12,
   runner charlie). Re-verified fresh again round 39 (2026-09-14, runner
   echo); two new reshapes tried on the two established residues (an
   uncast `void *e = entity;` alias local at case 4 -- a fifth form on the
   already-closed "cast/typing axis", byte-identical; and a named `s32 idx
   = effect; switch (idx)` for the switch-index register choice -- also
   byte-identical). Both axes remain confirmed compiler-level, invisible to
   source spelling. */
void DreamSys__InstanceEffectsOnJournal(DreamSys *this, void *entity, s32 effect)
{
	if (this->pendingLinkType != 0) {
		return;
	}

	switch (effect) {
	case 4:
		((DreamSysEntityObj *)entity)->methods->slot0x38(entity, this);
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
		this->vt->LogInstanceMood(this, ((DreamSysEntityObj *)entity)->methods->slot0x14C(entity));
		this->instanceFlasbackUnlockScore += ((DreamSysEntityObj *)entity)->methods->slot0x150(entity);
		this->vt->FlashbackSaving(this, 0, 0x10);
		break;
	case 10: {
		s32 saved = this->currentStage;
		this->currentStage = -((DreamSysEntityObj *)entity)->methods->slot0x154(entity);
		this->vt->DynamicLink(this);
		if (this->currentStage < 0) {
			this->currentStage = saved;
		}
		break;
	}
	case 11:
		if (this->isFlashbackSession != 0) {
			return;
		}
		this->nextCinematic.bank = -1;
		this->dreamTimer = this->dreamTimeLimit;
		this->nextCinematic.entry = ((DreamSysEntityObj *)entity)->methods->slot0x158(entity);
		break;
	case 12:
		if (this->isFlashbackSession != 0) {
			return;
		}
		this->dreamTimer = this->dreamTimeLimit;
		break;
	}
}
#endif
INCLUDE_ASM("asm/nonmatchings/DreamSys", DreamSys__InstanceEffectsOnJournal);

void DreamSys__GetPreviousDayMood(DreamSys *this, MoodGraphPoint *target, bool unknown)
{
	s32 upper = 0;
	s32 dynamic = 0;

	if (unknown) {
		if (this->currentYear != 0 || this->currentDay != 0) {
			s32 idx;

			idx = this->currentDay - 1;
			dynamic = this->moodPreviousDays[idx].axis.dynamic;
			upper = this->moodPreviousDays[idx].axis.upper;
		}
	} else {
		s32 count;
		count = 0x16D;
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

void DreamSys__InitMoodContibutors(DreamSys *this, MoodGraphPoint *special)
{
	this->vt->ClearMoodGraph(this, &this->areaMoods);
	this->vt->ClearMoodGraph(this, &this->entityMoods);
	if (special != NULL) {
		this->vt->LogMood(this, &this->areaMoods, special);
		this->vt->LogMood(this, &this->entityMoods, special);
	}
}

void DreamSys__LogChunkMood(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	MoodGraphPoint *mood;

	mood = GetMoodFromStageChunk(this->currentStage, (StageChunk *)currentPos);
	this->vt->LogMood(this, &this->areaMoods, mood);
}

void DreamSys__LogInstanceMood(DreamSys *this, MoodGraphPoint *source)
{
	this->vt->LogMood(this, &this->entityMoods, source);
}

void DreamSys__UpdateDreamChart(DreamSys *this, MoodGraphPoint *ret)
{
	MoodGraphPoint areaAvg;
	MoodGraphPoint entityAvg;

	this->vt->GetMoodAverage(this, &this->areaMoods, &areaAvg);
	this->vt->GetMoodAverage(this, &this->entityMoods, &entityAvg);
	if (this->entityMoods.amountMoods == 0) {
		entityAvg.value = areaAvg.value;
	}
	ret->axis.dynamic = (areaAvg.axis.dynamic + entityAvg.axis.dynamic) / 2;
	ret->axis.upper = (areaAvg.axis.upper + entityAvg.axis.upper) / 2;
}

DreamColors DreamSys__GetDreamColor(DreamSys *this)
{
	MoodGraphPoint local;

	this->vt->UpdateDreamChart(this, &local);
	return CalcDreamColor(&local);
}

#ifdef NON_MATCHING
/* NON_MATCHING: 28/35 words, exact length (zero address drift). Residue:
 * the sixth confirmed instance of the project-wide commutative-add
 * operand-order/register-identity class (round 20) -- retail computes the
 * table-base address early, this build loads `upper` early instead, both
 * final `addu`s register-swapped. PERMUTER-EXHAUSTED (~40400 iterations,
 * no zero-scoring candidate); hand-derived.
 * docs/match-reports/CalcDreamColor.md. */
DreamColors CalcDreamColor(MoodGraphPoint *mood)
{
	MoodGraphPoint local;
	s8 *p;
	s32 i;
	s8 val;

	local.value = mood->value;
	p = (s8 *)&local;
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
	{
		s32 index;
		s8 *entry;

		index = local.axis.dynamic * 3;
		entry = &DREAM_COLOR_TABLE[index];
		return entry[local.axis.upper];
	}
}
#else
INCLUDE_ASM("asm/nonmatchings/DreamSys", CalcDreamColor);
#endif

void DreamSys__ClearMoodGraph(DreamSys *this, MoodGraphContributor *contributor)
{
	contributor->lastMood.value = 0;
	contributor->sumMoods.upper = 0;
	contributor->sumMoods.dynamic = 0;
	contributor->amountMoods = 0;
}

void DreamSys__LogMood(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *mood)
{
	layer->lastMood.value = mood->value;
	layer->sumMoods.dynamic = mood->axis.dynamic + layer->sumMoods.dynamic;
	layer->sumMoods.upper = mood->axis.upper + layer->sumMoods.upper;
	layer->amountMoods = layer->amountMoods + 1;
}

void DreamSys__GetMoodAverage(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *ret)
{
	if (layer->amountMoods != 0) {
		ret->axis.dynamic = CalcMoodAxis(layer->lastMood.axis.dynamic, layer->sumMoods.dynamic, layer->amountMoods);
		ret->axis.upper = CalcMoodAxis(layer->lastMood.axis.upper, layer->sumMoods.upper, layer->amountMoods);
	} else {
		ret->value = layer->lastMood.value;
	}
}

s32 CalcMoodAxis(s32 lank, s32 sum, s32 amount)
{
	s32 result;

	result = sum / amount;
	result += lank / 3;
	if (result >= 10)
		result = -9;
	else if (result < -9)
		result = 9;
	return result;
}

void DreamSys__CalcUnlockScore(DreamSys *this)
{
	this->navigationFlasbackUnlockScore = CalcNavigationScore();
	if (this->instanceFlasbackUnlockScore < 0) {
		this->instanceFlasbackUnlockScore = 0;
	} else if (this->instanceFlasbackUnlockScore > 50000000) {
		this->instanceFlasbackUnlockScore = 50000000;
	}
	this->totalFlasbackUnlockScore = this->navigationFlasbackUnlockScore + this->instanceFlasbackUnlockScore;
}

void DreamSys__AddFlashback(DreamSys *this, s32 stage, PlayerSpawnPoint *pos, s32 *angles, s32 unknown, s32 time, s32 day)
{
	FlashbackEntry *entry;

	entry = this->storedFlasbacks;
	if (this->amountFlashbacksAvailable < 10) {
		entry += this->amountFlashbacksAvailable++;
	} else {
		entry += (u32)this->dreamTimer % 9;
	}
	entry->stageID = stage;
	entry->position = *pos;
	entry->rotation = *(FlashbackRotation *)angles;
	entry->unknown_value_0x1c = unknown;
	entry->timeLimit = time;
	entry->day = day;
}

void DreamSys__FlashbackSaving(DreamSys *this, s32 arg1, s32 arg2)
{
	PlayerSpawnPoint *pos;
	s32 local[4];

	if (this->linkMgr != NULL && rand() % 3 == 0) {
		pos = this->linkMgr->methods->slot0x10C(this->linkMgr, 0, 0);
		Class6B5CC__GetRotationDegrees(this, local);
		this->vt->AddFlashback(this, this->currentStage, pos, local, arg1, arg2, this->currentDay);
	}
}

void DreamSys__ResetFlashbackList(DreamSys *this)
{
	this->amountFlashbacksAvailable = 0;
}

void DreamSys__SaveLinkSnapshot(DreamSys *this)
{
	DreamSysUnk14 *p = this->unk_0x14;

	this->unk14Snapshot = *p;
	this->unk14TailSnapshot = *p->unk_0x44;
}

void DreamSys__RestoreLinkSnapshot(DreamSys *this)
{
	DreamSysUnk14 *p = this->unk_0x14;

	*p = this->unk14Snapshot;
	*p->unk_0x44 = this->unk14TailSnapshot;
	p->unk_0x0 = 0;
}

s32 DreamSys__func_5ba20(DreamSys *this, s32 value)
{
	s32 old;

	if (value >= 0) {
		old = this->unk_0x924;
		this->unk_0x924 = value;
	} else {
		old = this->unk_0x924;
	}
	return old;
}

struct vtable_DreamSys *Get_vtable_DreamSys(void)
{
	return &DREAMSYS_METHODS;
}

void InitNavChallengesArray(s8 (*arrayMem)[30], s32 *linkCounter)
{
	s32 i;

	for (i = 29; i >= 0; i--)
		(*arrayMem)[i] = 0;
	gpNavChallengesComplete = arrayMem;
	*linkCounter = 0;
	gpDinamicLinkPenalty = linkCounter;
}

s32 CalcNavigationScore(void)
{
	s32 sum;
	s8 *p;
	s32 i;

	sum = 0;
	p = *gpNavChallengesComplete;
	i = 0;
	do {
		if (p[i] != 0)
			sum += 1000000;
		i++;
	} while (i < 30);
	if (sum > 29999999)
		sum = 50000000;
	sum -= *gpDinamicLinkPenalty * 11024;
	if (sum < 0)
		sum = 0;
	return sum;
}

s32 GetStageTimeLimit(s32 stage)
{
	return STAGE_TIME_LIMITS[stage];
}

s32 GetRandomSpawnFromStage(PlayerSpawnPoint *target, s32 stg, s32 unused)
{
	s32 stage;
	s32 index;
	StageSpawn *entry;
	s32 six;

	six = 6;
	if (stg >= 0) {
		stage = rand() % six;
		if (stage == stg) {
			stage++;
			if (stage >= 6)
				stage = 0;
		}
	} else {
		stage = -stg;
	}

	index = rand() % LEN_STAGE_SPAWNPOINTS[stage];
	entry = &STAGE_SPAWNPOINTS[stage][index];
	*(PlayerSpawnGridPos *)target = *(PlayerSpawnGridPos *)entry;
	target->position = SPAWN_POS_ADJUST[entry->adjustment];
	(*gpDinamicLinkPenalty)++;
	return stage;
}

s32 TestForStaticLink(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, LEN_STAGE_PERMALINK_TRIGGERS,
	                       STAGE_PERMALINK_TRIGGERS, STAGE_PERMALINK_SPAWNS, 1);
}

s32 Test4TunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, LEN_TUNNEL_TRIGGERS,
	                       TUNNEL_TRIGGERS, TUNNEL_SPAWNS, 1);
}

/* Unit-local reading of the second parameter: the caller (DreamSys__CheckTunnelHeading)
   passes down a `s32 local[4]` buffer that Class6B5CC__GetRotationDegrees (code_d294_c) fills
   with a 3-entry WholeFrac_d294 table; the byte offset +4 read here lands on
   that table's `out[1].whole` (a degrees value, per Class6B5CC__GetRotationDegrees's own
   report). This function reads it unsigned (`lhu`), independent of
   WholeFrac_d294's own `s16 whole` -- a second, disjoint view of the same
   bytes, so it is kept local rather than folded into that shared struct.
   Moved above DreamSys__CheckTunnelHeading (round 43) because that function's own arg2 is
   cast to this type before being forwarded to IsHeadingAligned below. */
typedef struct DirectionCheckArg {
	s8 unk0[4];
	u16 heading;
} DirectionCheckArg;

/* 4-entry cardinal-direction table (12-byte stride); only the first u16 of
   each entry (the angle: 0/90/180/270) is read anywhere in this unit's
   queue. Kept local for the same reason as DirectionCheckArg above.
   Round 66: this view is a window into CARDINAL_ROTATIONS (below), 4 bytes
   further on -- `angle` is that entry's yaw NUMERATOR, i.e.
   CARDINAL_ROTATIONS[i].y.numerator, and `unk2` is its denominator (always
   1). The two views are kept separate because this one reads the angle as a
   bare u16 for arithmetic while the other is only ever address-taken and
   handed to func_8001CEB4 as a rotation. */
typedef struct DirectionTableEntry {
	u16 angle;
	u16 unk2;
	u16 unk4;
	u16 unk6;
	u16 unk8;
	u16 unkA;
} DirectionTableEntry;

extern DirectionTableEntry CARDINAL_ANGLES[];

/* Forward declaration: defined below in ROM order, called by DreamSys__CheckTunnelHeading
   just above it. */
extern s32 IsHeadingAligned(DirectionCheckArg *a0, u8 a1);

/* TUNNEL_ENTER_HEADINGS: a per-stage table of pointers to byte arrays (4-byte stride,
   indexed by gLinkSrcStage), each further indexed by gLinkTriggerIndex to read the
   "heading" byte passed to IsHeadingAligned. TUNNEL_EXIT_HEADINGS is the analogous
   table for gLinkDstStage/gLinkSpawnIndex. Neither array's own element type is
   dereferenced beyond a single `u8` here. */
extern u8 *TUNNEL_ENTER_HEADINGS[];
extern u8 *TUNNEL_EXIT_HEADINGS[];

/* The 12-byte-stride table whose first element sits 4 bytes before the
   separately-referenced `CARDINAL_ANGLES` -- splat drew the boundary there
   because `CARDINAL_ANGLES` is independently referenced, not because the
   underlying data is two different tables. Round 66 types it
   `RotationRatios` (include/DreamSys.h) rather than as a stride-only
   placeholder: every entry is three {numerator, denominator} degree ratios
   in exactly the form func_8001CEB4 consumes, and the four entries' yaw
   numerators are 0, 0x5A, 0xB4, 0x10E -- 0, 90, 180 and 270 degrees. That is
   also what the two functions below do with an element: they store its
   ADDRESS into DreamSys::enterRotation / ::exitRotation, and the only things
   those two fields are ever used for are func_8001CEB4(this, 1, ptr) calls
   in DreamSys__SetMoveOverride, DreamSys__SpawnAtLink and
   DreamSys__TryStaircaseLink. */
extern RotationRatios CARDINAL_ROTATIONS[];

s32 DreamSys__CheckTunnelHeading(s32 *arg0, s32 *arg1, void *arg2)
{
	u8 heading;
	s32 idx;
	s32 result;

	heading = TUNNEL_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex];
	if (IsHeadingAligned((DirectionCheckArg *)arg2, heading)) {
		if (arg1 != NULL)
			*arg1 = (s32)&CARDINAL_ROTATIONS[heading];

		if (arg0 != NULL) {
			idx = TUNNEL_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex];
			*arg0 = (s32)&CARDINAL_ROTATIONS[idx];
		}
		result = 1;
	} else {
		result = 0;
	}
	return result;
}

s32 IsHeadingAligned(DirectionCheckArg *a0, u8 a1)
{
	s16 diff;

	diff = a0->heading - CARDINAL_ANGLES[a1].angle;
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

s32 Test4StageTransition(PlayerSpawnPoint *target, s32 stage, PlayerSpawnPoint *currentPos, s32 timer)
{
	s32 result;

	if (stage == 3)
		goto shared;
	if (stage == 1)
		goto shared;
	if (stage == 5)
		goto case5;
	if (stage == 9)
		goto shared;
	if (stage != 0xC)
		return -1;

shared:
	if (stage != 5)
		goto case9check;
case5:
	if (currentPos->position.y < -0xFFF)
		goto merge;
	if (*(s32 *)currentPos == STAGE5_TRIGGER_GRIDPOS)
		goto merge;
	return -1;

case9check:
	if (stage != 9)
		goto merge;
	if (currentPos->position.y < 0x800)
		return -1;

merge:
	if (timer & 1)
		stage = -0xC;
	result = GetRandomSpawnFromStage(target, stage, timer);
	gLinkDstStage = result;
	return result;
}

/* Set (whole word) into `this->stageLinkAngle` by DreamSys__TryStageTimerLink just before an
   ExecuteLink; only ever address-taken here, never dereferenced by this
   unit's queued functions. */
extern s32 LINK_ANGLE_180;

s32 GetStageLinkAngle(void)
{
	s32 result;

	result = 0;
	if (gLinkDstStage != 0xC)
		result = (s32)&LINK_ANGLE_180;
	return result;
}

/* Flag set here, tested by Test4InstantTeleporters right below; local to
   this unit -- code_4cd08.c calls the setter through its own extern
   (`extern void SetInstantTeleportersEnabled(bool value);`), never touches the flag
   directly. */
extern s32 gInstantTeleportersEnabled;

void SetInstantTeleportersEnabled(bool value)
{
	gInstantTeleportersEnabled = value;
}

/* Table triple for Test4InstantTeleporters, same roles as the
   LEN_TUNNEL_TRIGGERS/LEN_STAIRCASE_TRIGGERS triples above but for instant-teleporter links. */
extern s8 LEN_TELEPORT_TRIGGERS[];
extern StaticLinkTrigger* TELEPORT_TRIGGERS[];
extern StageSpawn* TELEPORT_SPAWNS[];

s32 Test4InstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	s32 result;

	if (gInstantTeleportersEnabled == 0) {
		result = -1;
	} else {
		result = GetStaticSpawn(target, currentPos, stage, LEN_TELEPORT_TRIGGERS,
		                         TELEPORT_TRIGGERS, TELEPORT_SPAWNS, 0);
	}
	return result;
}

s32 func_8005BFC4(void)
{
	return (gLinkSrcStage == 0) ? 0xA : 0;
}

s32 Test4StaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 arg2)
{
	if (arg2 == 0)
		return GetStaticSpawn(target, currentPos, 0, LEN_STAIRCASE_TRIGGERS,
		                       STAIRCASE_TRIGGERS, STAIRCASE_SPAWNS, 0);
	return -1;
}

/* Same role as TUNNEL_ENTER_HEADINGS/TUNNEL_EXIT_HEADINGS for DreamSys__CheckTunnelHeading above, but for this
   function's own "link test" (indexed the same way: gLinkSrcStage/gLinkTriggerIndex
   for the heading lookup, gLinkDstStage/gLinkSpawnIndex for the second table). */
extern u8 *STAIRCASE_ENTER_HEADINGS[];
extern u8 *STAIRCASE_EXIT_HEADINGS[];

s32 DreamSys__CheckStaircaseHeading(s32 *arg0, s32 *arg1, void *arg2)
{
	u8 heading;
	s32 idx;
	s32 result;

	heading = STAIRCASE_ENTER_HEADINGS[gLinkSrcStage][gLinkTriggerIndex];
	if (IsHeadingAligned((DirectionCheckArg *)arg2, heading)) {
		if (arg1 != NULL)
			*arg1 = (s32)&CARDINAL_ROTATIONS[heading];

		if (arg0 != NULL) {
			idx = STAIRCASE_EXIT_HEADINGS[gLinkDstStage][gLinkSpawnIndex];
			*arg0 = (s32)&CARDINAL_ROTATIONS[idx];
		}
		result = 1;
	} else {
		result = 0;
	}
	return result;
}

s32 GetLastSpawnExtra(void)
{
	return STAIRCASE_SPAWNS[gLinkDstStage][gLinkSpawnIndex].extra;
}

s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                    s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag)
{
	s32 count;
	StaticLinkTrigger *trig;
	s32 i;
	StageSpawn *entry;
	s32 triggerStage;
	u32 spawnIndex;

	count = *(u8 *)&triggerLens[stage];
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
		spawnIndex = *(u8 *)&trig->spawnpointIndex;
		entry = &spawns[triggerStage][spawnIndex];
		gLinkSpawnIndex = spawnIndex;
		*(PlayerSpawnGridPos *)target = *(PlayerSpawnGridPos *)entry;
		target->position = SPAWN_POS_ADJUST[entry->adjustment];
		if (flag != 0)
			(*gpNavChallengesComplete)[entry->extra] = 1;
		return gLinkDstStage;
	}
	return -1;
}

s32 GenerateInitialSpawn(PlayerSpawnPoint *dest, s32 *timeLimit, MoodGraphPoint *mood, s32 day)
{
	StageChunk chunk;
	s32 stage;
	s32 count;
	s32 i;
	StageSpawn *entry;

	stage = GetStageChunkFromMood(&chunk, mood);
	if (stage >= 0) {
		*timeLimit = STAGE_TIME_LIMITS[stage];

		count = LEN_STAGE_SPAWNPOINTS[stage];
		entry = STAGE_SPAWNPOINTS[stage];
		for (i = 0; i < count; i++, entry++) {
			if (*(s16 *)&chunk == *(s16 *)&entry->chunk)
				goto found;
		}
		entry = &STAGE_SPAWNPOINTS[stage][*(s16 *)&chunk % count];

	found:
		*(PlayerSpawnGridPos *)dest = *(PlayerSpawnGridPos *)entry;
		dest->position = SPAWN_POS_ADJUST[entry->adjustment];
		return stage;
	}

	stage = GetRandomSpawnFromStage(dest, stage, day);
	*timeLimit = STAGE_TIME_LIMITS[stage];
	return stage;
}

/* rand() is not declared by any header this unit includes; Entity.h's own
   local view is s32 rand(void). */
extern s32 rand(void);

MoodGraphPoint *IsDaySpecial(CinematicCall *cinematic, int day)
{
	s32 i;

	for (i = 0; (u32)i < 42; i++) {
		if (day == SPECIAL_DAYS[i]) {
			cinematic->entry = rand() % 6;
			cinematic->bank = i % 12;
			return &SPECIAL_DAY_MOOD;
		}
	}
	return NULL;
}
