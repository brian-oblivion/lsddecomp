# DreamSys__RunTickCallbacks

> Renamed from `func_800593D8` on 2026-09-22 (tools/rename.py). Address 0x800593d8.

**Unit:** DreamSys · **Size:** 21 instructions · **Status:** MATCHED (21/21 words)

## What it does

Fires two optional callbacks in order, each null-checked. Both take the object as their only argument.

## The C

```c
void DreamSys__RunTickCallbacks(DreamSys *this)
{
	if (this->callback_0x80 != NULL)
		this->callback_0x80(this);
	if (this->callback_0x98 != NULL)
		this->callback_0x98(this);
}
```

## Provenance

Derived by runner/echo in round 2026-08-29-a. That runner was killed by an
account-wide session limit before it committed anything at all -- it had
written C for 14 of its 15 assigned functions and filed zero reports. The head
recovered the body per docs/PARALLEL-RUNS.md 4c, rescored it in the main
checkout from a clean build, and filed this report.

This is a MATCH, not a mid-attempt snapshot: applied on its own to a green
tree it gives 21/21 words with the whole-image SHA1 verifying.

Scoring note: the head's first pass at rescoring these 14 bodies read numbers
from a build that had failed to compile (the spliced file was missing echo's
`#include "dream_sys.h"`, so every `DreamSys *` was a parse error). funcdiff's
STALE BUILD guard caught it. The numbers here are from the corrected pass --
see docs/DECOMPILATION_LEARNINGS.md on salvage splicing.

## Naming

- **Tier B.** Invokes callback_0x80 and callback_0x98 if set -- the second of the two statements DreamSys__TimerTick's tick-only path runs every tick.
