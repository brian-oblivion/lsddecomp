# TestForTunnelLinks

> Renamed from `Test4TunnelLinks` on 2026-09-28 (tools/rename.py). Address 0x8005bcf8.

**Unit:** DreamSys · **Size:** 17 instructions · **Status:** MATCHED (17/17 words)

## What it does

Same forwarding-wrapper shape as `TestForStaticLink` (matched round
2026-08-30-c): passes its own three arguments straight through to
`GetStaticSpawn`, appending a fixed trailing quadruple identifying a
different table triple (tunnel links instead of permalinks).

## The C

```c
s32 TestForTunnelLinks(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	return GetStaticSpawn(target, currentPos, stage, sTunnelTriggersCount,
	                       sTunnelTriggers, sTunnelSpawns, 1);
}
```

## A stale `build/lsdde.map` cost a real detour here

The first attempt (plain symbol references, shown above) scored 16/17 with
a `WARNING: the build differs OUTSIDE this range too` -- textbook
size-drift shape. Checking `build/lsdde.map` for the three new symbols
showed them ALL 4 bytes past their own name (`sTunnelTriggers` linked at
`0x80088984`), matching the exact `sTurnRotationYaw[-1] == sTurnRotations` pattern
already documented for a different data slot. That pattern looked like a
strong match, so the natural fix was `&sTunnelTriggers[-1]` etc. -- which made
the score WORSE (14/17, wrong direction). The `.map` being consulted was
stale: an earlier attempt in this same round had failed to COMPILE (not
just SHA-mismatched), so the link step never ran and the `.map` on disk was
left over from an older, unrelated build. A fresh `./build-and-verify.sh`
after fixing the actual compile error showed all three symbols at their
literal name-address with no offset, and the plain unindexed form (shown
above) matched immediately.

### Proposed learning

`build/lsdde.map` is a build ARTIFACT, not a live view of anything --
reading it after a build that failed to LINK (compile error, not just a
SHA mismatch) hands you numbers from whatever the PREVIOUS successful link
produced, silently. Confirm the map is fresh (a clean, just-completed
`build exit=0` from THIS exact source, or at minimum a build that reached
the link step) before trusting any address it reports, especially before
concluding a new symbol needs index/offset correction based on it.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
