# DreamSys__DreamSys

**Unit:** DreamSys · **Size:** 63 words (0xFC bytes) · **Status:** MATCHED
(63/63 words, whole-image `./build-and-verify.sh` green)

## What it does

The class constructor, called through the method table's own `Constructor`
slot (`+0x008`, resolved via `tools/classtable.py gDreamSysMethods`) by
`New_DreamSys`. Chains into the shared base class's own constructor, installs
DreamSys's own vtable, stashes the three forwarded constructor arguments,
performs a "buddy-link" step against its `arg1` companion object, and tail-
returns whatever the constructor's own last vtable step returns:

```c
DreamSys *DreamSys__DreamSys(DreamSys *this, void *arg1, s32 arg2, s32 arg3)
{
	void *val;

	GetActorMethods()->ctor(this);
	this->vt = GetDreamSysMethods();
	this->unk_0x58 = arg2;
	this->unk_0x5C = (DreamSysUnk5C *)arg3;
	this->unk_0x64 = 0;
	this->unk_0x60 = arg1;
	val = ((DreamSysCtorArgObj *)arg1)->methods->slot0x80(arg1, 0);
	this->vt->slot10(this, val);
	this->vt->GetSetDreamTimeLimit(this, -1);
	this->unk_0x70 = 1;
	this->unk_0x6c = 0;
	this->unk_0x878 = 1;
	this->vt->InitNewGame(this);
	return this->vt->func_800588EC(this);
}
```

Matched first attempt.

## The head's broadcast lever applied directly

This round's head broadcast (`New_GameApplication`, closed 24/24) was: a
constructor-family function often should NOT restate a return value with an
explicit `return X;` when the value is already sitting in `$v0` from the
immediately preceding call. This function's disassembly ends with a `jalr` to
`vt->func_800588EC(this)` and NOTHING overwrites `$v0` before the epilogue —
so `return this->vt->func_800588EC(this);` as the function's literal last
statement is *already* the non-restating form: the call's own return value
is the function's return value, with zero extra `move`. No iteration was
needed to find this; reading the disassembly's tail directly (delay slot after
the final `jalr` flows straight into the `lw $ra`/epilogue, no `move $v0,...`
anywhere) made it obvious before writing any C.

## New knowledge (all `include/DreamSys.h`)

- **`DreamSysBaseMethods` (this unit's local view of the shared `gActorMethods`
  base table) gets a new slot at `+0x008`: `ctor`.** Cross-confirmed against
  `src/world/TodActor.c`'s `D800878D4Methods`, which ALREADY names and
  resolves this exact slot as `Actor__Actor`, taking/returning
  `TodActor *self` — the same base constructor, just viewed through a
  different subclass's local header (per this project's established
  "multiple independent local views of the same table" convention). Typed
  `struct DreamSys *(*ctor)(struct DreamSys *self)` here, matching this
  unit's existing forward-tag convention for `DreamSysBaseMethods`. Its
  return value is discarded at this call site (the base ctor returns `self`
  for chaining, unneeded since the caller already holds `this`).
- **`vtable_DreamSys+0x010` is `slot10`, shared with TodActor's OWN vtable
  at the identical offset.** `TodActor.c` already names and resolves it
  there as `Actor__AddChild`, and its own comment identifies it as the "link"
  companion of `slot14`/`Actor__RemoveChild` — a slot THIS unit's header already
  names (at `+0x014`, same offset relationship) with the same companion
  description, just from `DreamSys`'s side. This round's call
  (`this->vt->slot10(this, val)`) is the constructor performing that exact
  link step.
- **`DreamSysCtorArgObj`**, a new minimal opaque class for the constructor's
  own `arg1` parameter (same "vtable pointer at offset 0" shape as this
  unit's other opaque views) — its `slot0x80(self, 0)` returns a companion
  pointer forwarded straight into `vt->slot10`, matching the buddy-link shape.
- **`DreamSys::unk_0x60`** carved out of a 4-byte padding gap: set
  unconditionally to the constructor's `arg1`, `void *` typed (no further use
  in this unit's queued functions).
- **`vtable_DreamSys+0x03C` (`func_800588EC`) retyped from an untyped `void *`
  placeholder to `DreamSys *(*func_800588EC)(DreamSys *this)`** — the
  constructor's last step and this function's own return value (see above).

### Proposed learning

None beyond what the head's own broadcast already captured for
`New_GameApplication` — this is a second, independent, zero-attempt confirmation
of the same lever ("a constructor's tail call already leaves the return value
in the right place; do not restate it"), which is worth noting only as
reinforcement, not as a new finding.

## Track 4 (LinkResource)

2026-09-26, round 89 (delta): `arg1` is a LinkResource (include/LinkResource.h:
GameApplication__GameApplication passes New_LinkResource("ETC\DREAME5.TMD")), so the
DreamSysCtorArgObj/DreamSysCtorArgMethods view is gone and the call is
`arg1->methods->getModel(arg1, 0)` (+0x080, LinkResource__GetModel: the
first TmdModel, added as a child). `unk_0x60`, the ctor slot's and
New_DreamSys's parameter are typed `struct LinkResource *`. Byte-identical.

## Comment moved from src/world/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* DreamSys -- the object that IS a dream in progress: it owns the dream
 * clock, the player-ish body that walks around the stage, the mood record
 * that decides the next day's dream, and the "link" (teleport) machinery
 * that ends one stage and starts another. See include/DreamSys.h for the
 * class as a whole; several of its methods live in sibling units
 * (ObjMStyleActor/t/r/o) because the class spans more than one segment.
 *
 * Four groups of functions live here.
 *
 * 1. Construction and reset. New_DreamSys/DreamSys__DreamSys,
 *    DreamSys__ResetSessionState (the constructor's last step: no tick
 *    callbacks, sound cue set freed, staircase-walk state cleared, base
 *    orientation applied), DreamSys__ResetLinkState, DreamSys__SpawnAtLink.
 *
 * 2. The per-tick chain. DreamSys__TimerTick advances the clock (Actor's `tick`) and, at
 *    dreamTimeLimit, either loads the next flashback or ends the dream;
 *    below the limit it runs DreamSys__UpdateTickState and
 *    DreamSys__RunTickCallbacks, which call the two callback slots
 *    DreamSys__SelectLookCallback/98 install. Slot 80's mode 1 is
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
 *    that DreamSys__InitNewGame initializes and that start with sSaveMagic.
 *
 * 4. Linking. DreamSys__WallLink/DynamicLink, the "Try...Link" family and
 *    the free "Test4..."/GetStaticSpawn static-link testers underneath them;
 *    ExecuteLink writes the link type into Actor's `state` (+0x044).
 *
 * Naming pass: round 66 (Opus). Everything above is measured, not guessed --
 * the evidence for each name is in that function's match report under
 * `## Naming`. Four functions are deliberately still `func_`-shaped
 * (DreamSys__SetEtcTim/59590/59598/5ba20 and the free GetTeleportTimeBonus): each
 * touches exactly one field or global that nothing in any carved unit ever
 * reads back, so there is nothing to name them after. */
```

## Naming (round 92, track 7)

- Parameters `modelSource`, `soundObj`, `viewport` (were `arg1..arg3`): the
  ctor stores the second and third in `soundObj` / `viewport` (both also
  reachable through `setSoundObj` / `setViewport`), and takes model 0 of the
  first -- the LinkResource GameApplication__GameApplication builds from
  "ETC\DREAME5.TMD" -- as its child.
- Field `DreamSys::unk_0x60` -> `modelSource` (tier A: its one writer is this
  ctor, storing that LinkResource; nothing reads it back). Every accessor is in
  src/world/DreamSys.c.

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * Round 66's naming (per-name evidence in docs/match-reports/<func>.md)
 * rests on two cross-unit identifications: updateRotation (+0x044) is the
 * rotation setter, so every constant passed to it is three degree ratios
 * (RotationRatios below); and soundObj is a VabStreamObj, whose +0x080/
 * +0x084/+0x09C (playTone/stopVoice/setPitchOffset) name voiceSelect and
 * voiceIndex.
```

```c
/* For StageChunk / GetMoodFromStageChunk, used by DreamSys__LogChunkMood
   (round 2026-08-30-d). */
```

```c
/* .sbss values. The 7B4B4 sbss segment that actually holds these is still
   plain `data` (un-flipped to dot-form), so it already provides these
   symbols; declaring them `extern` here lets this header be #included
   without a multiple-definition link error. Whoever flips that segment to
   `.data, DreamSys` should drop `extern` here in the same commit. */
```

```c
   threshold. Still raw `nonmatching` data (round 2026-08-30). */
```

```c
/* A single {numerator, denominator} degree ratio. This is not a guess about
   the LAYOUT any more (round 66): SceneNode__UpdateRotation -- vtable slot +0x044, the
   inherited rotation setter, MATCHED in src/SceneNode.c -- reads exactly
   three of these from its `data` argument, one per axis, converts each with
   RatioToFixed12 and divides by 360, then either STORES them into the
   object's rotation vector (flag != 0) or ADDS them modulo a full turn
   (flag == 0). Every constant this unit hands that slot is three of these,
   and every one of them decodes to a plausible angle: see
   ROTATION_YAW_180 / _PLUS45 / _MINUS45 and CARDINAL_ROTATIONS below. */
```

```c
/* DreamSys::viewport is a Viewport (include/Viewport.h; tag only here,
   DreamSys.c includes the header). Named heightCurve / DreamSysUnk5C until
   track 4 (round 88); the offsets that view named are the Viewport's
   GsRVIEW2 refView: +0x014 vp and +0x020 vr (the two "points"
   ProjectPointAtDistance interpolates between), +0x018 vp.y and +0x024
   vr.y (AdvanceMoveCycle's view bob moves both; StepLookOffset, StopDrift
   and TickDrift move vr.y, i.e. look up and down). DayTask__Init installs
   a New_NodeGuardedViewport through setViewport, and Entity__MoodCue74 calls its
   setClearColor (+0x064). */
```

```c
/* DreamSys::soundObj is a VabStreamObj (include/VabStreamObj.h): DreamSys.c
   casts it there. StartVoice / ExecuteLink call playTone (+0x080; the voice
   it returns goes to voiceIndex), StopVoice calls stopVoice (+0x084), and
   StartVoice calls setPitchOffset (+0x09C). This header used to carry that
   view as DreamSysUnk58 / DreamSysUnk58Vtable (deleted round 87, track 4). */
```

```c
/* DreamSys__DreamSys's `arg1` is a LinkResource (include/LinkResource.h;
   GameApplication__GameApplication passes New_LinkResource("ETC\DREAME5.TMD")): the
   ctor keeps it in modelSource and adds its getModel(0), a TmdModel, as a
   child. This header used to carry that view as DreamSysCtorArgObj /
   DreamSysCtorArgMethods (deleted round 89, track 4). */
```

```c
/* Actor::grid is the grid manager, StageMap (include/StageMap.h, track 4
   round 89); DreamSys.c includes that header and calls it directly. */
```

```c
/* A `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a` argument
   by DreamSys__TickStaircaseCase2 (round 2026-09-02). */
```

```c
/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase0 -- same call shape as STAIRCASE_OFFSET_2 above, just a
   different constant (round 2026-09-02). */
```

```c
/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase1 -- same call shape as STAIRCASE_OFFSET_0/STAIRCASE_OFFSET_2
   above, just a different constant (round 2026-09-02). */
```

```c
/* Another `struct RelativePos` constant, passed as DreamSys__ApplyRelativeOffset's `a`
   argument by DreamSys__TickStaircaseCase3 -- same call shape as STAIRCASE_OFFSET_2/STAIRCASE_OFFSET_0
   above, just a different constant (round 2026-09-02). */
```

```c
   DreamSys__TickStaircaseCase2. Typed RotationRatios round 66: its three
   {numerator, denominator} words are {0,1} {0x2D,1} {0,1}, byte-identical in
   form to ROTATION_YAW_180 and to every CARDINAL_ROTATIONS entry. */
```

```c
/* 3x3 lookup table indexed by [dynamicClass][upperClass], each axis
   classified into {0,1,2} by CalcDreamColor first (round 2026-08-30-d). */
```

```c
    /* Set unconditionally to the constructor's `arg1` by DreamSys__DreamSys
	   (round 2026-09-02): the LinkResource its model 0 came from. No other
	   observed use in this unit's queued functions. */
```

```c
    /* Read by DreamSys__TickMove; compared against 0 / 1, else-branch otherwise.
	   Meaning unidentified beyond that (round 2026-08-30). */
```

```c
    /* Set unconditionally to arg1 by DreamSys__SelectCallback80(this, arg1); no other
	   observed use (round 2026-08-30). */
```

```c
    /* Index into the (LOOK_OFFSET_STEPS, LOOK_OFFSET_LIMITS) delta/threshold table pair,
	   consumed and reset to 0 by DreamSys__StepLookOffset (round 2026-08-30). */
```

```c
    /* Running accumulator nudged by lookOffsetCommand's table entry, or decayed by
	   600/call towards 0 when lookOffsetCommand is 0; also propagated into
	   viewport->refView.vr.y. Set by DreamSys__StepLookOffset (round 2026-08-30). */
```

```c
    /* Index into the (LOOK_YAW_STEPS, LOOK_YAW_LIMITS) delta/threshold table pair,
	   consumed and reset to 0 by DreamSys__StepLookYaw (round 2026-08-30). */
```

```c
    /* Running delta accumulator paired with lookYawCommand; see DreamSys__StepLookYaw
	   (round 2026-08-30). */
```

```c
    /* Set by DreamSys__SelectCallback80(this, arg1) exactly like lookCallback, but from
	   a *different* trio of vtable slots. Called with (this) by
	   DreamSys__RunTickCallbacks, if non-NULL. */
```

```c
    /* "Mode" field read/written by DreamSys__SelectCallback98(this, arg1): when ==2 on
	   entry, this->methods->stopDrift(this, 0) fires first; then it is set
	   unconditionally to arg1 (round 2026-08-30). */
```

```c
    /* (this->moveCommand ^ 1) < 1u, i.e. (moveCommand == 1), written by
	   DreamSys__StepLookYaw; also toggled/incremented by DreamSys__FlipMoveCommand and forced
	   to 1 by DreamSys__TickMoveForced (round 2026-08-30). */
```

```c
    /* Index into the 12-byte-stride TURN_ROTATIONS table; consumed and reset
	   to 0 by DreamSys__ApplyPendingTurn (round 2026-08-30-b). */
```

```c
    /* (moveCommand == 1) as computed by DreamSys__StepLookYaw; unconditionally cleared
	   to 0 by DreamSys__FlipMoveCommand on every call (round 2026-08-30). */
```

```c
    /* "Current" value; DreamSys__RestorePreviousMoveMode overwrites this with previousMoveMode.
	   DreamSys__GetSetMoveMode's bounds-checked setter (vtable +0x180) writes both
	   this and previousMoveMode together; DreamSys__ChangeMoveMode copies the OLD value of
	   this into previousMoveMode before overwriting it, when the new value
	   differs (round 2026-08-30-b). */
```

```c
    /* "Previous"/paired value; see moveMode (round 2026-08-30-b). */
```

```c
	   moveMode) to force moveCommand back to 0 (round 2026-09-02). */
```

```c
    /* Derived from `linkTarget->flags36` masked to 0x7F, or forced to
	   0 (if >= 0x18) or 2 (if `state == 15` and this is still 0)
	   by DreamSys__NotifyLinkAttempt's `arg1 == -1` path (round 2026-09-02). Also an index:
	   DreamSys__StartVoice (round 2026-09-06) does nothing when this is 0, else
	   uses it to index VOICE_BY_SELECT/VOICE_PITCH_BY_SELECT (see those externs), compares
	   it against 0x16 (22) to decide whether to keep or discard
	   voiceIndex's new value, and against 0xB (11) to gate two extra vtable
	   calls. */
```

```c
    /* Gate flag: DreamSys__StopVoice runs its body (a call through
	   soundObj's stopVoice (+0x084), then resets this to -1) only while this is
	   >= 0 (round 2026-08-30-b). */
```

```c
    /* Set to 1 by DreamSys__SelectCallback98's arg1==2 case, alongside cueServiceActive and
	   moveCallback (round 2026-08-30). */
```

```c
    /* Set to 1 by DreamSys__SelectCallback98's arg1==2 case, alongside driftActive
	   (round 2026-08-30). */
```

```c
    /* tickBoundary/0x128/0x12C/0x130 are also bounds-checked-set as a group of
	   four by DreamSys__SetGateFlags (vtable +0x18C): each is overwritten with the
	   corresponding argument only when that argument is >= 0
	   (round 2026-08-30-b). */
```

```c
    /* Set (whole word) by DreamSys__TryStageTimerLink to GetStageLinkAngle()'s return value,
	   right before an ExecuteLink (round 2026-09-02). */
```

```c
    /* Gate flag read by DreamSys__SetMoveOverride (round 2026-08-30-b): when nonzero
	   (reusing the SAME loaded value, not a fresh 0/1 test), forwarded as
	   SceneNode__UpdateRotation's arg2 -- cast from s32 to void*, not dereferenced. */
```

```c
    /* Zeroed (whole word) by DreamSys__TryStageTimerLink alongside enterRotation
	   (round 2026-09-02). */
```

```c
/* Table triple for Test4TunnelLinks (round 2026-08-30-d), same roles as the
   STAGE_PERMALINK_* triple above but for tunnel links specifically. */
```

```c
/* Table triple for Test4StaircaseNodes (round 2026-08-30-d). */
```
