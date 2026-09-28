# DreamSys__GetSetDreamTimeLimit

**Unit:** DreamSys · **Size:** 14 instructions · **Status:** MATCHED (14/14 words)

## What it does

Swaps the dream time limit and returns the previous value, converting on both sides: the incoming value is in seconds and stored as frames (x15), the outgoing value converts back (/15). Both conversions are guarded on `>= 0`, so a negative sentinel passes through untouched in each direction.

## The C

```c
s32 DreamSys__GetSetDreamTimeLimit(DreamSys *this, s32 value)
{
	s32 result;

	if (value >= 0)
		value = value * 15;
	result = this->dreamTimeLimit;
	this->dreamTimeLimit = value;
	if (result >= 0)
		result = (u32)result / 15;
	return result;
}
```

## Provenance

Derived by runner/echo in round 2026-08-29-a. That runner was killed by an
account-wide session limit before it committed anything at all -- it had
written C for 14 of its 15 assigned functions and filed zero reports. The head
recovered the body per docs/PARALLEL-RUNS.md 4c, rescored it in the main
checkout from a clean build, and filed this report.

This is a MATCH, not a mid-attempt snapshot: applied on its own to a green
tree it gives 14/14 words with the whole-image SHA1 verifying.

Scoring note: the head's first pass at rescoring these 14 bodies read numbers
from a build that had failed to compile (the spliced file was missing echo's
`#include "dream_sys.h"`, so every `DreamSys *` was a parse error). funcdiff's
STALE BUILD guard caught it. The numbers here are from the corrected pass --
see docs/DECOMPILATION_LEARNINGS.md on salvage splicing.
