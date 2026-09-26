# DreamSys__TryStaircaseLink -- MATCHED round 75 (88/88, whole image OK): nested `if` chain instead of early-return guards (no CODE_LABEL between the entry `move s0,a0` and the first call, so jump2 deletes the redundant `move a0,s0`), plus ONE whole-`PlayerSpawnPoint` copy

REVISITED, round 75: MATCHED (alpha); names/types used (existing field names; one local `PlayerSpawnPoint *` cast, no header edit)

## Round 75 (alpha): MATCHED

**Preserved body rebuilt first**, exactly as the `#if 0` block gave it:
`57/88`, `insertions 3 / deletions 3 (opcode-level; positional skeleton
diffs 31)`, zero drift -- same as every prior round.

**Two independent residues, both source shape.**

1. **The struct copy.** Retail's `lwl/lwr v0; lwl/lwr v1; lh a0; swl/swr v0;
   swl/swr v1; sh a0` is load-all-then-store-all of TEN bytes: one
   whole-struct copy of a `PlayerSpawnPoint` (alignment 2), not the
   `PlayerSpawnGridPos` + `RelativePos` pair the old body wrote (which
   interleaved: load/store the first word, then load the rest, and needed a
   load-delay `nop` -- that `nop` is what kept the old body at exactly 88
   words and hid residue 2's real size). Spelled
   `*(PlayerSpawnPoint *)&this->staircaseGridPos = *currentPos;`. With that
   alone the function went to 87 words: retail really has one MORE
   instruction than the hoisting build.
2. **The delay slot.** Diagnostic that decided it: casting the first call to
   take no argument (`((s32 (*)(void))this->staircaseTickFn)()`) removed the
   `move a0,s0` from the `jalr` slot but still hoisted the `addiu` -- so
   retail's `$a0` is LIVE at that call (it is passed) and yet never set:
   the move was deleted. `-da` dumps showed who deletes it: in the working
   shape the `(set a0 s0)` is present in `.greg` and gone in `.jump2` --
   jump.c's "no-op move resulting from fortuitous allocation" path, which
   asks `find_equiv_reg` whether `$a0` already equals `$s0`. That backward
   search stops at a CODE_LABEL. `if (x != 0) return false;` expands to
   `if (x == 0) goto L; v0 = 0; goto ret; L:` -- a label between the entry
   `move s0,a0` and the call. The nested form (`if (pendingLinkType == 0) {
   if (tickFn != 0) { if (tickFn(this)) {...} } else if (...) {...} }
   return false;`) has no label on that path, the move is deleted, `$a0`
   stays live on the fall-through, reorg cannot steal the staircase arm's
   `addiu a0,s0,0x16c`, both slots stay `nop`. Found on a 12-line
   reproducer first (flat: `move a0,s0` in both slots; nested: two `nop`s),
   then applied: **88/88 on the first build, `OK: build matches retail`.**

Every prior round's verdict ("nothing in the C source controls this",
"below the C source's visibility") was wrong for the same reason: all of
them kept the early-return guard structure and varied only things inside
it (`if/else` vs `goto` spell the same CFG, and both leave the label).

Header note, not acted on (shared header, comment-only): the
`PlayerSpawnGridPos` comment in `include/DreamSys.h` says retail copies
the two halves separately; it copies them as one `PlayerSpawnPoint`. A
union of `PlayerSpawnPoint staircaseSpawn` over `staircaseGridPos` /
`staircaseOrigin` would remove the cast; the `DreamSys__TickStaircaseCase*`
readers of `staircaseOrigin` would need checking.

```c
bool DreamSys__TryStaircaseLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 local[4];

	if (this->pendingLinkType == 0) {
		if (this->staircaseTickFn != 0) {
			if (this->staircaseTickFn(this)) {
				this->staircaseActive = 0;
				this->staircaseTickFn = 0;
				this->staircaseMoveGate = 0;
				if (this->moveMode == 4) {
					this->vt->DreamSys__RestorePreviousMoveMode(this);
				}
			}
		} else if (Test4StaircaseNodes(&this->linkCoordinates, currentPos, this->currentStage) >= 0) {
			SceneNode__GetRotationDegrees(this, local);
			if (DreamSys__CheckStaircaseHeading(&this->exitRotation, &this->enterRotation, local)
			    && this->moveCommandLatch != 0) {
				*(PlayerSpawnPoint *)&this->staircaseGridPos = *currentPos;
				this->staircaseActive = 1;
				this->staircaseMoveGate = 1;
				this->staircaseFrame = 0;
				this->staircaseTickFn = STAIRCASE_TICK_FNS[GetLastSpawnExtra()];
				this->vt->SceneNode__UpdateRotation(this, 1, (void *)this->enterRotation);
				this->staircaseTickFn(this);
			}
		}
	}
	return false;
}
```

### Proposed learning (round 75)

**A missing argument move (`$a0` passed straight through from entry, no
`move a0,sN` before the call) means there is no CODE_LABEL between the
entry copy and the call.** jump2 deletes `(set a0 s0)` when
`find_equiv_reg` finds the entry `(set s0 a0)` scanning backward, and that
scan stops at a label. Every `if (cond) return X;` guard leaves one; a
nested `if (!cond) { ... }` does not. Tell: a call whose argument register
is set nowhere after the prologue while the same value sits in an
`$s` register -- and, downstream, a delay slot that retail leaves `nop`
because that argument register is live on the fall-through. Diagnostic
that separates it from a scheduling choice: make the call argument-less
with a cast; if the hoist survives but the move vanishes, the argument is
live in retail and a label is what you have to remove.

**A whole-struct copy of N bytes is load-all-then-store-all; two adjacent
member copies interleave.** If retail loads every word before storing any,
it is one struct assignment covering the whole range.

---

## Earlier history


> Renamed from `func_8005A9CC` on 2026-09-22 (tools/rename.py). Address 0x8005a9cc.

> **ROUND 49 (2026-09-16, runner bravo): re-verified fresh, one new axis
> tried and REJECTED as a regression -- and it teaches something about how
> fragile this function's downstream codegen is.** This unit had not been
> touched since round 39. Spliced the exact preserved body back in and
> rebuilt: byte-identical **57/88, zero address drift**, same single
> residue (`fill_eager_delay_slots` duplicating `.L8005AA44`'s first
> instruction into the `beqz`'s delay slot) as every prior round.
>
> **New axis tried:** round 47's re-send lever, `do { goto staircase; }
> while(0);` wrapped around the existing `if (this->unk_0x910 == 0) { goto
> staircase; }` -- a form not previously tried here (the existing report
> only tried plain `if`/`else` vs. `goto`, never a `do`-`while` wrapper
> around the `goto` itself).
>
> **Result: this is NOT a no-op here, and it makes things WORSE in a
> completely different part of the function.** The wrapper adds a spurious
> leading instruction to the prologue region and, far more importantly,
> **restructures the ALREADY-MATCHED `PlayerSpawnGridPos`/`RelativePos`
> struct-copy sequence** (the `lwl`/`lwr`/`swl`/`swr` unaligned-copy idiom
> this report's own "residue, precisely" section already flags as "a
> genuine (if minor) register choice difference riding on top of the
> shift"): instead of retail's shape (load BOTH halves, then store both),
> the do-while variant makes GCC interleave load/store per half (load first
> half, store it immediately, THEN load the second half), which is a
> different -- and wrong -- schedule, confirmed via `asm-differ`:
>
> ```
> retail/kept-body:  lwl v0,3(s1); lwr v0,0(s1); lwl v1,7(s1); lwr v1,4(s1); lh a0,8(s1); swl v0,...; swr v0,...; swl v1,...; swr v1,...; sh a0,...
> do-while variant:  lwl v0,3(s1); lwr v0,0(s1); nop; swl v0,...; swr v0,...; lwl v0,7(s1); lwr v0,4(s1); lh v1,8(s1); swl v0,...; swr v0,...; sh v1,...
> ```
>
> **Tested anyway through the full oracle, per project discipline to verify
> rather than assume from the diff shape**: `funcdiff.py`/`asm-differ` both
> confirm this is strictly worse (more diverging words, the same
> fill_eager_delay_slots residue at the top UNCHANGED, plus a brand-new
> register-churn residue in the struct copy that was previously
> byte-correct except for the one already-documented `v1`-vs-`v0` riding
> effect). Reverted immediately; `INCLUDE_ASM` restored, whole-image SHA1
> verified green.
>
> **Why this is worth recording rather than discarding as "just another
> negative": it demonstrates the do-while lever perturbs GLOBAL scheduling
> state, not a local block**, on a function where a sibling function in the
> SAME unit (`DreamSys__TryInstantTeleportLink`, this same round) showed the exact opposite --
> two do-while wraps at two different sites, both **completely inert**,
> zero observable effect anywhere. The two functions have superficially
> identical residue classes (a delay-slot-fill choice at exact length) and
> the SAME lever produces opposite behavior: total inertness on one,
> function-wide cascading disruption on the other. Nothing about either
> function's `.s` predicts which outcome you get before you try it.
>
> ### Proposed learning (round 49)
>
> **A `do{...}while(0)` wrapper's blast radius cannot be predicted from the
> residue's local shape alone -- it must be measured per function, even
> among functions in the same unit with the same residue CLASS.** Round
> 47's finding that the lever "changes basic-block shape enough to alter
> register allocation elsewhere in the function" is confirmed here in its
> most disruptive form (corrupting an already-matched, unrelated struct-copy
> sequence dozens of instructions downstream), while the sibling function
> `DreamSys__TryInstantTeleportLink` in the identical unit, tested the same round, showed zero
> effect from the same lever at two different sites. Treat every do-while
> trial as its own experiment requiring full-oracle verification, never as
> a lever whose safety a sibling's result can vouch for.

**Unit:** DreamSys · **Size:** 88 words · **Status:** STALL, best-reached
57/88, no address drift. Attempted round 2026-09-06 (charlie). Screened
clean against both remaining blockers (`gp_rel`, `nop_mflo_mfhi`) by the
head before assignment. **Title rebuilt round 37 (2026-09-12, charlie)** to
carry the three required figures -- this report previously had none in its
title line; re-measured fresh, byte-identical to what follows (see "Round 37"
below).

## What it does

Called through `vtable_DreamSys::DreamSys__TryStaircaseLink` (+0x1DC) by `DreamSys__ApplyMoveCommand`
(matched this round, see its own report) as the first of three "link test"
tries. Signature `bool (DreamSys *this, PlayerSpawnPoint *currentPos)`,
matching its siblings `DreamSys__TryTunnelLink`/`DreamSys__TryStageTimerLink`.

```c
bool DreamSys__TryStaircaseLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;
	s32 local[4];

	if (this->unknwon_int_0x44 != 0) {
		return false;
	}

	if (this->unk_0x910 == 0) {
		goto staircase;
	}
	if (!this->unk_0x910(this)) {
		return false;
	}
	this->unk_0x908 = 0;
	this->unk_0x910 = 0;
	this->unk_0x90C = 0;
	if (this->unk_0xAC != 4) {
		return false;
	}
	this->vt->DreamSys__RestorePreviousMoveMode(this);
	return false;

staircase:
	result = Test4StaircaseNodes(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0) {
		return false;
	}
	SceneNode__GetRotationDegrees(this, local);
	if (!DreamSys__CheckStaircaseHeading(&this->unk_0x888, &this->unk_0x884, local)) {
		return false;
	}
	if (this->unk_0xA8 == 0) {
		return false;
	}

	this->unk_0x918 = *(PlayerSpawnGridPos *)currentPos;
	this->unk_0x91C = currentPos->position;
	this->unk_0x908 = 1;
	this->unk_0x90C = 1;
	this->unk_0x914 = 0;
	this->unk_0x910 = STAIRCASE_TICK_FNS[GetLastSpawnExtra()];
	this->vt->SceneNode__UpdateRotation(this, 1, (void *)this->unk_0x884);
	this->unk_0x910(this);
	return false;
}
```

**Every path returns `false`** -- confirmed against the disassembly, not
assumed: every `j`/fallthrough in the function lands on either an explicit
`li v0,0` or the shared epilogue label that itself does `li v0,0`. There is
no path that reaches the epilogue with a nonzero `$v0`. This makes the
function's practical behavior in its only caller (`DreamSys__ApplyMoveCommand`'s
`!this->vt->DreamSys__TryStaircaseLink(...) && ...` chain) equivalent to always
continuing to the next link test -- a real quirk of retail's own logic, not
a decompilation error.

## New struct/vtable knowledge committed alongside this round

All in `include/DreamSys.h`:

- **`DreamSys::unk_0x910` retyped `s32` -> `s32 (*)(struct DreamSys *this)`**
  (a function pointer, called through directly, `0` used as its "unset"
  sentinel -- both existing writers set it to literal `0`, safe). Note the
  `struct DreamSys *` spelling: `DreamSys *` cannot be used inside the
  `DreamSys` struct's own definition before its typedef completes (same
  reason `DreamSysBaseMethods::ctor` uses the tag form -- see that field's
  own comment).
- **`DreamSys::unknown_values_0x918[4]` retyped to a new `PlayerSpawnGridPos`
  struct** (`{struct MapChunk chunk; struct MapTile tile;}`, 4 bytes) so a
  single whole-struct assignment reproduces retail's unaligned
  `lwl`/`lwr`+`swl`/`swr` 4-byte copy of `currentPos`'s first two members --
  same idiom as the existing `unk_0x91C` (`struct RelativePos`) next to it,
  which already covers `currentPos->position`. No other reader of this
  field existed before this round.
- **`extern s32 (*STAIRCASE_TICK_FNS[4])(DreamSys *this)`** -- a table of the four
  already-matched `s32 (DreamSys *this)` functions `DreamSys__TickStaircaseCase0`/
  `DreamSys__TickStaircaseCase1`/`DreamSys__TickStaircaseCase2`/`DreamSys__TickStaircaseCase3`, confirmed by their own
  existing definitions in `src/DreamSys.c`.
- **`extern s32 Test4StaircaseNodes(...)`** and **`extern s32
  DreamSys__CheckStaircaseHeading(...)`** forward/call-site prototypes added near the
  existing `DreamSys__CheckTunnelHeading` one (same 3-arg shape; `DreamSys__CheckStaircaseHeading` is
  blocked by the same gp-relative+addiu_at pair as `DreamSys__CheckTunnelHeading`, per its
  own existing stub report). `Test4StaircaseNodes` is defined later in this
  same unit's ROM order, so its prototype here is a plain forward
  declaration, not a cross-unit one.
- **`extern s32 GetLastSpawnExtra(void)`** -- corrected from a guessed
  `(DreamSys *this)` signature: the disassembly's call site leaves `$a0`
  holding an unrelated leftover value (`currentPos->position.z`, from the
  immediately preceding `lh`) with no explicit argument setup, matching the
  existing "empty delay slot, no a0-a3 setup" shape already documented for
  `GetStageLinkAngle`.

## The residue, precisely

**Zero address drift; the entire body other than one spot is
instruction-for-instruction identical to retail**, including the exact
`lwl`/`lwr`/`swl`/`swr` shape for both the `PlayerSpawnGridPos` copy and the
`RelativePos` copy, and the entire "unk_0x910 call vs staircase" branch
structure and every literal constant. The one divergence:

Retail's `beqz $v0,.L8005AA44` (the `this->unk_0x910 == 0` test) has a
`nop` in its delay slot, and the subsequent `jalr $v0` (calling
`this->unk_0x910(this)`) ALSO has a `nop` in ITS delay slot -- i.e. no
`move $a0,$s0` before that call at all, because `$a0` still holds the
original `this` argument unclobbered since function entry (nothing between
the top of the function and this call touches `$a0`), so the call is
correct without an explicit re-move. My compiled version instead HOISTS
`.L8005AA44`'s first real instruction (`addiu $a0,$s0,0x16c`, computing
`&this->linkCoordinates` for the staircase branch) into the first `nop`
slot, which clobbers `$a0`, forcing a real `move $a0,$s0` into the second
`nop` slot to restore it before the `this->unk_0x910(this)` call. Both
versions have the same total instruction count in this stretch (2 real
instructions filling 2 delay slots either way), so nothing drifts, but the
`addiu` ends up ONE INSTRUCTION EARLIER than retail, shifting everything
between it and the next matching landmark by exactly one word. The
alignment recovers by itself once both streams reach the shared "return
false" epilogue, but not before the `PlayerSpawnGridPos`/`RelativePos` copy
section, which then shows a genuine (if minor) register choice difference
riding on top of the shift (`$v1` vs `$v0` holding the second 4-byte chunk
of the copy) that I did not get to isolate from the shift itself.

**What was tried, all rebuilt and measured:**

- `if`/`else` vs `goto`-to-a-later-label for the exact same branch (both
  spellings of the identical CFG) -- **byte-identical**, confirming the
  hoist is a scheduling choice independent of how the branch is spelled in
  C.
- A bare `__asm__("")` scheduling barrier at three positions: the very top
  of the function, immediately before the `this->unk_0x910 == 0` test, and
  as the first statement of the `staircase:` block. The first and third
  placements caused real drift (136065 bytes off a clean whole-image
  rebuild) -- the barrier is not free of side effects on a body this size,
  it suppresses other optimizations too, not just the one hoist. The
  second placement (immediately before the branch) produced **zero
  observable effect at all** -- the compiler optimized the empty asm
  statement away without changing scheduling, since nothing there
  conflicts with it. None of the three placements isolated the fix CLAUDE.md's
  test requires (removing it should change only ORDER, not identity) --
  here it either changed nothing or changed far more than intended.

**Reading it**: this is the CLAUDE.md/DECOMPILATION_LEARNINGS residue class
"A `nop` retail has and you do not... delay slot filling differs when the
source statement order differs" -- except here source statement order was
tried and does NOT move it, which is itself worth recording: not every
delay-slot-fill difference is reachable by reordering the C, and a
same-instruction-count reordering across a forward branch target is a
plausible case where it never will be, since the compiler's decision to
hoist appears to depend on something below the C source's visibility (which
optimization pass runs, or global scheduling state) rather than on
statement order.

### Proposed learning

Add a documented NEGATIVE for the `__asm__("")` scheduling-barrier lever:
placing it at a branch or label boundary to suppress a specific delay-slot
hoist is not reliable -- it can be silently optimized away with no effect
(if nothing local conflicts with it) or can suppress unrelated scheduling
elsewhere in the same function, producing drift far larger than the single
instruction it was aimed at. The lever's only confirmed-safe use so far
remains its original one: as the function's very first statement, fixing
PROLOGUE callee-save STORE ORDER specifically (see
`DECOMPILATION_LEARNINGS.md`'s existing entry) -- not general delay-slot
hoisting later in a function's body.

---

## Round 35 (2026-09-12, runner charlie): re-verified fresh, mechanism named precisely, one new axis tried and still negative

Re-measured before touching anything, per the standing discipline: rebuilt
the exact preserved body above and reproduced **57/88 words, zero address
drift**, byte-identical to what this report already describes.

**The residue's mechanism, read directly off `tools/asm-differ/diff.py`
rather than inferred:** this is GCC's classic `fill_eager_delay_slots`
transformation -- retail leaves the delay slot after `beqz v0,.L8005AA44`
(the `this->unk_0x910 == 0` test) as a genuine `nop`, and separately leaves
the delay slot after the following `jalr v0` (calling
`this->unk_0x910(this)`) as a genuine `nop` too. My build instead
**duplicates the branch target's first real instruction**
(`addiu a0,s0,0x16c`, i.e. `&this->linkCoordinates`) into the branch's own
delay slot, and re-targets the label one instruction later; on the
not-taken (fallthrough) path this duplication is harmless because the
following `jalr`'s own delay slot is then forced to hold a corrective
`move a0,s0` restoring `this` before the call executes -- and since a
`jalr`'s delay-slot instruction always completes before the call takes
effect, this is semantically safe either way. Same total instruction count
both ways (confirmed: zero drift), just one instruction earlier than
retail. Nothing in the C source controls this -- it is a scheduler-internal
decision about whether hoist-and-shift is profitable, made independently of
statement order, consistent with the existing report's `if`/`else`-vs-`goto`
experiment (byte-identical either spelling).

**One new axis tried this round:** a real (non-empty) `__asm__ __volatile__`
with a clobber, rather than the previously-tried bare `__asm__("")`, placed
at the same position (immediately after the `goto staircase` branch,
before the `this->unk_0x910(this)` call):

```c
if (this->unk_0x910 == 0) {
    goto staircase;
}
__asm__ __volatile__("" ::: "$4");
if (!this->unk_0x910(this)) {
```

**No effect whatsoever** -- byte-identical to the unmodified 57/88 build,
confirmed via `asm-differ`. GCC 2.6.3 discards a clobber-only asm statement
with no memory side effect entirely once it determines the clobbered
register is about to be redefined regardless, so it does not survive to
constrain the delay-slot scheduler. This is a genuinely new data point (the
existing report only tried the empty, clobber-less form at this position)
and it closes the same door from a different angle: neither an empty nor a
clobber-bearing inline-asm barrier at this position perturbs the
scheduler's decision.

**Not attempted, and not recommended:** pinning `$a0` via a real operand
constraint. That would be exactly the register-identity fix CLAUDE.md bans
here -- this residue is an instruction-*order* choice (same registers used
throughout, just one instruction moved one slot earlier), so the rule's own
test ("does removing it change which register holds a value, or only
order") says a scheduling barrier would be the licensed tool if any were --
and two different forms of that tool have now both come back inert.

No further attempts made. `INCLUDE_ASM` restored, whole-image SHA1 verified
green, `git diff --stat` empty against `main`.

### Proposed learning (round 35)

Naming the actual GCC transformation involved (`fill_eager_delay_slots`
duplicating a conditional branch's target's first instruction into its own
delay slot, then advancing the target label) is more useful to the next
attempt than describing the symptom ("addiu ends up one instruction
earlier") -- it tells you *which* GCC internals decision is in play, and
that a per-call-site `__asm__` barrier (empty or clobbering) has now been
tried twice at the one spot that could plausibly interrupt it, with the
same null result both times. Further attempts on this specific residue
should assume the lever space of "reorder statements" and "local asm
barriers" is exhausted and either escalate as a genuine toolchain-scheduling
question or move on.

---

## Round 37 (2026-09-12, runner charlie): first-ever permuter search, two false leads, residue still open

Re-measured before touching anything: rebuilt the exact preserved body above
and reproduced **57/88 words, zero address drift**, byte-identical to rounds
35 and earlier. This function was screened by the round's brief as
never-searched (per-function permuter shape: exact length, high word-match,
zero drift), so a scaffold was set up and sanity-checked (`--debug
--stack-diffs`, base score 470 -- 6 register-diff, 4 reordering, 1 insertion,
1 deletion penalty points, consistent with the delay-slot-hoist residue
already on record).

**Search** (`-j 6 --stop-on-zero --best-only --stack-diffs`, `timeout 900`):
ran 66479 iterations before the bound expired (`rc=124`, captured on the very
next command). No zero was reached. `--best-only` saved three local-best
candidates (scores 268, 310, 311, all better than the base 470); **all three
were tested through the full oracle and all three are false leads**:

- **Score 268** wrapped the whole first branch in `do { ... } while (0);` --
  semantically inert (the `goto staircase` still jumps out of the loop to
  the same label), but it changed the compiled STACK FRAME (a different
  register saved at a different prologue offset) and **regressed to 40/88
  with 135807 bytes of real whole-image drift**. This is exactly the
  "permuter score improvement that is really a wrong size" trap
  CLAUDE.md/DECOMPILATION_LEARNINGS.md warn about -- the permuter's own
  scorer does not see the frame-size change the way the real oracle does.
- **Score 310** is not even semantically valid: it moved the `goto
  staircase;` into the WRONG branch (the `unknwon_int_0x44 != 0` check,
  making the following `return false;` dead code) and left the SECOND
  branch's body empty, silently deleting the real `goto staircase` the
  function depends on. Not tested through the oracle -- rejected on
  inspection as a structurally invalid mutation, not a candidate at all.
- **Score 311** introduced a second pointer, `new_var = this;`, assigned
  only on the fallthrough path, then used `new_var` (not `this`) at every
  site from that point through the `staircase:` label onward -- including on
  the path that reaches `staircase:` via the EARLIER `goto`, which never
  executes `new_var = this;` at all. This reads `new_var` uninitialized on
  that path: a genuine undefined-behavior form, not the legitimate
  register-forcing idiom that closed `DreamSys__StartVoice` this same round (there,
  the duplicate local was assigned on EVERY path before use). Tested anyway
  out of thoroughness: **regressed to 30/88**, confirming it is not a useful
  lever even ignoring the UB.

Per this project's own rule ("a permuter zero is a lead, not an answer" --
and, a fortiori, a non-zero local-best is an even weaker one), all three were
translated and verified rather than trusted from the permuter's own score,
and none held up. `INCLUDE_ASM` restored; whole-image SHA1 verified green;
`git diff --stat` empty against `main` at the point of this write-up.

**Status: still STALL, 57/88, residue unchanged from round 35.** The
`fill_eager_delay_slots` branch-target-duplication residue named in round 35
remains open; this round's search did not find a form that avoids it.

### Proposed learning (round 37)

**A permuter local-best that is not zero deserves the SAME skepticism as a
zero, and arguably more**, since by definition it hasn't reached the target
and the permuter's scorer is measuring an already-imperfect proxy on top of
an already-imperfect candidate. This round found three "improvements" over
the base score and all three were either a real regression once measured
through the full oracle (frame-size change invisible to the permuter's own
metric) or not even a valid semantic transformation (a `goto` moved to the
wrong branch, silently changing control flow) or relied on reading an
uninitialized variable. None of this is visible from the permuter's score
number alone -- only from reading the actual diff and, for anything that
looks plausible, running it through `./build-and-verify.sh`.

## ROUND 39 (runner echo): hoist-lever precondition doesn't hold; one new reshape tried, regressed hard

Re-verified fresh (spliced the preserved body back in): confirmed
byte-identical **57/88, zero address drift**, same single residue
(`fill_eager_delay_slots` duplicating `.L8005AA44`'s first instruction into
the `beqz`'s delay slot) as every prior round. Checked the hoist-both
precondition: the residue is not a load/mult pair at all -- it is which of
two independent, already-computable instructions (`addiu $a0,$s0,0x16c` vs.
a genuine `nop`) fills a branch's delay slot, structurally identical to
`DreamSys__TryInstantTeleportLink`'s residue this same round. The lever has nothing to attach
to here either.

**One new reshape tried:** hoisting `&this->linkCoordinates` into a
function-top local (`PlayerSpawnPoint *coords = &this->linkCoordinates;`,
used at the `staircase:` call site instead of re-deriving it there) --
**regressed hard to 18/88 with 135807 bytes of whole-image drift**, an
extra callee-saved register persisting across the whole function (same
"function-scope pointer cast/hoist costs a register" class already
documented for `DreamSys__InstanceEffectsOnJournal` and, this round,
confirmed a second time here). Reverted immediately.

**Verdict unchanged: STALL at 57/88, residue unchanged.** No source
changes retained; `INCLUDE_ASM` restored, whole-image SHA1 verified green.

### Proposed learning (round 39)

A THIRD independent function this round (after `DreamSys__InstanceEffectsOnJournal`
and, in spirit, `DreamSys__TryInstantTeleportLink`'s attempt-5 `pLocal`) confirms: hoisting a
pointer/address value that is naturally re-derivable at each of its use
sites into a function-scope local, even when the value itself is
loop-invariant and "obviously" the same everywhere, tends to cost an extra
callee-saved register rather than help scheduling -- because the register
now has to survive across every intervening branch and call, not just the
one call it was meant to feed. This is the same lesson as the
`DreamSys__InstanceEffectsOnJournal` cast-local finding, generalized past
"cast of a `void *` parameter" to "any repeatedly-recomputed address
expression."

## Naming

- **Tier B.** STALL (still INCLUDE_ASM). Wraps Test4StaircaseNodes and DreamSys__CheckStaircaseHeading, and on success wires up the DreamSys__TickStaircaseCase0..3 dispatch via GetLastSpawnExtra, per the preserved #if 0 body; same family as DreamSys__TryTunnelLink.

## Comment moved from src/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
/* MATCHED round 75 (alpha): the body is one nested `if` chain, not a run of
   early `return false;` guards. Each early return leaves a CODE_LABEL after
   its jump, and a label between the entry `move s0,a0` and the first
   `staircaseTickFn(this)` call stops jump2's find_equiv_reg from seeing
   that $a0 still holds `this` -- so the redundant `move a0,s0` survives,
   $a0 goes dead on that path, and reorg steals the staircase arm's
   `addiu a0,s0,0x16c` into the `beqz` delay slot. Nested, the move is
   deleted and both slots stay `nop`, as retail. The 10-byte copy into
   staircaseGridPos/staircaseOrigin is ONE whole-PlayerSpawnPoint copy
   (load-all-then-store-all), hence the local cast. */
```
