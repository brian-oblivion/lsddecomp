# CalcMoodAxis

**Unit:** DreamSys · **Size:** 29 instructions · **Status:** MATCHED (29/29 words)

## What it does

Already documented in the header:
`@brief Turns the values of a given mood contribution axis into an useable
average` / `@return Normalized integer between -9 and 9`. Computes
`sum/amount + lank/3`, then clamps.

## The C

```c
s32 CalcMoodAxis(s32 lank, s32 sum, s32 amount)
{
	s32 result;

	result = sum / amount;
	result += lank / 3;
	if (result >= 10)
		result = -9;
	else if (result < -9)
		result = 9;
	return result;
}
```

## Note: the clamp looks inverted, and it matched byte-exact as written

`if (result >= 10) result = -9;` and `else if (result < -9) result = 9;`
read backwards at first glance -- an "in-range [-9,9]" clamp would normally
saturate a too-high value UP to +9, not down to -9. This is exactly what
retail's own disassembly shows, though, and it matched on the first
attempt with no reshaping. Not "fixed" here; transcribed as observed. If
this genuinely is a shipped quirk (rather than a misreading that happens to
produce identical bytes some other way), it's worth flagging for whoever
next touches `DreamSys__GetMoodAverage`'s callers or the mood-graph
gameplay logic -- a value pushed to +10 becomes -9 fully across the axis,
not just capped.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
