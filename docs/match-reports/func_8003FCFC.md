# func_8003FCFC -- STALL, MISFILED CLASS CORRECTED (return-type fix found, register residue narrower but not closed)

## Round 20 (runner delta): mechanism identified precisely, two more attempts, both negative

Rebuilt attempt 6's body (`s16 *func_8003FCFC(s16*,s16*)`, no named
`result`, direct `return dst;`) fresh: reproduces 2/20 raw words exactly,
`--debug` would confirm the reported 135 (not re-run this round, no
reason to doubt the prior measurement -- the raw word count and
instruction pattern match the report's own description exactly).

**Found the actual MECHANISM behind the residue via `tools/asm-differ`,
which the prior three rounds' reports did not pin down**: the
`move $v0,$a1` retail places second-instruction-early is NOT there
because of any explicit early "cache the return value" C statement --
it is the R3000's LOAD-DELAY-SLOT FILLER for the function's very FIRST
`lh` (a real hardware hazard: the instruction immediately after any load
cannot read that load's destination register, so the scheduler needs
SOME safe, independent instruction to fill that slot, and "materialize
the return value early" is a free one since `dst`/`a1` is live the whole
function and never touched by anything before the return). **This
attempt-6 body ALREADY reproduces that placement byte-for-byte** --
`asm-differ` shows `30500: move v0,a1` identical on both sides, at the
identical address, in the identical delay slot. The residue is NOT about
WHEN `v0=a1` happens (that already matches); it is entirely about what
happens AFTER: retail continues using `$a1` directly for every
subsequent store, while this body's compiled form switches to using
`$v0` for every subsequent store instead (and shifts `t1`/`t2`/`t3`'s
bank down by one register to compensate, `$t1/t2/t3` -> `$v1/a1/a2`).
Confirmed via `asm-differ`'s per-instruction diff, not by reading the
raw word count.

This reframes the residue precisely: it is not "coalescing" in the
vague sense the round-19 report used, it is specifically **which
register identity (the parameter's own `$a1`, or the freshly-computed
`$v0` copy of it) the compiler chooses to keep using for the REST of the
function once both hold the identical value**. GCC 2.6.3 picks `$v0`
here; retail's real compile picked `$a1`.

### Attempts (2)

1. **Bare `__asm__("")` barrier** immediately after an explicit
   `result = dst;` at the top (before the load/store body), attempting
   to pin the early materialization AND discourage the switch to using
   `result`'s register for the rest of the function: **regressed
   sharply** -- it did not merely fail to fix the residue, it broke the
   ALREADY-MATCHING delay-slot-filler placement too, forcing the
   `move v0,a1` to happen strictly BEFORE the first `lh` (rather than
   in its delay slot) and inserting a genuine extra `nop` where the
   filler used to go (confirmed via `asm-differ`: an extra instruction
   appears, plus an outside-range drift warning). This is the
   documented "`__asm__("")` is not a local lever -- it perturbs the
   WHOLE function's scheduling" caution, now with a THIRD confirmed
   instance beyond the two CLAUDE.md/MATCHING-GUIDE already record.
2. **Flipped which variable is the "derived" one**: introduced a
   separate local `d = dst;` used for every STORE (`d[i] = ...`
   throughout), while `return dst;` returns the ORIGINAL, untouched
   parameter directly (the mirror image of round-18's attempt 7, which
   cached the RETURN value and kept `dst` for the stores). Reasoning:
   if retail's real source has the OPPOSITE variable playing the
   "derived, gets $v0" role, swapping which one is nominally the "copy"
   at the C level might flip the register allocator's choice too. No
   change: still 2/20, byte-identical instruction pattern to the
   baseline (down to which specific bytes differ). GCC 2.6.3 evidently
   does not distinguish "which C name is the alias" once both provably
   hold the same value throughout -- confirms this choice is being made
   by the register allocator's own internal ordering/preference, not by
   anything expressible through source-level aliasing.

### Proposed learning

**The delay-slot-filler EXPLANATION for where `move $v0,$a1` lands is
now confirmed and already reproduced correctly** -- three rounds of
reports described this as a vague "coalescing" or "register bank"
residue without checking WHERE the copy instruction actually sits
relative to retail's own load-delay hazard. It sits in EXACTLY the right
place already. The remaining, unclosed piece is a strictly narrower
question: once a value is live in two registers simultaneously (the
original parameter register and a freshly-copied return-value register),
which one GCC 2.6.3 continues to reference for later, independent uses.
Neither naming (attempt 2, and round-18's attempts 6-8), nor a scheduling
barrier (this round's attempt 1, worse), nor an unguided permuter (round
19, ~45,000 iterations) has found a lever for this specific choice.
**A THIRD confirmed case of `__asm__("")` failing to generalize past its
one documented use** (callee-save prologue ordering) -- worth folding
into that caution's running count rather than treating as a one-off.

Restored to `INCLUDE_ASM`. Full oracle re-confirmed green
(`build exit=0`, `OK: build matches retail`) before moving on to the
next assigned unit.

Unit `code_2cc8c_e`, carved round 14. Screened clean (no `gp_rel`, no
`addiu $at,$at,%lo`, 0 callee-saved registers) -- not a toolchain blocker.

## Shape

A 3x3 matrix transpose over `s16` elements (matches PSYQ's `MATRIX.m[3][3]`
shape): `dst[i][j] = src[j][i]`. Diagonal elements (0,4,8) are unmoved;
the six off-diagonal elements are permuted.

## Best body reached (structurally exact, register bank differs)

```c
#if 0
void func_8003FCFC(s16 *src, s16 *dst) {
    s32 t1, t2, t3;

    t1 = src[0];
    dst[0] = t1;
    t2 = src[3];
    t1 = src[6];
    dst[1] = t2;
    t3 = src[1];
    dst[2] = t1;
    t2 = src[4];
    dst[3] = t3;
    t1 = src[7];
    dst[4] = t2;
    t3 = src[2];
    dst[5] = t1;
    t2 = src[5];
    dst[6] = t3;
    t1 = src[8];
    dst[7] = t2;
    dst[8] = t1;
}
#endif
```

## Residue

Retail's instruction PATTERN and COUNT are byte-for-byte reproduced by the
body above -- same interleaving of loads ahead of their stores to hide the
load-delay slot (no `nop`s), same load width (`lh`, confirmed: with the
temps typed `s16` GCC chose `lhu` instead and the load's own opcode byte
differed; typing them `s32` reproduces retail's `lh` exactly, since a
16-to-32 sign extension is only needed on the wider type). The ONLY
remaining difference is the physical register bank: retail cycles
`$t1`/`$t2`/`$t3` ($9/$10/$11); every C shape tried lands in `$v0`/`$v1`/`$a2`
instead. Every load/store OPERATION, OPERAND ORDER and OFFSET matches; only
the register NUMBER differs, which is exactly CLAUDE.md HARD RULE 6's test
for a STALL ("if removing it changes WHICH REGISTER holds a value, it is
banned").

## Attempts (4)

1. `s16` temps, natural `dst[i]=src[j]` nine separate statements (no named
   temps): wrong SIZE too (compiler serialized load-then-store per statement
   with a `nop` after each `lhu`, since nothing gave it a reason to
   interleave) -- 27 words instead of retail's 20.
2. `s16` temps, hand-interleaved per retail's own load/store order (the body
   above but with `s16 t1,t2,t3`): correct SIZE and pattern, but `lhu` not
   `lh`, and `$v0`/`$v1`/`$a2` not `$t1`-`$t3`.
3. Declaration order `t3,t2,t1` instead of `t1,t2,t3`: no change at all
   (byte-identical to attempt 2).
4. `s32` temps (the body above, as committed to this report): fixes the
   `lhu`->`lh` residue exactly, but the register bank is unchanged
   (`$v0`/`$v1`/`$a2`).

## What I did NOT try, and why

- **The permuter.** Given this is a pure register-identity residue on a
  20-word leaf function with a byte-exact structural match otherwise, this
  is close to the ideal permuter target the project's docs describe. Not
  run due to time budget in this round -- the sibling `func_8003F764` stall
  in this same unit already spent a permuter attempt (bare randomization,
  no `PERM()` macros, abandoned after ~10k non-converging iterations) and I
  judged a second unguided permuter run unlikely to do better without first
  writing real `PERM_VAR` macros around the three temps -- worth trying on
  a re-attempt, seeded from attempt 4's near-exact body.
- **Wrapping the function body in its own translation-unit-local ordering
  trick** (e.g. `static` vs external linkage, or moving it earlier/later
  among this unit's OTHER functions) on the theory that GCC 2.6.3's pseudo
  register numbering might be influenced by how many temporaries earlier
  functions IN THE SAME FILE used. I don't believe this is actually how
  GCC 2.6.3's per-function register allocator works (pseudo numbers reset
  per function), so I did not spend an attempt confirming a mechanism I
  suspect doesn't exist -- flagging the reasoning explicitly in case a
  future attempt wants to test it properly rather than trust this dismissal.

## Proposed learning

Typing a small pass-through temp `s16` when the source values are `s16` but
never used in wider arithmetic can pick `lhu` where retail (needing an
explicit widen for some OTHER reason not visible from a single instance
alone) has `lh` -- widening the temp to `s32` reproduces the sign-extending
load even when the value is immediately narrowed back on store. Worth
checking on any other narrow-passthrough residue that shows a load OPCODE
mismatch rather than an operand mismatch.

## Round-bravo sweep: the "void, register bank differs" classification is WRONG -- this is a discarded-return-value case

Read as part of a coordinator-requested sweep of register-shaped reports
in this unit, not a full re-attempt. Retail's raw disassembly (checked
directly, not just this report's transcription) has a dead giveaway the
original report's own diff quotes but does not explain: the SECOND
instruction in the whole function is `addu $v0, $a1, $zero` -- copying
`dst` (the `a1` parameter) into `$v0`, the return-value register --
computed once, very early, and never read again by anything else in the
function (every actual `sh` store keeps using `$a1` directly). A
`void`-declared function has no business ever writing `$v0` at all. This
is exactly DECOMPILATION_LEARNINGS' own "a discarded return value is never
evidence of `void`" trap, and this report's 4 attempts never questioned
the `void` return type.

**Retyping the function to `s16 *func_8003FCFC(s16 *src, s16 *dst)` with
`return dst;` added at the end is a real, verified improvement**: `--debug`
score drops from 290 (confirmed base score for the ORIGINAL `void` attempt-4
body -- worse than this report's word-count framing suggested, once
measured with the tool rather than read by eye) to 135. The remaining 135
is NOT a clean isolated residue, though: retail computes `v0 = a1` ONCE,
early, and then keeps using `a1` directly for every store, while every C
shape tried (plain `return dst;`, an explicit `s16 *result = dst;` cached
before the loop and returned at the end, and a branch-forced-copy trick
`if (src) { result = dst; } else { result = dst; }` copied from
`func_80051858`'s own precedent) coalesces `dst` and its returned copy into
ONE register throughout (`$v0` used for the return AND every store),
where retail keeps them as two separate register identities (`$a1` for
the stores, `$v0` for the return alone). All three variants scored
identically (135) -- none closed the remaining gap.

**Verdict for the coordinator's register-shaped-class sweep: MISFILED, not
a genuine register-identity stall as originally classified.** The
underlying function almost certainly does return `dst` (matching a
`memcpy`-like idiom this codebase uses elsewhere), and the "register bank
differs" framing was measuring the CONSEQUENCE of a wrong return type, not
an intrinsic property of the loads/stores. Recommend re-staffing this one
specifically (not dismissing it as an unfixable register class) --the
remaining 135 looks like the SAME "keep an argument live in its own
register separately from a coalesced copy" shape `func_8004042C.md` hit
and did not fully solve either (see that report's own final residue), so a
lever that closes one may close both. Not spent further attempts this
round; flagging as the strongest re-attempt candidate this sweep found.

### Attempts this round (4, beyond the original 4)

5. `--debug` on the original attempt-4 (`void`, `s32` temps) body:
   confirmed 290, not merely "structurally exact" as originally read.
6. Retyped `s16 *`, `return dst;` added, otherwise identical: 135.
7. `s16 *result = dst;` cached before the loop, `return result;`: 135, no
   change -- copy-propagated to the same thing as attempt 6.
8. Branch-forced-copy trick (`if (src) { result = dst; } else { result =
   dst; }`, `func_80051858`'s own idiom) applied to `result`/`dst`: 135, no
   change -- the trick that worked elsewhere in this project does not
   transfer to this exact shape.

## Round 19 (runner alpha): permuter run, still 135; whole-struct-assignment axis does not apply here

Re-verified attempt 6's body (`s16 *func_8003FCFC(s16*,s16*)`, `return
dst;`, the interleaved `t1`/`t2`/`t3` load/store body) rebuilds clean and
scores 2/20 words by raw funcdiff (expected -- this is a pure
register-identity residue, so almost every word's bytes differ even
though the instruction PATTERN is exact; `--debug` confirms `135`, exactly
matching this report's attempt 6-8 figure).

**Ran the permuter properly this round** (`tools/setup-permuter.sh` +
`--stack-diffs`, which round 18's own report explicitly skipped due to
time budget). `--debug --stack-diffs` confirms base 135 = 27 Register
Differences (weight 5 each), 0 Stack/Branch/Reordering/Insertion/Deletion
-- a pure, isolated register-identity signature, nothing hidden by a
missing `--stack-diffs` flag. Bounded search: `timeout 300`,
`-j 6 --stack-diffs --stop-on-zero --best-only`, no `PERM()` macros (none
written -- this is bare randomization over the attempt-6 seed). **~45,000
iterations in 300s, best score found: 135 -- never beaten, not even
matched by a differently-shaped candidate.** Not phrasing this as
permuter-exhausted (no PERM macros were written to guide the search, and
45k iterations under a `-j 6` shared-machine budget is not an exhaustive
search of this space) -- phrasing precisely: **not closed in ~45,000
iterations under load.**

**Why the `func_8004042C`/`func_800407F8` whole-struct-assignment lever
does not transfer here, checked explicitly rather than assumed:** both of
those functions' residues were a two-SEPARATE-SCALAR-ASSIGNMENT shape
copying one struct's fields into another struct's fields, where GCC 2.6.3
interleaves the pair and retail batches it (or vice versa) -- rewriting
the two scalars as one aggregate assignment removed the two-statement
shape entirely and, as a side effect, also fixed neighbouring register
allocation. This function's residue is structurally different: it is a
3x3 MATRIX TRANSPOSE (a permutation across 9 independent scalar elements,
not a copy of one contiguous struct's bytes into another), and the
specific residue is a return-value/argument COALESCING choice (`dst`
kept live in `$a1` throughout the body vs. copied once into `$v0` at
entry and never touched again), not an interleave-vs-batch scheduling
difference between adjacent loads/stores. There is no natural aggregate
to assign as a whole here -- `dst[0..8] = f(src[0..8])` is a scatter, not
a struct-to-struct copy -- so the lever's precondition (two statements
that could become one aggregate assignment) does not exist in this body.
Recorded so the next reader does not re-derive this by re-attempting the
lever from scratch.

**Verdict unchanged: STALL, register-identity, best 135 (`--debug`
score) / 2/20 words (raw funcdiff), restored to `INCLUDE_ASM`.** This
remains the strongest re-attempt candidate in the corpus by elimination
(cleanest, most isolated residue found across the round-18/19 sweep) but
needs a genuinely new lever, not a repeat of the return-type fix, the
three coalescing-avoidance shapes, or unguided permutation -- all of
which are now closed negatives.

### Attempts this round (1, beyond the original 8)

9. Bounded permuter search (`-j 6 --stack-diffs --stop-on-zero
   --best-only`, `timeout 300`, no PERM macros), seeded from attempt 6:
   ~45,000 iterations, best score 135, never improved. Not upgraded to
   permuter-exhausted -- no PERM macros were written, so the search space
   actually covered is narrower than "exhausted" would imply.
