# DreamSys__SetSoundObj

> Renamed from `func_8005937C` on 2026-09-22 (tools/rename.py). Address 0x8005937c.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Setter for `unk_0x58`.

## The C

```c
void DreamSys__SetSoundObj(DreamSys *this, s32 value)
{
	this->unk_0x58 = value;
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

`DreamSys__SetSoundObj` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_8005937C`.

A pure setter for `soundObj`. The field's identity is the
evidence, and it is cross-unit, three ways:
  1. `FlushSoundCueSet(this->soundObj, this->soundCueSet)` -- that function is
     matched in src/PlacementGridVabSound.c with the signature
     `void FlushSoundCueSet(VabStreamObj *self, SoundCueSet *set)`.
  2. This unit reads the same field as an object with a vtable at offset 0 and
     calls +0x84 through it (`DreamSys__StopVoice`). `VabStreamObjMethods::slot84`
     is at the same offset with the same signature and is
     `VabStreamObj__StopVoice`; `DreamSys__StopVoice`'s
     `if (idx >= 0) slot0x84(obj, idx)` is the same guard-and-call shape as
     `FlushSoundCueSet`'s own `slot->index = self->methods->slot84(self,
     slot->index)`.
  3. +0x9C, called by `DreamSys__StartVoice` with the small signed values in
     `VOICE_PITCH_BY_SELECT`, is `VabStreamObjMethods::slot9C` ==
     `VabStreamObj__SetPitchOffset`.
Tier B rather than A only because the body alone shows a bare store; the
identification comes from other units.

## Proposed field names

**None.** Round 66 renamed 31 `struct DreamSys` / `DreamSysUnk5C` fields and 33
`D_8008xxxx` globals in this unit, and every one of them came back
compiler-confirmed unit-local: the renames went into the struct DEFINITIONS in
`include/DreamSys.h` and the only accessors the compiler then listed were in
`src/DreamSys.c` (FINISHING-PLAN track 3 step 3). So there is nothing here for
the head to apply by type scope at merge time.

Worth recording because it falsifies a plausible assumption rather than
confirming one: `DreamSys.h` is shared with four sibling units
(`class_3bb8c_k/t/r/o`), and a textual `grep` for `unk_0xA4` and `unk_0xA8`
finds hits in `class_3bb8c_t.c` and `class_3bb8c_k.c` that look exactly like
DreamSys accessors. They are fields of unrelated structs with the same
placeholder spelling -- the round-57 over-count, met again. The compiler said
so for free; the grep would have cost a revert.

The field names themselves, and the evidence for each, are in the commit
`round 66 (alpha): DreamSys field names` and in the struct comments they
replaced.
