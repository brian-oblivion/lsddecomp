# DreamSys__UpdateDreamChart

**Unit:** DreamSys · **Size:** 48 words · **Status:** MATCHED (48/48)

## What it does

Already forward-declared (`void DreamSys__UpdateDreamChart(DreamSys *this,
MoodGraphPoint *ret);`). Averages both mood contributors, falling back to
the area average for the entity side when no entity samples have been
logged yet, then averages the two per-axis with rounding toward zero.

## The C

```c
void DreamSys__UpdateDreamChart(DreamSys *this, MoodGraphPoint *ret)
{
	MoodGraphPoint areaAvg;
	MoodGraphPoint entityAvg;

	this->vt->GetMoodAverage(this, &this->areaMoods, &areaAvg);
	this->vt->GetMoodAverage(this, &this->entityMoods, &entityAvg);
	if (this->entityMoods.amountMoods == 0) {
		entityAvg.value = areaAvg.value;
	}
	ret->axis.dynamic = (areaAvg.axis.dynamic + entityAvg.axis.dynamic) / 2;
	ret->axis.upper = (areaAvg.axis.upper + entityAvg.axis.upper) / 2;
}
```

## Evidence

- `this->vt->GetMoodAverage`: already-established vtable slot `+0x20C`
  (`void (*GetMoodAverage)(DreamSys *this, MoodGraphContributor *layer,
  MoodGraphPoint *ret);`), immediately after `LogMood` (`+0x208`, used by
  `DreamSys__InitMoodContributors` this round) in the struct declaration --
  no new header work needed.
- `this->entityMoods.amountMoods`: offset `0x160` falls inside the already-
  named `entityMoods` field (`0x154`..`0x164`, a `MoodGraphContributor`);
  `0x160` is exactly `entityMoods`'s own `amountMoods` member
  (`lastMood`(2) + 2 pad + `sumMoods.dynamic`(4) + `sumMoods.upper`(4) =
  `0x160`). No new field -- the disassembly's raw offset resolves inside
  an existing struct member, not the top-level `DreamSys` layout.
- `x / 2` for the two rounding-toward-zero averages: the `srl`/`addu`/`sra`
  triplet is GCC 2.6.3's standard signed-division-by-a-power-of-2 sequence
  (extract sign bit, add it as a bias, arithmetic-shift) -- writing the
  plain `/` operator reproduces it directly, same as the already-documented
  `x % N` idiom in `DECOMPILATION_LEARNINGS.md`.
- The two stack temporaries (`areaAvg`/`entityAvg`, `MoodGraphPoint`, 2
  bytes each) are output buffers for the two `GetMoodAverage` calls;
  `sp+0x10`/`sp+0x12` map onto them directly by call order.

## Third-learning check (per head's request)

**Not needed.** `areaAvg` and `entityAvg` are plain stack locals (not
`self->field` reads), each written once by its own call and read
afterward with no aliasing ambiguity -- there is no memory location GCC
needs to doubt after a `jalr` here, since neither is reachable through any
pointer the callee could see. `this->entityMoods.amountMoods` is read
exactly once. No locals were needed beyond the two output buffers the
calls themselves require.

## Proposed learning

Confirms the existing signed-division-by-2 idiom generalizes cleanly to a
`u8`-per-byte (well, `s8`-per-byte) rounding-average computation, not just
whole-word arithmetic -- worth remembering next time a `srl ..,31` /
`addu` / `sra ..,1` triplet shows up in a byte-sized context.
