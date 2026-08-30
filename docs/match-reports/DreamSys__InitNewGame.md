# DreamSys__InitNewGame

**Unit:** DreamSys · **Size:** 30 words · **Status:** BLOCKED, not attempted ·
Classified by the head in round 2026-08-30-a.

This is a **stub report**, filed so `tools/progress.py` stops counting this
function as fresh ground and staffing a runner onto it. It records a routing
decision, not an attempt.

## Why it is blocked — gp-relative

1 `%gp_rel` reference(s), the first to `D_8008ABE0`.
That is the **gp-relative addressing blocker**,
`docs/research/gp-relative-blocker.md`: the pinned `-G0` pipeline emits the
two-instruction absolute (`lui`+`lw`) form where retail has the one-instruction
`$gp`-relative form. The `-G` experiment was run on 2026-08-29 with operator
authorisation and REJECTED — a clean non-zero-`-G` rebuild damages 19148 bytes,
and `-G4`/`-G8` damage identically, ruling out the size threshold as the
mechanism. The pin stays at `-G0`.

## Do not re-derive this

Both blockers are already escalated with reproducers and corpus censuses attached.
Do not spend attempts here, do not propose a toolchain change, and do not classify
a residue from this construct as a scheduling or delay-slot choice. Check cheaply
before attempting any function:

```sh
grep -n 'gp_rel' asm/nonmatchings/<unit>/<func>.s
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```
