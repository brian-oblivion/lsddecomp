# func_8005AFD0

**Unit:** DreamSys · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What it does

Vtable `+0x1E0`. Getter for `currentStage`.

## The C

```c
s32 func_8005AFD0(DreamSys *this)
{
	return this->currentStage;
}
```

Split out of `unknown_functions_0x1d0[5]` (was the LAST word of that gap,
`+0x1E0`; resolved via `tools/classtable.py DREAMSYS_METHODS`, which
confirmed `DreamSys__ProcessChunkChange` immediately follows at `+0x1E4`,
matching the existing header entry exactly).

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
