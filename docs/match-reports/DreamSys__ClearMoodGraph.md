# DreamSys__ClearMoodGraph

**Unit:** DreamSys · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

## What it does

Vtable `+0x204`. Zeroes all four fields of a `MoodGraphContributor`.
`this` is unused in the body (`contributor` alone carries everything
needed) -- normal for a "virtual" method whose slot signature still
includes `this` for the vtable-call ABI.

## The C

```c
void DreamSys__ClearMoodGraph(DreamSys *this, MoodGraphContributor *contributor)
{
	contributor->lastMood.value = 0;
	contributor->sumMoods.upper = 0;
	contributor->sumMoods.dynamic = 0;
	contributor->amountMoods = 0;
}
```

Store order matters and matches retail exactly: `value`, then `upper`, then
`dynamic`, then `amountMoods` (the last store lives in the `jr` delay slot).
Not source-field order (`dynamic` comes before `upper` in the struct) --
this is scheduling, and writing the statements in retail's actual store
order reproduced it directly on the first try.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
