# DreamSys__RestorePreviousMoveMode

> Renamed from `func_8005A1A4` on 2026-09-22 (tools/rename.py). Address 0x8005a1a4.

**Unit:** DreamSys · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What it does

Vtable slot `+0x188` (third of the five-function run; see
`DreamSys__GetSetMoveMode.md`). Unconditionally copies `unk_0xB0` into `unk_0xAC`.

## The C

```c
void DreamSys__RestorePreviousMoveMode(DreamSys *this)
{
	this->unk_0xAC = this->unk_0xB0;
}
```

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

- **Tier B.** One-line: moveMode = previousMoveMode. moveMode also gates a `!= 4` family split reused across DreamSys__TickStaircaseYawPlus90..3 and the Try*Link family, consistent with a movement-mode concept; what value 4 specifically represents is not confirmed.
