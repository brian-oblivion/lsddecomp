# StageMap__BuildFootprintSlots -- MATCHED 109/109, round 75 (echo)

> Renamed from `Class866E8__BuildFootprintSlots` on 2026-09-26 (tools/rename.py). Address 0x8004c93c.

> Renamed from `func_8004C93C` on 2026-09-24 (tools/rename.py). Address 0x8004c93c.

REVISITED, round 75: MATCHED 109/109, whole image `OK: build matches retail`,
`tools/check-nonmatching.sh` green; names/types not relevant (no struct or
header change; one unit-local prototype for `StageMap__SplitFootprintSlot`).

**Lever: source shape, three parts -- one reused slot pointer, a separate
`over` local, and the `h6 += 0x14` placement retail actually executes.** Filed
since round 19 as a whole-function register-identity rotation (HARD RULE 6
STALL); it was not one.

## Baseline (preserved round-19 body rebuilt live first, unchanged)

`45/109`, `insertions 8 / deletions 8`, positional skeleton diffs 60, exact
length, zero drift -- identical to the recorded figure.

## What differed, read word by word

Retail stores slot 0's fields through `self` (`sw v0,0x8C($s2)`,
`sh v1,0x90($s2)`, ...), keeps `&self->slots8C[0]` in `$s3` only to pass it
to `StageMap__SplitFootprintSlot`, and then REUSES `$s3` for `&self->slots8C[count]`. The two
`addiu $s3,$s2,0x8C` copies are not two source statements: there is one at
the join after the `h6 < 0` test; reorg filled the `bgez $s5` delay slot from
that target (redirecting to the next label) and then deleted the copy on the
`flag != 0` path as redundant (its `j` goes past it). The `__asm__("")` +
duplicated assignment in the preserved body manufactured that shape by hand.

The `beqz $v0` delay slot `addiu $s5,$s5,0x14` also means what round 33 read
it to mean: `h6 += 0x14` runs on BOTH `flag` paths. The preserved body only
added it on `flag == 0`, which is a semantic difference from retail, not a
scheduling one. Round 33's hoist test was negative only because the rest of
the body was still the wrong shape; with the shape fixed, reverting just this
one statement to the preserved placement scores 100/109.

## Levers, each measured on a whole-image build

| variant | result |
| --- | --- |
| preserved body (baseline) | 45/109, ins/del 8/8 |
| stores via `self->slots8C[0].x`, single `slot0 =` at the join (no barrier), `h6 += 0x14` unconditional in the `h6<0` arm, flag arms reordered, `unk88 = count + 1` | 50/109, ins/del 2/2, exact length |
| ONE `slot` variable for slot 0 and slot 1, stores through it | 9/109, 1 word short (7 s-regs: `span` in `$v1` then `move $s0`) -- but top half now structurally identical to retail |
| + `count = call(); count += 1;` in the big arm and `count += 1; self->unk88 = count;` at the join | 35/109, still 1 short, only `span` wrong |
| + `over = span - 0x14` (its own local) instead of `span -= 0x14` | **109/109** |
| control: matched body with `h6 += 0x14` back on the `flag == 0` path only | 100/109 |

Six builds total. No permuter.

## Matched body

```c
void StageMap__BuildFootprintSlots(Obj866E8 *self) {
    s32 flag;
    s32 h4;
    s32 width;
    s32 height;
    s32 quadrant;
    s32 h6;
    CellRect *slot;
    s32 span;
    s32 count;
    s32 over;

    flag = 0;
    h4 = self->unk7C;
    width = self->unk80;
    height = self->unk84;
    quadrant = 3;
    if (h4 < 0) {
        h4 += 0x14;
        flag = 1;
        quadrant = 2;
    }
    h6 = self->unk7E;
    if (h6 < 0) {
        h6 += 0x14;
        if (flag != 0) {
            h4 -= 0xA;
            quadrant = 0;
        } else if (h4 < 0xA) {
            h4 += 0xA;
            quadrant = 0;
        } else {
            h4 -= 0xA;
            quadrant = 1;
        }
    }
    slot = &self->slots8C[0];
    slot->elemIdx = self->methods->slot120(self, quadrant);
    slot->h4 = (h4 >= 0) ? h4 : 0;
    slot->h6 = h6;
    span = h4 + width;
    if (span >= 0x15) {
        over = span - 0x14;
        slot->h8 = width - over;
        count = StageMap__SplitFootprintSlot(self, slot, 0, quadrant, h4, h6, width, height);
        count += 1;
        slot = &self->slots8C[count];
        slot->elemIdx = self->methods->slot120(self, quadrant + 1);
        slot->h4 = 0;
        slot->h6 = self->slots8C[0].h6;
        slot->h8 = over;
        slot->hA = self->slots8C[0].hA;
    } else {
        slot->h8 = width;
        count = StageMap__SplitFootprintSlot(self, slot, 0, quadrant, h4, h6, width, height);
    }
    count += 1;
    self->unk88 = count;
}
```

It calls `StageMap__SplitFootprintSlot` (defined next in ROM order) through a unit-local
prototype identical to that function's definition.

### Proposed learning

- **Two identical address computations on two paths into a join, one of them
  in a branch delay slot, are ONE statement at the join**: reorg's
  fill-from-target puts a copy in the delay slot and redirects the branch, and
  `redundant_insn` then drops the copy on a path that already executed it.
  Duplicating the statement and holding the copies apart with `__asm__("")`
  can hit the length and still fix the wrong register allocation.
- **A pointer register reused for a second array element is one variable
  reassigned**, not `slot0`/`slot1`: the reuse changes which pseudos conflict.
- **A delay-slot instruction on a conditional branch that runs on both paths
  is evidence about SEMANTICS when no other path computes the value.** Round
  33 was right that `h6 += 0x14` runs unconditionally; the test that
  "refuted" it was run on a body that was wrong elsewhere. Per-lever negatives
  are not joint negatives.
- **`x -= K` that is then read after a call can get a different allocation
  from `y = x - K`**: the in-place form kept `span` in a scratch register and
  cost one `move` plus one s-reg; a fresh local matched.

---

# Earlier history (round 18-60)

> **ROUND 46 (charlie): three individual-scalar narrowing attempts, all
> negative; permuter scaffold rebuilt and confirms the mismatch already
> suspected, so no search run.** Rebuilt the exact preserved 45/109 body
> from a clean `INCLUDE_ASM` baseline first per Gate 1b: confirmed 45/109,
> exact length, no drift, identical residue to round 19's own figure.
>
> This unit's own outstanding next-step (see the "First target for the next
> round" note below) named two untried axes: narrow `flag`/`quadrant`/`span`
> individually (rather than the already-tried-and-regressed wholesale
> `h4`/`h6` retype), and launch the never-run permuter search. Both done
> this round.
>
> **Narrowing, one scalar at a time, each rebuilt against the whole-image
> oracle, baseline restored between attempts:**
>
> | variant | result |
> | --- | --- |
> | `s8 quadrant` (values are provably 0-3) | **6/109, 182160 bytes of drift** -- regressed hard, length itself changed |
> | `s8 flag` (values are provably 0/1) | **45/109, IDENTICAL diff, no drift** -- fully inert |
> | `s16 span` | **18/109, 189939 bytes of drift** -- regressed hard, length itself changed |
>
> All three reverted. Consistent with round-33's finding on `h4`/`h6`
> wholesale (`s16` retype went to 1/109 with drift): this function's
> intermediate scalars are NOT the `TaskObjF__WriteMemcardSaveFile`-class "one parameter
> read narrower than its declared width" shape. Narrowing here either does
> nothing (a value GCC already treats as narrow internally, `flag`) or
> removes a real sign/range computation the compiled code actually performs
> at full width (`quadrant`, `span`), producing a shorter, wrong function
> rather than a differently-registered one. **Narrow-the-scalar is now
> tried on every non-`h4`/`h6` local with an independently-boundable range
> in this function and found inert-or-regressive on all of them; treat this
> axis as exhausted for this function specifically**, distinct from (and not
> contradicting) the still-valid general lever documented on
> `TaskObjF__WriteMemcardSaveFile`.
>
> **Permuter scaffold rebuilt from scratch** (`tools/setup-permuter.sh
> StageMap__BuildFootprintSlots <seed>`, seed = this report's own 45/109 body verbatim,
> plus a forward declaration for the not-yet-matched sibling
> `StageMap__SplitFootprintSlot`). `--debug --stack-diffs`: **base score 2153 -- 8
> insertions, 8 deletions, 4 reorderings, 61 register differences.** The
> real in-context build's own residue (confirmed immediately above, same
> build) is a **pure zero-drift, zero-insertion, zero-deletion register
> rotation** -- exact length, no missing or extra instructions anywhere.
> An isolated scaffold reporting 8 insertions and 8 deletions against a
> target the real build reproduces with NEITHER is exactly this round's
> Gate 3 failure mode ("does the scaffold's base score agree with the same
> body's score in the real in-tree build?" -- no). This independently
> reproduces round 19's own scaffold-mismatch finding (that round's
> scaffold: 11 insertions/11 deletions) on a freshly-built scaffold four
> rounds later, with a different absolute count but the same qualitative
> mismatch. **Per Gate 3, a disagreement means STOP -- no search launched.**
> This function now belongs in the same class as `StageMap__SplitFootprintSlot`,
> `StageMap__ComputeFootprintDescriptor` and `StageMap__ApplyRateEntries`: real residue is a clean register
> rotation, but no runner-buildable isolated scaffold reproduces it, so the
> permuter route needs more surrounding-file context than
> `setup-permuter.sh` currently provides (head-scale work, not a bounded
> runner attempt).
>
> **Disposition unchanged: 45/109, `INCLUDE_ASM` restored, `git diff`
> against the round-19 commit for this function is empty.** Not
> re-attempted further this round.
>
> ### Proposed learning
>
> **A function's "which single scalar to narrow" search is worth running to
> completion (every independently-range-bounded local, one at a time)
> before calling the narrow-scalar axis exhausted, but a completed search
> is itself a real, citable result** -- this report already ruled out the
> two locals with running arithmetic (`h4`/`h6`, round 33) and this round's
> three additions (`quadrant`, `flag`, `span`) complete the set of locals
> with a provable value range in this function. All five are now
> individually tested and negative; nothing narrower remains to try here.
> **A fourth function (`StageMap__BuildFootprintSlots`, joining `StageMap__SplitFootprintSlot`,
> `StageMap__ComputeFootprintDescriptor`, `StageMap__ApplyRateEntries`) now confirms the same scaffold-mismatch
> class in this one unit** -- worth flagging as a property of this specific
> header/class's functions (heavy `Obj866E8` self-pointer traffic, deep
> call chains through `self->methods->slotNN`) rather than four unrelated
> coincidences, since every one of the unit's genuinely "pure register
> rotation, zero drift" residues has now failed the same Gate 3 check.

> **ROUND 33, head — THE DELAY-SLOT HOIST HYPOTHESIS IS TESTED AND NEGATIVE.
> Its READING of retail is correct; its inference about the source is not.**
>
> A runner, tracing the asm by hand after its worktree was gone, proposed that
> the preserved body has a genuine LOGIC bug rather than a register residue:
> `h6 += 0x14;` sits nested inside `if (flag == 0)`, but retail's
> `beqz $v0, .L8004C9B0` at `0x8004C99C` carries `addiu $s5, $s5, 0x14` in its
> **delay slot**, so the increment executes on BOTH paths whenever `h6 < 0`.
>
> **The delay-slot reading is confirmed** — `asm/nonmatchings/class_3bb8c_b/
> StageMap__BuildFootprintSlots.s` lines 32-33, verified by the head. The source inference
> does not follow, and both spellings of it measure WORSE than the 45/109
> baseline:
>
> | variant | in-range | length |
> | --- | --- | --- |
> | preserved body, unchanged | 45/109 | correct |
> | `h6 += 0x14;` hoisted unconditional in the `h6 < 0` arm | **15/109** | **1 word LONG** |
> | `h6 += 0x14;` duplicated into BOTH arms | **9/109** | **1 word LONG** |
>
> Both were measured on a real build; the whole-image SHA1 went red for both
> and the out-of-range byte count (182219) is the one-word shift, not
> independent residue. Reverted; the unit is back to `INCLUDE_ASM`.
>
> **Why the inference fails is the part worth keeping.** An instruction in a
> delay slot executes unconditionally, but that is a fact about SCHEDULING,
> not about where the statement sat in the source. GCC fills a branch's delay
> slot from either side when it can prove the move safe, so a statement
> written inside ONE arm can legitimately end up there. Reading a delay slot
> as evidence of source-level placement is the same error as
> DECOMPILATION_LEARNINGS' "do not derive source order from EMISSION order",
> one level down — and the second variant above is the direct test of the
> duplicate-in-both-arms idiom, which is the shape that WOULD justify the
> hoist. It is 36 words worse.
>
> So the report's standing register-identity verdict is unchanged, and this
> lever is now spent rather than untried.


> **ROUND 33, head — ONE MORE LEVER TRIED HERE AND INERT.** A runner left an
> uncommitted experiment in the worktree at teardown: the preserved body with
> `(u16)` narrow casts added on the two `self->slots8C[0]` halfword reads
> (`h6`, `hA`), plus a forward declaration of `StageMap__SplitFootprintSlot`. That is
> round 33's `narrow-cast-defeats-strength-reduction` idiom, which was worth
> 10 words on `vmNoiseOn2` in a different unit the same round.
>
> **It does nothing here: still 45/109, and the whole-image SHA1 goes red**
> (measured by the head before discarding the change, rather than inferred
> from the fact that it was uncommitted). The residue remains the clean
> register-identity rotation this report already documents.
>
> Recorded because the idiom is newly generalised and will look like an
> obvious untried lever to the next reader. It has been tried.


**Unit:** class_3bb8c_b · **Size:** 109 words · **Status:** STALL, now 45/109 (up from 28/109) after two real CFG fixes -- register-identity rotation is confirmed as the remaining residue, not a misclassification

> **ROUND 19 (bravo) UPDATE, acting on the round-18 flag that this
> classification needed re-examination (its `--debug` showed 11
> insertions/11 deletions, not the "clean register-only" signature the
> filing implied).** Re-derived from scratch by re-reading the raw
> disassembly against the preserved 28/109 body, rather than trusting
> either number. **Two genuine, non-register-identity structural
> differences were found and fixed, both instances of idioms already
> documented elsewhere in this project:**
>
> 1. **`flag = 0;` needed hoisting to a "default value in the branch's
>    delay slot" shape.** The preserved body wrote `flag = 0;` as a plain
>    first statement followed by `if (h4>=0) {quadrant=3;} else {h4+=0x14;
>    flag=1; quadrant=2;}` -- syntactically already "first", but GCC still
>    scheduled `flag`'s zero-write INSIDE the `h4>=0` arm (needing an
>    extra unconditional jump to skip the `else`, since the value had to be
>    set specifically on that path). Retail computes `flag=0` (in `$v0`)
>    UNCONDITIONALLY before the branch even executes. Rewriting as
>    `quadrant = 3; if (h4 < 0) { h4 += 0x14; flag = 1; quadrant = 2; }`
>    (inverted polarity, no `else`, `flag=0` truly free-standing before it)
>    reproduced retail's hoisted placement and the correct `bgez` branch
>    exactly -- this is the project's established "default value, then
>    conditionally overwritten by an if with no else" idiom
>    (`CheckTriggerParity` in `DECOMPILATION_LEARNINGS.md`), applied here to a
>    flag/quadrant pair rather than a single scalar.
> 2. **`slot0 = &self->slots8C[0];` needed to be a genuinely duplicated
>    statement, one per arm of the `h6 < 0` test, not a single statement
>    after the merge.** Retail computes this address TWICE -- once in the
>    `h6>=0` branch's own delay slot, again after the nested `flag`
>    dispatch on the `h6<0` path -- the same value, recomputed
>    independently on each path rather than hoisted to a shared join
>    point. Writing it as two literal source statements (one per arm)
>    was not sufficient by itself: GCC's cross-jump pass merged them
>    right back into one shared computation (confirmed: without any
>    barrier, the compiled function is 108 words, ONE SHORT of retail,
>    because only one copy of the identical trailing instruction survives).
>    A bare `__asm__("");` placed immediately before the SECOND
>    occurrence stopped the merge and reproduced retail's exact double
>    computation, restoring the correct 109-word length. **Verified this
>    is the permitted scheduling-barrier use, not a banned register pin,
>    per CLAUDE.md rule 6's test:** removing the barrier changes ONLY
>    whether the instruction is emitted once or twice (confirmed via
>    direct rebuild-without-barrier -- same registers throughout the rest
>    of the function, just one fewer `addiu`), never which register any
>    value lands in.
>
> **After both fixes: 28/109 -> 45/109, function length matches retail
> exactly (no drift), and the CFG at every branch now agrees with
> retail's** (confirmed via `asm-differ`: every remaining structural
> marker is a plain register-content difference, `r`, at each
> instruction -- the only non-`r` markers left are alignment artifacts of
> the diff tool from the register rotation itself, not real inserted/
> deleted instructions when read against the correct total word count).
>
> **The register rotation itself (self/h4/h6/slot0/span, `$s0`-`$s7`) was
> NOT closed and does not look reachable by further reshaping**, per this
> round's established finding on this exact class (9-10 confirmed inert
> declaration-order attempts across 4 functions this round alone). One
> plausible-looking hypothesis was tried and FAILED: moving `h6 += 0x14;`
> to be unconditional within the `h6 < 0` block (reasoning from the delay
> slot's literal unconditional execution) regressed sharply to 15/109 with
> drift -- reverted immediately. **The delay slot's physical execution is
> not reliable evidence that the SOURCE computes the value unconditionally
> when an equally-valid conditional C shape reproduces the identical
> runtime behavior via the compiler's own scheduling** -- worth flagging
> as a caution against over-interpreting delay-slot placement as a
> semantic requirement.
>
> **A fresh permuter scaffold was set up with this round's actual
> 45/109-scoring body and its `--debug` base score STILL shows structural
> penalties (8 insertions, 8 deletions, 8 stack differences) that the
> real in-context build does not have** (real build: 0 drift, exactly
> 109 words). This is the THIRD confirmed instance in this exact
> unit/header of the scaffold-vs-real-build mismatch already documented
> in `IsPointOutOfBounds`'s and `StageMap__SplitFootprintSlot`'s reports -- no search was run
> against it. **This mismatch is now common enough in this one header
> (`class_3bb8c.h`, three functions) that it looks systemic to something
> about this class's real in-context register pressure, not
> coincidental** -- worth investigating directly (comparing the scaffold's
> own `base.c` against the real compile unit) before the next round trusts
> any of this header's permuter scaffolds without re-verifying `--debug`
> against a CURRENT preserved body first.
>
> Restored to `INCLUDE_ASM`; updated preserved body below.
>
> **Checked against the head's second mid-round broadcast (aggregate/
> whole-struct assignment closing 5 sibling functions elsewhere): PARTIALLY
> present but not in a form the lever applies to.** `slot1->h6 =
> self->slots8C[0].h6;` and `slot1->hA = self->slots8C[0].hA;` ARE verbatim
> copies from one `CellRect` instance to another, but `h8` (sitting
> between them in memory) is set to a DIFFERENT computed value in the same
> block, not copied -- so there is no CONTIGUOUS run of fields to fold into
> one aggregate assignment. Not applicable in this instance; noted in case
> a future round finds the lever generalizes to partial/strided copies too.
> Also checked the register-rotation residue for the "missing field-offset
> term" mechanism the same broadcast named: every remaining mismatched
> word (`asm-differ`) is a same-opcode, same-immediate, different-register
> substitution -- no missing or extra arithmetic term anywhere, ruling that
> mechanism out here.

---

# Original filing (superseded in part -- the classification held, the score and CFG match did not)

## What it does

`void StageMap__BuildFootprintSlots(Obj866E8 *self)`. Computes a "quadrant" index and a
clamped `(h4, h6)` sub-cell offset from `self->unk7C`/`self->unk7E`
(each signed, clamped into `[0, 0x14)` with a `-0xA`/`+0xA` secondary split
when both axes are negative), fills `self->slots8C[0]` via
`self->methods->slot120(self, quadrant)`, then decides whether the
horizontal footprint (`h4 + self->unk80`) fits in one 20-unit grid cell or
needs a second `CellRect` — filling that second slot directly when it
does — before calling the documented-STALL `StageMap__SplitFootprintSlot` (signature per
its own report, `docs/match-reports/StageMap__SplitFootprintSlot.md`) to (possibly) append
further slots on the OTHER axis, and finally writing the total slot count to
`self->unk88`.

## Where it stands

**The control flow, every field write, every arithmetic operation, and the
function's total compiled LENGTH are all confirmed correct.** The build
compiles to exactly 109 words with **zero bytes of drift outside the
function's own range** (`funcdiff.py` prints no "differs outside this
range" warning) — meaning the instruction COUNT and shape genuinely match
retail's, word for word. The entire residue is which of the eight
callee-saved registers (`$s0`-`$s7`) holds which of this function's roughly
eight simultaneously-live values. This is a pure register-identity residue
per CLAUDE.md's classification — same instructions, different registers —
and per that rule, not something to chase with `register T v asm("$N")` or
any operand constraint.

Retail's assignment: `$s0`=span (`h4+self->unk80`, clamped), `$s1`=h4,
`$s2`=self, `$s3`=slot0 (`&self->slots8C[0]`), `$s4`=quadrant, `$s5`=h6,
`$s6`=`self->unk80`, `$s7`=`self->unk84`. The best attempt reached
(preserved below) compiles the identical 8 values into the identical 8
registers **as a set**, but with `self` landing in `$s3` (not `$s2`) and a
corresponding shift for several others (`h4`→`$s0`, `h6`→`$s2`,
`slot0`→`$s5`, `span`→`$s1`) — a genuine permutation, not a missing or
extra value.

## What was tried (7 source-shape variants, each verified — the last 5 via an isolated `cpp|cc1|maspsx|as` reproducer for fast iteration, confirmed to faithfully reproduce the real function's exact register choices before being trusted)

1. **Initial transcription**, `if (span < 0x15) { small } else { big }` —
   wrong BRANCH POLARITY relative to retail (retail keeps the "big"/clamped
   arm as the fallthrough and the "small" arm as an out-of-line jump
   target), plus the register permutation. Score 5/109, large address
   drift (this was a real structural miss, not yet isolated to registers).
2. **Inverted the branch** to `if (span >= 0x15) { big } else { small }`
   (matching the "small arm out-of-line" idiom already established in this
   project's learnings) — fixed the polarity exactly (branch instruction at
   the CFG's first real decision point now byte-identical) and eliminated
   ALL address drift. This is the version whose register permutation is
   described above. Score 28/109 in-range.
3. Moved `slot0 = &self->slots8C[0];` to just before the h6-dispatch nested
   `if` (mirroring where retail's OWN redundant recomputation of the same
   pointer first appears in the delay-slot pattern) — made things
   substantially WORSE (4/109, frame shrank by a word), so reverted
   immediately. Confirms the redundant delay-slot occurrence is a
   scheduling artifact of retail's version, not a hint about where the
   defining C statement belongs.
4. Reordered the ten local declarations to match first-use order more
   closely (C89 allows this since they're all still top-of-block) — no
   effect whatsoever on the compiled bytes, confirming (unsurprisingly)
   that GCC 2.6.3's register allocator does not key off declaration
   order, only actual use order in the generated RTL.
5. Replaced the separate `flag` boolean (tracking whether `unk7C` was
   negative) with re-testing `quadrant == 3` (provably equivalent at the
   point it's read, since `quadrant` is set to exactly 3 or 2 by the same
   branch that would have set `flag`) — semantically cleaner, zero
   register-assignment change.
6. Reordered the three initial field reads (`width`/`height` before `h4`
   instead of after) — no effect on `self`'s register.
7. Moved `slot0 = &self->slots8C[0];` to the very top of the function
   (before even the first branch) — no effect on `self`'s register (still
   `$s3`), though it did shuffle `slot0` itself onto `$s5` in that variant.

None of the seven moved `self` off `$s3` and onto retail's `$s2`. The isolated
reproducer (repro3.c-style: exact struct layout, exact call signature,
minimal surrounding code) reproduces this SAME `$s3`-for-self assignment
independent of source reshaping tried, which argues the register choice is
not sensitive to any of the axes varied here (branch structure, declaration
order, boolean-vs-requery, read order, pointer-assignment timing).

## Preserved near-miss body (`#if 0`) -- round 19, 45/109, zero address drift

Requires `StageMap__SplitFootprintSlot`'s forward extern (already declared with this
exact signature elsewhere in this unit, see
`docs/match-reports/StageMap__SplitFootprintSlot.md`) if spliced back in — add
`extern s32 StageMap__SplitFootprintSlot(Obj866E8 *self, CellRect *slot, s32 count, s32 baseIdx, s32 p5, s32 p6, s32 p7, s32 p8);`
before it (this function is defined AFTER `StageMap__BuildFootprintSlots` in ROM order in
`src/class_3bb8c_b.c`).

```c
#if 0
void StageMap__BuildFootprintSlots(Obj866E8 *self) {
    s32 flag;
    s32 h4;
    s32 width;
    s32 height;
    s32 quadrant;
    s32 h6;
    CellRect *slot0;
    s32 span;
    s32 count;
    CellRect *slot1;

    flag = 0;
    h4 = self->unk7C;
    width = self->unk80;
    height = self->unk84;
    quadrant = 3;
    if (h4 < 0) {
        h4 += 0x14;
        flag = 1;
        quadrant = 2;
    }
    h6 = self->unk7E;
    if (h6 < 0) {
        if (flag == 0) {
            h6 += 0x14;
            if (h4 < 0xA) {
                h4 += 0xA;
                quadrant = 0;
            } else {
                h4 -= 0xA;
                quadrant = 1;
            }
        } else {
            h4 -= 0xA;
            quadrant = 0;
        }
        slot0 = &self->slots8C[0];
    } else {
        __asm__("");
        slot0 = &self->slots8C[0];
    }
    slot0->elemIdx = self->methods->slot120(self, quadrant);
    slot0->h4 = (h4 >= 0) ? h4 : 0;
    slot0->h6 = h6;
    span = h4 + width;
    if (span >= 0x15) {
        span -= 0x14;
        slot0->h8 = width - span;
        count = StageMap__SplitFootprintSlot(self, slot0, 0, quadrant, h4, h6, width, height);
        count += 1;
        slot1 = &self->slots8C[count];
        slot1->elemIdx = self->methods->slot120(self, quadrant + 1);
        slot1->h4 = 0;
        slot1->h6 = self->slots8C[0].h6;
        slot1->h8 = span;
        slot1->hA = self->slots8C[0].hA;
    } else {
        slot0->h8 = width;
        count = StageMap__SplitFootprintSlot(self, slot0, 0, quadrant, h4, h6, width, height);
    }
    count += 1;
    self->unk88 = count;
}
#endif
```

Round 9-era body (kept for the record, 28/109 -- superseded by the above):
used `if (h4 >= 0) { quadrant = 3; } else { ... }` (branch polarity that
compiled with an extra jump instead of retail's hoisted-default shape),
and a single `slot0 = &self->slots8C[0];` after the whole `h6<0` test
instead of two, barrier-separated, one per arm.

## New struct knowledge (`include/class_3bb8c.h`, additive only)

Splits the existing `pad7C[0x88-0x7C]` gap into four named fields, no
retype of anything already named:

- `s16 Obj866E8::unk7C` (+0x7C) — signed sub-cell horizontal offset.
- `s16 Obj866E8::unk7E` (+0x7E) — same convention, vertical.
- `s32 Obj866E8::unk80` (+0x80) — horizontal span, forwarded to
  `StageMap__SplitFootprintSlot`'s `p7`.
- `s32 Obj866E8::unk84` (+0x84) — forwarded to `StageMap__SplitFootprintSlot`'s `p8`.

These four exactly fill `0x7C..0x88` (`0xC` bytes: `2+2+4+4`), matching the
existing gap's size, so no other offset in the struct moves.

## Proposed learning

- **A confirmed-zero-drift, exact-length register PERMUTATION (not a
  1-2 register swap, but most/all of a function's ~8 live values shifted)
  can resist branch-structure, declaration-order, and pointer-assignment-
  timing reshaping simultaneously, and an isolated reproducer that
  faithfully matches the real function's register choices is still useful
  even when it doesn't explain WHY — it lets 5+ variants be checked in
  under a second each rather than paying a full project rebuild, which is
  what made 7 real attempts affordable inside one budget.** Consider this
  exactly the "biggest lever untried" caution from `StageMap__SplitFootprintSlot`'s own
  report (also in this header/unit): that stall's fix (per its head note)
  is "give each distinct value its own named local" to let the frame grow
  — already the case here (10 named locals, matching retail's implied live
  set) — so the SAME lever does not obviously generalize to a permutation
  residue with no length/frame difference. This looks like a genuinely
  distinct residue class from `StageMap__SplitFootprintSlot`'s missing-variable one, and
  from every register-identity case documented so far in this project's
  `%N`/receiver-timing lore (all of which involved 1-2 registers, not a
  whole-function permutation) — worth flagging for whoever revisits this
  unit's remaining stalls, since `StageMap__ComputeFootprintFromRotation` (this round's other
  target) and `IsPointOutOfBounds`/`StageMap__ConfigureRateEntry` (already-documented stalls)
  are all in the same header/class and may share whatever is driving it.

---

## Round 18 (echo) — reviewed under the corrected reading; one hypothesis tested (negative); permuter scaffold prepared but NOT run

**This is the sibling `TaskObjF__WriteMemcardSaveFile`'s own report cites as corroborating
evidence for the "register-identity permutation, not fixable by
reshaping" class** -- but `TaskObjF__WriteMemcardSaveFile`'s real cause turned out to be
one mistyped parameter (`s32` where the value is only ever used masked to
a byte), not a register rotation at all, which means every OTHER function
still filed under that class deserves a second look through the same lens
before the class itself is trusted. This function got that second look
this round, but not a full re-attempt.

**Hypothesis tested (1 real-oracle build, reverted):** the two derived
sub-cell offsets `h4`/`h6` are computed from `s16` struct fields
(`self->unk7C`/`self->unk7E`) and ultimately stored back into `s16`
`CellRect` fields (`slot0->h4`, `slot0->h6`) -- exactly the "narrower
than declared" shape that closed `TaskObjF__WriteMemcardSaveFile`. Retyping BOTH locals
from `s32` to `s16` (keeping the rest of the preserved 28/109 body
unchanged) was tried directly against `src/class_3bb8c_b.c` and
`build-and-verify.sh`: **1/109, with substantial address drift** -- far
worse than the existing 28/109 baseline, not better. Reverted immediately
(`git checkout -- src/class_3bb8c_b.c`; confirmed `build exit=0` and a
clean `OK: build matches retail SLPS_015.56` afterward). Unlike
`TaskObjF__WriteMemcardSaveFile`'s `a3` (used ONLY as `a3 & 0xFF`, no arithmetic on the
wider value), `h4`/`h6` here have running arithmetic (`+= 0x14`, `+= 0xA`,
`-= 0xA`, compared across branches) BEFORE their final narrow store, so
narrowing the intermediate computation itself changes codegen
substantially rather than just changing how one incoming register is
read at function entry -- this rules out the naive "retype the whole
local" version of the lever, though it does not rule out narrowing SOME
OTHER single value in the function (e.g. `flag`, `quadrant`, or `span`
individually) the way `TaskObjF__WriteMemcardSaveFile` narrowed exactly one parameter and
nothing else.

**Permuter scaffold prepared (`permuter-work/StageMap__BuildFootprintSlots`, plain seed,
no PERM macros -- default randomization already covers type mutations,
which is what actually found `TaskObjF__WriteMemcardSaveFile`'s fix) but the bounded search
was NEVER LAUNCHED this round** -- wind-down landed first. `--debug` base
score: **2730** (70 register diffs, 3 reorderings, 11 insertions, 11
deletions) -- notably NOT the clean "register-diffs-only" signature
`TaskObjF__WriteMemcardSaveFile`'s and `ItemList__LoadResources`'s scaffolds showed; the insertion/
deletion counts suggest the permuter's own alignment algorithm sees more
than a pure rotation here even though `funcdiff.py` independently confirms
zero address drift and exact 109-word length. Flagging this discrepancy
per the guide's own instruction to believe `--debug` over a report's
claimed class when they disagree -- **this function's classification as
"pure register permutation" should be re-verified by re-running `--debug`
against the CURRENT preserved body before the next round trusts either
number.**

**Disposition:** still `INCLUDE_ASM`, unchanged this round. First target
for the next round on this unit: launch
`PATH=$PWD/permuter-work/bin:$PATH timeout 600 .venv/bin/python3
tools/decomp-permuter/permuter.py -j 6 --stop-on-zero --best-only
permuter-work/StageMap__BuildFootprintSlots` (scaffold already provisioned, no setup
needed) and, independently, try narrowing `flag` and `quadrant` (both
provably 2-3 valued) one at a time rather than `h4`/`h6` wholesale.

### Proposed learning

**`TaskObjF__WriteMemcardSaveFile`'s fix does NOT validate this function's "not fixable by
reshaping" classification, and citing one as evidence for the other was
the mistake to catch.** Both were filed under the same class name because
they presented identically (same instruction count, same length, "just"
different registers) -- but they had different causes, and the fix that
closed one (retype a masked parameter) does not transfer to the other
(tried directly, made things dramatically worse). The class name
describes a SYMPTOM (zero-drift full-function register rotation), not a
diagnosis, and every function filed under it should be independently
checked for a narrow-typed value before being counted as corroboration
for any other instance's difficulty.

---

## Round 60 (charlie) — NON_MATCHING body promoted, round 60

Track 1b promotion. Score re-verified unchanged (45/109, exact length,
zero drift) before promoting. Placed the existing preserved body — the
one carried in `src/class_3bb8c_b.c` since round 19, git-diff-empty
against that commit — inside `#ifdef NON_MATCHING`, with `INCLUDE_ASM`
restored in the `#else`. No source change beyond the wrapper and comment;
both oracles green: `./build-and-verify.sh` (exit 0, `OK: build matches
retail`) and `tools/check-nonmatching.sh` (exit 0). Disposition otherwise
unchanged (still `INCLUDE_ASM` in the verified build, still a HARD RULE 6
register-identity stall).

## Naming

**Tier B.** Turns the column/row/width/height computed by
`StageMap__ComputeFootprintFromRotation` into one or two
`self->gridSlots[]` entries, splitting via `StageMap__SplitFootprintSlot`
when the footprint would run past the grid's 20-unit edge. Mechanics
evidenced; game-level purpose not.
