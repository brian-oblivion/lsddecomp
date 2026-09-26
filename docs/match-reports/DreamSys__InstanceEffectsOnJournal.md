# DreamSys__InstanceEffectsOnJournal -- MATCHED round 75 (110/110, whole image OK): case 4's `+0x38` method takes a THIRD argument, `effect` -- both residues (switch index in `$a2` vs `$v1`, case 4's missing `nop`) were that missing argument

REVISITED, round 75: MATCHED (alpha); names/types used (`DreamSysEntityMethods::slot0x38` prototype corrected to 3 arguments -- the only caller is this function, checked with `grep -rn slot0x38 src/ include/`)

## Round 75 (alpha): MATCHED

**Preserved body rebuilt first**, exactly as the `#if 0` block gave it:
109 words against 110, funcdiff `insertions 2 / deletions 2 (opcode-level;
positional skeleton diffs 93)` with 134342 bytes of out-of-range drift (the
2/2 is the one-word size shift folding the next function's first word into
the window; `asm-differ` shows the true picture: one deletion, one
reordering, three register replacements).

**The lever: a missing argument.** Retail's switch index goes into a FRESH
register (`addiu v1,a2,-4`) because `effect` is still needed after the
bounds check -- as the third argument of case 4's call, passed in `$a2`
where it already sits, so no instruction shows it. `-da` dumps of the old
body confirmed the mechanism from the other side: the index pseudo had a
copy preference for `$a2` (`106 preferences: 6` in `.greg`), which
`expand_preferences` only records when the source register DIES in the
index insn and does not conflict with it. Give `effect` a later use and it
conflicts, the preference is gone, and the index takes `$v1` (`$v0` is held
by the `sltiu` result). Spelled:

```c
	case 4:
		((DreamSysEntityObj *)entity)->methods->slot0x38(entity, this, effect);
		break;
```

with the header's `slot0x38` retyped `(void *self, struct DreamSys *arg1,
s32 effect)`. **110/110 on the first build, `OK: build matches retail`;
`tools/check-nonmatching.sh` green.** Case 4's `move a0,s1; lw v0,0(a0);
nop` shape (the missing word) came back with it -- no separate lever.

The preserved body otherwise stands unchanged (early `return;` guard,
`case 5..8: break;`); a nested-`if` restructure of the same body, tried
first, also matches with the argument and is byte-identical, so the
argument is the whole lever.

What did NOT move either residue this round, all measured on the nested
body before the argument was found (each `insertions 2 / deletions 2`,
unchanged): `switch (effect - 4)` with cases 0..8; `effect` typed `u32`
(same), `s16` (adds `sll`/`sra`), `u16` (adds `andi`) -- the last two DO put
the index in `$v1`, which is what pointed at "`effect`'s lifetime, not its
type"; case-4 local `DreamSysEntityObj *e = entity`; a local function
pointer; `(*fp)(...)` spelling; a cast to an `s32`-returning slot; a
function-top `obj` used only in case 4 (entry `move a0,a1`, worse); a
case-4 `obj` that is also used in case 11 (fixes case 4, moves the same
shape into case 11 -- confirmed the `$a0`-canonical mechanism but is not
the source). One bounded permuter search was started on the nested body
(Gate 3: scaffold `--stack-diffs` 0 ins / 1 del / 1 reorder; real build
2/2 from the size shift; objdumps of scaffold and in-tree object identical
modulo relocations, so AGREE) and stopped at ~3736 iterations, best score
unchanged from the base 180, when the argument lever matched.

Every prior round (22, 35, 37, 39, 49) called both residues
"compiler-level, invisible to source spelling" because every lever tried
was a spelling of the SAME computation. A register chosen for a temporary
because another value is still live is decided by what is live -- and
here the live value was an argument nobody had written.

### Proposed learning (round 75)

**A switch index computed into a fresh register instead of in place on the
parameter (`addiu v1,a2,-4` where you get `addiu a2,a2,-4`) means the
parameter is still live after the bounds check.** GCC 2.6.3's global
allocator ties the index to the parameter's register only when the
parameter dies in the index insn. If no later instruction visibly reads
the parameter, look for a call that passes it in the SAME argument
register it arrived in: that use costs zero instructions and is invisible
in the asm. Check every call in the function for an argument register that
is never set before the call. (Same family as charlie's round-75 `slot4C`
arity finding: a register that "should" be free is held by an argument the
prototype is missing.)

---

## Earlier history

> **ROUND 49 (2026-09-16, runner bravo): re-verified fresh, one new axis
> tried on residue 2, clean negative.** This unit had not been touched
> since round 39. Spliced the exact preserved body back in and rebuilt:
> byte-identical **109/110 built, 106/110 truly correct via `asm-differ`**,
> same two residues (switch-index register choice at case dispatch; case
> 4's vtable-dereference delay-slot-fill ordering) as every prior round.
>
> **New axis tried**, on residue 2 (case 4's `move a0,s1` vs. `lw v0,0(s1)`
> ordering): wrapped case 9's `if (this->isFlashbackSession != 0) { return;
> }` early return in `do {...} while(0)` -- round 47's lever, untried on
> this function, and tried here specifically to see whether it perturbs the
> case-4 delay-slot choice the way it perturbed an unrelated struct copy in
> `DreamSys__TryStaircaseLink` this same round. **Byte-identical to the kept 106/110
> body -- no effect anywhere**, confirmed via `asm-differ` across the whole
> function, not just the two named residue sites. Reverted immediately;
> `INCLUDE_ASM` restored, whole-image SHA1 verified green.
>
> This is a clean negative and, combined with `DreamSys__TryInstantTeleportLink`'s two clean
> negatives and `DreamSys__TryStaircaseLink`'s one destructive positive-that-regresses
> (all three this round), completes a small but instructive matrix: the
> do-while lever's effect (none / cascading-elsewhere) does not correlate
> with residue class, function size, or which statement is wrapped -- it is
> a per-function, per-position fact that must be measured, not inferred.
>
> No further attempts this round; the four-forms-tested cast/typing axis
> (rounds 22, 35, 39) and the three-forms-tested switch-index axis (rounds
> 2026-09-06, 39) both remain closed. Not re-attempted.
>
> ### Proposed learning (round 49)
>
> A `do{...}while(0)` wrap around one early-return case in a `switch` can
> leave a COMPLETELY UNRELATED case's delay-slot-fill choice (case 4, three
> cases away) untouched, even in the same function -- unlike
> `DreamSys__TryStaircaseLink` this round, where the exact same lever, applied to a
> `goto` rather than a `return`, corrupted an unrelated struct-copy dozens
> of instructions later. The two functions share a unit and a residue class
> but not the lever's blast radius; see `DreamSys__TryStaircaseLink.md`'s round-49 entry
> for the contrasting case.

**Unit:** DreamSys · **Size:** 110 words · **Status:** STALL, 106/110 words
truly correct (see "Reading the score" below for why `funcdiff.py`'s own raw
number reads far lower; there IS one real word missing, hence the address
drift funcdiff also reports). Attempted round 2026-09-06 (charlie). Screened
clean against both remaining blockers (`gp_rel`, `nop_mflo_mfhi`) by the head
before assignment. **Title rebuilt round 37 (2026-09-12, charlie)** to carry
the three required figures -- this report previously had none in its title
line; re-measured fresh, byte-identical to what follows (see "Round 37"
below).

## What it does

Resolved via `tools/classtable.py gDreamSysMethods` at +0x1E8 (this round's
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
`StageMap__ApplyRateEntries`-shaped trap `DECOMPILATION_LEARNINGS.md` already documents
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
zero effect, same as `DreamSys__TryStaircaseLink`'s finding this round).

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

---

## Round 35 (2026-09-12, runner charlie): re-verified fresh, one new axis tried, still rejected

Re-measured before touching anything: rebuilt the exact preserved body and
reproduced the same residue. **Note for the next reader**: at this
function's actual length (one word short of retail's 110), `funcdiff.py`'s
own raw in-range count reads as low as 15/110 with a "differs OUTSIDE this
range too" drift warning -- this is NOT a regression, it is the exact
`StageMap__ApplyRateEntries`-shaped trap this report already documents. Reading
`tools/asm-differ/diff.py`'s realigned output instead confirms **106/110
words correct, same two residues, unchanged.**

**One new axis tried**, prompted by round 22's head adjudication naming the
untested lead (a case-4-scoped, rather than function-scoped or
whole-parameter, cast local):

```c
case 4: {
    DreamSysEntityObj *obj = (DreamSysEntityObj *)entity;
    obj->methods->slot0x38(obj, this);
    break;
}
```

**Regressed hard** -- same failure signature as the original function-scope
cast local charlie tried in round 2026-09-06 (an extra callee-saved register
persists across the call), even though this cast is scoped to a single
`case` block rather than the whole function. Confirms the register cost is
about the local's *live range crossing the call*, not its lexical scope --
scoping the declaration more tightly does not help when the value still has
to survive from its assignment to its one use across an indirect call.
Reverted immediately.

**All four plausible forms of the cast/typing axis are now tested and
rejected**: function-scope local (regressed), case-local *methods* pointer
temp (byte-identical, round 2026-09-06), whole-parameter retype (unchanged,
round 22), and case-local *entity-object* temp (regressed, this round). Do
not spend a further round on this axis; the residue is a genuine
scheduler/allocator choice tied to the two-argument (`entity`, `this`) call
shape at case 4 specifically (every other, single-argument call site in
this function already matches retail exactly), not to how `entity` is
spelled or scoped in C.

No further attempts made. `INCLUDE_ASM` restored, whole-image SHA1 verified
green, `git diff --stat` empty against `main`.

### Proposed learning (round 35)

When a report finds that a function-scope temporary costs an extra
callee-saved register, narrowing the temporary's *lexical scope* (to one
`switch` case, say) is a natural next thing to try -- and this function is a
clean confirmation that it does not help when the *live range* is unchanged
(assignment, then a single use across an intervening call). Scope and
live-range are different axes; only the second one drives this register-cost
class. Worth stating plainly since "scope it more tightly" is the obvious
next lever a reader reaches for after seeing "function-scope local costs a
register", and it is a dead end here.

---

## Round 37 (2026-09-12, runner charlie): title rebuilt, permuter search launched

This report predated round 23's three-figure title rule and carried no
length/word-match/first-diff figures in its title line at all -- round 37's
brief named it explicitly as one of two functions in this unit needing that
repair, since `tools/progress.py`/the next round's staffing reads the title,
not the body.

Re-measured before touching anything: rebuilt the exact preserved body above
and reproduced the identical residue --  `./build-and-verify.sh` gives
`build exit=2` with zero compile-error grep hits (a fresh, non-matching
build, exactly as expected), and `tools/asm-differ/diff.py` confirms:

- **Length: one word short.** This build assembles to 109 instructions
  (0x4B83C-0x4B9F0 realigned) against retail's 110 (0x4B83C-0x4B9F4); the
  `j 5b1dc` branch target itself is one word earlier throughout the realigned
  stream (`5b1d8` vs retail's `5b1dc`), which is the direct symptom of the
  missing word, not a separate issue.
- **Raw word-match**: `funcdiff.py`'s own in-range count reads 15/110 with an
  explicit "differs OUTSIDE this range too (134342 bytes)" warning --
  confirmed this is the `StageMap__ApplyRateEntries`-shaped trap the report already
  documents, not a regression. Reading `asm-differ`'s realigned output
  instead: **106/110 words truly correct**, same two residues as every
  earlier round (switch-index register choice, case-4 vtable-dereference
  delay-slot fill), unchanged.
- **First real diff, read off `asm-differ`**: **file offset 0x4B860**
  (`vram 0x8005B060`) -- retail's `addiu v1,a2,-4` (copying the bounds-check
  input into `$v1` before using it) against this build's `addiu a2,a2,-4`
  (same computation, done in place on `$a2`, the parameter's own register).
  This is Residue 1 from the existing analysis above; nothing new to add to
  its cause, which was already exhaustively covered (declaration form is
  invisible to this register choice).

No new C axis was tried this round -- the existing report already closes out
four forms of the cast/typing axis (round 22, round 35) and this round's
brief prioritizes a fresh permuter search over re-deriving hand analysis on a
function already argued this thoroughly. A permuter scaffold was set up
(`permuter-work/DreamSys__InstanceEffectsOnJournal`) from this exact
preserved body; see the runner's final summary for the search's outcome
(iteration count and `rc`), since the project's parallel-run discipline
requires exactly one search running at a time and this function's search was
queued behind `DreamSys__StartVoice`'s and `DreamSys__TryStaircaseLink`'s.

`INCLUDE_ASM` restored immediately after measuring; whole-image SHA1 verified
green; `git diff --stat` empty against `main` for this unit's non-report
files at the point of this write-up.

### Proposed learning (round 37)

A report can carry a fully worked-out residue analysis in its body while
still having no title figures at all, if it predates the three-figure title
rule -- `tools/progress.py` and next-round staffing read titles, so an
old-format title with a well-documented body is invisible to automated
ranking in exactly the same way an undocumented stall is. Worth a one-time
sweep of the whole `docs/match-reports/` corpus for titles missing the three
figures, independent of this unit -- this round only touched the two the
brief already named, but the trap is generic to any report older than round
23.

---

## Round 37 continued: permuter search run, one candidate examined and rejected as a real regression

**Search** (`-j 6 --stop-on-zero --best-only --stack-diffs`, `timeout 900`):
ran 92452 iterations. No zero was reached. (`rc` could not be captured this
run -- the `echo "permuter rc=$?" | tee -a` tail never landed in the log
despite the background task reporting a clean exit; the iteration count and
absence of an `output-0-*` directory are otherwise fully confirmed, and the
total iteration count is consistent with the same ~900s bound as the other
three searches this round, so this is read as `rc=124` by inference, not
measurement -- flagged rather than asserted.)

`--best-only` saved two local-best candidates (scores 80 and 120, both
better than the base 180). **Both were examined; both are false leads, for
two different reasons:**

- **Score 120** relies on undefined behavior in the same shape
  `DreamSys__TryStaircaseLink.md` already found this round: it declares `new_var = entity`
  inside `case 7:`'s fallthrough into `case 8:`, then reads `new_var` in
  cases 9-12 -- separate switch targets reached via the jump table, NOT via
  fallthrough from 7/8, so `new_var` is uninitialized on every path that
  actually reaches those cases. Rejected on inspection, not tested.
- **Score 80** targets exactly Residue 2 (case 4's vtable-dereference
  ordering) with a permuter-native form of the same lever that closed
  `DreamSys__StartVoice` this round: `new_var = entity;` unconditionally before the
  switch, then `slot0x38(new_var, this)` instead of `slot0x38(entity, this)`
  at case 4. **Translated and run through the full oracle -- and this is the
  interesting negative result.** `funcdiff.py`'s outside-range drift
  collapsed from 134342 bytes to **1 byte** (the function is now the full
  110 words long, closing the length gap entirely), but the realigned
  `asm-differ` view shows this did NOT reduce the true residue count. GCC
  hoisted the now-unconditional `new_var = entity` assignment into the empty
  delay slot after the earlier `beqz` (the switch bounds check) rather than
  leaving it where case 4's block needs it -- a real instruction retail does
  not have, immediately followed by the ORIGINAL orphan `move a0,s1` at
  0x4b88c now being gone (netting the same total instruction count) but at
  the cost of a NEW register mismatch at 0x4b890 (`lw v0,0(a0)` vs
  `lw v0,0(s1)`). Counting every diff marker in the realigned output: this
  candidate has residue 1 (3 words, unchanged) PLUS one insertion, one
  deletion, and one new register mismatch at the case-4 boundary -- net
  **worse** than the established 106/110, even though `funcdiff`'s raw
  in-range count (98/110) and near-zero outside-range drift both look like
  improvements at a glance. Reverted; `INCLUDE_ASM` restored; whole-image
  SHA1 verified green.

**Status: still STALL, 106/110 (via asm-differ), residue unchanged.**

### Proposed learning (round 37, continued)

**Eliminating a function's LENGTH mismatch (the address-drift trap this
report itself is built around) is not the same axis as eliminating its
WORD-count residue, and a permuter candidate can improve one while making
the other worse.** This candidate looked like a clear win by two different
proxies at once -- `funcdiff`'s drift warning nearly vanished, and its raw
in-range count went from a misleadingly-low 15/110 to a much healthier-
looking 98/110 -- and was still a net regression once counted against the
real oracle via `asm-differ`'s realignment. The lesson from the existing
"four ways a score lies" list generalizes one step further: a permuter
candidate can look better on BOTH of funcdiff's own signals (drift AND raw
count) simultaneously while being worse on the metric that actually matters
(total instructions that differ from retail). `asm-differ`'s realigned
count, not `funcdiff`'s raw window, remains the only trustworthy read for
any function with a length mismatch -- true before this round's permuter
search and still true of its output.

---

## Round 39 (2026-09-14, runner echo): two more forms tried on both established axes, both inert; hoist-lever precondition doesn't hold

Re-verified fresh (spliced the preserved body back in): confirmed
byte-identical **109/110 built, 106/110 truly correct via asm-differ**, same
two residues as every prior round (switch-index register choice at case
dispatch; case 4's vtable-dereference delay-slot-fill ordering). Checked
this round's hoist-both-before-either precondition: neither residue is an
adjacent-load/mult pair with a later consumer -- residue 1 is a single
`addiu`-derived bounds check invisible to source spelling, residue 2 is a
delay-slot-fill choice between an independent register move and a load,
the same shape as `DreamSys__TryInstantTeleportLink`/`DreamSys__TryStaircaseLink`'s residues this round.
The lever does not apply to either.

Two new forms tried on the already-documented axes, both rebuilt and
measured, both byte-identical to the kept 106/110 body (no change to
either residue):

1. **Residue 2 (case 4):** an UNCAST alias local, `void *e = entity;` then
   `((DreamSysEntityObj *)e)->methods->slot0x38(e, this);` -- a fifth form
   on the "cast/typing axis" round 22/35 already closed with four other
   forms (function-scope cast, case-local methods pointer, whole-parameter
   retype, case-local entity-object temp). Byte-identical; the axis stays
   closed.
2. **Residue 1 (switch index):** `s32 idx; idx = effect; switch (idx) {...}`
   -- a third form beyond the two already tried (`switch (effect - 4)` with
   shifted case labels; a named `s32 idx = effect - 4;`). Byte-identical;
   GCC re-derives the identical `a2`-only codegen regardless.

**Status: still STALL, 106/110 (via asm-differ), both residues unchanged.**
`INCLUDE_ASM` restored; whole-image SHA1 verified green; both experiments
reverted immediately after measuring.

### Proposed learning (round 39)

Both of this function's residues, and both of `DreamSys__TryInstantTeleportLink`'s/
`DreamSys__TryStaircaseLink`'s this round, share a common shape worth naming explicitly:
**a delay-slot-fill or register-class choice between two ALREADY-INDEPENDENT,
already-schedulable instructions, with no data dependency chain for a
source-level hoist to shorten.** The hoist-both-before-either lever targets
a different, specific shape (two values computed via load/mult, one
consumed early and one late, that the natural C fails to make simultaneously
live) and simply has nothing to attach to on this class of residue. This
unit contributed three of round 39's clearest confirmations that the lever's
precondition is a real filter, not just a label to check after the fact.

## Comment moved from src/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* MATCHED round 75 (alpha): case 4's +0x38 method takes `effect` as a third
   argument. Forwarding it keeps `effect` live in $a2 past the switch, so the
   bounds-check index gets its own register ($v1) instead of being computed
   in place on $a2 -- the "switch-index register" and "case-4 delay slot"
   residues were both this missing argument. */
```
