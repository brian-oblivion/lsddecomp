# DreamSys__UpdateDreamChart -- MATCHED (48/48 words)

Unit: `DreamSys`. Round 2026-09-01-e (runner echo).

## Final C

```c
void DreamSys__UpdateDreamChart(DreamSys *this, MoodGraphPoint *ret)
{
	MoodGraphPoint area;
	MoodGraphPoint entity;

	this->vt->GetMoodAverage(this, &this->areaMoods, &area);
	this->vt->GetMoodAverage(this, &this->entityMoods, &entity);
	if (this->entityMoods.amountMoods == 0) {
		entity = area;
	}
	ret->axis.dynamic = (area.axis.dynamic + entity.axis.dynamic) / 2;
	ret->axis.upper = (area.axis.upper + entity.axis.upper) / 2;
}
```

## Derivation

Two local `MoodGraphPoint`s (2 bytes each, adjacent on the stack) are filled
by calling `this->vt->GetMoodAverage` (slot `+0x20C`, already matched as
`DreamSys__GetMoodAverage`) once for `areaMoods` and once for
`entityMoods`. If `entityMoods.amountMoods == 0` (no instance-level mood data
yet), the entity result is overwritten with the area result -- read directly
off `this->entityMoods.amountMoods`, which lines up with offset `0x160`
(`entityMoods` starts at `0x154`; `amountMoods` is `MoodGraphContributor`'s
last field at `+0xC`). Retail's per-axis `srl 31 / addu / sra 1` sequence is
GCC 2.6.3's ordinary signed-divide-by-2 codegen (round-toward-zero); writing
the plain `/2` reproduces it exactly, same as `func_800260A4`'s `%N` idiom
already documented for constant divisors.

Matched on the first attempt -- the only real work was resolving vtable slot
`+0x20C` (already done by a previous round) and reading which struct field
lands at `+0x160`.

## Proposed learning

None beyond what is already documented: this is another confirmation of the
existing "`x / N` for a compile-time-constant power-of-two divisor: just
write the plain operator" idiom, this time for signed divide-by-2 rather
than the mult/mfhi form for larger constants.
