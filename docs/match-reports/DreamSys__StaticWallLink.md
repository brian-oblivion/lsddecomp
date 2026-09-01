# DreamSys__StaticWallLink

**Unit:** DreamSys · **Size:** 25 instructions · **Status:** MATCHED (25/25 words)

## What it does

Vtable `+0x1C8`. If `unknwon_int_0x44 == 0`, tests for a static link at
`linkCoordinates`; if found (non-negative result), executes it and returns
`true`; otherwise returns `false`.

## The C

```c
bool DreamSys__StaticWallLink(DreamSys *this, PlayerSpawnPoint *currentPos)
{
	s32 result;

	if (this->unknwon_int_0x44 != 0)
		return false;
	result = TestForStaticLink(&this->linkCoordinates, currentPos, this->currentStage);
	if (result < 0)
		return false;
	ExecuteLink(this, result, 0xD, 1);
	return true;
}
```

This is the SAME function whose disassembly was read (but not itself
matched) in the previous round to derive `TestForStaticLink`'s
3-argument signature -- `currentPos` (this function's own second
parameter) flows through to `TestForStaticLink` untouched, which is why
that earlier reverse-engineering held up unchanged here.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
