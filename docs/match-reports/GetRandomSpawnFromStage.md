# GetRandomSpawnFromStage — MATCHED 97/97

**Unit:** DreamSys · **Size:** 97 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on BOTH `gp_rel` (`gpDinamicLinkPenalty`)
AND `addiu_at` (5 runtime-indexed global loads). Round 21 resolved
`addiu_at`; round 42 resolved `gp_rel`. Attempted fresh this round (a
stretch pick beyond the assigned queue, the largest function tackled this
round).

## What it does

Picks a random stage (0-5, avoiding the current stage `stg` when `stg >= 0`
by bumping and wrapping; or `-stg` when `stg < 0`, the "specific stage
forced" caller shape), then a random spawn point within that stage,
publishes the resolved `PlayerSpawnPoint` into `*target` (grid position +
world-space adjustment, same `PlayerSpawnGridPos`/`sSpawnPosAdjust` idiom
as the already-matched `GenerateInitialSpawn`), increments
`*gpDinamicLinkPenalty`, and returns the chosen stage. Called throughout
this unit as the shared "give me somewhere to land" primitive (already
visible at several now-matched call sites: `TestForStageTransition`,
`TestForInstantTeleporters`'s trigger tables, `DreamSys__DynamicLink`).

## `D_80087F34` is NOT a second table -- same discovery as `sCardinalRotations`/`sCardinalAngles`

The retail disassembly references `D_80087F34` for reading the `z` field of
the chosen `sSpawnPosAdjust[adjustment]` entry, looking like a second,
parallel array. It is not: `sSpawnPosAdjust`'s own splat symbol only
claims 4 bytes (`asm/data/783DC.data.s`, one `.word`) before `D_80087F34`
begins immediately after -- exactly 4 bytes in, which is the `.z` OFFSET
within `RelativePos` (`{s16 x,y,z}`, 6 bytes). Walking `D_80087F34`'s raw
shorts confirms it: every third `short` (the position that would be entry
`i`'s `.z`) is `0x0000`, and the OTHER two positions per 6-byte group hold
the actual varying values -- i.e. it is the SAME flat `RelativePos[]` array
as `sSpawnPosAdjust`, just split into two splat symbols because
`D_80087F34` happens to sit at exactly the byte the compiler's own `.z`
field-address computation resolves to. A plain whole-struct copy,
`target->position = sSpawnPosAdjust[entry->adjustment];` -- literally the
same line `GenerateInitialSpawn` already uses, byte-exact -- reproduces
this without ever referencing `D_80087F34` in the C source at all; the
compiler's own unaligned-copy codegen (`lwl`/`lwr` for the 4-byte `x,y`
pair, `lh`/`sh` for the 2-byte `z`) happens to compute the `z` address in a
way that resolves to that symbol.

## Attempt 1: `rand() % 6` as a literal constant divisor, wrong instruction shape entirely (4/97, ~140KB drift)

```c
stage = rand() % 6;
```

GCC 2.6.3 -O2 recognizes a COMPILE-TIME CONSTANT divisor and replaces the
division with magic-number multiplication (`lui`/`ori` loading
`0x2AAAAAAB`, `mult`, `sra`, `mfhi`, two `subu`s) -- a completely different,
LONGER instruction sequence than a real `div`. Retail uses a genuine
`div`/`mfhi` pair with the standard divide-by-zero/`INT_MIN`-overflow
`break` guards (the same idiom already confirmed for `InterpolateYAtZ` and
every other runtime-divisor division in this unit), meaning the ORIGINAL
source's divisor was NOT visible to the compiler as a literal at that
point.

**Fix:** introduce a plain `s32 six = 6;` local and divide by the VARIABLE
(`rand() % six`) instead of the literal. This is enough to defeat the
strength-reduction pass entirely (GCC does not constant-propagate through a
local variable inside a function this simple context here), forcing a real
`div`.

## Attempt 2: right instruction, wrong register lifetime (88/97)

With the `div` restored, `six` still loaded into a fresh CALLER-SAVED
register (`$v1`) immediately before the division -- but retail loads the
constant into a CALLEE-SAVED register (`$s1`) BEFORE the first `rand()`
call, meaning its live range in retail SPANS the call. This happens only
if the assignment sits, in source order, before the branch that decides
whether the random pick or `-stg` path runs -- i.e. `six = 6;` must be
UNCONDITIONAL at the very top of the function, not inside the `if (stg >=
0)` block where the division itself lives.

**Fix:** hoist `six = 6;` above the `if (stg >= 0)` check. Byte-exact.

## Final body (97/97)

```c
s32 GetRandomSpawnFromStage(PlayerSpawnPoint *target, s32 stg, s32 unused)
{
	s32 stage;
	s32 index;
	StageSpawn *entry;
	s32 six;

	six = 6;
	if (stg >= 0) {
		stage = rand() % six;
		if (stage == stg) {
			stage++;
			if (stage >= 6)
				stage = 0;
		}
	} else {
		stage = -stg;
	}

	index = rand() % sStageSpawnPointsCount[stage];
	entry = &sStageSpawnPoints[stage][index];
	*(PlayerSpawnGridPos *)target = *(PlayerSpawnGridPos *)entry;
	target->position = sSpawnPosAdjust[entry->adjustment];
	(*gpDinamicLinkPenalty)++;
	return stage;
}
```

The second division (`rand() % sStageSpawnPointsCount[stage]`) needed no
such treatment -- its divisor is already a runtime ARRAY LOAD, never a
compile-time constant, so GCC never considers strength-reducing it; only
the literal `6` triggered the optimization.

`unused` (the third parameter, `this->dreamTimer` at most call sites) is
never read anywhere in the function body, matching every disassembled
instruction: `$a2` is untouched from entry to return.

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py GetRandomSpawnFromStage` -> `97/97 words match`.

### Proposed learning

Two independent, generalizable levers, both about a runtime `%`/`/` whose
RHS is a small literal:

1. **A `% <small literal>` in source can silently become a magic-multiply
   sequence under `-O2`, which is a fundamentally different (and usually
   longer) instruction shape than the `div`/`mfhi` + overflow-guard idiom
   this project's other runtime divisions already use.** If retail shows a
   real `div` where your translation of a literal-divisor `%`/`/` doesn't,
   route the constant through a plain local variable first
   (`s32 six = 6; x % six;`) to defeat the strength-reduction pass. This is
   a distinct, more drastic failure mode than the "off by a scheduling
   choice" residues seen elsewhere -- the whole instruction COUNT changes,
   not just register identity, which is a strong tell to look for a
   const-vs-variable divisor mismatch specifically.
2. Once the `div` is restored, the constant's OWN register lifetime still
   has to match retail's -- if retail's disassembly shows the constant
   surviving a function call (i.e. living in a callee-saved register), its
   assignment must be hoisted to before that call in SOURCE order, not left
   next to the division that consumes it.

## Provenance

round 43, runner ALPHA, unit DreamSys (stretch pick beyond the assigned
queue).
