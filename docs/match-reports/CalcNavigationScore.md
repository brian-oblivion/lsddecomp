# CalcNavigationScore — MATCHED 38/38

**Unit:** DreamSys · **Size:** 38 words · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-a as BLOCKED on `gp_rel` (two references,
`gpNavChallengesComplete` and `gpDinamicLinkPenalty`). Round 42 RESOLVED that
blocker. Rebuilt fresh this round.

## What it does

Sums `1000000` for every nonzero byte in the 30-entry
`gpNavChallengesComplete` array, caps the sum at 50,000,000, subtracts
`*gpDinamicLinkPenalty * 11024` (GCC's shift/add strength-reduced constant
multiply -- `<<1;+v1` `<<2;-v1` `<<2;-v1` `<<4;+v1` `<<4` works out to
`*3,*4=12,-1=11,*4=44,-1=43,*16=688,+1=689,*16=11024`, reproduced
automatically by `-O2` from a plain `*` in C, no manual strength reduction
needed), and floors the result at 0.

## Attempt 1: pointer-pair loop (`p`/`end`), one word off from a `slt` vs `sltu` mismatch (35/38, 5 bytes of drift)

```c
s8 *p, *end;
p = *gpNavChallengesComplete;
end = p + 30;
do {
	if (*p != 0) sum += 1000000;
	p++;
} while (p < end);
```

Comparing two `s8 *` pointers directly compiles to `sltu` (unsigned) --
verified in isolation through the pinned pipeline (a two-pointer `do/while`
reproducer emits `sltu`, confirming this is not DreamSys-specific). Retail's
own instruction is `slt` (signed) at this exact spot. 35/38 words, with
drift outside the range (traced to this unit's still-unmatched
`InterpolateKeyframeValue`, unrelated to this change -- see that report; not a new
class of the "shared-struct" drift hazard, just a residue from a DIFFERENT
already-tracked stall bleeding into the same whole-image diff).

## Final body: index-based loop, `slt` reproduced (38/38)

```c
s32 CalcNavigationScore(void)
{
	s32 sum;
	s8 *p;
	s32 i;

	sum = 0;
	p = *gpNavChallengesComplete;
	i = 0;
	do {
		if (p[i] != 0)
			sum += 1000000;
		i++;
	} while (i < 30);
	if (sum > 29999999)
		sum = 50000000;
	sum -= *gpDinamicLinkPenalty * 11024;
	if (sum < 0)
		sum = 0;
	return sum;
}
```

Comparing a plain `s32 i` loop counter against a literal bound emits `slt`
even though GCC still fuses `i` into the same register as the array base
pointer (induction-variable strength reduction turns `p[i]`/`i++` into the
same "increment a pointer register" machine code as the pointer-pair form)
-- the SIGN of the comparison instruction is decided by the C-level type of
the compared variable (`int` vs pointer), not by what the register ends up
holding after optimization. Confirmed in isolation: a `do { ... } while (i <
end)` reproducer with `int i` and pointer-arithmetic `end` emits `slt`; the
literal pointer-pair reproducer emits `sltu`, everything else about the two
identical.

## Verification

`./build-and-verify.sh` -> whole-image SHA1 matches retail (the run in this
report's own history shows `build exit=2` because `InterpolateKeyframeValue`, a
DIFFERENT still-STALLED function in this same unit, was 30/33 at the moment
this was tested; `cmp -l` against retail confirms all remaining differing
bytes fall inside `InterpolateKeyframeValue`'s own address range, `0x80059510`-
`0x8005952E`, none inside `CalcNavigationScore`).
`tools/funcdiff.py CalcNavigationScore` -> `38/38 words match`.

### Proposed learning

When retail emits `slt` where your translation emits `sltu` (or vice versa)
at a loop-bound comparison, the fix is not necessarily about the VALUES
being compared -- it is about whether the COMPARED VARIABLE is declared as a
pointer or a plain signed integer in the C source, even when `-O2`'s
induction-variable pass ends up fusing that integer into the same register
as a pointer anyway. A `do { ...; p++; } while (p < end)` two-pointer loop
and a `do { ...; i++; } while (i < end)` indexed loop can compile to
IDENTICAL instructions except for this one signedness bit on the comparison
-- pick based on retail's `slt`/`sltu` choice, not on which reads more
naturally.

## Provenance

round 43, runner ALPHA, unit DreamSys.
