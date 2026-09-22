# InterpolateKeyframeValue — MATCHED 33/33

> Renamed from `func_8005950C` on 2026-09-22 (tools/rename.py). Address 0x8005950c.

**Unit:** DreamSys · **Size:** 33 instructions · **Status:** MATCHED, round 43.

## History

Filed round 2026-08-30-d as a STALL, best-reached 17/33, on `nop_mflo_mfhi`
(a `mult`/`mflo` immediately followed by `div` with no intervening `nop`,
where the pinned toolchain inserted two spurious `nop`s retail does not
have). Round 42 (2026-09-15) RESOLVED that construct with
`--no-nop-mflo-mfhi` and rebuilt the preserved body fresh: **17/33 -> 30/33**,
with the two `nop`s gone and the only remaining residue "the `/ 0x400`
preamble, where retail copies `a2` into `a3` ... and the build adjusts `a2`
in place" (round 42's own note, carried into this report's REOPENED header).
This round closed the remaining 3 words.

## What it does

Linear interpolation between two "keyframe" points (`DreamSysInterpPoint`,
each with a `value` at `+0x4` and a `position` at `+0x8`), scaled by `arg2`:
`(b->value - a->value) * (arg2 / 0x400) / dt + a->value`, where `dt =
(b->position - a->position) / 0x400`, floored at `1` if it rounds to `0`.
Called by still-`INCLUDE_ASM` `func_8005942C` as
`InterpolateKeyframeValue(&this->unk_0x5C->unknown_values_0x0[0x14], ...)`.

## Round 42's rebuild: 30/33, register-identity residue

```c
s32 InterpolateKeyframeValue(DreamSysInterpPoint *a, DreamSysInterpPoint *b, s32 arg2)
{
	s32 scaledArg2;
	s32 dt;
	s32 dv;

	scaledArg2 = arg2 / 0x400;
	dt = (b->position - a->position) / 0x400;
	if (dt == 0)
		dt = 1;
	dv = b->value - a->value;
	return (dv * scaledArg2) / dt + a->value;
}
```

Retail's own asm computes the rounded-but-unshifted `arg2` into a FRESH
register (`$a3`, via an unconditional `addu $a3,$a2,$zero` in a branch delay
slot, then a conditional `addiu $a3,$a2,0x3FF` override -- both reading the
ORIGINAL `$a2`, never overwriting it), and only shifts that into
`scaledArg2` much later, opportunistically scheduled into an unrelated
branch's delay slot (`sra $a2,$a3,10`, reusing `$a2` only once the parameter
is verifiably dead). This build's single-statement `scaledArg2 = arg2 /
0x400;` computed everything in place using `$a2` throughout, never freeing a
register for the "rounded" intermediate. Two more reshapes were tried by
hand (an explicit `roundedArg2` local computed with a ternary before `dt`,
and swapping the `dt`/`scaledArg2` statement order) -- both REGRESSED to
24/33 and 15/33 respectively, confirming this residue is sensitive to
exactly HOW the two statements interact, not simply "needs a temp variable"
or "needs different order" in isolation.

## Fix: permuter-found split assignment (33/33)

A bounded permuter search (`tools/setup-permuter.sh InterpolateKeyframeValue
<seed>.c`, `-j 4 --stop-on-zero --best-only`, ~20000 iterations under load)
found a non-obvious two-statement split at score 200 (down from the 410
baseline, though not the permuter's own zero):

```c
s32 InterpolateKeyframeValue(DreamSysInterpPoint *a, DreamSysInterpPoint *b, s32 arg2)
{
	s32 scaledArg2;
	s32 dt;
	s32 dv;

	scaledArg2 = arg2;
	scaledArg2 = scaledArg2 / 0x400;
	dt = (b->position - a->position) / 0x400;
	if (dt == 0)
		dt = 1;
	dv = b->value - a->value;
	return (dv * scaledArg2) / dt + a->value;
}
```

Splitting `scaledArg2 = arg2 / 0x400;` into a plain copy (`scaledArg2 =
arg2;`) followed by a separate in-place division (`scaledArg2 = scaledArg2 /
0x400;`) was enough to make GCC 2.6.3's register allocator treat `arg2` and
the rounded/shifted value as genuinely separate live ranges -- reproducing
retail's `$a2`-preserved/`$a3`-computed split exactly, even though the two
forms are semantically identical and a human reading either would not
expect a register-allocation difference. Rebuilt: `build-and-verify.sh` ->
`build exit=0`, whole-image SHA1 matches retail. **33/33, byte-exact.**

### Proposed learning

When a single combined assignment (`x = expr1 op expr2;`) leaves a
register-identity residue against retail (the VALUE is right, the register
holding an intermediate isn't), splitting it into two separate statements
(`x = expr1; x = x op expr2;`) over the SAME destination variable is worth
trying even though it looks like a no-op reformulation -- it changes nothing
about C semantics but can change which live ranges GCC 2.6.3's allocator
treats as coalescible. This is a cheap, mechanical permuter-discoverable
lever distinct from the already-documented "separate named temporary"
lever (which, for this exact function, made things WORSE -- a bigger
structural change is not automatically a safer one).

## Verification

`./build-and-verify.sh` -> `build exit=0`, whole-image SHA1 matches retail.
`tools/funcdiff.py InterpolateKeyframeValue` -> `33/33 words match`.

## Provenance

round 2026-08-30-d, runner alpha2 (original stall, 17/33). Round 42
(2026-09-15, gp_rel/nop_mflo_mfhi resolution round) rebuilt to 30/33.
Round 43 (2026-09-15), runner ALPHA: permuter search + hand translation,
MATCHED 33/33.

## Naming

- **Tier A.** Pure leaf: linear interpolation between two value/position keyframes with a divide-by-zero guard (dt forced to 1 when the two positions coincide). Mechanics are the whole story.
