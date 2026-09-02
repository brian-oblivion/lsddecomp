# CalcDreamColor — STALL

**Unit:** DreamSys · **Size:** 35 instructions · **Best reached:** 28/35 words. **PERMUTER-EXHAUSTED** for the legitimate search space (round 2026-09-02, runner BRAVO) -- see "Permuter run" below; one untried legitimate lead is flagged there for a future attempt.

## What it does

Already documented: `@brief Calculates the DreamColor for a given mood.`
Classifies each of the mood's two axis bytes into `{0,1,2}` (via
thresholds `<-3`, `[-3,4)`, `>=4`) in a local copy, then indexes a 3x3
lookup table `D_80087E14[dynamicClass*3 + upperClass]`.

## Best-reached body (does NOT compile to retail bytes)

```c
#if 0
DreamColors CalcDreamColor(MoodGraphPoint *mood)
{
	MoodGraphPoint local;
	s8 *p;
	s32 i;
	s8 val;

	local.value = mood->value;
	p = (s8 *)&local;
	for (i = 0; i < 2; i++, p++) {
		val = *p;
		if (val >= 4) {
			*p = 2;
		} else if (val < -3) {
			*p = 0;
		} else {
			*p = 1;
		}
	}
	{
		s32 index;
		s8 *entry;

		index = local.axis.dynamic * 3;
		entry = &D_80087E14[index];
		return entry[local.axis.upper];
	}
}
#endif
```

(`D_80087E14`'s `extern s8 D_80087E14[9];` declaration is kept live.)

## Two real fixes landed; one residue didn't move

**Fix 1 (worked): the classification loop's if/else arm order.** The
"obvious" nested reading (`if (val<4) { if(val<-3) 0; else 1; } else 2;`)
compiled to a genuinely different branch layout than retail's (inverted
outer condition sense, an extra `j`, wrong fallthrough) -- same
"arm-order-must-match-retail's-fallthrough" class as
`Test4StaircaseNodes` and `func_800590E8` (round 2026-08-30-c). Rewriting
as a flat `if (val>=4) 2; else if (val<-3) 0; else 1;` (the ">=4" case
FIRST, matching retail's actual branch-taken/fallthrough split) fixed the
whole first half of the function (word 0-22 all match) on one try.

**Fix 2 (worked): splitting the double-indexed table lookup.** The single
expression `D_80087E14[dynamicClass*3 + upperClass]` computed the FULL
index before adding the array base, one instruction shorter and 5 words
off from retail. Splitting into an intermediate `s8 *entry =
&D_80087E14[dynamicClass*3];` then `entry[upperClass]` matched the total
instruction COUNT (28/35 -> correct 35-word size, no more outside-range
drift) and got 5 more words matching.

**Residue (did not move): which register holds the table-base address vs.
the `upper` byte in the final two `addu`s.** Retail computes the array
base (`lui`/`addiu 0x8008.../0x7e14`) into one register EARLY (right after
reading `dynamic`, before reading `upper`), then adds `upper` last. This
body's natural codegen reads `upper` first instead, and the base-address
computation lands in the OTHER register -- both `addu`s end up register-
swapped relative to retail, with no value or branch-target difference at
all (28/35, same size, same total instruction count).

Reshapes tried on JUST this residue, all four producing the identical
28/35 result:
1. `s8 *entry = &D_80087E14[idx]; return entry[upper];` (shown above).
2. Same, with `upper` pulled into its own named local, assigned AFTER
   `entry` (to force the read to happen later in source order).
3. `dynamic*3` pulled into its own named `index` local before computing
   `entry` (shown above -- this is what's kept live).
4. Two independent named index locals (`idx1 = dynamic*3; idx2 = upper;
   return D_80087E14[idx1+idx2];`) -- this one actually regressed to the
   single-expression form's 23/35, confirming the intermediate-pointer
   split (attempts 1-3) is the right general shape, just not fully
   reachable.

**Argument-register test:** no calls anywhere after the loop (the function
ends with one `lb` and returns) -- `$a0`/`$v1` here are never live into a
subsequent call. Confirmed register-identity, not a missing parameter.

### Proposed learning

Splitting a double-indexed array access (`arr[a*N + b]`) into an
intermediate one-indexed pointer (`&arr[a*N]`) THEN adding the second
index closes both a size gap AND most of a register-identity residue in
one move -- but the LAST piece (which of the two final operands loads
first) can still resist further reshaping. Worth trying as a first move on
any `table[f(x) + g(y)]` residue before assuming it's unreachable; expect
it to get you most of the way, not necessarily all the way.

## Permuter run (round 2026-09-02, runner BRAVO) — PERMUTER-EXHAUSTED

Set up per `tools/setup-permuter.sh` with the kept near-miss body above as
the seed. Base score confirmed 610 in `--debug` mode, matching this
report's own diagnosis (the register-swapped final `addu` pair).

Ran in two windows totaling **~40400 iterations over ~8m55s wall clock**
(`-j 8`, `--stop-on-zero --best-only`):

- **Window 1: cut short by an external, unscoped `pkill -f "decomp-permuter"`
  run by another runner in this round tidying up ITS OWN permuter session**
  -- not a bug in this search; confirmed after the fact by the process
  dying well before this round's own timebox and the log's
  multiprocessing "leaked semaphore" warning (the `SIGKILL` signature).
  27708 iterations before the external kill landed.
- **Window 2: a fresh restart, run to its own bounded, PID-scoped timeout**
  (no pattern-based kill) -- a further ~12700 iterations, no improvement
  over window 1's best.

**Best score reached: 120** (base 610), never 0, across twelve distinct
saved "best" outputs.

### The 120-score candidate was REJECTED

It retypes the function's return as `volatile unsigned int` in place of
the real `DreamColors` enum return type:

```c
volatile unsigned int CalcDreamColor(MoodGraphPoint *mood)
{
	...
	return entry[local.axis.upper];
}
```

`volatile` forces the compiler to treat every access as having an
observable side effect, which changes scheduling/reload behavior enough
to move the residue -- but it is not a claim about the retail source; the
real declaration (`typedef enum DreamColors {...} DreamColors;`, used as
this function's return type everywhere else it's referenced, e.g.
`src/DreamSys.c`'s two call sites) has no `volatile` anywhere in this
codebase. Rejected as a different function, not a match.

### One candidate at 135 is legitimate C and UNTRIED in the real build

A separate saved output (`output-135-1`), score 135, keeps the correct
`DreamColors` return type and makes no UB-adjacent move -- it only routes
the table-base pointer through the loop's own `s8 *p` variable before
assigning it to `entry`:

```c
{
	s32 index;
	s8 *entry;

	index = local.axis.dynamic * 3;
	p = &D_80087E14[index];
	entry = p;
	return entry[local.axis.upper];
}
```

This is a real, legitimate C reshape (reusing `p` as scratch rather than
computing `entry` directly) that was NOT tried by hand in the four manual
attempts already on record above, and it scored better (135 vs. the kept
attempt's 610... note: attempt 3's kept form above is actually the
610-scoring BASE for this permuter run, i.e. the intermediate-pointer
split alone was not enough; THIS candidate adds the `p`-then-`entry`
indirection on top of it). Per this round's "do not start anything new"
instruction, it was NOT applied to `src/DreamSys.c` or verified against
`build-and-verify.sh` this round -- flagging it here as the concrete next
manual attempt rather than a fresh blind permuter search.

**Verdict: PERMUTER-EXHAUSTED for the volatile/UB region of the search
space** (twelve candidates, all either `volatile`-typed or otherwise
non-idiomatic, none reaching 0) -- **but NOT exhausted overall**, because
the 135-score candidate above is genuine C that was found but not yet
manually verified. A future round should try that ONE specific reshape by
hand (three lines) before reaching for the permuter again.

## Provenance

round 2026-08-30-d, runner ALPHA, unit DreamSys (whole-unit, third pass).
Restored to `INCLUDE_ASM`. Permuter run added round 2026-09-02, runner
BRAVO; still `INCLUDE_ASM`.
