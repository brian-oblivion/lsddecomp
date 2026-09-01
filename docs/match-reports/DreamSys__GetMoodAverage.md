# DreamSys__GetMoodAverage

**Unit:** DreamSys · **Size:** 31 instructions · **Status:** MATCHED (31/31 words)

## What it does

Already documented: `@brief Calculates the average point of a given
Contributor.` If `amountMoods != 0`, calls `CalcMoodAxis` for each axis
using `lastMood` as `lank`; otherwise just copies `lastMood` through
verbatim.

## The C

```c
void DreamSys__GetMoodAverage(DreamSys *this, MoodGraphContributor *layer, MoodGraphPoint *ret)
{
	if (layer->amountMoods != 0) {
		ret->axis.dynamic = CalcMoodAxis(layer->lastMood.axis.dynamic, layer->sumMoods.dynamic, layer->amountMoods);
		ret->axis.upper = CalcMoodAxis(layer->lastMood.axis.upper, layer->sumMoods.upper, layer->amountMoods);
	} else {
		ret->value = layer->lastMood.value;
	}
}
```

Matched on the first attempt -- `CalcMoodAxis` (matched earlier in this
same round) made this straightforward once its signature was solid.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
