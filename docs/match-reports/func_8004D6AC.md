> **REOPENED -- ASSIGNABLE, round 42 (2026-09-15).** This function was
> screened as blocked by `gp_rel`. **That blocker is RESOLVED**: maspsx gained
> `--gp-symbols` and `--no-nop-mflo-mfhi` (`tools/patches/maspsx-lsd-flags.patch`,
> passed by the Makefile), the whole image stays byte-exact, and previously
> blocked functions now match (see `docs/research/gp-relative-blocker.md`,
> "RESOLVED"). Everything below is evidence from before the fix: its
> derivation may still be right, its VERDICT is not. Rebuild before believing
> any score in it.

# func_8004D6AC

**Unit:** class_3bb8c_c · **Size:** 22 words · **Status:** BLOCKED, not attempted ·
Classified by the head in round 9 (2026-09-02) at carve time.

This is a **stub report**, filed so `tools/progress.py` stops counting this
function as fresh ground and staffing a runner onto it. It records a routing
decision, not an attempt. Nobody has written a line of C for it.

## Why it is blocked — gp-relative addressing

3 `%gp_rel` references, to two distinct small-data globals:

```
    /* 3DEB8 8004D6B8 1C02848F */  lw   $a0, %gp_rel(D_8008AA24)($gp)
    /* 3DEC8 8004D6C8 1002838F */  lw   $v1, %gp_rel(D_8008AA18)($gp)
    /* 3DECC 8004D6CC 1C02828F */  lw   $v0, %gp_rel(D_8008AA24)($gp)
```

That is the **gp-relative addressing blocker**,
`docs/research/gp-relative-blocker.md`: the project's pinned `-G0` pipeline
emits the two-instruction absolute (`lui`+`lw`) form where retail has the
one-instruction `$gp`-relative form. The instruction count differs, so the
mismatch is not contained — every function after it in the unit shifts by a
word, and the score lands far below what "one instruction off" suggests.

The `-G` experiment was run in round 2026-08-29-a **with operator
authorisation and REJECTED**; the obvious test gives a false green. Read that
document before proposing anything. No source-level reshaping reaches this.

## Do not re-derive this

Both open blockers are escalated with reproducers and corpus censuses
attached. Do not spend attempts here, do not propose a toolchain change, and
do not classify a residue from this construct as a scheduling or delay-slot
choice. Screen cheaply before attempting any function:

```sh
grep -n 'gp_rel' asm/nonmatchings/<unit>/<func>.s
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```

## Unit context

It is the ONLY blocked function in `class_3bb8c_c`. The other 13 queued
functions in the unit are clear of both blockers, so this stub is a
single-function detour rather than a reason to avoid the unit.
