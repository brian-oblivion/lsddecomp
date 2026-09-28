# DreamSys__ResetFlashbackList

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Vtable `+0x21C`. Zeroes `amountFlashbacksAvailable`.

## The C

```c
void DreamSys__ResetFlashbackList(DreamSys *this)
{
	this->amountFlashbacksAvailable = 0;
}
```

Already forward-declared with this exact signature; only the vtable side
needed work -- split out of `unknown_functions_0x21c[3]` alongside
`DreamSys__SaveLinkSnapshot`/`DreamSys__RestoreLinkSnapshot` (named, not typed further; out of scope
this round) and a correction to the neighboring `func_228` field's comment
(see `DreamSys__GetSetConfigOption.md`).

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
