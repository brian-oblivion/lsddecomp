# DreamSys__TimerTick — MATCHED

**Unit:** DreamSys · **Size:** 62 words (0xF8 bytes) · **Status:** MATCHED
(62/62 words), whole-image SHA1 verified green by `build-and-verify.sh`.
Closed round 2026-09-02, runner BRAVO, by pointing the previously-untried
`__asm__("")` lever at the exact single-instruction residue this report's
first author (round 2026-08-30-something, see "What was tried" below) had
already isolated and flagged as worth trying. See "MATCHED: the
`__asm__("")` lever" below for what closed it and why. The rest of this
report (the original 61/62 STALL writeup) is kept verbatim as the
derivation history.

## What it does

`gDreamSysMethods` slot `+0x98` (`TimerTick`). Advances the dream timer once
per call when `arg2 == 2`; once the timer catches up to `dreamTimeLimit`, it
either handles a flashback-session continuation/abort or a normal-session
flashback save + link-abort:

```c
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
		if (this->unknwon_int_0x44 != 0 || this->vt->LoadNextFlashback(this, 0)) {
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
```

`arg1` is never read anywhere in the disassembly; kept in the signature only
because `$a2` (the used parameter) implies at least 3 arguments and no other
caller of this slot exists in this unit to confirm `arg1`'s true role.

## Residue

One word, `0x495DC`: `bnez $v0, ADDR`. Retail branches to `0x49604`; every
reshape tried instead produces `0x49640`. **Both addresses hold byte-identical
code** — `j 0x58e78` / `sw $zero, 0x24($s0)` in the delay slot — the function's
"just zero `dreamTimer` and return" tail, which the compiler emits in two
physical copies (one right after the `isFlashbackSession` fallthrough test, one
falling out of the `slot30` call at the bottom). The residue is *which*
identical copy this one branch targets, not a register, an instruction, or a
missing/extra nop — everything else in the function, all 61 other words,
matches exactly, including the *other* branch that reaches the very same tail.

This is the already-documented, still-open **"identical assignment reaching
different merge points"** class from `DECOMPILATION_LEARNINGS.md`
(`Entity__UpdateActivationState`, 72/74) — a GCC 2.6.3 cross-jump/tail-duplication artifact
that source-level reshaping has not moved in either instance now on record.

## What was tried (5 reshapes, all landing on the identical residue)

1. `if (old < limit) { two calls; return; } /* rest as one big if/else */` —
   this ALSO moved which physical block was placed first (worse: 19/62,
   because the "old < limit" case's block was placed as the fallthrough
   instead of a distant jump target, an unrelated but confirmatory finding
   about branch-polarity/block-placement being source-derived, not free).
2. Explicit `goto tick_only;` to fix (1)'s placement, plus `goto zero_only;`
   (two call sites) / `goto call_slot30;` for the merge points — 61/62, this
   exact residue.
3. Same as (2) but with the "zero_only" tail written out literally three
   times instead of shared via `goto` — 61/62, identical residue (GCC's own
   cross-jump pass re-merges the duplicates regardless of whether the source
   shares or repeats them).
4. Combined the `unk_0x44 != 0` and `LoadNextFlashback` tests into one
   short-circuited `if (A || B) { zero; return; }` (preserving the "don't call
   `LoadNextFlashback` when `unk_0x44 != 0`" short-circuit) — 61/62, identical
   residue. This is the version left as the best candidate (inlined above).
5. Inverted the outer test to `if (!isFlashbackSession) {...} else if (A || B)
   {...}` — much worse (14/62) and shifted the function's total size
   (asm-differ's "differs OUTSIDE this range" warning fired), confirming this
   is the wrong shape family, not a nearby variant.

Not tried: an `__asm__("")` barrier. `DECOMPILATION_LEARNINGS.md`'s own
characterization of this class is that it needs a branch or a call to reason
about scheduling, and the established barrier levers operate on
delay-slot/register-allocation choices *within* one already-selected block,
not on cross-jump target selection between two co-existing duplicate blocks
several words apart — there is no local instruction position for a barrier to
occupy that would plausibly influence which whole tail this branch reaches.
Flagging as untried rather than silently assuming it doesn't apply, in case a
future attempt wants to burn one try confirming that.

## New knowledge

- **Confirms `gDreamSysMethods+0x98` is this function's own slot**, already
  named `TimerTick` in the header.
- **Second confirmed instance of the "identical assignment reaching different
  merge points" class**, this time on a `bnez` rather than `Entity__UpdateActivationState`'s
  case. Worth promoting the class's confidence level in
  `DECOMPILATION_LEARNINGS.md` if a head reviews this: two independent
  functions in the same unit, two different authors' rounds, same
  unreachable-by-reshaping signature.
- Re-confirms **"instruction order only" vs "which duplicate" are different
  problems**: attempt (1) shows block PLACEMENT (fallthrough vs distant jump)
  IS reachable by reshaping (`if (cond) {small}; else {big};` vs the
  reverse), while attempt (2)-(4) show which of two *already correctly placed
  and physically distinct* duplicate blocks a specific branch targets is NOT
  reachable that way, at least by every C-level shape tried so far.

### Proposed learning

Promote the confidence of the existing `DECOMPILATION_LEARNINGS.md` entry:
"identical assignment reaching different merge points" now has two
independent instances (`Entity__UpdateActivationState` 72/74, `DreamSys__TimerTick` 61/62)
across two rounds, both stalled after multiple reshapes and both closest
possible short of full match. Worth flagging as a permuter candidate rather
than continuing to spend manual reshape attempts on new instances.

## MATCHED: the `__asm__("")` lever (round 2026-09-02, runner BRAVO)

The previous author explicitly flagged this as untried and reasoned that
the barrier "operates on delay-slot/register-allocation choices *within*
one already-selected block, not on cross-jump target selection between two
co-existing duplicate blocks" — a plausible-sounding argument that turned
out to be wrong for this specific case. Placement mattered enormously:

1. **`__asm__("")` as the function's very first statement** (the
   canonical placement per `DECOMPILATION_LEARNINGS.md`'s existing
   `Pad__DispatchEvents` precedent): made it MUCH worse — 3/62, plus whole-file
   address drift (an extra instruction). Reverted immediately.
2. **Right before `if (this->isFlashbackSession)`** (i.e. right after the
   `goto tick_only;` early-exit, at the top of the block containing the
   residue): no change at all — still 61/62, identical residue, but at
   least no drift this time.
3. **Inside the `if (this->unknwon_int_0x44 != 0 ||
   this->vt->LoadNextFlashback(this, 0))` true-branch, as the very FIRST
   statement — immediately before `this->dreamTimer = 0;`, i.e. right at
   the top of the block whose entry point is the ambiguous branch target
   itself**: **62/62, full match.**

So the working placement is not "function entry" (the default reflex from
the one documented precedent) but literally inside the specific basic
block the residue's branch target selection was choosing between — a much
narrower target than the barrier has needed elsewhere in this project so
far.

**Rule-6 verification, done explicitly rather than assumed:** removed just
the barrier (kept everything else identical) and rebuilt. The result
reverts to exactly the ORIGINAL 61/62 residue — same instruction
(`bnez $v0, ...`), same register (`$v0`), only the branch TARGET address
differs (`0x49604` vs `0x49640`, both holding byte-identical code). No
register identity changed with the barrier removed, only which of the two
physical copies of the tail block the branch reaches. This is squarely the
permitted "changes instruction ORDER only" case CLAUDE.md rule 6 describes,
not the banned "changes WHICH REGISTER holds a value" case.

### Updated proposed learning (supersedes the one below for THIS function)

**A cross-jump/tail-duplication "which identical copy does this branch
reach" residue CAN be closed by `__asm__("")`, but only when the barrier
sits inside the specific candidate block whose selection is ambiguous —
not at the function's top, and not merely "before the branch that picks
between them."** The barrier at the top of `DreamSys__TimerTick` made
things categorically worse (drift, not just a non-improvement), while the
same barrier one block over did nothing, and one further in — at the top
of the actual duplicate-tail block being selected between — closed it
completely. Before assuming this class is permuter-only work (as the
original writeup here recommended, and as `Entity__UpdateActivationState`'s report still
does), try the barrier at EACH block that is a candidate cross-jump target,
not just at the function entry. The original "worth flagging as a permuter
candidate" note below is retracted for this function; `Entity__UpdateActivationState`
(72/74, different function, not yet re-attempted this round) may still
be worth a similar targeted retry before reaching for the permuter.

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the one bare `__asm__("")`,
inside `if (this->state != 0 || ...loadNextFlashback(this, 0))` before
`this->tick = 0;`, is **justified** and now commented at the site. Measured by
deleting it alone: the image went red by 1 byte, `funcdiff` 61/62, and the one
differing word is the `bnez v0` at 0x495DC (the `state != 0` test): retail
branches to 0x49604, this block's own `j`/`sw zero,0x24(s0)` tail, while the
barrier-less build branches to 0x49640, the identical tail after the
`notifyParents(this, 0xA)` call. So what it forces is block identity (no
cross-jump between two identical tails), not register identity.

## Comment moved from src/world/DreamSys.c (round 92, track 7)

Replaced in the source by a comment that says what the code does; kept here as written.

```c
            /* Keeps the `state != 0` branch targeting this block's own
             * `this->tick = 0` + return tail; without it GCC cross-jumps that
             * branch to the identical tail after the notifyParents(0xA) call. */
```
