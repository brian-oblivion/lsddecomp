# TestForInstantTeleporters — MATCHED 20/20

> Renamed from `Test4InstantTeleporters` on 2026-09-28 (tools/rename.py). Address 0x8005bf74.

**Unit:** DreamSys · **Size:** 20 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (`gInstantTeleportersEnabled`). Round 42
RESOLVED that blocker. Rebuilt fresh this round; took three attempts to
recover retail's exact if/else block ORDER (values were right from the
first attempt, layout was not).

## What it does

`if (gInstantTeleportersEnabled == 0) return -1; else return GetStaticSpawn(target,
currentPos, stage, sTeleportTriggersCount, sTeleportTriggers, sTeleportSpawns, 0);` -- same
forwarding shape as `TestForStaticLink`/`TestForTunnelLinks`/
`Test4StaircaseNodes` a few hundred lines above/below in this unit, using a
dedicated table triple (`sTeleportTriggersCount`/`sTeleportTriggers`/`sTeleportSpawns`) and trailing
flag `0` instead of `1`.

## Attempts 1-2: right values, wrong block order (2/20, then 0/20, both with 131461 bytes of whole-image drift)

```c
/* attempt 1 */
if (gInstantTeleportersEnabled == 0)
	return -1;
return GetStaticSpawn(target, currentPos, stage, sTeleportTriggersCount, sTeleportTriggers, sTeleportSpawns, 0);

/* attempt 2 (same compiled result as attempt 1 -- -O2 normalizes early-return
   to if/else) */
s32 result;
if (gInstantTeleportersEnabled != 0) {
	result = GetStaticSpawn(...);
} else {
	result = -1;
}
return result;

/* attempt 2b: hoisting the default before the branch, still wrong order */
result = -1;
if (gInstantTeleportersEnabled != 0) {
	result = GetStaticSpawn(...);
}
return result;
```

All three compile the CALL block first (immediately after the branch) and
the `-1` case second, which forces an extra unconditional `j`+delay-`nop`
pair after the call to skip over the `-1` case and reach the shared
epilogue -- retail has no such pair. That extra pair is exactly one word,
which is why every one of these attempts is 21 words long (0x54 bytes)
instead of retail's 20 (0x50) and reports 131461 bytes of drift across the
rest of the image (`build/lsdde.map` confirms: built `GetTeleportTimeBonus` lands
at `0x8005BFC8`, four bytes past retail's `0x8005BFC4`).

Retail's OWN layout is the opposite: the `-1` case comes FIRST in memory
(right after the initial `bnez`, itself in the branch's own fallthrough),
and the CALL case comes LAST, immediately before the epilogue -- so the
`-1` path needs the jump-over instead, and the call path falls straight
into the epilogue with no jump at all. Net instruction count: identical
either way, but only ONE of the two shapes reproduces retail's SPECIFIC
20-word total (the other needs an extra pair because in that arrangement
neither path can reach the shared tail for free).

## Final body: condition written as `== 0` first, `else` second (20/20)

```c
s32 TestForInstantTeleporters(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage)
{
	s32 result;

	if (gInstantTeleportersEnabled == 0) {
		result = -1;
	} else {
		result = GetStaticSpawn(target, currentPos, stage, sTeleportTriggersCount,
		                         sTeleportTriggers, sTeleportSpawns, 0);
	}
	return result;
}
```

Writing the zero-check as the `if` (not the `else`) put GCC's `bnez`-to-else
layout in the SAME order as retail: test, branch-if-nonzero to the call
block (placed last), fall through when zero to set `result = -1` and jump
to the shared tail. Byte-exact.

Added the table triple as new externs (same pattern as the two existing
triples for `TestForTunnelLinks`/`Test4StaircaseNodes`):

```c
extern s8 sTeleportTriggersCount[];
extern StaticLinkTrigger* sTeleportTriggers[];
extern StageSpawn* sTeleportSpawns[];
```

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py TestForInstantTeleporters` -> `20/20 words match`.

### Proposed learning

For an `if (cond) A; else B;` where retail's disassembly shows the SECOND
source block (`B`/`else`) falling straight into the function's shared
epilogue/tail with no jump, and the FIRST block instead carrying the
jump-over -- that is backwards from GCC's default placement (which keeps
source order: `A` first with a jump-over, `B` last falling through). Writing
the condition on whichever branch should compile FIRST-with-jump-over (i.e.
possibly negating the natural-reading condition) recovers retail's word
count; the "obvious" condition polarity can cost exactly one word (an extra
`j`/`nop` pair) with byte-identical VALUES but wrong LAYOUT, and this is
invisible from the values alone -- only the address-drift warning
(`build-and-verify.sh` / `funcdiff.py`'s out-of-range byte count) catches it,
since a length mismatch inside a single function still passes a naive
"does the value match" check on every earlier function.

## Provenance

round 43, runner ALPHA, unit DreamSys.

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* Same (target, currentPos, stage) forwarding shape as Test4TunnelLinks
   above (see that function's own comment) -- called by DreamSys__TryInstantTeleportLink as
   (&this->linkCoordinates, currentPos, this->currentStage), result compared
   with `bltz` (round 2026-09-02). MATCHED, defined later in this unit's own
   ROM order -- forward declaration only (gp-relative blocker resolved; see
   docs/match-reports/Test4InstantTeleporters.md). */
```
