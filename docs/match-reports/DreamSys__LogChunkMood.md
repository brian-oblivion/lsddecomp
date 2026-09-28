# DreamSys__LogChunkMood

**Unit:** DreamSys · **Size:** 19 instructions · **Status:** MATCHED (19/19 words)

## What it does

Vtable `+0x1F4`. Looks up the mood for the chunk at `currentPos` on the
current stage (`GetMoodFromStageChunk`, from `stage_grid.h`), then logs it
into `this->areaMoods` via the already-established `vt->LogMood` shape.

## The C

```c
void DreamSys__LogChunkMood(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	MoodGraphPoint *mood;

	mood = GetMoodFromStageChunk(this->currentStage, (StageChunk *)currentPos);
	this->vt->LogMood(this, &this->areaMoods, mood);
}
```

`GetMoodFromStageChunk`'s second parameter is `StageChunk *`
(`{s8 column; s8 row;}`), which is byte-layout-identical to
`PlayerSpawnPoint`'s own leading `MapChunk` member -- the cast is exact,
not approximate. Added `#include "stage_grid.h"` to `dream_sys.h` for this
(the only cross-unit include this unit needed this round).

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
