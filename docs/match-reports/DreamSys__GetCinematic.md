# DreamSys__GetCinematic

**Unit:** DreamSys · **Size:** 8 instructions · **Status:** MATCHED (8/8 words)

## What it does

Vtable `+0x1BC`. Returns `this->nextCinematic` by value.

## The C

```c
CinematicCall DreamSys__GetCinematic(DreamSys *this)
{
	return this->nextCinematic;
}
```

## Note: the disassembly's own parameter registers are misleading here

`CinematicCall` is a 4-byte aggregate (`{s16 bank; s16 entry;}`), and GCC
2.6.3's o32 ABI returns aggregates through a HIDDEN pointer parameter in
`$a0`, shifting the function's own declared parameters to `$a1` onward. The
raw asm shows `$a0` used as the copy DESTINATION and `$a1` (misaligned,
hence `lwl`/`lwr`) as the source -- i.e. `$a1` is really `this`, not `$a0`.
Do not write the hidden pointer yourself; `return this->nextCinematic;`
lets GCC generate the whole calling-convention dance, matching byte for
byte.

## Provenance

round 2026-08-30-c, runner ALPHA, unit DreamSys (whole-unit second pass).
