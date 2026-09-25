# DreamSys__NoOpSlot12C

> Renamed from `func_800595A0` on 2026-09-22 (tools/rename.py). Address 0x800595a0.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Returns 0 unconditionally, ignoring the object. A stub slot that later subclasses presumably override.

## The C

```c
s32 DreamSys__NoOpSlot12C(DreamSys *this)
{
	return 0;
}
```

## Provenance

Derived by runner/echo in round 2026-08-29-a. That runner was killed by an
account-wide session limit before it committed anything at all -- it had
written C for 14 of its 15 assigned functions and filed zero reports. The head
recovered the body per docs/PARALLEL-RUNS.md 4c, rescored it in the main
checkout from a clean build, and filed this report.

This is a MATCH, not a mid-attempt snapshot: applied on its own to a green
tree it gives 2/2 words with the whole-image SHA1 verifying.

Scoring note: the head's first pass at rescoring these 14 bodies read numbers
from a build that had failed to compile (the spliced file was missing echo's
`#include "DreamSys.h"`, so every `DreamSys *` was a parse error). funcdiff's
STALE BUILD guard caught it. The numbers here are from the corrected pass --
see docs/DECOMPILATION_LEARNINGS.md on salvage splicing.

## Naming

`DreamSys__NoOpSlot12C` -- tier A (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_800595A0`.

`return 0;` and nothing else. The unit already names its empty
slots this way (`DreamSys__NoOpSlot14C`, `DreamSys__NoOpSlot150`,
`Actor__NoOpSlotD8`, `DreamSys__NoOpSlotE8Default`); the offset +0x12C is
`tools/classtable.py DREAMSYS_METHODS`. Its one caller,
`DreamSys__ApplyMoveCommand`, discards the result, so this is an override hook that
DreamSys itself declines. Tier A: evident from the body alone, and the name asserts
nothing the body does not show.
