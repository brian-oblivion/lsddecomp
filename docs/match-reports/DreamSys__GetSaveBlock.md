# DreamSys__GetSaveBlock

> Renamed from `func_8005A350` on 2026-09-22 (tools/rename.py). Address 0x8005a350.

**Unit:** DreamSys · **Size:** 6 instructions · **Status:** MATCHED (6/6 words)

## What it does

Vtable `+0x1B0`. If `arg1` is non-NULL, writes the literal `0x700` through
it. Always returns `&this->unknown_sdata_0x178` (the address computation is
in the `jr` delay slot, so it executes unconditionally on both paths).

## The C

```c
s32 *DreamSys__GetSaveBlock(DreamSys *this, s32 *arg1)
{
	if (arg1 != NULL)
		*arg1 = 0x700;
	return &this->unknown_sdata_0x178;
}
```

Retyped the vtable field from `void *DreamSys__GetSaveBlock;` to
`s32 *(*DreamSys__GetSaveBlock)(DreamSys *this, s32 *arg1);`.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
