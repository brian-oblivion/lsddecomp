# DreamSys__CalcUnlockScore

**Unit:** DreamSys · **Size:** 27 instructions · **Status:** MATCHED (27/27 words)

## What it does

Vtable `+0x210`. Recomputes `navigationFlasbackUnlockScore` from
`CalcNavigationScore()`, clamps `instanceFlasbackUnlockScore` to
`[0, 50000000]`, and sums both into `totalFlasbackUnlockScore`.

## The C

```c
void DreamSys__CalcUnlockScore(DreamSys *this)
{
	this->navigationFlasbackUnlockScore = CalcNavigationScore();
	if (this->instanceFlasbackUnlockScore < 0) {
		this->instanceFlasbackUnlockScore = 0;
	} else if (this->instanceFlasbackUnlockScore > 50000000) {
		this->instanceFlasbackUnlockScore = 50000000;
	}
	this->totalFlasbackUnlockScore = this->navigationFlasbackUnlockScore + this->instanceFlasbackUnlockScore;
}
```

All three fields (`0x184`/`0x188`/`0x18C`) were already named from an
earlier round's struct work; the raw offsets in the disassembly resolved
directly, no new header fields needed.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
