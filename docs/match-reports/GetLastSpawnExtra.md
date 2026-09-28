# GetLastSpawnExtra — MATCHED 14/14

> Renamed from `func_8005C118` on 2026-09-22 (tools/rename.py). Address 0x8005c118.

**Unit:** DreamSys · **Size:** 14 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on BOTH `gp_rel` (two references,
`gLinkDstStage`/`gLinkSpawnIndex`) AND `addiu_at` (one runtime-indexed global load
through `STAIRCASE_SPAWNS`). Round 21 resolved `addiu_at`; round 42 resolved
`gp_rel` and `nop_mflo_mfhi`. With all three flags now in the Makefile, this
function matched on the first rebuild.

## What it does

A single array-of-array index: `sStaircaseSpawns[sLinkDstStage]` is a `StageSpawn*`
(one of a table of per-stage spawn-point arrays, same table
`TestForStaircaseNodes` already uses a few lines above), indexed a second time
by `sLinkSpawnIndex`, reading that entry's `.extra` byte (`StageSpawn`'s last
field, a signed byte at offset 5 -- matches the retail `lb` at `+0x5`).

## Final body

```c
s32 GetLastSpawnExtra(void)
{
	return sStaircaseSpawns[sLinkDstStage][sLinkSpawnIndex].extra;
}
```

`sStaircaseSpawns`, `sLinkDstStage`, `sLinkSpawnIndex` were all already declared in
`include/DreamSys.h`. Updated the stale header comment on the
`extern s32 GetLastSpawnExtra(void);` prototype (used to type its still-
`INCLUDE_ASM` caller `DreamSys__TryStaircaseLink`, same unit) from "blocked by both
gp-relative and addiu_at" to MATCHED.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py GetLastSpawnExtra` -> `14/14 words match`.

## Provenance

round 43, runner ALPHA, unit DreamSys.

## Naming

- **Tier A.** Pure leaf: returns the already-named `.extra` field of the last static spawn point found via the sLinkDstStage/C8 scratch indices; its return value indexes the DreamSys__TickStaircaseYawPlus90..3 dispatch table.

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* Called by DreamSys__TryStaircaseLink with NO explicit argument setup (the disassembly's
   call site leaves `$a0` holding an unrelated leftover value from the
   preceding statement, same "empty delay slot, no a0-a3 setup" shape as
   GetStageLinkAngle above); return value used as STAIRCASE_TICK_FNS's index. MATCHED
   round 43 (2026-09-15) -- both the gp-relative and addiu_at blockers it was
   filed under are resolved (see docs/research/gp-relative-blocker.md and
   docs/research/addiu-at-blocker.md), and the one-line body
   `STAIRCASE_SPAWNS[gLinkDstStage][gLinkSpawnIndex].extra` matched on the first rebuild
   (docs/match-reports/GetLastSpawnExtra.md). Still declared here to type
   DreamSys__TryStaircaseLink's call site, which remains INCLUDE_ASM in this unit. */
```
