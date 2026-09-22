# DreamSys__BlockMovement

> Renamed from `func_80059310` on 2026-09-22 (tools/rename.py). Address 0x80059310.

**Unit:** DreamSys · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What it does

Sets the `unk_0x70` flag to 1 -- the guard that `DreamSys__UpdateTickState` tests before recomputing its tick state.

## The C

```c
void DreamSys__BlockMovement(DreamSys *this)
{
	this->unk_0x70 = 1;
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

- **Tier B.** Sets a flag read by two independent sites: DreamSys__UpdateTickState skips its whole body, and DreamSys__TickMoveFree (not renamed) returns early, both only while it is nonzero.
