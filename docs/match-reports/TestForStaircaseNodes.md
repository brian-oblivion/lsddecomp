# TestForStaircaseNodes

> Renamed from `Test4StaircaseNodes` on 2026-09-28 (tools/rename.py). Address 0x8005bfdc.

**Unit:** DreamSys · **Size:** 20 instructions · **Status:** MATCHED (20/20 words)

## What it does

A guarded variant of the `GetStaticSpawn`-wrapper family (see
`TestForStaticLink`, `TestForTunnelLinks`): only calls through when `arg2 ==
0`, forcing `stage` to the literal `0` and `flag` to `0` (not `1`, unlike
the other two wrappers) regardless of its own third argument. Returns `-1`
directly when `arg2 != 0`.

## The C

```c
s32 TestForStaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 arg2)
{
	if (arg2 == 0)
		return GetStaticSpawn(target, currentPos, 0, sStaircaseTriggersCount,
		                       sStaircaseTriggers, sStaircaseSpawns, 0);
	return -1;
}
```

## Residue: branch polarity had to match retail's fallthrough, not the "obvious" reading

First attempt wrote the guard the other way round --
`if (arg2 != 0) return -1; return GetStaticSpawn(...);` -- which reads
identically in plain English but compiled to a real size mismatch (the
`GetStaticSpawn` call became a forward branch target instead of the
fallthrough, adding an extra `j`). Retail's actual layout has the
`GetStaticSpawn` call as the BRANCH-TAKEN path and the `-1` return as the
fallthrough default, which only the form above reproduces. Same class of
residue as `DreamSys__GetSetFlashbackSession` from the previous round (arm order, not just
condition correctness) -- worth checking on every early-return-plus-call
function before trusting the "natural" phrasing.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* Called by DreamSys__TryStaircaseLink (round 2026-09-06) as (&this->linkCoordinates,
   currentPos, this->currentStage) -- same forwarding shape as
   Test4TunnelLinks/TestForStaticLink above. Defined later in this unit's own
   ROM order (`src/DreamSys.c`); this is a forward declaration for that
   earlier call site, not a cross-unit prototype. */
```

## History: track 12 (round 106, charlie), comments moved out of the source

The API documentation pass moved these comments here, verbatim (commit
`2aa001d93`); the source keeps the API doc and, where a C spelling needs
it, a one-line `MATCHING:` note.

From `include/dream_sys.h`:

```c
/* Called by DreamSys__TryStaircaseLink as (&self->linkCoordinates,
   currentPos, self->currentStage) -- same forwarding shape as
   TestForTunnelLinks/TestForStaticLink above. Defined after its caller. */
extern s32 TestForStaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage);
```
