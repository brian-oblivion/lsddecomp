# DreamSys__GetSetScreenShake

**Unit:** DreamSys · **Size:** 5 instructions · **Status:** MATCHED (5/5 words)

## What it does

Vtable `+0x19C`. Swaps `this->screenShakeOn` with `*value`: reads the old
value, writes the new one in, and writes the old value back out through
`value`.

## The C

```c
void DreamSys__GetSetScreenShake(DreamSys *this, bool *value)
{
	bool old;

	old = this->screenShakeOn;
	this->screenShakeOn = *value;
	*value = old;
}
```

Already forward-declared with this exact signature from an earlier round;
no header changes needed.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
