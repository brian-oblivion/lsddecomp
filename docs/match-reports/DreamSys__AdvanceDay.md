# DreamSys__AdvanceDay

**Unit:** DreamSys · **Size:** 14 instructions · **Status:** MATCHED (14/14 words)

## What it does

Vtable `+0x1A4`. Increments `currentDay`; if it reaches 365 (`0x16D`), rolls
over to 0 and increments `currentYear`. Returns the (possibly rolled-over)
new `currentDay`.

## The C

```c
s32 DreamSys__AdvanceDay(DreamSys *this)
{
	this->currentDay++;
	if (this->currentDay >= 0x16D) {
		this->currentDay = 0;
		this->currentYear++;
	}
	return this->currentDay;
}
```

Store order inside the rollover branch matters and matches retail exactly:
`currentDay = 0` before `currentYear++`, even though retail loads the OLD
`currentYear` for the increment before storing the new `currentDay` -- the
read/write scheduling sorted itself out without any extra reshaping.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
