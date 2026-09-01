# DreamSys__InitSpawnLoc

**Unit:** DreamSys · **Size:** 30 instructions · **Status:** MATCHED (30/30 words)

## What it does

Vtable `+0x1C0`. Gets the previous day's mood, generates an initial spawn
from it, stores the resulting stage, applies the returned time limit via
`GetSetDreamTimeLimit`, and sets `unknwon_int_0x44 = 0xB`.

## The C

```c
void DreamSys__InitSpawnLoc(DreamSys *this)
{
	MoodGraphPoint mood;
	s32 timeLimit;

	this->vt->GetPreviousDayMood(this, &mood, 1);
	this->currentStage = GenerateInitialSpawn(&this->linkCoordinates, &timeLimit, &mood, this->currentDay);
	timeLimit = this->vt->GetSetDreamTimeLimit(this, timeLimit);
	this->unknwon_int_0x44 = 0xB;
}
```

## Residue: `GetSetDreamTimeLimit`'s return value is stored back into the SAME stack slot it read its argument from

First attempt used a separate, never-read `unused` local for the discarded
return value. GCC 2.6.3 dead-store-eliminated the write entirely (one word
short); marking it `volatile` "fixed" the missing store but bloated the
prologue with an extra saved register (a much worse diff). The actual
retail shape reuses `timeLimit`'s own stack slot for the return -- i.e. the
real source is `timeLimit = this->vt->GetSetDreamTimeLimit(this,
timeLimit);`, not a fresh discard. `GetSetDreamTimeLimit` returns the
PREVIOUS time limit (its own doc comment), so this reads as "restore
whatever time limit was already set" -- a real (if oddly effect-free at
runtime) statement, not dead code needing a `volatile` workaround.

### Proposed learning

Before reaching for `volatile` to keep a call's return value from being
eliminated, check whether the value is meant to overwrite a variable
that's STILL LIVE nearby (especially one just used as an argument to the
same call) rather than a fresh, genuinely-unused local. Reusing the real
variable reproduces retail's stack layout exactly and avoids `volatile`'s
side effect of forcing extra register pressure.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
