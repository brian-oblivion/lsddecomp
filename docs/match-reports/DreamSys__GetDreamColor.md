# DreamSys__GetDreamColor

**Unit:** DreamSys · **Size:** 14 instructions · **Status:** MATCHED (14/14 words)

## What it does

Vtable `+0x200`. Computes the current overall mood point via
`this->vt->UpdateDreamChart(this, &local)`, then converts it to a color via
`CalcDreamColor(&local)`.

## The C

```c
DreamColors DreamSys__GetDreamColor(DreamSys *this)
{
	MoodGraphPoint local;

	this->vt->UpdateDreamChart(this, &local);
	return CalcDreamColor(&local);
}
```

`UpdateDreamChart` was already declared with the right signature; confirmed
at vtable `+0x1FC` via `tools/classtable.py gDreamSysMethods`
(`DreamSys__UpdateDreamChart`, `0x8005B420`, the function immediately
before `DreamSys__GetDreamColor` itself at `+0x200`).

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
