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
	this->vt = Get_vtable_DreamSys();
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
