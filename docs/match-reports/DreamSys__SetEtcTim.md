# DreamSys__SetEtcTim

> Renamed from `DreamSys__func_5938c` on 2026-09-26 (tools/rename.py). Address 0x8005938c.

> Renamed from `func_8005938C` on 2026-09-22 (tools/rename.py). Address 0x8005938c.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Setter for `unk_0x64`.

## The C

```c
void DreamSys__SetEtcTim(DreamSys *this, s32 value)
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
`#include "dream_sys.h"`, so every `DreamSys *` was a parse error). funcdiff's
STALE BUILD guard caught it. The numbers here are from the corrected pass --
see docs/DECOMPILATION_LEARNINGS.md on salvage splicing.

## Naming

`DreamSys__SetEtcTim` -- tier A (round 92, runner delta, FINISHING-PLAN track 7).
Renamed from `DreamSys__func_5938c` (tier C since round 66).

A pure setter (`this->etcTim = value`, vtable slot +0x114), so its mechanics are
its purpose; what was missing in round 66 was a name for the field. The one
caller, `DayTask__DayTask` (src/world/dream_day.c), passes `self->etcTim` --
the `TimImage` it has just loaded, uploaded and freed the buffer of -- right after
handing the DreamSys its sound object through `setSoundObj`. The field
(`unk_0x64` until this round, renamed `etcTim` in include/dream_sys.h; its only
accessors are this setter and `DreamSys__DreamSys`, which clears it) is typed
`s32` because nothing in the DreamSys ever dereferences it. Slot +0x114 is still
`slot114`: its accessor is in src/world/dream_day.c, outside this job, so the slot
name `setEtcTim` is a proposal for the head.
