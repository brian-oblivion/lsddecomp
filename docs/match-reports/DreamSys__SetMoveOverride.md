# DreamSys__SetMoveOverride

> Renamed from `func_80059148` on 2026-09-22 (tools/rename.py). Address 0x80059148.

**Unit:** DreamSys · **Size:** 27 instructions · **Status:** MATCHED (27/27 words)

## What it does

Sets `unk_0x6c` (note lowercase `c` -- easy to typo as `unk_0x6C` and get a
hard compile error, which is exactly what happened on the first pass here)
to `value`; if `value != 0`, calls `this->vt->DreamSys__GetSetMoveMode(this, 1)`
(resolved via `tools/classtable.py`, vtable `+0x180` -- see
`DreamSys__GetSetMoveMode.md`, matched in the same round), then, if `this->unk_0x884 !=
0`, forwards that SAME loaded value on to
`this->vt->Class6B5CC__UpdateRotation(this, 1, (void *)this->unk_0x884)`.

## The C

```c
void DreamSys__SetMoveOverride(DreamSys *this, s32 value)
{
	this->unk_0x6c = value;
	if (value != 0) {
		this->vt->DreamSys__GetSetMoveMode(this, 1);
		if (this->unk_0x884 != 0)
			this->vt->Class6B5CC__UpdateRotation(this, 1, (void *)this->unk_0x884);
	}
}
```

## Note: `Class6B5CC__UpdateRotation`'s third argument is a REUSED register, not a fresh load

Retail sets up `$a2` once (loading `this->unk_0x884` for the `beqz`
comparison) and never reloads it before the `jalr` -- the same register value
becomes the call's third argument. Writing the natural-looking C
(`this->unk_0x884 != 0` for the test, `(void *)this->unk_0x884` for the
argument) reproduces this via ordinary GCC 2.6.3 CSE at `-O2` -- no manual
temp variable needed, it just works because it's the identical expression
with no intervening store to `unk_0x884`.

## New field: `unk_0x884`

Split out of `unknown_values_0x880[12]` (offset `0x880`-`0x88C`, right
before `storedDay`). `unk_0x884` is nonzero-tested and forwarded, cast to
`void*` -- gate/handle semantics beyond that are unconfirmed.

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.

## Naming

`DreamSys__SetMoveOverride` -- tier B (round 66, runner alpha, FINISHING-PLAN track 3).

Renamed from `func_80059148`.

Sets `moveOverride` and, when the new value is nonzero, forces
`moveMode` to 1 and applies `enterRotation` absolutely. "Override" is read off
`DreamSys__TickMove`, which dispatches on exactly this field: 0 runs the free
movement path (`DreamSys__ApplyPendingTurn` + `DreamSys__TickMoveFree`), 2 runs
`DreamSys__TickMoveHeld` (set the move command, do not move), anything else runs
`DreamSys__TickMoveForced` (force the move command to 1 first). So a nonzero value
replaces the normal per-tick movement with a forced variant -- which is what the
name says and all it says.
