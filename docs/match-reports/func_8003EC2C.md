> **REOPENED -- ASSIGNABLE, round 42 (2026-09-15).** This function was
> screened as blocked by `nop_mflo_mfhi`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# func_8003EC2C -- STALL (nop_mflo_mfhi toolchain blocker, not attempted)

Unit `code_2cc8c_d`, carved round 13. **Not attempted.**

## Classification

Classified by the head at carve time (round 13, 2026-09-03), with the THIRD
blocker screen -- the one that is not in CLAUDE.md's two-grep list, lives in
`docs/research/addiu-at-blocker.md`, and was added to `docs/PARALLEL-RUNS.md`'s
Gate 1 this round after runner bravo's `func_8001CEB4` exposed its absence:

```sh
grep -A2 -nE '\b(mflo|mfhi)\b' asm/nonmatchings/code_2cc8c_d/func_8003EC2C.s \
  | grep -E '\b(mult|multu|div|divu)\b'
```

Retail reads a multiply/divide result and feeds it into another
`mult`/`multu`/`div`/`divu` with **no `nop` between them**; the pinned pipeline
inserts `nop`s there (maspsx's `nop_mflo_mfhi`), so the function comes out
longer than retail regardless of how the C is written. Blocked exactly like
`addiu_at`, and part of the same flag group -- not something to experiment with
per-function.

## Why this one matters beyond itself

It is the **second** of `addiu-at-blocker.md`'s 7 uncarved `nop_mflo_mfhi` hits
to become a queued one, and the second to be caught BEFORE a runner was
assigned to it. That document's round-13 addendum re-measured the queued count
at 2 -> 4 and predicted the uncarved row would keep draining into the queued
row as carving proceeds; this is that happening, twice in one round, which is
the evidence that the Gate 1 placement was the right call rather than the
per-runner screen that was correctly rejected on cost.

Whoever re-measures next should expect the uncarved row below 7 and the queued
row above 4.

## What is known

Nothing beyond the screen. No C was written and no score was measured.
