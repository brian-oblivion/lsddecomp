# DreamSys__InstanceEffectsOnJournal

**Unit:** DreamSys · **Size:** 110 words · **Status:** STALL, 106/110 words
truly correct with no address drift (see "Reading the score" below for why
`funcdiff.py`'s own raw number reads far lower). Attempted round 2026-09-06
(charlie). Screened clean against both remaining blockers (`gp_rel`,
`nop_mflo_mfhi`) by the head before assignment.

## What it does

Resolved via `tools/classtable.py DREAMSYS_METHODS` at +0x1E8 (this round's
correction of the previous placeholder name, see the vtable's own comment).
`effect` (4..12) selects one of five behaviours through a dense
`switch`/jump-table dispatch; `entity` is an opaque object (almost certainly
`Entity*`, see `DreamSysEntityObj` below) whose own vtable is called through
for several of them.

```c
void DreamSys__InstanceEffectsOnJournal(DreamSys *this, void *entity, s32 effect)
{
	if (this->unknwon_int_0x44 != 0) {
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
```

Cases 5-8 are genuinely empty (the jump table's four middle entries all
target the shared epilogue directly) -- confirmed from the disassembly, not
assumed.

## Field identifications made along the way

All of these came from plain cumulative offset arithmetic through the
ALREADY-DOCUMENTED `DreamSys` struct (no guessing): `this->unk_0x68` is
`isFlashbackSession`; `this->unk_0x134` is `dreamTimeLimit`; `this->unk_0x164`
is `currentStage`; `this->unk_0x24` is `dreamTimer`; `this->unk_0x168`/
`this->unk_0x16A` are `nextCinematic.bank`/`nextCinematic.entry`; and
`this->unk_0x18C` is `instanceFlasbackUnlockScore` -- the last one a
particularly satisfying confirmation, since a journal-effects handler
incrementing exactly the "instance flashback unlock score" field is exactly
what the function's own name promises. None of these needed a header edit;
they just needed writing out the name that was already there instead of a
fresh `unk_` offset.

## New struct/vtable knowledge committed alongside this round

Both in `include/DreamSys.h`:

- **`DreamSysEntityMethods` (the existing minimal local view of `entity`,
  previously typing only its own `+0x10C`) extended with `+0x38`, `+0x14C`,
  `+0x150`, `+0x154`, `+0x158`.** `+0x38` takes the `DreamSys` instance as
  its own second argument (same shape as `DreamSysBaseMethods::slot0x50`);
  `+0x14C` returns a pointer forwarded straight into the already-typed
  `LogInstanceMood(DreamSys*, MoodGraphPoint*)`, so `MoodGraphPoint *`;
  `+0x150`/`+0x154` return plain `s32` (added to / negated into `DreamSys`
  fields); `+0x158`'s result is stored with a bare `sh`, consistent with
  either `s16` or `s32` at this call shape, kept `s32` for uniformity. Only
  one prior reader (`+0x10C`, DreamSys__ProcessChunkChange) existed before
  this round; the new slots are purely additive, no offset shifted.
- `vtable_DreamSys::LogInstanceMood` (+0x1F8) and `::FlashbackSaving`
  (+0x218) were already fully typed from earlier rounds and needed no
  changes -- both call sites here confirm those existing signatures rather
  than adding new ones. `::DynamicLink` (+0x1C4), likewise already typed
  `void (DreamSys *this)`, is called here as `this->vt->DynamicLink(this)`.

## Reading the score

**funcdiff's own in-range count (73504-ish of a much larger apparent range)
looks catastrophic and is not informative here** -- this is exactly the
`func_8004BB3C`-shaped trap `DECOMPILATION_LEARNINGS.md` already documents
("funcdiff's byte range is not a stalled function's true length"), for the
same underlying reason: my compiled function is ONE WORD SHORT of retail's
110, so the whole-image comparison range balloons to cover everything after
it in the entire executable, and the printed count is dominated by that
irrelevant tail. Read `tools/asm-differ/diff.py`'s output instead (which
aligns branch targets rather than raw bytes): **every single instruction in
the function matches retail except four words**, three of them a pure
register swap and the fourth a genuinely missing/misplaced instruction. That
is 106/110 words correct.

## The residue, precisely

**Residue 1 (3 words): the bounds-check/switch-index computation lands in
`$v1` in retail, `$a2` (the `effect` parameter's own register) in mine.**
Retail:
```
addiu v1,a2,-4
sltiu v0,v1,9
...
sll    v0,v1,0x2
```
Mine computes the identical three operations directly on `a2` in place
(never copying to `v1`), which is smaller and arguably better code, but not
retail's. Tried and reverted, all producing byte-identical output to the
plain `switch (effect)` form: `switch (effect - 4)` with case labels `0..8`
instead of `4..12`; a named `s32 idx = effect - 4;` local switched on
instead of the expression directly. GCC re-derives the exact same `a2`-only
codegen regardless of how the subtraction is spelled in C -- this looks like
an optimizer-level choice (which register the switch-lowering pass reuses)
rather than anything visible in the source shape.

**Residue 2 (net -1 word): case 4's vtable dereference is ordered
differently.** Retail:
```
move a0,s1          ; a0 = entity (set up BEFORE the dereference)
lw   v0,0(a0)        ; v0 = entity->methods, read THROUGH a0
nop
lw   v0,0x38(v0)
nop
jalr v0
 move a1,s0
```
-- two load-delay `nop`s, neither filled. Mine:
```
lw   v0,0(s1)         ; v0 = entity->methods, read through s1 directly
move a0,s1             ; the OTHER independent instruction, hoisted into
                         ; what would otherwise be the first nop
lw   v0,0x38(v0)
nop
jalr v0
 move a1,s0
```
Same total work, one fewer instruction, because I filled a delay slot
retail left empty. Tried and reverted: introducing a function-scope
`DreamSysEntityObj *obj` cast variable (this REGRESSED further -- an extra
persistent callee-saved register across the whole function, confirmed by
objdump showing 3 saved registers where retail uses 2, discussed and fixed
separately below); a case-local `DreamSysEntityMethods *methods` temp
(byte-identical to the current form); a bare `__asm__("")` immediately
before the cast-and-call statement (byte-identical -- optimized away with
zero effect, same as `func_8005A9CC`'s finding this round).

**A related but ALREADY-FIXED trap worth recording explicitly**: the very
first draft of this function used a function-scope
`DreamSysEntityObj *obj = (DreamSysEntityObj *)entity;` assigned once at
the top and reused by every case. That compiled to a THIRD callee-saved
register (`$s2`) alongside `$s0`(`this`)/`$s1`(`entity`), where retail only
ever needs two -- an 8-byte-larger frame that cascaded into 85287+ bytes of
whole-image drift. The fix was mechanical: drop the separate `obj` local
and cast `entity` inline at each of the five use sites instead (identical
value, and each use site is independent -- nothing needs `obj` to persist
across a call). This single change took the function from wildly wrong to
106/110 words correct in one step, and is the biggest lesson of this
report.

### Proposed learning

**A local variable that is just a same-size pointer CAST of an existing
parameter, if scoped to the WHOLE function and used across multiple calls,
can silently cost a callee-saved register the retail source never spent** --
confirmed here (an extra register that shifted 85+KB of the whole image)
and worth checking for specifically whenever a function takes a `void *`
parameter that gets reinterpreted through a local "view" type: prefer
casting inline at each use site over caching the cast in a persistent local,
and only promote it to a real local if removing it demonstrably changes
nothing (verified here too -- the case-local `DreamSysEntityMethods *`
temp made no difference at all, confirming the register cost is about
LIFETIME/SCOPE, not the mere existence of a named local).

---

## Head adjudication, round 22: classification CONFIRMED, one hypothesis tested and rejected

The head re-read both residues. **Charlie's classification stands** -- this is a
genuine allocation/scheduling residue, honestly scored and honestly bounded,
and the "read the asm-differ output, not funcdiff's raw count" caveat above is
correct and important. Recorded here so the next round does not re-derive it.

**Tested and REJECTED: retyping the parameter.** Charlie tried a function-scope
cast local (regressed -- extra callee-saved register) and a case-local temp
(byte-identical). It did not try the third form, which is the one a reader of
this report reaches for next: **give the parameter the view type directly and
delete every cast**, so no cast pseudo exists at all.

```c
void DreamSys__InstanceEffectsOnJournal(DreamSys *this, DreamSysEntityObj *entity, s32 effect)
/* ... and `entity->methods->slotNN(entity, ...)` at all five sites */
```

Built green with the forward declaration updated to match; the residue is
**unchanged**, both halves of it. Still one word short, still 132780 bytes of
whole-image drift. Reverted.

**What the experiment did establish**, from `tools/asm-differ/diff.py`: the
`move a0, s1` / `lw v0, 0(a0)` ordering is specific to **case 4 alone**. Every
other case in retail reads `lw v0, 0(s1)` and sets `a0` afterwards -- exactly
the shape this C already produces:

```
  retail case 4      move a0,s1 ; lw v0,0(a0) ; nop ; lw v0,0x38(v0)
  retail case 9      lw v0,0(s1) ; nop ; lw v0,0x14c(v0)   <- matches ours
```

So it is not a whole-function property of how `entity` is typed or cast --
which is what all three cast experiments were implicitly testing, and why all
three came back negative. Case 4 is also the only call site here that passes a
SECOND argument (`this`, into `$a1`, in the `jalr`'s delay slot). That
correlation is the remaining untested axis and the natural next lead: the
difference is between a one-argument and a two-argument method call, not
between a cast and a non-cast object expression.

**Do not spend another round on the cast/typing axis.** Three forms are now
tested (function-scope local, case-local, parameter retype) and all three are
byte-identical or worse.

Residue 1 (`v1` vs `a2` for the switch index) is likewise unmoved by the
retype, consistent with charlie's finding that the switch-lowering register
choice is invisible to source spelling.
