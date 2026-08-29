# The gp-relative addressing blocker

**Status: OPERATOR ESCALATION. Not acted on. No toolchain change has been made.**

Found independently by two runners in two unrelated units during round
2026-08-29-a, and adjudicated by the head. This is the first stall class on
this project that survived head scrutiny as a genuine toolchain issue.

## The claim

Retail reaches small-data globals gp-relatively, in one instruction:

```
lw   $v0, 0x40($gp)          # $gp = 0x8008A808, so this is D_8008A848
sw   $a0, %gp_rel(D_8008A854)($gp)
```

The project's pinned pipeline cannot emit that form from C. It emits the
two-instruction absolute form instead:

```
lui  $v0, 0x0809
lw   $v0, -0x57b0($v0)
```

Because the instruction count differs, the mismatch is not contained: every
function after it in the same translation unit shifts by a word.

## The reproducer

```c
extern void *D_8008A854;
void setter(void *value) { D_8008A854 = value; }
```

Through the pinned pipeline, varying only `-G` at cc1 and at `as`:

| cc1 | as  | result |
| --- | --- | --- |
| -G0 | -G0 | `lui at,0x0` + `sw a0,0(at)` — absolute (**this is the project's setting**) |
| -G0 | -G8 | `lui at,0x0` + `sw a0,0(at)` — absolute |
| -G8 | -G0 | `lui at,0x0` + `sw a0,0(at)` — absolute |
| -G8 | -G8 | `sw a0,0(gp)` — **gp-relative** |

So the gp-relative form requires a non-zero `-G` at **both** stages. Either
one alone is not enough. This is worth stating because the runner that first
reported it attributed the whole effect to cc1, which is only half right.

The mechanism at cc1 is visible in its output. At `-G8` it emits a size hint
that `-G0` omits entirely:

```
.extern	D_8008A854, 4
```

That hint is what lets the assembler place the symbol in small data and
address it off `$gp`.

## What the project currently pins

From the Makefile:

- `CC_FLAGS`  … `-G0`
- `AS_FLAGS`  … `-G0`
- `MASPSX_FLAGS` … `--aspsx-version=2.34 --dont-force-G0 --expand-div`

Note `--dont-force-G0` is passed but **no `-G` value is passed to maspsx**.
maspsx's own README says, of its `-G` option:

> **EXPERIMENTAL** If your project uses `$gp`, maspsx needs to be explicitly
> passed a non-zero value for `-G`.

This project uses `$gp` constantly — CLAUDE.md says so in its own words — and
passes maspsx no `-G`.

## Why this went unnoticed until now

All 20 functions matched before this round happen to touch no small-data
globals at all. Verified mechanically: disassembling every genuinely-C matched
function in `build/src/*.o` gives **zero** `(gp)` references. The 69 `(gp)`
references those objects do contain are all inside `INCLUDE_ASM` retail bytes,
which prove nothing about what the compiler can emit.

So the `-G0` pin has never actually been exercised against a global access.
The first eight functions to exercise it all failed, plus a ninth in another
unit.

## Scope

At minimum: 8 functions in `code_171e0` (runner/delta) and 1 in `class_16334`
(runner/alpha). Almost certainly far more — any function touching a
small-data global is affected, and `$gp` addressing is pervasive in retail.
This likely gates a large fraction of the remaining 1300+ game functions.

## What is NOT yet established

- Whether `-G8` at both stages, plus `-G8` to maspsx, keeps the currently
  matched 20 functions byte-identical. **Untested.** It could regress them.
- What `-G` value retail actually used. `-G8` is the obvious guess and the
  reproducer's success is consistent with it, but the guess is not proof.
- Whether the `.sdata`/`.sbss` segments in the splat yaml would need
  restructuring to suit.

## Recommended next step (operator's call, not the head's)

Try `-G8` at cc1, `as` and maspsx together on a branch, and run
`./build-and-verify.sh`. The whole-image SHA1 answers it in under a second: if
the image still verifies, the pin was simply wrong and a large class of
functions unblocks at once. If it goes red, the currently-matched functions
tell you exactly which assumption broke.

Per CLAUDE.md rule 5 and docs/PARALLEL-RUNS.md, the head does not perform
toolchain changes. This document is the evidence, not the fix.
