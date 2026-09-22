# DreamSys__func_5938c

> Renamed from `func_8005938C` on 2026-09-22 (tools/rename.py). Address 0x8005938c.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Setter for `unk_0x64`.

## The C

```c
void DreamSys__func_5938c(DreamSys *this, s32 value)
{
	this->unk_0x64 = value;
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

`DreamSys__func_5938c` -- tier C (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005938C`.

Kept as a tier-C placeholder in the unit's existing
`Class__func_xxxxx` form. Known: a one-line setter, `this->unk_0x64 = value`,
vtable slot +0x114; `DreamSys__DreamSys` initializes the same field to 0. Nothing
in any carved unit ever READS `unk_0x64`, so there is nothing to name the setter
after. Name it when a reader turns up.
