# DreamSys__SetTickPeriod

> Renamed from `func_8005A1EC` on 2026-09-22 (tools/rename.py). Address 0x8005a1ec.

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Vtable slot `+0x190` (fifth and last of the five-function run; see
`DreamSys__GetSetMoveMode.md`). Plain setter: `this->unk_0x120 = value`. `unk_0x120`
was already named (divisor for `DreamSys__UpdateTickState`'s `dreamTimer %
unk_0x120` check).

## The C

```c
void DreamSys__SetTickPeriod(DreamSys *this, s32 value)
{
	this->unk_0x120 = value;
}
```

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.
