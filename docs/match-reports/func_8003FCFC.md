# func_8003FCFC -- STALL, MISFILED CLASS CORRECTED (return-type fix found, register residue narrower but not closed)

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
