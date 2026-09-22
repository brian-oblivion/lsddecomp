# DreamSys__func_59590

> Renamed from `func_80059590` on 2026-09-22 (tools/rename.py). Address 0x80059590.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Setter clearing `unk_0x7C`.

## The C

```c
void DreamSys__func_59590(DreamSys *this)
{
	this->unk_0x7C = 0;
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

`DreamSys__func_59590` -- tier C (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059590`.

Tier-C placeholder. Known: `this->unk_0x7C = 0`, vtable slot
+0x124. `unk_0x7C` has no other writer and no reader anywhere in carved code -- not
even a clear at construction -- so nothing names it.
