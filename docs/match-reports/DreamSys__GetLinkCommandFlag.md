# DreamSys__GetLinkCommandFlag

> Renamed from `func_8005931C` on 2026-09-22 (tools/rename.py). Address 0x8005931c.

**Unit:** DreamSys · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What it does

Plain getter for `unk_0x74`, the field `DreamSys__UpdateTickState` clears.

## The C

```c
s32 DreamSys__GetLinkCommandFlag(DreamSys *this)
{
	return this->unk_0x74;
}
```

## Provenance

Derived by runner/echo in round 2026-08-29-a. That runner was killed by an
account-wide session limit before it committed anything at all -- it had
written C for 14 of its 15 assigned functions and filed zero reports. The head
recovered the body per docs/PARALLEL-RUNS.md 4c, rescored it in the main
checkout from a clean build, and filed this report.

This is a MATCH, not a mid-attempt snapshot: applied on its own to a green
tree it gives 3/3 words with the whole-image SHA1 verifying.

Scoring note: the head's first pass at rescoring these 14 bodies read numbers
from a build that had failed to compile (the spliced file was missing echo's
`#include "DreamSys.h"`, so every `DreamSys *` was a parse error). funcdiff's
STALE BUILD guard caught it. The numbers here are from the corrected pass --
see docs/DECOMPILATION_LEARNINGS.md on salvage splicing.

## Naming

`DreamSys__GetLinkCommandFlag` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005931C`.

A pure getter for `linkCommandFlag`, so the getter half is
tier A by the leaf rule; the name of the FIELD is what makes this tier B. The
field's only setter anywhere in carved code is `DreamSys__ApplyLinkCommand`'s
`case 23:` (i.e. its `mode` argument == 25); its only other writers clear it --
`DreamSys__ResetLinkState`, and `DreamSys__UpdateTickState` on every tick the
object is not movement-blocked. So it is the flag a link command raises and the
next tick consumes. What command 25 MEANS is not established.
