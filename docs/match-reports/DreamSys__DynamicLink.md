# DreamSys__DynamicLink

**Unit:** DreamSys · **Size:** 22 instructions · **Status:** MATCHED (22/22 words)

## What it does

Vtable `+0x1C4`. If `unknwon_int_0x44 == 0`, finds a random spawn on the
current stage and executes a dynamic link to it.

## The C

```c
void DreamSys__DynamicLink(DreamSys *this)
{
	s32 stage;

	if (this->unknwon_int_0x44 == 0) {
		stage = GetRandomSpawnFromStage(&this->linkCoordinates, this->currentStage, this->dreamTimer);
		ExecuteLink(this, stage, 0xC, 1);
	}
}
```

Same shape as `DreamSys__StaticWallLink` (matched this round): guard on
`unknwon_int_0x44`, spawn lookup, `ExecuteLink` with a link-type literal
(`0xC` here, `0xD` there) and a fixed final argument of `1`.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
