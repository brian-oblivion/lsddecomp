# DreamSys__TimerTick

**Unit:** DreamSys · **Size:** 62 words (0xF8 bytes) · **Status:** STALL —
61/62 words (best reshape). Single-instruction residue, `build exit=0` restored
(`INCLUDE_ASM` reinstated), whole-image SHA1 green.

## What it does

`DREAMSYS_METHODS` slot `+0x98` (`TimerTick`). Advances the dream timer once
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
	this->vt->func_80059394(this);
	this->vt->func_800593D8(this);
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
(`func_8005DBF0`, 72/74) — a GCC 2.6.3 cross-jump/tail-duplication artifact
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

- **Confirms `DREAMSYS_METHODS+0x98` is this function's own slot**, already
  named `TimerTick` in the header.
- **Second confirmed instance of the "identical assignment reaching different
  merge points" class**, this time on a `bnez` rather than `func_8005DBF0`'s
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
independent instances (`func_8005DBF0` 72/74, `DreamSys__TimerTick` 61/62)
across two rounds, both stalled after multiple reshapes and both closest
possible short of full match. Worth flagging as a permuter candidate rather
than continuing to spend manual reshape attempts on new instances.
