# DreamSys__ClearNewGameFlag

> Renamed from `func_8005A33C` on 2026-09-22 (tools/rename.py). Address 0x8005a33c.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Vtable `+0x1A8`. Unconditionally zeroes `unk_0x878`.

## The C

```c
void DreamSys__ClearNewGameFlag(DreamSys *this)
{
	this->unk_0x878 = 0;
}
```

Retyped the vtable field from `void *DreamSys__ClearNewGameFlag;` to
`void (*DreamSys__ClearNewGameFlag)(DreamSys *this);`.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
