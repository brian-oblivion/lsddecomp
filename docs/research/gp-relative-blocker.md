# The gp-relative addressing blocker

**Status: ESCALATED, TESTED 2026-08-29 WITH OPERATOR AUTHORISATION, REJECTED.
The pin remains `-G0` and the tree is unchanged. See "The experiment was run"
below before proposing any `-G` change — the obvious test gives a false green.**

**RE-ESCALATED 2026-09-10 (round 27) ON SCOPE, NOT ON MECHANISM. Nothing about
the diagnosis or the rejected experiment has changed. What changed is the
stake: `gp_rel` now blocks 82 of 233 queued functions AND 55 of the 79
functions still uncarved, which leaves only 10 blocker-clean functions in the
entire uncarved remainder of the game. It is no longer one blocker among
several — it is the constraint that has ended carving as a source of new work.
See "Scope" below. The three unexplored leads at the foot of this document are
unchanged and remain the operator's call.**

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

**As first written (2026-08-29):** at minimum 8 functions in `code_171e0`
(runner/delta) and 1 in `class_16334` (runner/alpha). Almost certainly far
more — any function touching a small-data global is affected, and `$gp`
addressing is pervasive in retail. This likely gates a large fraction of the
remaining 1300+ game functions.

**REMEASURED 2026-09-10 (round 27), and the prediction held: this is now the
single binding constraint on the project's remaining ground.** Two censuses,
both tool-derived, both re-runnable:

```sh
python3 tools/nearmiss.py | head -2      # the live INCLUDE_ASM queue
python3 tools/uncarved.py  | head -2      # the uncarved monoliths
```

| corpus | total | `gp_rel`-blocked |
| --- | --- | --- |
| live `INCLUDE_ASM` queue | 233 | **82** |
| uncarved game code | 79 | **55** |

That second row is the one that changed the project's shape, and it is why
this is worth re-escalating rather than leaving as a standing note. **Of the
79 game functions still sitting in uncarved monoliths, only 10 are
blocker-clean.** The rest are 55 `gp_rel`, 13 BIOS trampolines that no C
compiles to at all, and 1 `nop_mflo_mfhi`. So `gp_rel` does not merely block
82 queued functions — **it has ended carving as a way to refill the work
queue.** Round 27 rewrote Gate 2 in `docs/PARALLEL-RUNS.md` accordingly:
"most of the game is still uncarved, so this gate fires early and often" was
true for twenty rounds and is now false.

The `gp_rel` load in the live queue also concentrates, which matters for
judging what a fix would return: `code_171e0` (14), `DreamSys` (13),
`code_4cd08` (8) and `code_179d8_e` (8) hold over half of it between them.
`code_171e0` is 14 of its 27 queued functions and `code_4cd08` is 8 of 17 —
two units a fix would roughly halve on its own.

Do not read these figures as current; re-run the two commands. The SHAPE is
what will not change without a toolchain move: the clean remainder is
scattered a few functions at a time across segments that are otherwise
`gp_rel`, so there is no window left to carve around it.

## What is NOT yet established

- Whether `-G8` at both stages, plus `-G8` to maspsx, keeps the currently
  matched 20 functions byte-identical. **Untested.** It could regress them.
- What `-G` value retail actually used. `-G8` is the obvious guess and the
  reproducer's success is consistent with it, but the guess is not proof.
- Whether the `.sdata`/`.sbss` segments in the splat yaml would need
  restructuring to suit.

## THE EXPERIMENT WAS RUN (2026-08-29, operator-authorised)

Result: **a global non-zero `-G` is not the answer. The pin stays at `-G0`.**

### First, a trap that invalidates the obvious test

An earlier version of this document recommended "set `-G8` and run
`./build-and-verify.sh`; if the image still verifies, the pin was wrong."
**That test gives a FALSE GREEN and must not be used.**

The Makefile makes every object depend on every source and header, but **not
on the Makefile itself**. Changing `CC_FLAGS` therefore rebuilds nothing: the
build stays green because it is still the *previous* build, compiled with the
old flags. The first run of this experiment reported a clean green at `-G8`
and it was entirely fictional.

**Any flag experiment must `rm -rf build` first.** There is no incremental
path that is safe, because the flags are invisible to the dependency graph.

### What is confirmed

The diagnosis is right. At `-G8`, the simplest blocked function compiles to
exactly retail's shape — verified at the object level, not inferred:

```
built at -G8:   af840000   sw   a0,0(gp)        # 3 words, 12 bytes
retail:         4C0084AF   sw   $a0, %gp_rel(D_8008A854)($gp)
```

`D_8008A854` links at `0x8008a854` and `_gp` at `0x8008a808`, a displacement
of `0x4C`, which is retail's encoding. So the small-data mechanism does work,
and `-G0` really is what blocks it.

### What kills it

A **clean** rebuild at `-G8` does not reproduce retail:

- **19148 bytes differ (3.785% of the image), across 3203 runs**, spanning
  almost the entire executable (`0x80011848` to `0x8008A804`).
- `-G4` produces **byte-for-byte identical damage** — the same 19148 bytes.

That last point is the informative one. If the breakage came from symbols
crossing the small-data size threshold, `-G4` and `-G8` would differ, because
they admit different symbols. They do not differ at all. So the damage is not
about *which* symbols become small — it is something structural that any
non-zero `-G` switches on, and it costs far more than the ~136 functions it
would unblock.

### Where that leaves it

The nine known-blocked functions stay blocked, and the `grep -l 'gp_rel'`
routing rule in DECOMPILATION_LEARNINGS stands — it is still correct that
these cannot be reached by reshaping C.

What is now ruled out: flipping `-G` globally. What is not yet explored, in
rough order of promise:

1. **Find what the non-zero `-G` structurally changes.** Diff the assembler
   output of one unchanged unit at `-G0` and `-G8` and look at what moved.
   Identical damage at `-G4`/`-G8` makes this a single mechanism, so it should
   be findable, and it may be separable from the size threshold.
2. **maspsx's `--use-comm-section` / `--use-comm-for-lcomm`**, which change
   where common and `.lcomm` symbols land. Untested here.
3. Whether the `.sdata` segments in the splat yaml need declaring differently
   so a non-zero `-G` does not relocate them.

Per CLAUDE.md rule 5 the head does not choose among these unprompted; this
section is the evidence for whoever does.
