# StageMap__StartScaleRamp — MATCHED (round 58), 28/28, whole-image SHA1 green

> Renamed from `StageMap__ConfigureRateEntry` on 2026-09-26 (tools/rename.py). Address 0x8004cfb8.

> Renamed from `Class866E8__ConfigureRateEntry` on 2026-09-26 (tools/rename.py). Address 0x8004cfb8.

> Renamed from `func_8004CFB8` on 2026-09-24 (tools/rename.py). Address 0x8004cfb8.

> **HEAD VERIFICATION, round 9. Classification CONFIRMED as a stall, but
> RECLASSIFIED as to kind: this is a SOURCE-SHAPE stall and a permuter
> candidate, NOT a toolchain blocker.** The distinction matters for routing.
> The two escalated blockers (`gp-relative`, `addiu_at`) are maspsx/flag
> issues with a known mechanism, a measured corpus census and a known
> non-remedy; filing this next to them would imply an operator decision is
> pending, and none is. Nothing here is waiting on a toolchain change. What is
> true is narrower and the runner stated it correctly: cc1 makes a different
> RTL-expansion choice for every statement shape tried so far. That is the
> ordinary condition of an unsolved function, and the reproducer work below is
> exactly right — it rules the downstream tools out, which is the useful half.
>
> **One further shape tested by the head, NEGATIVE.** The runner's six
> variants all keep two separate products. I tried the reading where retail
> has only ONE logical multiply and the `mult` in the branch's delay slot is a
> speculative hoist — negating in place before a single multiplication:
>
> ```c
> merge:
>     if (rate < 0) {
>         rate = ~rate + 1;
>     }
>     self->unk1E0 = self->unk1E4->unk6 * rate;
> ```
>
> **17/28 — materially worse than the runner's 25/28.** So retail really does
> compute two products, and the deferred single `mflo` is not a delay-slot
> hoist of one multiply. That closes off the most plausible remaining
> non-permuter reading, which is worth knowing before anyone spends attempts
> re-deriving it.
>
> Routing: permuter candidate, and a good one — the residue is 3 words in a
> 28-word function with the first half already byte-exact, so the search space
> is small and well isolated. Do NOT escalate this as a toolchain lead.

`self->unk1E4 = &(one of four static 0xC-byte table entries)`, selected by
`(rate > 0, flag != 0)`, then `self->unk1E0 = table->unk6 * abs(rate)`
computed retail's way: an unconditional `mult` with the raw (possibly
negative) `rate` in the branch's delay slot, corrected by a second `mult`
with the negated value if `rate < 0`, with only ONE `mflo` extracting
whichever product is current at the merge point.

## What got solved

The table-selection half is **byte-exact** — verified independently by
building it alone (isolating it from the second half showed 0 residue in
that window). It needed the "default value, then conditionally
overwritten" idiom applied via explicit `goto`, not plain nested
`if`/`else`:

- Plain nested `if`/`else` (each of the four leaves doing its own
  `self->unk1E4 = CONST;`) compiled 3 words **too long**: GCC generated
  an independent `sw` in every leaf, none shared. Retail shares ONE `sw`
  across three of the four leaves (the two `flag == 0` cases plus the
  `rate <= 0, flag != 0` fallthrough), with only the `rate > 0, flag != 0`
  leaf taking an early exit that stores in its own jump's delay slot.
- Writing it as an explicit CFG with `goto` (default assigned per `rate`
  branch, then conditionally overridden, with a shared `store:` label)
  reproduced this exactly.

## What did NOT get solved

The `self->unk1E0 = table->unk6 * abs(rate)` half. Retail's asm:

```
lh   $v1, 0x6($v0)         ; v1 = table->unk6
bgez $a1, MERGE
 mult $v1, $a1              ; delay slot: v1 * rate, UNCONDITIONAL
nor  $v0, $zero, $a1        ; (not taken, rate < 0): v0 = ~rate
addiu $v0, $v0, 1           ;                        v0 = -rate
mult $v1, $v0               ; redo: v1 * -rate
MERGE:
mflo $v0                    ; ONE extraction, whichever mult is current
jr   $ra
 sw  $v0, 0x1e0($a0)
```

Every C shape tried for this reproduces the VALUE correctly but not this
exact instruction sequence:

1. `val = table->unk6 * (rate >= 0 ? rate : -rate);` — cc1 computes the
   abs value first (cond-move + `negu`, ONE instruction) then a SINGLE
   `mult`/`mflo`. Correct value, wrong shape (missing the wasted first
   `mult`) — 1 word short in this section.
2. `val = table->unk6 * rate; if (rate < 0) { val = table->unk6 *
   -rate; }` (the literal "default, then conditionally overwritten"
   idiom that fixed the FIRST half of this same function) — cc1
   generates the retail-matching double-`mult` structure AND the
   retail-matching `nor`/`addiu` negation (confirmed by using `~rate + 1`
   instead of unary `-rate` for the second operand — that alone gets the
   negation instructions byte-identical), but eagerly extracts the FIRST
   `mult` via `mflo` immediately after computing it, before testing the
   branch — retail defers that `mflo` to the merge point. Net: 3 extra
   words (an `mflo` right after the first `mult`, redundantly
   materializing a value the branch may immediately discard).
3. Same as #2 but with an explicit `goto`-based CFG instead of
   `if`/`else` (mirroring what fixed the first half) — cc1 output
   byte-identical to #2. The `goto` restructuring that worked for the
   STORE-sharing problem does not touch this residue at all.
4. Plain `if (rate >= 0) { val = A; } else { val = B; }` (no shared
   default) — worse: cc1 re-loads `self->unk1E4` from memory a second
   time in the `else` arm (an extra `lw`) and does not share the tail
   `sw` between arms at all. 6 words too long.
5. Assigning directly to `self->unk1E0` inside the branches instead of
   through a local `val` (removing the intermediate variable entirely,
   combined with #4's shape) — worse again, same extra reload plus two
   independent `sw`s.
6. A bare `__asm__("")` scheduling barrier inserted between the first
   `val = ...;` and the `if` — no effect (confirms this is not a
   delay-slot/scheduling choice reorg.c could be argued into; it is an
   eager-extraction decision cc1 itself makes during RTL expansion,
   verified by inspecting cc1's OWN `.s` output directly, bypassing
   maspsx/gas/the linker entirely).

All six variants were confirmed against cc1's raw output (piped directly
through the pinned `cpp | cc1` stage, per CLAUDE.md's reproducer recipe)
to rule out any maspsx/gas/linker involvement — this is a genuine cc1
RTL-expansion choice for this exact statement shape, not a downstream
tool artifact.

## Best-attempt body (inline, `#if 0`)

```c
#if 0
void StageMap__StartScaleRamp(Obj866E8 *self, s32 rate, s32 flag) {
    EntryDesc866E8 *table;
    s32 val;

    if (rate <= 0) {
        goto rate_le;
    }
    table = &sScaleStepUpSlow;
    if (flag == 0) {
        goto store;
    }
    self->unk1E4 = &sScaleStepUpFast;
    goto merge;
rate_le:
    table = &sScaleStepDownSlow;
    if (flag == 0) {
        goto store;
    }
    table = &sScaleStepDownFast;
store:
    self->unk1E4 = table;
merge:
    val = self->unk1E4->unk6 * rate;
    if (rate < 0) {
        val = self->unk1E4->unk6 * (~rate + 1);
    }
    self->unk1E0 = val;
}
#endif
```

`EntryDesc866E8`, the four `D_80086*` externs, and the `unk1E4` retype
(from `void *` to `EntryDesc866E8 *`) are committed to
`include/class_3bb8c.h` regardless — they are correct and proven by the
byte-exact first half, and useful for whoever picks this back up.

## New struct knowledge (`include/class_3bb8c.h`)

- New type `EntryDesc866E8`: a 0xC-byte struct, only `+0x006` (`s16`,
  `unk6`) typed — the rest is unproven padding.
- `sScaleStepUpSlow`, `sScaleStepUpFast`, `sScaleStepDownSlow`, `sScaleStepDownFast` — four static
  instances of `EntryDesc866E8`, addresses confirmed 0xC apart. The
  existing `sScaleOne` (`extern s32 sScaleOne[3]`, declared by an
  earlier round from `StageMap__ResetCellScale`) is a plausible fifth entry of the
  same table by the same stride, but is left independently declared —
  nothing in this unit reaches it through the `EntryDesc866E8` type.
- **`Obj866E8::unk1E4` retyped** from `void *` to `EntryDesc866E8 *`.
  Only reference elsewhere in this unit is `StageMap__AddScaleStepToCell`, which
  forwards it opaquely to a `void *` parameter (`EntryChildObjMethods::
  slot48`'s `arg2`) — implicit pointer-to-`void *` conversion, so this
  does not disturb that already-matched function's bytes. Flagging per
  the shared-header rule: this is a change to an existing declaration's
  type, not a pure addition.

## Attempts

6 (see above). Restored to `INCLUDE_ASM` — no score short of byte-exact
stays in `src/`.

### Proposed learning

**"Default value, then conditionally overwritten" does not generalize
from a simple assignment to a multiply-producing value.** The idiom
(confirmed in this same function for a plain pointer assignment, and
elsewhere in `docs/DECOMPILATION_LEARNINGS.md`) relies on GCC 2.6.3
sliding the default's *materialization* into the guard branch's delay
slot for free. For a `mult`, the "materialization" is a separate `mflo`
instruction, not the `mult` itself — and cc1 extracts that `mflo`
eagerly right after the assignment statement, before evaluating the
next `if`, regardless of whether the value survives to be used. Retail's
matching shape (unconditional `mult` in the delay slot, single deferred
`mflo` at the merge point) was NOT reproduced by any source-level
rephrasing tried here; suspect this needs either a different, unfound
statement shape, or is a case where cc1's per-statement RTL expansion
genuinely cannot avoid the early `mflo`, making the deferred-extraction
version something only OTHER compilers/versions would produce. If a
future attempt finds the shape, it should update this entry rather than
re-deriving the six ruled-out variants above. Also a plausible permuter
target: the search space here is small (one statement's phrasing) and
well-characterized.

---

## Permuter run (round 18, echo) — bounded search, no improvement (NOT re-classified as permuter-exhausted)

Seed: the "Best-attempt body" above (25/28-equivalent shape after the
first, byte-exact half). `--debug` base score: **360** (1 reordering + 3
insertions, `StageMap__StartScaleRamp` header), consistent with the report's own
"3 extra words" reading of the second half's residue (the eager `mflo`
plus its knock-on reordering).

Bounded search: `timeout 600 -j 6 --stop-on-zero --best-only`. Ran
**36023 iterations**; the score never dropped below the base (360) at
any point in the run -- every single candidate the permuter tried was
either exactly as good as the seed or worse (spot check of the log: scores
observed include 360, 465, 505, 610, 660, 750, 825, 885, 940, 1055, 1085,
1090, 1100, 1120 ... 1160, 1205, 1210, 1220, 1265, 1300, 1435, 1505, 1640,
1795, 1865, 1940, 2060, 2270, 2580, 2790, 2945, 2970, 3205, 3215, 3865,
5005, 5070, 5730, ... never anything under 360). **Zero was never reached.**

`timeout`'s own exit code could not be captured cleanly: the trailing
`echo "permuter exit=$?" >> ...` never landed in the log (verified: `grep
-c 'permuter exit=' <log>` = 0), almost certainly because the outer Bash
tool's own 600000ms bound and the inner `timeout 600` (600000ms) expired
within the same window and the wrapping shell was reaped before it could
run the trailing append. The run's own log shows 36023 completed
iterations and a clean `multiprocessing` shutdown (only the routine
leaked-semaphore `UserWarning`, not a traceback), which is the same
signature as every other bounded run in this round that DID land a clean
`exit=124` -- treating this as **an ordinary self-stop, not a crash**, but
flagging that the exit code itself is not independently verified this
time.

**Disposition: bounded search exhausted its time budget with no
improvement, NOT "permuter-exhausted" in the guide's technical sense** --
that term is for when zero IS reached but only via UB/duplicate-arm forms;
here zero was never reached at all, so per the head's standing instruction
this must not be upgraded to a closed classification on the strength of a
clock running out. What this run DOES add: 36023 mutations of the
second-half statement shape never found anything at or below the seed's
own score, which corroborates the existing classification (a genuine cc1
RTL-expansion choice for this exact statement shape) from an independent
angle -- not just six hand-written variants, but a broad randomized search
over the same statement's mutation space. It does not prove no C shape
exists; it means this seed's local mutation neighborhood is empty within
36k tries. Restored to `INCLUDE_ASM` (never touched -- the function was
never moved into `src/` this round). Still open to a differently-seeded
permuter run (e.g. seeded from a shape that varies the FIRST half's
already-solved structure jointly with the second, rather than holding it
fixed) or a fresh hand-written idea; not closed off.

---

## Variant 7 (round 25, head) — NEGATIVE, 5 words too long

Tried because the round-25 block-order lever (see
`docs/match-reports/CheckDreamAuxTriggerCondition.md`) suggested this function's deferred
single `mflo` might be a CROSS-JUMPING result rather than an RTL-expansion
one: if both arms end in an identical `mflo`/`sw` tail, GCC 2.6.3 can merge
those tails and absorb the shorter arm's lone `mult` into the `bgez` delay
slot, which is exactly retail's shape. The variant hoists `m` into a local
(so `self->unk1E4` is loaded once, as retail does) and then assigns
`self->unk1E0` **directly in both arms**, giving the two arms identical
tails to merge:

```c
merge:
    m = self->unk1E4->unk6;
    if (rate < 0) {
        self->unk1E0 = m * (~rate + 1);
    } else {
        self->unk1E0 = m * rate;
    }
```

with the first half unchanged from the best-attempt body above.

**Result: 5 words TOO LONG.** GCC did not merge the tails; it emitted an
`mflo` and an `sw` in each arm. Measured from the linked image rather than
from `funcdiff`'s in-range count, which is the point below.

This is variant #4/#5's family (both also too long) with the `m` hoist
added, which the earlier six did not try. So the hoist fixes the redundant
reload those two suffered and does NOT buy the tail merge. **The report's
existing conclusion stands: the eager `mflo` is a cc1 per-statement RTL
expansion choice, not a missing cross-jump.**

### Two methodological notes, both worth more than the negative

1. **`funcdiff`'s in-range instruction diffs were unreadable here and I
   nearly read them anyway.** The variant is 5 words long, so everything
   after the length change is misaligned and the per-word `retail=… built=…`
   lines compare instructions that are not counterparts. The guard fired
   correctly (`the build differs OUTSIDE this range too (189873 bytes)`),
   and the trustworthy number came from somewhere else entirely — see 2.
2. **A shifted DATA symbol sizes a text-length change exactly.** All four
   `%lo(D_80086*)` immediates came back a constant `+0x14` from retail's,
   because a change in this object's text length moves everything after it
   including `.data`. `0x14 = 5 words`, which is the length delta, read
   straight off the relocation without needing `nm` or asm-differ. Any
   function that references a data symbol by `%hi`/`%lo` gives you this for
   free.

### Proposed learning

**A one-word residue that survives every expression reshape is evidence
about block order (round 25's `CheckDreamAuxTriggerCondition`) — but a residue that
survives reshaping AND is an `mflo`/HI-LO extraction is NOT, and this
function is the measured counterexample.** The block-order lever applies to
which BLOCK a computation lands in; it cannot move a `mult`/`mflo` pair
apart, because cc1 expands that pair together during RTL expansion, before
any block layout decision. So when screening for block-order candidates,
a `mult`/`div`/`mflo`/`mfhi` residue is a *negative* indicator, not a
neutral one.

---

## Round 53 (bravo) — CALIBRATION attempt, one fresh shape, regressed the already-solved half, negative

Assigned as one of three functions in a round-53 Sonnet calibration slot for
the track-1 stop rule (`docs/FINISHING-PLAN.md`), alongside `IsPointOutOfBounds`
and `StageMap__SplitFootprintRect`. This report's disposition (round 18: "bounded search
exhausted its time budget with no improvement, NOT permuter-exhausted") is
the reason it was picked over the plan's higher-ranked but
levers-measurably-spent `code_2cc8c_e` job.

**Checked whether anything relevant changed since round 25.** The
`nop_mflo_mfhi` toolchain blocker was resolved in round 42
(`--no-nop-mflo-mfhi`, already in `Makefile`'s `MASPSX_FLAGS`) — a flag
change touching exactly the `mflo`/`mult` pairing this function's residue is
about, and this report's most recent entry predates it. On inspection this
does not apply: round 25's attempt 6 (and the report's own text) already
established the eager-vs-deferred `mflo` placement as a cc1 RTL-expansion
choice, verified by inspecting cc1's OWN `.s` output directly — upstream of
maspsx entirely. `--no-nop-mflo-mfhi` changes how maspsx pads a
`mflo`/`mfhi` immediately followed by `mult`/`div`; it cannot change WHEN
cc1 decides to emit the `mflo` in the first place. Rebuilt the "Best-attempt
body" fresh to confirm: still exactly **25/28**, same shape as originally
recorded — the flag changed nothing here, as expected.

**Attempt 8** (a new shape): keep the "default value, then conditionally
overwritten" idiom (already proven for the FIRST half's store-sharing
problem, and for this same second half via a `val` local in attempt 2), but
write directly to `self->unk1E0` both times instead of through an
intermediate `val`:

```c
self->unk1E0 = self->unk1E4->unk6 * rate;
if (rate < 0) {
    self->unk1E0 = self->unk1E4->unk6 * (~rate + 1);
}
```

This differs from attempt 2 (used a `val` local, eager `mflo`, first half
still byte-exact) and from attempts 4/5 (no shared default at all, direct
field writes, both worse). Attempt 8 is the direct-field-write variant OF
the shape that already gets the closest — untried combination.

**Result: 15/28, WORSE, and it broke the already-solved first half too.**
The table-selection code (byte-exact since round 9) diverged starting at the
very first `%lo(sScaleStepUpSlow)` immediate — the built object referenced a
DIFFERENT static table entry at that position than retail/every prior
attempt. Nothing about the first half's source changed; only the second
half's assignment target did. This means writing directly to the struct
field (vs. through a local) perturbed cc1's block-layout choice for the
WHOLE function, not just the second half — the two halves are not
independently compiled the way their separately-diagnosed residues implied.
Reverted (`git checkout -- src/class_3bb8c_b.c`; clean `OK: build matches
retail` confirmed immediately after).

**Disposition unchanged: `INCLUDE_ASM`, still 25/28 (best-attempt body from
round 9/18), still a genuine cc1 RTL-expansion choice, not
permuter-exhausted** (round 18's bounded search: 36023 iterations, never
below the seed's own score). This is the honest negative half of this
round's Sonnet track-1 calibration measurement.

### Proposed learning

**A direct-field-write variant of an already-working local-variable idiom is
not a safe substitution to try in isolation — it can regress an
UNRELATED, already-solved half of the same function.** The intermediate
`val` local in attempt 2 looked purely cosmetic (same value, same eventual
store) but removing it changed cc1's block layout for the whole compilation
unit enough to pick a different table-selection code path in the FIRST
half, which no earlier attempt's source touched. Treat "swap a local for a
direct field write" as a whole-function-scope experiment, not a local one —
build after every such swap even when the target statement is nowhere near
the part you believe you are testing.

---

## Round 58 (bravo) — MATCHED, 28/28. The "cc1 cannot defer the `mflo`" verdict was wrong: retail's single pre-branch load of the scale is a SOURCE-LEVEL local, and it only works together with an explicit if/else

**Result: `28/28`, `build exit=0`, `OK: build matches retail SLPS_015.56`,
SHA1 `76322eeade5ebb22dca57fdeac7d68c30f06308d`. Confirmed with the unit's
object deleted and the image rebuilt from scratch, not from an incremental
build.** Four attempts this round (and 27 words of C).

### The lever

The residue every previous round chased was the `mult`/`mflo` pairing in the
second half. Retail:

```
3d7fc: lw   v0,0x1e4(a0)
3d804: lh   v1,6(v0)          <- the scale, loaded ONCE, before the branch
3d808: bgez a1,3d81c
3d80c:  mult v1,a1            <- arm 1's mult, in the delay slot
3d810: nor  v0,zero,a1
3d814: addiu v0,v0,1
3d818: mult v1,v0             <- arm 2's mult
3d81c: mflo v0                <- ONE mflo, after the join
3d820: jr   ra
3d824:  sw  v0,0x1e0(a0)
```

Two things had to be true in the source at once, and **neither works
without the other**:

1. **The scale is a named local, assigned once before the sign test.**
   `scale = self->unk1E4->unk6;` — not `self->unk1E4->unk6` written inside
   each arm. Retail's single `lh` is not cc1 finding a common subexpression
   for you; it is one load because the source had one load.
2. **The sign test is an explicit `if`/`else` assigning a local `val`** —
   not the "default value, then conditionally overwritten" idiom every
   earlier attempt used.

```c
    scale = self->unk1E4->unk6;
    if (rate >= 0) {
        val = scale * rate;
    } else {
        val = scale * (~rate + 1);
    }
    self->unk1E0 = val;
```

The first half (the `goto` ladder selecting one of the four
`EntryDesc866E8` entries, with its deliberate store asymmetry) is
**unchanged from round 9** and stayed byte-exact throughout.

### Measured, all four in one session, so the joint requirement is not a guess

| # | second-half shape | result |
| --- | --- | --- |
| s0 | the report's recorded best body: default-then-overwrite, field access in both places | **17/28, length changes (drift)** |
| s1 | s0 with a `do { … break; … } while (0)` around it (round 58's new lever from `StageMap__SplitFootprintRect`) | **17/28, drift** — byte-identical to s0 |
| s2 | explicit `if`/`else` with `val`, but the field access still written in both arms | **10/28, drift** — worse than s0 |
| s3 | explicit `if`/`else` **plus** the cached `scale` local | **28/28, exact, whole image green** |

s2 is the informative row. The if/else alone makes things *worse*; it is
only correct once the scale load has been lifted out of the arms. That is
why six rounds of single-axis variation never found it — each half of the
lever, tried alone, measures as a regression.

### What this corrects, and the measurement warning underneath it

**Round 25's conclusion was wrong**, and it is worth quoting because it is
the kind of verdict that stops later rounds from looking:

> …or is a case where cc1's per-statement RTL expansion genuinely cannot
> avoid the early `mflo`, making the deferred-extraction version something
> only OTHER compilers/versions would produce.

It can, with this compiler, in 27 lines of ordinary C. The reasoning that
led there was sound about the mechanism (cc1 does extract the `mflo` eagerly
after an assignment statement) and wrong about the conclusion, because it
assumed the assignment statement had to be the one the attempts kept using.

**And the score this function was queued on was never trustworthy.** The
recorded best was `25/28`, and the round-18 entry describes the same state
as "3 extra words" / "1 reordering + 3 insertions" — i.e. the body is three
words LONG. A length change is exactly way 3 of CLAUDE.md's four ways a
score lies, and `funcdiff.py` warns about it. Rebuilding that same body this
round gives **17/28 with an outside-range warning**, not 25/28; the two
numbers differ because a shifted window scores different instructions
against each other. The `25/28` propagated into the report title and into
`plan.py`'s ranking regardless.

So: **before ranking work on a recorded near-miss, check whether that
score was measured under drift.** If a report's own structural description
says the attempt is N words long or N words short, its word-match figure is
one of the four lies and should not be compared against a clean one. A
function whose "best" is a drifting 17/28 is a very different prospect from
one whose best is a clean 25/28, and this one was filed as the latter.

### Proposed learning

**When retail loads a value once before a branch and both arms use it, that
single load is a source-level local — write it as one, and expect the
payoff only in combination with the branch shape.** The pattern to look for
is a `lh`/`lw` sitting *above* a conditional branch whose arms both consume
it. Writing the field access inside each arm and trusting cc1 to CSE it
gives a different RTL expansion order for anything with a two-instruction
materialisation (`mult`/`mflo`, `div`/`mflo`, `mfhi`), because cc1 extracts
the result per assignment statement. The corollary is the one that cost six
rounds here: **this lever and the branch-shape lever are multiplicative, not
additive.** Each alone measured as a regression (17/28 and 10/28 against a
17/28 baseline); together they were byte-exact on the first build. When two
axes are both suspected, try the product before concluding either is inert.

`do { } while (0)`, round 58's new lever from `StageMap__SplitFootprintRect`, was tried
here (s1) and is **byte-identical to s0** — a clean negative that helps
scope it: it moves scheduling and delay-slot placement, and does not touch
how cc1 expands a statement into `mult` + `mflo`.

## Naming

**Tier B.** Not a vtable slot. Picks one of four static `EntryDesc866E8`
table entries by the sign of `rate` and by `flag`, stores it in
`self->rateEntry`, then sets `self->rateCountdown` to `abs(rate)` scaled
by the entry's own `scale` field. Mechanics fully evidenced (see the
existing source-shape comment on the function); what "rate" represents
in-game is not.

## Track 6 (2026-09-26, round 93, alpha)

The class `Class866E8` (table `gClass866E8Methods`, id 0x114, LightRig's
subclass) is now `StageMap` (`python3 tools/renametype.py Class866E8
StageMap`, tier B): it keeps seven slots loaded with map chunks of the
current stage (LbdFile, `STGnn\Mnnn.LBD`) around a tracked target, the
centre chunk and its six staggered neighbours (`sChunkNeighbourDeltas`), laid
out by the stage's `StageGridDimensions` (`setConfig`, from ObjM's
`GetStageGridDimensions(stage)`), each slot's placements linked into a 20 x
20 lattice of GridCells whose drawn window follows the target. Tier B: the
mechanics are established; "the stage's map" rests on the files it loads and
the per-stage config. Header now `include/StageMap.h`; evidence in its banner.

Member types, same pass: `Unk68Struct` is `StageGridDimensions`
(include/StageGrid.h), `Unk54Struct` is `LongVec3` (include/SceneNode.h),
`EntryDesc866E8` is `Ratio16[3]` (include/SceneNode.h), all by layout and
use; `Class866E8Elem` -> `ChunkSlot`, `QueryPos866E8` -> `SplitLongVec3`,
`SetupEntry866E8` -> `ChunkLoadEntry`, `SetupSub866E8` ->
`ChunkLoadEntryTail`, `TargetSpec866E8` -> `ChunkSlotSpec`, `GridSlot866E8`
-> `CellRect`, `GridSlotList866E8` -> `CellRectSet`, `Bounds866E8_3bb8c_b`
-> `CellBounds`, `Class866E8ValueFn` -> `ChunkFileFn`,
`Class866E8OnElementEventFn` -> `StageMapOnSlotEventFn`,
`Class866E8ElemFn` -> `ChunkSlotFn`, `Class866E8CellFn` -> `StageMapCellFn`;
new `ChunkNeighbourDelta` for `sChunkNeighbourDeltas` (was typed as the
3-word placeholder). renametype.py also rewrote the old names inside
earlier sections' history prose in this and sibling reports (known, pending
an operator decision; not hand-reverted).

This function: `StageMap__ConfigureRateEntry` -> `StageMap__StartScaleRamp` (`python3 tools/rename.py StageMap__ConfigureRateEntry StageMap__StartScaleRamp`, tier B): picks one of four Ratio16[3] steps (y +1/64, +1/4, -1/64, -1/4 by the sign of the amount and the flag) and sets `scaleRampTicks` to |amount| times the step's y den, so the ramp changes every cell's y scale by the amount. Callers: Entity_b/e/g with (1,1), (-1,0), (4,0).

## Round 96 (track 7, delta)

Moved here from the unit, verbatim (history, derivation or retail addresses a
source comment no longer carries; the code keeps one `MATCHING:` line):

```c
/* Picks one of the four static Ratio16[3] scale steps by the sign of
 * `rate` and by `flag`, then sets scaleRampTicks to |rate| scaled by the chosen
 * step's y denominator (scaleStep[1].den).
 *
 * Two source shapes here are load-bearing and neither is cosmetic:
 *
 *  - The `goto` ladder, and its asymmetry. Retail emits TWO stores to
 *    scaleStep: the rate>0/flag!=0 path has its own (in a `j`'s delay slot at
 *    0x8004CFDC) and the other three SHARE one (0x8004CFF8). Writing the
 *    field directly on that one path and going through `table` on the other
 *    three is what reproduces that split. Byte-exact since round 9.
 *  - `scale` and `val`. Retail loads the step's y denominator ONCE (`lh $v1,6($v0)`)
 *    before the sign branch and keeps a single `mflo` after the join, with a
 *    `mult` in each arm. Caching the load in `scale` and letting an explicit
 *    if/else assign a local `val` is what defers that `mflo`; the
 *    default-then-overwrite spelling makes cc1 extract it eagerly, and
 *    storing to self->scaleRampTicks directly instead of through `val` perturbs the
 *    table-selection half as well. Both were measured -- round 58 and
 *    docs/match-reports/StageMap__StartScaleRamp.md.
 *
 * `~rate + 1` is retail's own negation (`nor`/`addiu`), not `-rate`. */
```

Replaced by a short description and a `MATCHING:` line naming the three
load-bearing shapes.

### Naming

- `flag` -> `fast` (parameter; also the prototype and slot +0x138): nonzero
  picks the 1/4 step over 1/64. Callers pass (1, 1), (-1, 0), (4, 0).
- The tables (tools/rename.py, tier A, their contents): `D_8008699C`
  `sScaleStepUpSlow` (y +1/64), `D_800869A8` `sScaleStepUpFast` (+1/4),
  `D_800869B4` `sScaleStepDownSlow` (-1/64), `D_800869C0`
  `sScaleStepDownFast` (-1/4); x and z 0/1 in each. Unit-static data: this
  unit alone reads them, so their externs moved from include/class_3bb8c.h
  into class_3bb8c_b.c.
