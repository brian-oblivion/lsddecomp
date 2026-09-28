# DreamSys__GetCurrentDayAndYear

> Renamed from `func_8005A2E4` on 2026-09-22 (tools/rename.py). Address 0x8005a2e4.

**Unit:** DreamSys · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

Vtable `+0x1A0`. If `arg1` is non-NULL, writes `this->currentYear` through
it. Always returns `this->currentDay + 1` (the `+1` and the store both live
in delay slots, executing unconditionally).

## The C

```c
s32 DreamSys__GetCurrentDayAndYear(DreamSys *this, s32 *arg1)
{
	if (arg1 != NULL)
		*arg1 = this->currentYear;
	return this->currentDay + 1;
}
```

## Retype: `arg1` was `s32`, is now `s32 *`

The vtable field was previously typed `s32 (*DreamSys__GetCurrentDayAndYear)(DreamSys *this,
s32 arg1);` from an earlier round's note about the one known external call
site (`GameApplication`'s slot58, `src/GameApplicationFileResource.c`, calling
`this->vt->DreamSys__GetCurrentDayAndYear(this, 0)`). That call passes a literal `0`, which
is a valid null-pointer constant, so retyping to `s32 *` needed no change
there. The function's OWN body makes clear `arg1` is an output pointer
(`sw v0, 0x0($a1)`, not a value use), not a plain scalar.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).

## Naming

- **Tier A.** Returns currentDay+1, optionally writes currentYear through an output pointer -- both already-named fields, pure getter.
