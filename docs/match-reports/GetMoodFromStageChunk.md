# GetMoodFromStageChunk

**Unit:** StageGrid · **Size:** 20 words · **Status:** BLOCKED, not attempted ·
Classified by the head in round 2026-08-30-a.

This is a **stub report**, filed so `tools/progress.py` stops counting this
function as fresh ground and staffing a runner onto it. It records a routing
decision, not an attempt.

## Why it is blocked — addiu_at indexed-load

2 runtime-indexed global load(s) using the fully-resolved `$at` form.
That is the **`addiu_at` blocker**, `docs/research/addiu-at-blocker.md`: maspsx
at the pinned `--aspsx-version=2.34` folds `%lo` into the load's displacement
(three instructions) where retail resolves the symbol first (four). Retail uses
the unfolded form 502 times across 39 files and the folded form zero times.
The remedy is NOT a version bump — below 2.30 four flags flip together, three
of them nop-insertion rules affecting already-matched code, and maspsx exposes
no per-flag override. Open operator escalation.

## Do not re-derive this

Both blockers are already escalated with reproducers and corpus censuses attached.
Do not spend attempts here, do not propose a toolchain change, and do not classify
a residue from this construct as a scheduling or delay-slot choice. Check cheaply
before attempting any function:

```sh
grep -n 'gp_rel' asm/nonmatchings/<unit>/<func>.s
grep -n 'addiu *\$at, *\$at, *%lo' asm/nonmatchings/<unit>/<func>.s
```
