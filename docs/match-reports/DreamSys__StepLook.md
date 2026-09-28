# DreamSys__StepLook — MATCHED

> Renamed from `func_800597C0` on 2026-09-22 (tools/rename.py). Address 0x800597c0.

Round 2026-08-30, runner ALPHA, unit `DreamSys`. 21/21 words, full match.

## Source

```c
void DreamSys__StepLook(DreamSys *this)
{
	this->vt->DreamSys__StepLookOffset(this);
	this->vt->DreamSys__StepLookYaw(this);
}
```

## Derivation

Vtable slot `0x140`. Two straight-line, single-argument (`this` only)
virtual calls; the second reloads `this->vt` fresh instead of caching it in
a local (no shared pointer between the two calls, so there's nothing to
hoist here — contrast `DreamSys__SelectLookCallback`/`DreamSys__SelectMoveCallback` earlier this round,
where the SAME `vt` value feeds multiple slot reads).

## Proposed learning

None — clean first-attempt match, no residue.

## Naming

`DreamSys__StepLook` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_800597C0`.

Calls `DreamSys__StepLookOffset` then `DreamSys__StepLookYaw`
and nothing else. It is the function `DreamSys__SelectLookCallback(this, 1)` installs
in `callback_0x80`, so it is one of the two things that can run every tick.
Grouping the pair as "look" is the tier-B part: they are the same
spring-with-decay shape driven by two command fields that
`DreamSys__OnPadEvent` sets from four adjacent command codes, one of them
turning the object in yaw and the other tilting the height curve. That they are the
player's look controls is the obvious reading; it is not proven here.
