# func_8005A9CC -- STALL: exact length (88/88 instructions, zero address drift), 57/88 raw word-match, first real diff at 0x4B1FC (retail's delay-slot `nop` after `beqz v0,.L8005AA44` vs a hoisted `addiu a0,s0,0x16c` -- `fill_eager_delay_slots` branch-target duplication, one instruction early)

**Unit:** DreamSys · **Size:** 88 words · **Status:** STALL, best-reached
57/88, no address drift. Attempted round 2026-09-06 (charlie). Screened
clean against both remaining blockers (`gp_rel`, `nop_mflo_mfhi`) by the
head before assignment. **Title rebuilt round 37 (2026-09-12, charlie)** to
carry the three required figures -- this report previously had none in its
title line; re-measured fresh, byte-identical to what follows (see "Round 37"
below).

## What it does

Called through `vtable_DreamSys::func_8005A9CC` (+0x1DC) by `func_80059E98`
(matched this round, see its own report) as the first of three "link test"
tries. Signature `bool (DreamSys *this, PlayerSpawnPoint *currentPos)`,
matching its siblings `func_8005A700`/`func_8005A7A0`.

```c
bool func_8005A9CC(DreamSys *this, PlayerSpawnPoint *currentPos)
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
	this->vt->func_8005A1A4(this);
	return false;

staircase:
	result = Test4StaircaseNodes(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0) {
		return false;
	}
	func_8001E6F8(this, local);
	if (!func_8005C02C(&this->unk_0x888, &this->unk_0x884, local)) {
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
	this->unk_0x910 = D_80087EEC[func_8005C118()];
	this->vt->func_8001CEB4(this, 1, (void *)this->unk_0x884);
	this->unk_0x910(this);
	return false;
}
```

**Every path returns `false`** -- confirmed against the disassembly, not
assumed: every `j`/fallthrough in the function lands on either an explicit
`li v0,0` or the shared epilogue label that itself does `li v0,0`. There is
no path that reaches the epilogue with a nonzero `$v0`. This makes the
function's practical behavior in its only caller (`func_80059E98`'s
`!this->vt->func_8005A9CC(...) && ...` chain) equivalent to always
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
- **`extern s32 (*D_80087EEC[4])(DreamSys *this)`** -- a table of the four
  already-matched `s32 (DreamSys *this)` functions `func_8005AB2C`/
  `func_8005AC24`/`func_8005AD68`/`func_8005AE40`, confirmed by their own
  existing definitions in `src/DreamSys.c`.
- **`extern s32 Test4StaircaseNodes(...)`** and **`extern s32
  func_8005C02C(...)`** forward/call-site prototypes added near the
  existing `func_8005BD3C` one (same 3-arg shape; `func_8005C02C` is
  blocked by the same gp-relative+addiu_at pair as `func_8005BD3C`, per its
  own existing stub report). `Test4StaircaseNodes` is defined later in this
  same unit's ROM order, so its prototype here is a plain forward
  declaration, not a cross-unit one.
- **`extern s32 func_8005C118(void)`** -- corrected from a guessed
  `(DreamSys *this)` signature: the disassembly's call site leaves `$a0`
  holding an unrelated leftover value (`currentPos->position.z`, from the
  immediately preceding `lh`) with no explicit argument setup, matching the
  existing "empty delay slot, no a0-a3 setup" shape already documented for
  `func_8005BF48`.

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
  register-forcing idiom that closed `func_80059D1C` this same round (there,
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
`func_8005A82C`'s residue this same round. The lever has nothing to attach
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
and, in spirit, `func_8005A82C`'s attempt-5 `pLocal`) confirms: hoisting a
pointer/address value that is naturally re-derivable at each of its use
sites into a function-scope local, even when the value itself is
loop-invariant and "obviously" the same everywhere, tends to cost an extra
callee-saved register rather than help scheduling -- because the register
now has to survive across every intervening branch and call, not just the
one call it was meant to feed. This is the same lesson as the
`DreamSys__InstanceEffectsOnJournal` cast-local finding, generalized past
"cast of a `void *` parameter" to "any repeatedly-recomputed address
expression."
