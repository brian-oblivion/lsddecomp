> **REOPENED -- ASSIGNABLE, round 42 (2026-09-15).** This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# func_8001DDF4 -- STALL (toolchain blocker, not attempted)

Round 12, runner delta. `code_d294_b`. 199 words.

## Classification

**BLOCKED: gp-relative addressing.** Full evidence and reproducer:
`docs/research/gp-relative-blocker.md`. This is an open, escalated operator
issue -- the `-G` experiment was already run with authorisation and
REJECTED (see `CLAUDE.md`, "Open toolchain blockers"). Not something to
experiment with mid-round.

Verified per the runner brief's own screening command before spending any
attempt on this function:

```
$ grep -n 'gp_rel' asm/nonmatchings/code_d294_b/func_8001DDF4.s
89:    /* E730 8001DF30 3000828F */  lw         $v0, %gp_rel(D_8008A838)($gp)
```

One hit -- `D_8008A838` is reached gp-relatively in retail. The pinned
toolchain pipeline emits the two-instruction absolute `lui`/`lw` form
instead of retail's one-instruction `lw $v0, N($gp)`, which shifts every
instruction after the reference and makes any per-function score against
this build meaningless (this is exactly the "extra instruction shifts
everything after it in the same unit" failure mode documented in
`docs/research/gp-relative-blocker.md`).

## Action taken

None -- restored/left as `INCLUDE_ASM("asm/nonmatchings/code_d294_b",
func_8001DDF4);` in `src/code_d294_b.c` (it was already in that state; this
report just documents the screen so `tools/progress.py` counts it as a
documented stall rather than fresh, unworked ground).

### Proposed learning

None new -- this is a straightforward instance of the already-documented
aggregate blocker, filed per the runner brief's explicit instruction to
stub it rather than spend attempts on it.
