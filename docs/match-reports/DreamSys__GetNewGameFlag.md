# DreamSys__GetNewGameFlag

> Renamed from `func_8005A344` on 2026-09-22 (tools/rename.py). Address 0x8005a344.

**Unit:** DreamSys · **Size:** 3 instructions · **Status:** MATCHED (3/3 words)

## What it does

Vtable `+0x1AC`. Getter for `unk_0x878` (the field `DreamSys__ClearNewGameFlag`, matched
in the same round, zeroes).

## The C

```c
s32 DreamSys__GetNewGameFlag(DreamSys *this)
{
	return this->unk_0x878;
}
```

Retyped the vtable field from `void *DreamSys__GetNewGameFlag;` to
`s32 (*DreamSys__GetNewGameFlag)(DreamSys *this);`.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).

## Naming

- **Tier B.** Getter counterpart of DreamSys__ClearNewGameFlag; same evidence.
