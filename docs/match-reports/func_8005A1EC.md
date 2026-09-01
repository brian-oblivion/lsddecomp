# func_8005A1EC

**Unit:** DreamSys · **Size:** 2 instructions · **Status:** MATCHED (2/2 words)

## What it does

Vtable slot `+0x190` (fifth and last of the five-function run; see
`func_8005A168.md`). Plain setter: `this->unk_0x120 = value`. `unk_0x120`
was already named (divisor for `func_80059394`'s `dreamTimer %
unk_0x120` check).

## The C

```c
void func_8005A1EC(DreamSys *this, s32 value)
{
	this->unk_0x120 = value;
}
```

## Provenance

round 2026-08-30-b, runner ALPHA, address range
`0x80058774`-`0x8005A1EC`.
