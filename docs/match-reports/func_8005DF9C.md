# func_8005DF9C

**Unit:** Entity_b · **Size:** 43 words · **Status:** BLOCKED, not attempted ·
Pre-screened by the head before this round started.

This is a **stub report**, filed so `tools/progress.py` stops counting this
function as fresh ground and staffing a runner onto it. It records a routing
decision, not an attempt.

## Why it is blocked — addiu_at indexed-load

2 runtime-indexed global load(s) using the fully-resolved `$at` form:

```
asm/nonmatchings/Entity_b/func_8005DF9C.s:15:    addiu $at, $at, %lo(D_80089EAB)
asm/nonmatchings/Entity_b/func_8005DF9C.s:22:    addiu $at, $at, %lo(D_80089EAC)
```

That is the **`addiu_at` blocker**, `docs/research/addiu-at-blocker.md`: maspsx
at the pinned `--aspsx-version=2.34` folds `%lo` into the load's displacement
(three instructions) where retail resolves the symbol first (four). Retail
uses the unfolded form 502 times across 39 files and the folded form zero
times. The remedy is NOT a version bump — below 2.30 four flags flip
together, three of them nop-insertion rules affecting already-matched code,
and maspsx exposes no per-flag override. Open operator escalation, not
something to experiment with mid-round.

## Known call-site shape (for whoever eventually derives this)

Called from `func_8005DD18` (already matched, in `Entity.c`) as
`func_8005DF9C(this, 0)`, and from `func_8005DAAC`/`func_8005E02C` per
`config/symbols.slps01556.lsdde.txt` cross-references. The learnings file
already flags one trap specific to this exact function: `func_8005DD18`
passes a literal `0` as the second argument that this callee's own body never
reads (overwritten as scratch on entry via `sll $a1,$v0,4`) — a signature
derived from this callee's body alone would miss that dead second parameter
entirely; it is only visible from the CALLER's register setup. See
`docs/DECOMPILATION_LEARNINGS.md`'s "A missing argument can be invisible from
the callee's own disassembly" entry.

## Do not re-derive this

Both blockers are already escalated with reproducers and corpus censuses
attached. Do not spend attempts here, do not propose a toolchain change, and
do not classify a residue from this construct as a scheduling or delay-slot
choice. Check cheaply before attempting any function:

```sh
grep -n 'gp_rel' asm/nonmatchings/<unit>/<func>.s
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```
