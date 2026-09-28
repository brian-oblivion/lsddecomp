# GetStaticSpawn — MATCHED 79/79

**Unit:** DreamSys · **Size:** 79 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (6 references, the first to
`gLinkSrcStage`). Round 42 RESOLVED that blocker. Attempted fresh this round
(a stretch pick beyond the assigned queue).

## What it does

The shared "static link" lookup used by
`TestForStaticLink`/`TestForTunnelLinks`/`TestForStaircaseNodes`/
`TestForInstantTeleporters` (all already matched or forwarding wrappers):
scans `triggers[stage][0..triggerLens[stage])` for an entry whose `chunk`
matches `currentPos->chunk` and whose `tile` either matches
`currentPos->tile` or is a wildcard (negative `tile.value`); on a match,
publishes several `D_8008ACxx` globals, copies the resolved `StageSpawn`
entry's grid position into `*target`, looks up its world-space adjustment
from `sSpawnPosAdjust`, optionally marks a nav-challenge byte complete, and
returns the matched trigger's `stage` byte. Returns `-1` on no match or an
empty trigger list.

## Attempt 1: correct algorithm, one word too long (22/79, ~130KB drift)

The first translation read `trig->stage` and `trig->spawnpointIndex` TWICE
each -- once to store into `sLinkDstStage`/`sLinkSpawnIndex` and again to index
`spawns[trig->stage][trig->spawnpointIndex]` -- as two textually separate
expressions. GCC 2.6.3 did not common-subexpression-eliminate the second
read of `trig->stage` (it kept the first in a register but re-issued an
`lb` for the second use), costing exactly one extra instruction and
shifting the whole function 4 bytes long, which in turn cascaded into every
absolute-address (`lui`/`addiu`) reference after it -- what looked like a
second bug (`sSpawnPosAdjust`'s apparent offset changing in the diff) was
purely a symptom of this function's own length being wrong, not a separate
issue.

**Fix:** introduce explicit locals (`triggerStage`, `spawnIndex`) computed
ONCE and reused for both the `D_8008ACxx` store and the indexing
expression -- the same "read a field once, reuse the register" idiom
documented elsewhere in this project, now confirmed inside a loop body with
two independently-reused fields.

## Attempt 2: length correct, two signed/unsigned mismatches (77/79)

Two residues, both the same family as `CalcNavigationScore`'s `slt`/`sltu`
lesson from earlier this round, but in the OPPOSITE direction (signed
declared type, unsigned retail load) for two different byte fields:

1. `triggerLens[stage]` (declared `s8 *`, matching the shared prototype
   other callers already rely on) -- retail reads it with `lbu`, not `lb`.
   Casting the pointer at the READ site only (`*(u8 *)&triggerLens[stage]`,
   assigned into a plain `s32 count`) reproduces the `lbu` without touching
   the shared parameter's signed type.
2. `StaticLinkTrigger::spawnpointIndex` (declared `s8`, matching the
   already-established struct) -- same fix, `*(u8 *)&trig->spawnpointIndex`.

Both times, the FIELD's own declared type stayed `s8` (correct, and shared
with other already-matched code); only the specific READ that retail widens
unsigned needed an explicit `u8 *` cast, exactly like
`sStageSpawnPointsCount`'s existing "retyped u8 because retail reads it with
`lbu`" precedent -- except here the retype would have broken other callers,
so the cast is local to this one read instead of on the declaration.

Getting the `lbu` read right for `count` ALSO fixed a third symptom for
free: retail's redundant loop-entry guard (`beqz $t1,...` twice, both an
EXACT-zero test) versus this build's second guard compiling to `blez`
(`<=0`, a SIGNED test) -- once `count` naturally participates in unsigned
comparisons implied by the `u8`-sourced value, `count == 0` and the
`for`-loop's own zero-trip-count guard both compile as the same `beqz`/`bnez`
shape retail uses.

## Final body (79/79)

```c
s32 GetStaticSpawn(PlayerSpawnPoint *target, PlayerSpawnPoint *currentPos, s32 stage,
                    s8 *triggerLens, StaticLinkTrigger **triggers, StageSpawn **spawns, s32 flag)
{
	s32 count;
	StaticLinkTrigger *trig;
	s32 i;
	StageSpawn *entry;
	s32 triggerStage;
	u32 spawnIndex;

	count = *(u8 *)&triggerLens[stage];
	if (count == 0)
		return -1;

	trig = triggers[stage];
	for (i = 0; i < count; i++, trig++) {
		if (*(s16 *)&currentPos->chunk != *(s16 *)&trig->chunk)
			continue;
		if (*(s16 *)&currentPos->tile != trig->tile.value && trig->tile.value >= 0)
			continue;

		sLinkSrcStage = stage;
		sLinkTriggerIndex = i;
		triggerStage = trig->stage;
		sLinkDstStage = triggerStage;
		spawnIndex = *(u8 *)&trig->spawnpointIndex;
		entry = &spawns[triggerStage][spawnIndex];
		sLinkSpawnIndex = spawnIndex;
		*(PlayerSpawnGridPos *)target = *(PlayerSpawnGridPos *)entry;
		target->position = sSpawnPosAdjust[entry->adjustment];
		if (flag != 0)
			(*gpNavChallengesComplete)[entry->extra] = 1;
		return sLinkDstStage;
	}
	return -1;
}
```

The two 4-byte-and-6-byte whole-struct copies (`*(PlayerSpawnGridPos *)target
= ...` and `target->position = sSpawnPosAdjust[...]`) both reproduce
retail's `lwl`/`lwr` + `swl`/`swr` unaligned copy idiom automatically -- no
special handling needed, consistent with the already-documented
"alignment-2 struct assignment compiles to `lwl`/`lwr`+`swl`/`swr`" pattern
(CLAUDE.md).

`sLinkTriggerIndex` was already declared in `include/DreamSys.h` from earlier
rounds' call-site analysis; no header changes needed.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py GetStaticSpawn` -> `79/79 words match`.

### Proposed learning

Two independent lessons, both worth generalizing:

1. **Re-reading the SAME struct field twice in source (even for two
   genuinely different purposes) can cost a real instruction if GCC 2.6.3
   doesn't CSE it** -- when a field feeds both a "publish to a global" store
   and an indexing expression, compute it into an explicit local ONCE and
   reuse that local for both, rather than writing `x->field` twice and
   trusting the compiler to notice they're identical.
2. **A shared field/parameter's OWN declared signedness should stay
   whatever its other call sites need -- fix an unsigned-load mismatch with
   a `*(u8 *)&expr` cast at the ONE read site that needs it**, not by
   changing the field or parameter's type, which would just move the
   mismatch to a different caller.

## Provenance

round 43, runner ALPHA, unit DreamSys (stretch pick beyond the assigned
queue).

## History (moved from include/DreamSys.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
/* Shared by TestForStaticLink/Test4TunnelLinks/Test4StaircaseNodes/
   Test4InstantTeleporters, each of which forwards its own three args
   straight through and appends a fixed trailing quadruple (length table,
   trigger table, spawn table, literal 1). Defined later in this unit's own
   ROM order; this is a forward declaration for the earlier call sites
   above, not a cross-unit prototype. MATCHED (the gp-relative/addiu_at
   blockers this was once filed under are resolved, see CLAUDE.md); return
   type is confirmed s32 by every call site's `bltz` check, not just a guess
   -- CLAUDE.md's tail-call-wrapper warning no longer applies once a
   function is its own real C body, only while it is still INCLUDE_ASM
   (round 2026-08-30-c note superseded). */
```
