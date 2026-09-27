# DreamSys__GetDreamTimerScaled

> Renamed from `DreamSys__GetDreamTimerSeconds` on 2026-09-22 (tools/rename.py). Address 0x80059360.

> Renamed from `func_80059360` on 2026-09-22 (tools/rename.py). Address 0x80059360.

**Unit:** DreamSys · **Size:** 7 instructions · **Status:** MATCHED (7/7 words)

## What it does

Returns the dream timer converted from frames to seconds. The unsigned divide is what retail emits; a signed one produces the extra sign-correction sequence.

## The C

```c
s32 DreamSys__GetDreamTimerScaled(DreamSys *this)
{
	return (u32)this->dreamTimer / 15;
}
```

## Provenance

Derived by runner/echo in round 2026-08-29-a. That runner was killed by an
account-wide session limit before it committed anything at all -- it had
written C for 14 of its 15 assigned functions and filed zero reports. The head
recovered the body per docs/PARALLEL-RUNS.md 4c, rescored it in the main
checkout from a clean build, and filed this report.

This is a MATCH, not a mid-attempt snapshot: applied on its own to a green
tree it gives 7/7 words with the whole-image SHA1 verifying.

Scoring note: the head's first pass at rescoring these 14 bodies read numbers
from a build that had failed to compile (the spliced file was missing echo's
`#include "DreamSys.h"`, so every `DreamSys *` was a parse error). funcdiff's
STALE BUILD guard caught it. The numbers here are from the corrected pass --
see docs/DECOMPILATION_LEARNINGS.md on salvage splicing.

## Naming

- **Tier A.** One-line getter, dreamTimer/15 -- the same tick-per-second scale DreamSys__GetSetDreamTimeLimit already established (value*15 on set, result/15 on get).

## Head naming review, round 65: tier-A name CORRECTED

Round 65's naming runner named this `DreamSys__GetDreamTimerSeconds`, tier A,
on the evidence "dreamTimer/15, matches GetSetDreamTimeLimit's scale". The
head's track-3 sample review rejected that name: it asserts a physical UNIT
that nothing in the tree establishes.

The body is `return (u32)this->dreamTimer / 15;` and nothing more. For the
quotient to be SECONDS, `dreamTimer` must advance at 15 Hz. Measured, it does
not follow from anything available:

- `dreamTimer` is incremented by exactly 1 per call of `DreamSys__TimerTick`
  (src/DreamSys.c, gated on `arg2 == 2`), and `TimerTick` is a vtable slot
  (`include/DreamSys.h`, `void *TimerTick;`) with no caller anywhere in `src/`
  that would fix its rate.
- `dreamTimeLimit`, the value `dreamTimer` is compared against, is written
  only through its own setter. No constant that would reveal a unit reaches
  it in matched code.
- The runner's stated evidence ("matches GetSetDreamTimeLimit's scale") shows
  the timer and its limit share UNITS AS EACH OTHER. It says nothing about
  what that unit is, which is the whole claim the name makes.

So the name is now `DreamSys__GetDreamTimerScaled`, **tier B**: the divisor is
15 and the return is a coarser-grained timer reading, which is what the code
does. If `TimerTick`'s dispatch rate is ever established at 15 Hz — the caller
is in a still-unnamed or still-`INCLUDE_ASM` unit — `...Seconds` becomes
correct and tier A, and this section is the evidence to retire.

This is the round-63 precedent applied again (`MatchesDreamAuxRange` ->
`IsDayInPeriodPhase`, corrected at naming review because the body
tests a stride-3 progression, not a contiguous range): the head corrects the
name in place rather than discarding the pass, and records why.
