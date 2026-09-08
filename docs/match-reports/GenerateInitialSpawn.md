# GenerateInitialSpawn — MATCHED, 107/107 words, byte-exact

Unit: `DreamSys` · Size: 107 words · Status: MATCH, whole-image SHA1 green
(`build exit=0`).

## Summary

```c
s32 GenerateInitialSpawn(PlayerSpawnPoint *dest, s32 *timeLimit, MoodGraphPoint *mood, s32 day)
{
	StageChunk chunk;
	s32 stage;
	s32 count;
	s32 i;
	StageSpawn *entry;

	stage = GetStageChunkFromMood(&chunk, mood);
	if (stage >= 0) {
		*timeLimit = STAGE_TIME_LIMITS[stage];

		count = LEN_STAGE_SPAWNPOINTS[stage];
		entry = STAGE_SPAWNPOINTS[stage];
		for (i = 0; i < count; i++, entry++) {
			if (*(s16 *)&chunk == *(s16 *)&entry->chunk)
				goto found;
		}
		entry = &STAGE_SPAWNPOINTS[stage][*(s16 *)&chunk % count];

	found:
		*(PlayerSpawnGridPos *)dest = *(PlayerSpawnGridPos *)entry;
		dest->position = SPAWN_POS_ADJUST[entry->adjustment];
		return stage;
	}

	stage = GetRandomSpawnFromStage(dest, stage, day);
	*timeLimit = STAGE_TIME_LIMITS[stage];
	return stage;
}
```

Looks up the stage chunk for a mood point; on success, finds the spawn point
in that stage matching the chunk (linear scan by treating the 2-byte
`{col,row}` pair as one `s16`), or falls back to `chunk-value % count` if no
exact match exists; on failure (`stage < 0`, no chunk assigned to this mood
point) picks a random spawn from a random stage instead. Both paths return
the chosen stage index and write the stage's time limit through `timeLimit`.

## Two header retypes, both scoped to fields this unit's queue exclusively reads

Both were `s8` and needed to be `u8` because retail reads them with `lbu`,
not `lb`. Checked before editing (`grep -rn` across `include/` and `src/`):
neither field nor array is read anywhere outside this one function, so
neither is a shared-header hazard in the sense the project's struct-edit
check exists to catch — no other already-matched function's codegen could
regress from either change, and no sibling unit includes `DreamSys.h` with
a second declaration of either name (`headercontention.py`-style check done
by hand via grep since these are plain externs, not a padded struct).

- **`include/DreamSys.h`: `extern s8 LEN_STAGE_SPAWNPOINTS[];` -> `u8`.**
  With `s8`, `count` (an `s32` sign-extended from the array) is not provably
  non-negative to the compiler, so the `for` loop's rotation into a
  do/while needs TWO guards (`beqz`+`blez`) to skip the body when count is
  `<= 0`. Retail has exactly one (`beqz`), which only suffices if the
  frontend already knows `count >= 0` -- i.e. the source array element type
  is unsigned. Symptom before the fix: an extra `nop`+`blez` pair (2 words)
  right after the `LEN_STAGE_SPAWNPOINTS[stage]` load, plus consequent
  address drift through the rest of the function.
- **`include/DreamSys.h`: `StageSpawn::adjustment` `s8` -> `u8`.** It indexes
  `SPAWN_POS_ADJUST` (a `struct RelativePos[]`, stride 6). With `s8`, the
  load is `lb`; retail's is `lbu`. Symptom before the fix: right register
  content, wrong sign-extension instruction, at the one spot the value is
  read.

## One real reshape: an early return inside the `if`, not a merged tail

First-pass C used a single `return stage;` after a plain `if (cond) {…}
else {…}` with no early return in either arm. That built clean and matched
everywhere except a run of register-identity swaps in the `found:` block's
tail (adjustment/position-write code): retail materializes `move v0,a3` (the
return value) immediately after loading `entry->adjustment`, then uses `v1`
for the `adjustment*6` chase and reuses `a0`/`a1` for the unaligned
load/store pair; my build deferred `move v0,a3` to just before the shared
epilogue jump and used `v0`/`v1`/`a0` in a different rotation for the same
values -- classic downstream register-identity residue from one value's
lifetime differing.

Restructuring so **each arm returns for itself** (`if (stage >= 0) { …;
return stage; } stage = GetRandomSpawnFromStage(...); …; return stage;`,
i.e. no shared merge point in source even though the compiler still
tail-merges the two physical epilogues into one) reproduced retail's
register rotation exactly on the first try. Whether this is the head's
block-order lever (making the register that becomes `v0` free EARLIER by
giving GCC a `return` statement to compile right there, rather than
deferring it to a source-level statement after the `if`) or a related
but distinct effect is not fully disentangled -- see the answer below.

## Head broadcast answer — bare-`j`-to-join / block-order lever, both shapes

**No bare `j`-to-a-join was involved, and neither of the two described
shapes (if/else arm ordering, duplicated assignment) applied verbatim.**
This function's own if/else *arm order* was already correct on the first
attempt (main path in the `if`, fallback in the `else`, matching retail's
`bltz`-jumps-to-else layout) -- that part needed no correction. What DID
need correction, and which the broadcast's underlying mechanism plausibly
explains, is a THIRD variant worth naming for the next round: **giving each
arm of an if/else its own explicit `return` (instead of one shared `return`
after the whole if/else) changed a downstream REGISTER ROTATION, not any
branch target or block order** -- no jump appeared or disappeared, no
instruction count changed system-wide, only which physical register held
"the return value" at one specific instruction and, cascading from that,
three other registers in the same basic block. This is consistent with the
broadcast's underlying claim that GCC 2.6.3 treats "when is a value's
register need first known" as sensitive to exactly which statement
computes it and how eagerly it can be freed elsewhere -- but it is a
register-identity effect, not a block/jump-order one, so I'm not
folding it into the two named shapes without a second instance to confirm
the mechanism.

## Proposed learnings

- Before typing an array element or struct field read only by the function
  under construction, check the actual load width/signedness
  (`lbu`/`lhu` vs `lb`/`lh`) in the `.s` FIRST. A `for`/`while` loop lowered
  from a signed vs. unsigned trip-count compiles to a different NUMBER of
  guard branches (one vs. two), not just a different sign-extension
  instruction -- so a wrong sign here costs words, not just bit patterns.
- When a residue is a cluster of register-identity swaps localized to one
  basic block that also contains the function's `return`-value
  materialization, try giving each control-flow arm its own explicit
  `return` statement (even where a shared merge point is equally valid C)
  before reaching for a barrier. Cost is one line; it fully resolved this
  residue on the first try.
