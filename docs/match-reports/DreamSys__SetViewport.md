# DreamSys__SetViewport

> Renamed from `DreamSys__SetHeightCurve` on 2026-09-26 (tools/rename.py). Address 0x80059384.

> Renamed from `func_80059384` on 2026-09-22 (tools/rename.py). Address 0x80059384.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Pointer setter for `unk_0x5C`.

## The C

```c
void DreamSys__SetViewport(DreamSys *this, void *value)
{
	this->unk_0x5C = value;
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

`DreamSys__SetViewport` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059384`.

A pure setter for `heightCurve`. The name describes what the
pointed-to object is USED as here and nothing more: `DreamSys__ProjectPointAtDistance`
reads two `DreamSysInterpPoint`s from it (+0x14 and +0x20) and calls
`InterpolateKeyframeValue(a, b, dist)`, which linearly interpolates their `value`
fields against their `position` fields; the result becomes the Y of a world-space
point. So the object holds a height-versus-distance curve. `DreamSys__StepLookOffset`
offsets the far point's value (`endValue`) and `DreamSys__AdvanceMoveCycle` nudges
both by +-50.
Deliberately NOT called `SetCamera` or `SetView`: nothing establishes that the
object is a camera, only that this curve is read out of it.

## Track 4 (2026-09-26, round 88)

Renamed from `DreamSys__SetHeightCurve`, and the field it sets from
`heightCurve` (DreamSysUnk5C *) to `viewport` (struct Viewport *). The
object is a Viewport: Class865C8__Init (class_39e08) passes it the
New_Class869D8 it built (a Viewport subclass) and Class865C8__Deinit passes
NULL; Entity__MoodCue74 calls +0x064 of its table, Viewport's
setClearColor; and the offsets DreamSysUnk5C named are refView's (GsRVIEW2
at +0x014): +0x014/+0x020 the viewpoint and reference point
ProjectPointAtDistance interpolates between, +0x018/+0x024 their y words,
which AdvanceMoveCycle bobs and StepLookOffset/TickDrift/StopDrift move.
Byte-identical.
