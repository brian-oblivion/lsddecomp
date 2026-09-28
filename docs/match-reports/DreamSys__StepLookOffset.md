# DreamSys__StepLookOffset -- MATCHED, 53/53 words, round 32 (2026-09-12, runner alpha2)

> Renamed from `func_80059814` on 2026-09-22 (tools/rename.py). Address 0x80059814.

**This function now matches retail byte-for-byte.** The round-25 residue below
(a register-identity swap in the decay branch, retail's `step` in `$a2`,
prior builds compiling it to `$a1`) closed on the first attempt this round by
testing finding 2 of this round's brief directly: *is "the registers differ"
actually a hard allocation fact, or is something in the C failing to force
the order/identity retail's build has?*

Two things were tried, in order:

1. **Rewrite the ternary as retail's own "unconditional default value,
   overwritten by the fallthrough" idiom** (`step = -0x258; if (x < 0) step =
   0x258;` instead of `step = (x < 0) ? 0x258 : -0x258;`), matching the exact
   delay-slot shape visible in `asm/nonmatchings/DreamSys/DreamSys__StepLookOffset.s`
   (`bgez v0,L / addiu a2,zero,-0x258 [delay slot, unconditional] / ori
   a2,zero,0x258 [fallthrough-only]`). **No effect** — byte-identical 49/53,
   same register swap. This rules out ternary-vs-explicit-overwrite as the
   cause; the CFG/idiom was already right, so this was a genuine, different,
   negative axis (not previously tried — earlier rounds only varied the
   ternary's spelling, never its control-flow shape).
2. **Reuse the SAME C variable (`delta`) for the decay branch's step value,
   instead of a separate `step` local.** The function has no stack frame at
   all (`nonmatching DreamSys__StepLookOffset, 0xD4` with no `addiu sp`) and is small
   enough that GCC 2.6.3's local register allocator appears to key hard-register
   preference on which pseudo-register a value's *first RTL definition*
   reuses, not on program order or declaration order (both already ruled out
   by round 25). `delta` (the table-indexed branch's delta value) had already
   claimed `$a2` earlier in the function; naming the decay branch's value
   `delta` again (both branches are mutually exclusive, so there is no real
   aliasing) let the allocator coalesce it onto the SAME register instead of
   picking a fresh one for a differently-named local. **Result: 53/53,
   whole-image SHA1 green.**

This confirms finding 2's discriminator directly: the mismatch was never a
hard GCC allocation preference, it was that a second C name for a
conceptually-reused value gave the allocator a reason to pick a *different*
pseudo/hard-register than the one already in use for the same physical value.
Renaming (not reshaping control flow) was the lever.

### Proposed learning

**A register-identity residue in a MUTUALLY-EXCLUSIVE branch pair, where
retail reuses the SAME hard register for two conceptually-different local
values, can be closed by reusing the SAME C variable name/declaration for
both — not by reshaping the second branch's expression.** This is a variant
of the existing "declaration order is inert" finding: it isn't the *order* of
the declarations that matters, it's whether the two values share ONE pseudo
register's lineage in the first place. Worth checking before filing a
same-length register-identity swap as a terminal stall: are the two
conflicting values in DIFFERENT, non-overlapping branches, and does retail's
disassembly show them sharing one physical register? If so, try folding them
into one C-level variable before accepting the stall.

## Final C (matches retail exactly)

```c
void DreamSys__StepLookOffset(DreamSys *this)
{
	s32 idx;
	s32 delta;
	s32 threshold;
	s32 sum;

	idx = this->unk_0x88;
	if (idx != 0) {
		delta = LOOK_OFFSET_STEPS[idx];
		threshold = LOOK_OFFSET_LIMITS[idx];
		sum = delta + this->unk_0x8C;
		if (sum >= 0) {
			if (sum < threshold)
				goto apply;
			this->unk_0x88 = 0;
			return;
		}
		if ((~sum + 1) >= threshold) {
			this->unk_0x88 = 0;
			return;
		}
	apply:
		this->unk_0x5C->unk_0x24 += delta;
		this->unk_0x8C = sum;
		this->unk_0x88 = 0;
		return;
	}
	if (this->unk_0x8C != 0) {
		delta = -0x258;
		if (this->unk_0x8C < 0)
			delta = 0x258;
		this->unk_0x5C->unk_0x24 += delta;
		this->unk_0x8C += delta;
	}
}
```

---

## History (pre-round-32, kept verbatim)

> **VERDICT CORRECTED AGAIN, round 25 (2026-09-08, runner echo).** The
> previous verdict below ("4 words SHORT at 49/53") is now STALE on the
> length claim: applying the round-25 head broadcast's "block-order lever"
> (from `CheckDreamAuxTriggerCondition`/`Entity__UpdateActivationState`, other units) to this function's
> sign-handling branch closed the LENGTH gap entirely. **The function is
> now the CORRECT length (53/53 instructions, zero address drift) and
> scores 49/53 raw words**, with all 4 remaining diffs in ONE place: a
> register-identity swap in the unrelated decay branch. Read the "Round 25"
> section below for the fix and the residue that's left; the "Original
> report" sections after it are kept verbatim as the historical record of
> residue 1 (`addiu_at`, RETIRED round 21) and the pre-round-25 state of
> residue 2.

> **VERDICT CORRECTED, round 24 (2026-09-08). RESIDUE 1 IS NO LONGER A
> BLOCKER, AND THE FUNCTION STILL DOES NOT MATCH.** Both halves of that
> matter, so do not read this as either a reopening or a dead end.
>
> Residue 1 below is the `addiu_at` folded-vs-unfolded form, root-caused in
> this report by reading `tools/maspsx/maspsx/__init__.py` directly. **It was
> RESOLVED in round 21** (maspsx `--addiu-at`,
> `docs/research/addiu-at-blocker.md`), and the pipeline now emits retail's
> five-instruction unfolded form here exactly. Verified this round: the two
> table loads at 0x4A024-0x4A030 and 0x4A038-0x4A044 are now
> **word-for-word identical to retail**. The eight isolated reproducer runs
> this report records were all correct and are all now moot.
>
> The preserved body was spliced back in unchanged and re-measured:
> **8/53 -> 14/53**, so retiring the blocker was worth 6 words. It is still
> **4 words SHORT**, and the whole remaining gap is in ONE place.

## The remaining residue: the sign-handling branch, worth exactly 4 words

Retail tests the negative case FIRST and gives each arm its own comparison
and its own branch to the shared apply block; it also spells the negation
`nor` + `addiu 1` rather than `negu`:

```
TARGET                                CURRENT
4a04c:  bltz    a1,4a064        |     4a04c:  bgez    a1,4a05c     <- polarity
4a050:  slt     v0,a1,v1        |     4a050:  negu    v0,a1        <- 1 insn, not 2
4a054:  bnez    v0,4a078        <
4a058:  nop                     <
4a05c:  j       598e0           i     4a054:  j       59860
4a060:  sw      zero,0x88(a0)   |     4a058:  slt     v0,v0,v1
4a064:  nor     v0,zero,a1      <
4a068:  addiu   v0,v0,1         <
4a06c:  slt     v0,v0,v1        r     4a05c:  slt     v0,a1,v1
4a070:  beqz    v0,4a094              4a060:  beqz    v0,4a084
```

Four TARGET-only words, which is the entire length gap. Everything before
0x4A04C and everything from 0x4A078 (the apply block) to the epilogue already
matches.

**One reshape tried and it is a NEGATIVE: spelling the negation as
`~sum + 1` instead of `-sum` makes it WORSE, 14/53 -> 11/53.** GCC folds it
straight back to `negu` and reorders around it, so the `nor`/`addiu` pair is
not reachable by writing the arithmetic out. It is more likely a consequence
of the branch STRUCTURE -- retail computing the comparison separately per arm
-- than of how the negation is spelled. The next attempt should reshape the
two arms as explicit nested `if`s with their own early exits, NOT touch the
negation expression.

## An attribution warning for whoever picks this up

Re-measuring produced `addiu at,at,0x7e40` against retail's
`addiu at,at,0x7e50` -- the table base off by exactly 0x10 -- plus a
101043-byte out-of-range diff. **That is NOT a symbol problem and NOT an
independent residue.** `build/lsdde.map` confirms `LOOK_OFFSET_STEPS` linked at
`0x80087e40`: the function is 4 words (0x10 bytes) short, so DreamSys.o's
text is 0x10 small and every symbol after it, including all of `.data`,
slides down by 0x10. The wrong-looking immediate IS the length bug, seen
from the other end.

This is CLAUDE.md's third way a score lies, arriving in its most convincing
disguise -- the drift shows up as a *plausible wrong constant* in the
function under the cursor rather than as obvious garbage. Check
`build/lsdde.map` for the symbol's linked address before believing a
symbol-shaped diff.

## Status (pre-round-25, kept for history)

`INCLUDE_ASM`, restored. The body below (still accurate, still the best
known) plus a fix to the four-word sign branch is all that is left.

---

## Round 25 (runner echo): the block-order lever closes the branch-structure gap

The prior report's residue 2 (below) had already localised the whole
4-word-short gap to the sign-handling branch and noted "the next attempt
should reshape the two arms as explicit nested `if`s with their own early
exits, not touch the negation expression" -- advice from BEFORE the
negation itself was understood. This round did both, informed by the head's
round-25 broadcast on `CheckDreamAuxTriggerCondition`/`Entity__UpdateActivationState` in other units (the
"block-order lever": GCC 2.6.3 gives the branch-free fallthrough to
whichever CFG arm is LAST in source order; a retail `j`-to-a-shared-join,
or a duplicated single-instruction tail kept as two copies, needs an
explicit `goto` in the C, not an if/else that lets the compiler pick which
arm falls through).

Reading `asm/nonmatchings/DreamSys/DreamSys__StepLookOffset.s` directly (not the old
report's abbreviated snippet) shows the real shape: the "clear `unk_0x88`
and return" tail is NOT one shared merge point -- it is **two separate
physical copies**, each ending in its own `j .L800598E0` with the store in
the delay slot:

```
bltz  a1, NEG                  ; a1 = sum
slt   v0, a1, v1                ; v1 = threshold; v0 = (sum < threshold)
bnez  v0, APPLY                 ; in range -> jump to the shared apply block
nop
j     END                       ; COPY 1: not in range (sum >= 0 arm)
 sw   zero, 0x88(a0)
NEG:
nor   v0, zero, a1              ; v0 = ~sum
addiu v0, v0, 1                 ;      +1  =>  v0 = -sum   (NOT a `negu`!)
slt   v0, v0, v1                ; v0 = (-sum < threshold)
beqz  v0, ZERO2                 ; not in range -> COPY 2
 nop
APPLY:                          ; falls straight in from NEG's fallthrough too
    ... this->unk_0x5C->unk_0x24 += delta; this->unk_0x8C = sum; ...
ZERO2:
j     END                       ; COPY 2: shared by (sum<0,not in range) AND apply's fallthrough
 sw   zero, 0x88(a0)
```

Two things follow directly from reading it this way instead of guessing
from the register list:

1. **The `sum >= 0` arm needs an explicit forward jump to the shared
   `apply` code** (retail's `bnez ... APPLY`), because `apply` is placed
   textually AFTER the `sum < 0` arm's own code. Writing `if (sum <
   threshold) goto apply;` for that arm (branching on the TRUE/in-range
   condition, matching retail's `bnez`-taken-when-in-range polarity
   exactly) and letting the `sum >= 0` arm's fallthrough be the
   "clear+return" path reproduces this without touching the sign
   comparison's boolean spelling at all.
2. **The `sum < 0` arm needs NO explicit jump to `apply`** -- it is placed
   immediately before `apply` in the instruction stream, so a plain `if
   (-sum >= threshold) { clear; return; }` falls straight through into
   `apply` on the untaken path, exactly matching retail's `beqz`-not-taken
   fallthrough.

Reshaped C (replacing the preserved `#if 0` body's ternary/`inRange` form):

```c
if (sum >= 0) {
	if (sum < threshold)
		goto apply;
	this->unk_0x88 = 0;
	return;
}
if ((~sum + 1) >= threshold) {
	this->unk_0x88 = 0;
	return;
}
apply:
	this->unk_0x5C->unk_0x24 += delta;
	this->unk_0x8C = sum;
	this->unk_0x88 = 0;
	return;
```

This alone (with the negation still spelled as plain `-sum`) reproduced
retail's ENTIRE branch/jump structure byte-for-byte -- every `bltz`, `slt`,
`bnez`, `j`, and `beqz` target now matches exactly, closing what the old
report called "4 words SHORT" down to a 1-word gap (`negu` vs `nor`+`addiu`)
localized to exactly one spot.

### The negation: `~sum + 1` written literally, not `-sum`

The prior report's own residue-1-era side note recorded that writing
`~sum + 1` explicitly "makes it WORSE (11/53)" and concluded the `nor`+
`addiu` pair "is not reachable by writing the arithmetic out" -- **but that
conclusion was reached UNDER THE WRONG BRANCH STRUCTURE** (the old
`inRange`-ternary shape, which was itself several words away from retail's
CFG for unrelated reasons). Re-tried under the corrected branch structure
above, `(~sum + 1) >= threshold` compiles to the exact `nor $v0,$zero,$a1`
/ `addiu $v0,$v0,1` pair retail has -- GCC does NOT fold it back to `negu`
in this context. **This is the opposite of what the old report found**,
and the lesson is the same one CLAUDE.md's "four ways a score lies" already
warns about at a different layer: a negative reshape result recorded under
one control-flow shape does not transfer to a different one. Re-test cheap
axes after a structural fix moves the ground under them; don't just trust
the old verdict.

With both fixes applied, the function reaches **49/53, exact length, zero
address drift** -- every diff localized to one place, described next.

### The one remaining residue: register identity in the decay branch, unrelated to the sign-handling fix

The decay branch (`this->unk_0x88 == 0`, the OTHER top-level arm, entirely
independent of everything above) computes a fixed step (`+0x258` or
`-0x258` depending on `this->unk_0x8C`'s sign) and applies it twice. Retail
assigns this value to `$a2`; the current build assigns it to `$a1`. Every
surrounding instruction (the `lw`/`beqz`/`bgez` guards, the `unk_0x5C`
load, the final stores) is byte-identical -- only the four bytes touched by
which physical register holds "step" differ (register-FIELD bits only,
same opcodes, same operands otherwise):

```
retail: li a2,-0x258 / li a2,0x258 / addu v1,a2,v1 / addu v0,a2,v0
build:  li a1,-0x258 / li a1,0x258 / addu v1,a1,v1 / addu v0,a1,v0
```

Tried and rejected (this round, all preserving the corrected sign-handling
branch above so as not to reintroduce the closed gap):

- **Swapping the ternary's polarity** (`(this->unk_0x8C >= 0) ? -0x258 :
  0x258` instead of `< 0 ? 0x258 : -0x258`): the `bgez`/`li` PAIRING also
  flipped (wrong instruction now paired with wrong branch sense) and the
  register still didn't match retail's `a2` -- net WORSE (48/53).
- **Writing the two arms as an explicit duplicated if/else** (`if
  (this->unk_0x8C < 0) { += 0x258; += 0x258; } else { -= 0x258; -= 0x258;
  }`, no shared `step` temp) instead of the ternary-into-one-shared-write
  form: this is the SAME "duplicated assignment" shape as the sign-handling
  fix above, tried on the theory it might transfer -- it does NOT. Retail's
  own asm for this branch reloads `this->unk_0x8C` exactly ONCE more after
  the sign test (a single `lw` for the final `+= step`, not two independent
  reloads per arm), so retail's source really does use one shared `step`
  value, not a duplicated body. Result: 15 words WORSE (34/53) with real
  address drift, confirming the duplicated-assignment shape doesn't apply
  to every register-swap symptom -- only to the ones genuinely about which
  physical copy of a JOIN gets the fallthrough.
- **Reordering the `step`/`idx`/`delta`/`threshold`/`sum` local
  declarations** (moving `step` first): no effect on the emitted register
  at all -- C89 declaration order doesn't drive GCC 2.6.3's pseudo-register
  numbering here, as expected once tested rather than assumed.
- **Splitting the outer `if (idx != 0) {...} else if (this->unk_0x8C != 0)
  {...}` into two independent top-level `if`s** (giving the first an
  explicit `return;` and dropping the `else`): no byte change at all --
  confirms the two branches really are compiled independently once the
  first one's own CFG is fixed, so the residue is local to the decay
  branch's own expression, not an artifact of how it's chained to the
  first.

This is a plain register-identity mismatch -- same instructions, same
operands, different physical register -- which CLAUDE.md's HARD RULES
explicitly bans fixing via `register T v asm("$N")` or an operand
constraint. None of the reshapes above changed WHICH VALUE went where in
the CFG (all preserved the byte-exact structure achieved above), so this is
reported as a STALL on this one axis rather than pursued further with a
banned lever.

**Status: `INCLUDE_ASM`, restored (whole-image SHA1 verified green with it
restored).** The preserved body below is the 49/53 state, inlined with the
declarations it needs, positioned to compile if reinstated.

```c
#if 0
/* best-reached body, round 2026-09-08 (runner echo): 49/53 words, exact
   length, zero address drift. The remaining 4-word residue is a pure
   register-identity swap (retail uses $a2 for `step`, this compiles to
   $a1) confined to the decay branch -- see the "Round 25" section above. */
void DreamSys__StepLookOffset(DreamSys *this)
{
	s32 idx;
	s32 delta;
	s32 threshold;
	s32 sum;
	s32 step;

	idx = this->unk_0x88;
	if (idx != 0) {
		delta = LOOK_OFFSET_STEPS[idx];
		threshold = LOOK_OFFSET_LIMITS[idx];
		sum = delta + this->unk_0x8C;
		if (sum >= 0) {
			if (sum < threshold)
				goto apply;
			this->unk_0x88 = 0;
			return;
		}
		if ((~sum + 1) >= threshold) {
			this->unk_0x88 = 0;
			return;
		}
	apply:
		this->unk_0x5C->unk_0x24 += delta;
		this->unk_0x8C = sum;
		this->unk_0x88 = 0;
		return;
	}
	if (this->unk_0x8C != 0) {
		step = (this->unk_0x8C < 0) ? 0x258 : -0x258;
		this->unk_0x5C->unk_0x24 += step;
		this->unk_0x8C += step;
	}
}
#endif
```

## Proposed learning (round 25, additive to the one at the very end of this file)

A negative permuter/hand-reshape result recorded against ONE control-flow
shape (e.g. "`~x+1` folds back to `negu`, unreachable") does not transfer to
a DIFFERENT control-flow shape for the same expression -- re-test cheap
arithmetic-spelling axes after any structural (branch/goto) fix, rather
than trusting a prior negative result measured under different code around
it. This function is the concrete instance: the exact same `~sum + 1`
spelling went from "makes it worse" to "matches retail exactly" once the
surrounding branch structure was corrected first.

---

## Original report, kept verbatim as the historical record

> **HEAD ADJUDICATION, round 2026-08-30-a.** The diagnosis in this report is
> CORRECT and the head reproduced it independently from scratch. It is now
> written up project-wide in **`docs/research/addiu-at-blocker.md`**, with an
> isolated reproducer and a corpus census: retail uses the unfolded (`addiu_at`)
> form for **502 of 502** runtime-indexed global accesses across 39 files, and
> the folded form **zero** times. There is no counterexample anywhere in the
> executable.
>
> **One correction to the proposed remedy.** Repinning `--aspsx-version` to 2.29
> is not surgical and should not be presented as the fix. `config_for_aspsx_version`
> flips **four** flags below 2.30, not one — `addiu_at` plus three nop-insertion
> rules (`nop_at_expansion`, `nop_mflo_mfhi`, `nop_lw_lw`) that affect constructs
> throughout the image, including inside the 57 functions that currently match.
> maspsx exposes no `--addiu-at` flag, so the behaviour cannot be enabled alone
> without patching maspsx. This is the same shape as the already-rejected `-G`
> experiment. See the research document; the ruling is the operator's.


Round 2026-08-30, runner ALPHA, unit `DreamSys`. Best reached: 8/53 words in
range, with a large outside-range diff (the size is wrong, so that in-range
score is not itself trustworthy — see below). Restored to `INCLUDE_ASM`.

**Companion function `DreamSys__StepLookYaw` (same unit, same round) hits the
identical root cause** — see its own report, which cross-references this one
rather than repeating the analysis.

## What it does

```c
/* best-reached body, does NOT compile to retail bytes — preserved for the
   next attempt, not a working match */
#if 0
void DreamSys__StepLookOffset(DreamSys *this)
{
	s32 idx;
	s32 delta;
	s32 threshold;
	s32 sum;
	s32 step;
	bool inRange;

	idx = this->unk_0x88;
	if (idx != 0) {
		delta = LOOK_OFFSET_STEPS[idx];
		threshold = LOOK_OFFSET_LIMITS[idx];
		sum = delta + this->unk_0x8C;
		inRange = (sum < 0) ? (-sum < threshold) : (sum < threshold);
		if (inRange) {
			this->unk_0x5C->unk_0x24 += delta;
			this->unk_0x8C = sum;
		}
		this->unk_0x88 = 0;
	} else if (this->unk_0x8C != 0) {
		step = (this->unk_0x8C < 0) ? 0x258 : -0x258;
		this->unk_0x5C->unk_0x24 += step;
		this->unk_0x8C += step;
	}
}
#endif
```

Vtable slot `0x144`. Reads `this->unk_0x88` as an index into a
delta/threshold table pair (`LOOK_OFFSET_STEPS`/`LOOK_OFFSET_LIMITS`); if the resulting sum
stays within the threshold, nudges `this->unk_0x5C->unk_0x24` and
`this->unk_0x8C` by the delta and clears `unk_0x88`; otherwise (idx==0) it
decays `unk_0x8C` towards zero by a fixed step of 600, applying the same
step to `unk_0x5C->unk_0x24`. This part of the logic (the decay-step branch,
the second half of the function) verified as CORRECT and byte-identical in
isolation — see residue 2 below; only the *first* half (the table-indexed
branch) is blocked.

## Residue 1 (BLOCKING, confirmed toolchain-level — do not re-attempt by
reshaping): the pinned `aspsx-version=2.34` cannot emit retail's addressing
form for `globalArray[runtimeIndex]`

Retail computes `LOOK_OFFSET_STEPS[idx]` as **five** instructions:

```
lui   $at, %hi(LOOK_OFFSET_STEPS)
addiu $at, $at, %lo(LOOK_OFFSET_STEPS)   ; forms a COMPLETE pointer
addu  $at, $at, $v1                ; THEN adds the (already ×4) index
lw    $a2, 0x0($at)
```

Every C phrasing I tried compiles to the same **four**-instruction form
instead (verified both in the full project build and in eight isolated
`cpp|cc1|maspsx|as` reproducer runs against `/tmp/.../scratchpad/t1.c`
through `t7.c` — plain array subscript, an explicit intermediate pointer
local, an integer-cast-then-dereference, `LOOK_OFFSET_STEPS[]` with no declared
size, and an added-constant index):

```
lui  $at, %hi(LOOK_OFFSET_STEPS)
addu $at, $at, $v1                 ; index added FIRST, no complete pointer yet
lw   $a2, %lo(LOOK_OFFSET_STEPS)($at)     ; %lo folded straight into the load
```

**Root cause, confirmed by reading `tools/maspsx/maspsx/__init__.py` directly
(not inferred from behavior):** GCC 2.6.3's `cc1` never emits either of the
above — it emits the abstract pseudo-op `lw $2,LOOK_OFFSET_STEPS($4)` (confirmed
from `cc1`'s own raw output before `maspsx`/`as` see it). Expanding that
pseudo-op into real MIPS instructions is `maspsx`'s job, at
`__init__.py:972-998`, and the choice between the two forms above is
**gated by a single boolean, `self.addiu_at`**, which is set from
`--aspsx-version` in `maspsx.py:23,38`:

```python
addiu_at: bool = False
...
if aspsx_version < (2, 30):
    ...
    config.addiu_at = True   # <- retail's 5-instruction form
```

The project pins `--aspsx-version=2.34` (`Makefile`, `CLAUDE.md`). `(2, 34) <
(2, 30)` is `False`, so `addiu_at` is `False` **unconditionally**, for every
`globalArray[runtimeIndex]` load in the entire project, regardless of source
shape. No C-level reshaping reaches the other branch — the branch simply
isn't reachable at this pin.

**This is NOT the already-documented gp-relative blocker** (that one is
about `-G`/small-data addressing and doesn't apply here — neither array
touches `%gp_rel`, confirmed with `grep gp_rel`). It's a different
mechanism, same category: a maspsx/aspsx-version macro-expansion choice, not
a compiler-flag or source-shape question.

**Project-wide scale, for whoever picks this up:** `grep -rl 'addiu.*\$at, \$at, %lo' asm/`
finds this exact instruction sequence in 502 lines across `class_39e08.s`,
`code_179d8.s`, `Task.s`, `Entity.s`, and inside several
`DreamAux`/`DreamSys`/`Entity`/`StageGrid`/`psyq_*` `nonmatchings/*.s`
files. **None of those functions are matched yet** — I confirmed by checking
every unit's `.c` file; every occurrence is still `INCLUDE_ASM`. This isn't
a one-off: it's a pattern that will block the first attempt at
runtime-indexed global-array access anywhere in the project, and is worth an
operator decision on whether `--aspsx-version` needs to vary per-file/per-TU,
or whether some other flag exists to force `addiu_at` behavior without
dropping below 2.30 (which the maspsx table also ties to `nop_at_expansion`,
`nop_mflo_mfhi`, and `nop_lw_lw` — i.e. it isn't a single independent knob,
consistent with why the gp-relative blocker's `-G` experiment also had
unwanted side effects when flipped globally).

## Residue 2 (separate, unconfirmed whether it's real): branch direction on
`sum < 0` vs `sum >= 0`

Independent of residue 1, retail's branch at the top of the table-indexed
path is `bltz $a1,NEG` (falls through on `sum >= 0`, jumps away on `sum <
0`). Every phrasing I tried — `if (sum<0){A}else{B}`, the reverse `if
(sum>=0){A}else{B}`, and a ternary `(sum<0) ? A : B` computing a single
`inRange` local before a separate `if (inRange)` — compiled to `bgez
$a1,LABEL` (falls through on `sum < 0`, jumps away on `sum >= 0`) every
time. Three structurally different source shapes, one consistent (wrong)
compiler choice — this looks like a GCC-internal canonicalization of the
sign comparison, not something addressable from source, but I did not
exhaust every possible shape (e.g. `abs()`-based idioms, `unsigned` casts)
before residue 1 made further attempts on this function moot. **Do not
treat this as confirmed compiler-internal the way residue 1 is** — it's
reported for whoever revisits this function after residue 1 has an operator
ruling, so they don't have to re-derive the branch-order observation from
scratch, but it deserves its own few attempts once residue 1 is unblocked.

The second half of the function (the `unk_0x8C`-decay branch, no array
indexing involved) matched retail's exact shape on the FIRST attempt,
including its own sign-branch (`this->unk_0x8C < 0` — written to match
retail's `bgez`, which DOES fall through on `>=0` in that half, i.e. the
opposite of the FIRST half's residue) — so GCC's sign-comparison
canonicalization is not a blanket "always bgez" rule; it's specific to the
first half's particular expression shape, likely interacting with residue
1's address computation somehow. This is a lead for whoever revisits: the
branch-direction residue may itself be a downstream SYMPTOM of the
address-computation residue rather than an independent problem — worth
re-checking once residue 1 has a resolution, before spending attempts on it
in isolation.

## Proposed learning

**A missing/extra `addiu $at,$at,%lo(sym)` around a runtime-indexed global
array read is not a source-shape residue — check `tools/maspsx/maspsx/__init__.py`'s
`addiu_at`-gated branch (search `is_addend and r_source`) before spending any
attempts reshaping the C.** At the project's pinned `--aspsx-version=2.34`,
`addiu_at` is `False` unconditionally, so retail's 5-instruction form for
`globalArray[runtimeIndex]` is unreachable from any C source. Confirmed with
a standalone reproducer (`cpp|cc1|maspsx|as`, no linking needed) across eight
different C phrasings, and confirmed at the tool-source level, not just by
behavior. Recommend escalating to the operator: this pattern appears
(unmatched) in 502 places project-wide, per `grep -rl 'addiu.*\$at, \$at, %lo'
asm/`, so it will recur.

## Naming

`DreamSys__StepLookOffset` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059814`.

Steps `heightCurve->endValue` (the far keyframe's value) by the
per-tick delta `LOOK_OFFSET_STEPS[lookOffsetCommand]` (+-600), refuses the step
once the accumulator `lookOffset` would pass `LOOK_OFFSET_LIMITS` (+-9000), and when
no command is queued decays `lookOffset` back toward 0 by 600 a tick, applying that
decay to the curve as well. Consumes the command (resets it to 0) either way.
Tier B: the arithmetic is exact and measured off the tables; "look" is the shared
reading described in `DreamSys__StepLook.md`.
