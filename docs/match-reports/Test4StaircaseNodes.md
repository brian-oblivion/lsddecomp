# Test4StaircaseNodes

**Unit:** DreamSys · **Size:** 20 instructions · **Status:** MATCHED (20/20 words)

## What it does

A guarded variant of the `GetStaticSpawn`-wrapper family (see
`TestForStaticLink`, `Test4TunnelLinks`): only calls through when `arg2 ==
0`, forcing `stage` to the literal `0` and `flag` to `0` (not `1`, unlike
the other two wrappers) regardless of its own third argument. Returns `-1`
directly when `arg2 != 0`.

## The C

```c
s32 Test4StaircaseNodes(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 arg2)
{
	if (arg2 == 0)
		return GetStaticSpawn(target, currentPos, 0, D_80088CBC,
		                       D_80088C4C, D_80088BA4, 0);
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
