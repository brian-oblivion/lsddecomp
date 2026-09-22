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
`this->vt->func_8001CEB4(this, 1, (void *)this->unk_0x884)`.

## The C

```c
void DreamSys__SetMoveOverride(DreamSys *this, s32 value)
{
	this->unk_0x6c = value;
	if (value != 0) {
		this->vt->DreamSys__GetSetMoveMode(this, 1);
		if (this->unk_0x884 != 0)
			this->vt->func_8001CEB4(this, 1, (void *)this->unk_0x884);
	}
}
```

## Note: `func_8001CEB4`'s third argument is a REUSED register, not a fresh load

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
