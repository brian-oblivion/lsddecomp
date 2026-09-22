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
hoist here — contrast `DreamSys__SelectCallback80`/`DreamSys__SelectCallback98` earlier this round,
where the SAME `vt` value feeds multiple slot reads).

## Proposed learning

None — clean first-attempt match, no residue.
