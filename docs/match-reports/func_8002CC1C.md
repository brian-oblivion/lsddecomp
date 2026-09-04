# func_8002CC1C -- STALL (gp-relative blocker, not attempted)

Unit `code_179d8_e`, carved round 17 (2026-09-04). **Not attempted.**

## Classification

```sh
grep -n 'gp_rel' asm/nonmatchings/code_179d8_e/func_8002CC1C.s
```

Hit:

```
lw $v0, %gp_rel(D_8008A8C4)($gp)
```

Retail reaches this small-data global in ONE instruction off `$gp`. The
pinned pipeline (`-G0` at both cc1 and `as`) cannot emit that form from C --
it emits the two-instruction absolute `lui`/`lw` pair instead, so the
mismatch is not contained: every function after it in the same translation
unit shifts by a word.

`docs/research/gp-relative-blocker.md` records the reproducer and the
measured `-G` matrix: the gp-relative form needs a non-zero `-G` at BOTH
stages, and the `-G` experiment was run in 2026-08-29 WITH operator
authorisation and REJECTED. The pin stands. This is the operator's call, not
something to experiment with mid-round.

No C was written and no score was measured. The screen ran at carve time,
before the unit was offered to a runner, so no attempt budget was spent
discovering this.
