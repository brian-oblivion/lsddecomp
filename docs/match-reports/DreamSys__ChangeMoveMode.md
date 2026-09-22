# DreamSys__ChangeMoveMode

> Renamed from `func_8005A184` on 2026-09-22 (tools/rename.py). Address 0x8005a184.

**Unit:** DreamSys · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

Vtable slot `+0x184` (second of the five-function run; see
`DreamSys__GetSetMoveMode.md`). If the new `value` differs from the current
`unk_0xAC`, copies the OLD `unk_0xAC` into `unk_0xB0` before overwriting
`unk_0xAC` with `value`. No-op (and no return value used) when equal.

## The C

```c
void DreamSys__ChangeMoveMode(DreamSys *this, s32 value)
{
	s32 old;

	old = this->unk_0xAC;
	if (old != value) {
		this->unk_0xB0 = old;
		this->unk_0xAC = value;
	}
}
```

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

- **Tier B.** Saves the OLD moveMode into previousMoveMode, then overwrites moveMode with the new value, only when the value actually differs -- the history-preserving sibling of DreamSys__GetSetMoveMode.
