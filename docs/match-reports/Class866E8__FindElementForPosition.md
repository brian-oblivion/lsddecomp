# Class866E8__FindElementForPosition -- MATCHED round 73 (70/70, exact length, whole-image SHA1 green)

> Renamed from `func_8004C470` on 2026-09-24 (tools/rename.py). Address 0x8004c470.

REVISITED, round 73: MATCHED 70/70 (field named at the add, assigned inside the bound test); names/types not relevant (existing Unk14Obj/Unk54Struct views reused unchanged)

## Round 73 (bravo) -- revisit, MATCHED

**Preserved `#ifdef NON_MATCHING` body rebuilt first, unchanged:** 69/70,
`insertions 0 / deletions 0`, positional skeleton diffs 1, exact length;
the one word at vram 0x8004C500, retail `addu $v0,$v1,$s4`, built
`addu $v0,$s4,$v1`.

**Mechanism, measured with `cc1 -dr` on a standalone copy of the function
(pinned cc1 and flags):** in the `.rtl` dump (before any optimisation)
the second bound is already `(plus:SI (reg/v tol) (reg load))` for BOTH
`tol + r->unk20.w` and `r->unk20.w + tol`. The expander puts a MEM-derived
operand of a commutative `+` second whatever the source says, which is
why operand order alone was inert for five rounds. A NAMED VARIABLE
(`reg/v`) keeps its source position, which is why round 38's
`w = r->unk18.w; ... w + tol` fixed the first bound. Retail's order
(field first) therefore means the field was a named variable at the add
in the second bound as well.

The reason round 38's "hoist the second one too" regressed is the load
ORDER, not the add: a `w = r->unk20.w;` statement before the `if` puts
the field load ahead of the `arg1->unk8` load in insn order, and with no
load-delay dependency to break the tie the scheduler keeps it (the first
bound gets away with it because `r` has only just been loaded). Assigning
`w` inside the upper-bound test keeps `arg1->unk8 >= r->unk20.w` first,
loads in retail's order, and CSE merges the second field read into the
first.

Builds, one per line (funcdiff score):
- preserved NON_MATCHING body: 69/70, ins/del 0/0 (the addu)
- row-pointer local `t = &r->unk18.w`, `w = t[0]`, 2nd bound `t[2] + tol` (the broadcast's CalcDreamColor lever): 69/70, identical diff
- row-pointer local, no `w`, `t[0] + tol` / `t[2] + tol`: 68/70 (both adds flipped)
- reuse `w`: `w = r->unk20.w;` statement before the 2nd `if`, `w + tol`: 68/70; the addu now right, `lw a0,8(s3)` / `lw v1,0x20(a1)` swapped
- `arg1->unk8 >= (w = r->unk20.w) && ... < w + tol`: 68/70, same load swap (the assignment operand is expanded first)
- same shape for both bounds: 68/70, same load swap
- `static __inline__ InRange(v, lo, n)` for both bounds: 22/70 with drift (materialises a boolean)
- **`arg1->unk8 >= r->unk20.w && arg1->unk8 < (w = r->unk20.w) + tol`, first bound as filed: 70/70, `build exit=0`**
- **the same inline-assignment form for BOTH bounds, no separate `w =` statement: 70/70, `build exit=0`** (kept, because the two bounds read the same way)

The `#ifdef NON_MATCHING` / `#else INCLUDE_ASM` / `#endif` block is
replaced by the matched C.

### Proposed learning

**Commutative `+` operand order is decided at EXPAND time: a MEM operand
goes second, a named variable keeps its source position.** So in the
"commutative-add operand-order" class, if the operand retail puts FIRST
is a field load, the source had that field in a named variable at the
add. Operand-order flips on an unnamed field are inert by construction.
If a `v = field;` statement then reorders loads, assign it inside the
expression that uses it (`a < (v = p->f) + n`) so the other load stays
first in insn order.

## Earlier history (superseded by the match above)

#### Old title: Class866E8__FindElementForPosition -- STALL: length EXACT (70/70 words, no drift); **69/70 raw word-match (round 38, up from 68/70)**; first and ONLY real diff at file 0x3CD00 / vram 0x8004C500 -- the `addu` in the SECOND bounds comparison.

NON_MATCHING body promoted, round 73

> **ROUND 73 (echo): NON_MATCHING body promoted, track 1b.** The round-38
> 69/70 body (hoist the first bounds comparison's field into a local `w`,
> write `w + tol`; second comparison left unhoisted, per the confirmed
> asymmetry) was placed in `src/class_3bb8c.c` in the project's
> `#ifdef NON_MATCHING ... #else INCLUDE_ASM ... #endif` reader form, in
> ROM order, with a comment recording the score, the residue class
> (register-identity, HARD RULE 6 STALL), this report's path, and that the
> body is hand-derived -- the permuter (~183,331 iterations across two
> independent searches, both never beating base score) only supplied the
> lead, which was translated to idiomatic C and confirmed by hand against
> the whole-image oracle across the round-38 variant table, not copied
> from raw permuter output. No source bytes changed:
> `./build-and-verify.sh` and `tools/check-nonmatching.sh` both green.

> **ROUND 47 (charlie): Gate 1b re-verified 69/70, no drift** (rebuild via
> `make clean && make extract` then the standard `#if 0`->`#if 1` swap).
> Identical single-word diff at vram `0x8004C500`, unchanged from round
> 46.
>
> **Permuter check 3, both searches:** round 38's and round 41's scaffolds
> each independently recorded base score 10, 2 register differences, ZERO
> reorderings/insertions/deletions/branch/stack differences -- matching
> this function's own documented residue (a single commutative-`addu`
> operand-order flip, nothing else) exactly. This function is not one of
> the five confirmed family scaffold-mismatch cases; the combined
> ~183,331 iterations across two independent RNG runs stand as real,
> validated negatives.
>
> Checked the 12th lever (hoist a field pair used on every path into
> locals PER `if`) against the residue: does not apply -- the two bounds
> comparisons are separate `if`s over DIFFERENT fields (`unk18` and
> `unk20`), each read once, not a shared pair read in one `if`'s condition
> and reused in its body. This function's own asymmetric hoist-plus-
> operand-order combinatorics were already exhaustively tabulated in
> round 46 (six cells, all measured); nothing in this round's tool
> reopens that table. **Disposition unchanged: 69/70, one word.** No new
> attempt made; per round 46's own framing this function's cheap levers
> are spent and its 183k-iteration permuter floor is the most
> exhaustively tested in the corpus. `INCLUDE_ASM` untouched throughout.

> **ROUND 46 (charlie): Gate 1b re-verified 69/70, no drift; two more
> combinatorial variants tried, both worse; disposition unchanged and this
> round's own new levers checked and found inapplicable -- SKIPPING
> further attempts on this function this round.** Rebuilt the committed
> 69/70 body from a clean `INCLUDE_ASM` baseline first: confirmed 69/70,
> exact length, identical single-word diff at vram `0x8004C500`.
>
> **This round's two new project-wide levers do not apply here, checked
> directly rather than assumed:** the beq/bne-delay-slot-polarity lever
> needs a same-length branch-only residue -- this residue is a commutative
> `addu` feeding an `slt`, not a branch, so there is no delay slot to read.
> The split-combined-declaration lever is scoped (per this round's own
> negative from `Snd_setVabAttr`/`NoteOn`) to a value crossing a
> CALL boundary whose timing is already confirmed correct; nothing in this
> function's residue crosses a call at all -- both bounds comparisons are
> straight-line arithmetic inside the loop, no calls nearby. Neither lever
> is a fresh axis for this function.
>
> **Two variants tried anyway, filling in the one combinatorial cell round
> 38's own table left untested (hoist the SECOND comparison's field alone,
> mirroring the transform that fixed the FIRST):**
>
> | variant | result |
> | --- | --- |
> | guarded hoist of 2nd field into `h`, `__asm__("")` barrier before the 2nd `if` (targeting round 39's "hoist regresses to a different diff" instruction-order swap) | **65/70, regressed** |
> | hoist ONLY the 2nd field into `h`, write `h + tol`; 1st comparison left UNHOISTED (`tol + r->unk18.w`, the mirror image of what round 38 found for the 1st alone) | **67/70, regressed** (a NEW diff site also appears at `0x8004C4DC`, one earlier than the usual residue) |
>
> Both reverted (`git checkout -- src/class_3bb8c.c`; clean
> `OK: build matches retail` confirmed after each). This completes round
> 38's own combinatorial table: hoist-1st-only (69/70, the current best),
> hoist-2nd-only (67/70), hoist-both two ways (68/70, 67/70), hoist-1st
> plus a barrier on the 2nd (65/70), flip-both-no-hoist (68/70). **Every
> one-field, two-field and barrier-augmented combination of this lever has
> now been measured; the asymmetry favors the FIRST comparison exclusively
> and nothing narrows the gap further.**
>
> **Disposition: unchanged, 69/70, one word at vram `0x8004C500`.** This
> function now carries two independent ~90k-iteration permuter searches
> (~183k iterations total, both against a scaffold independently confirmed
> to score the same residue as the real build -- unlike the Gate-3
> failures found elsewhere in this unit this round), plus roughly 20 hand
> variants across rounds 20, 32, 38, 39, 41 and this one. **SKIPPING this
> function for the rest of this round** -- the ranking that puts it first
> in the queue (69/70, closest miss in the corpus) is itself evidence its
> cheap levers are spent, per this round's own framing, and nothing in
> this round's lever set or in the one remaining untested combinatorial
> cell moved it. Not re-attempted further.
>
> ### Proposed learning
>
> **Completing a combinatorial table is worth doing even when every
> individual cell is expected to lose, because "expected to lose" is not
> the same as "measured to lose", and the completed table is itself the
> deliverable that lets the next round skip with confidence instead of
> re-deriving partial coverage.** This function's asymmetry (hoisting the
> FIRST comparison's field helps, hoisting the SECOND does not, in any
> combination) was previously established from four of what are now six
> possible one/two-field hoist combinations; the two added this round
> (2nd-alone, 1st-hoist-plus-barrier) both lose, completing the set with no
> surprises -- but the fact that they had to be checked, not assumed, is
> the point. A function this exhaustively searched is a good candidate for
> "spend one round completing the known table, then stop", not for
> open-ended re-exploration.

> **ROUND 41 (alpha): Gate 1b re-verified 69/70, no drift; a SECOND
> independent permuter search run (fresh RNG, own scaffold), also negative
> -- 93,957 iterations never beat the base score.** Rebuilt the exact
> preserved body from a clean `INCLUDE_ASM` baseline first: confirmed
> 69/70, exact length, single-word diff at vram `0x8004C500`, identical to
> round 40's own figure.
>
> Round 40 explicitly judged this function exhausted and skipped it in
> favor of never-searched ground. This round staffed it anyway, once the
> unit's other targets (the dead-reload screen on `Class866E8__ApplyRateEntries`/
> `Class866E8__ComputeFootprintDescriptor`, both negative; two structural probes each on
> `Class866E8__BuildRateEntries`/`Class866E8__LoadElementResources`, all four negative) turned up nothing --
> per this round's own guidance to treat a "permuter-exhausted" verdict as
> needing a CHANGED state, not a repeat, a **second independent RNG run**
> (not a rerun of round 38's own search) qualifies, since round 38's run
> was a single seed and CLAUDE.md documents elsewhere in this project
> (`Class866E8__ComputeRateEntry`'s own round-32 history) that "a permuter plateau is not
> an exhaustion proof" until a SECOND independent search has also failed.
>
> **Scaffold, validated before searching:** `tools/setup-permuter.sh
> Class866E8__FindElementForPosition <seed>` (seed = this report's own 69/70 body verbatim),
> `--debug --stack-diffs` reported **base score 10, 2 register differences,
> 0 reorderings/insertions/deletions/branch/stack differences** --
> identical to round 38's own recorded residue (the single `addu v0,v1,s4`
> vs `addu v0,s4,v1` operand-order flip). Trusted.
>
> **Search:** `timeout 900 ... -j 6 --stack-diffs --stop-on-zero
> --best-only`, exceeded the tool's own foreground window and continued in
> the background; polled directly via the PID until it exited.
> **`permuter rc=124`** (the 900s bound fired cleanly, captured on the very
> next command, not inferred). **93,957 iterations. The best score
> observed across the entire run was 10 -- the base score itself; nothing
> ever scored lower.** (Score-10 candidates recurred 20,974 times,
> confirming the scorer is working and finding the seed's own equivalence
> class repeatedly; every other observed score was 15 or higher.)
>
> **Disposition: genuine second negative, not inconclusive.** Combined
> with round 38's own 89,374-iteration search (also base-10, never
> improved), this one-word residue has now survived **two independent
> permuter searches totaling ~183,331 iterations**, both against
> `--debug`-validated scaffolds confirmed to score the same residue as the
> real build, on top of round 38's five manual variants and round 39's
> four more. **This is now the most exhaustively permuter-tested residue
> in the unit alongside `Class866E8__ComputeRateEntry`'s two 144k/40k-iteration searches**
> -- read as a hard floor of this specific commutative-operand-order class
> under this compiler, not an unexplored lead. Not re-attempted further
> this round; restored to `INCLUDE_ASM` unchanged, `git diff` against the
> round-38 commit for this function is empty.
>
> ### Proposed learning
>
> **A second independent permuter run is worth the bounded cost even after
> a first run plateaued at the same non-zero score, but two flat runs in a
> row is a real stopping signal, not just an unlucky pair.** This report
> and `Class866E8__ComputeRateEntry`'s now each carry two independently-seeded searches
> (183k and 184k total iterations respectively) that never beat their own
> base score -- both single-word/two-register commutative-operand-identity
> residues. Given this project's confirmed, cross-function
> commutative-canonicalization class (this function, `Class866E8__LoadElementResources`'s
> `or`/`addu` sites, `Class866E8__ComputeRateEntry`'s `$v0`/`$v1` accumulators), a THIRD
> search on any of these is unlikely to be a good use of a bounded permuter
> slot; a differently-shaped seed (not just fresh RNG on the same shape)
> would be needed to find new ground, and no such reshaping has been
> identified for any of them yet.

> **ROUND 40 (bravo): Gate 1b re-verified 69/70, no drift; DELIBERATELY
> SKIPPED per this round's own staffing instruction.** Rebuilt the exact
> preserved body from a clean `INCLUDE_ASM` baseline before touching
> anything: confirmed 69/70, exact length, single-word diff at vram
> `0x8004C500`, identical to round 39's own figure. This round's brief
> named this function specifically as the one to read-then-skip if judged
> exhausted rather than re-confirm: round 38 already ran a real permuter
> search here (89,374 iterations against a validated scaffold, reaching
> score 10 from base 20 but never 0) and round 39 tried four more manual
> variants (operand-order flip, `&&`-clause swap, two hoist-before-either
> placements, a scheduling barrier) on top of round 38's own five, all
> inert or regressive. That is a real search plus nine manual attempts on
> a ONE-WORD residue with a well-understood, twice-independently-confirmed
> commutative-operand-order-plus-hoist asymmetry. Judged exhausted; no new
> attempt made. Time went to the three never-searched functions in this
> unit instead.

**Unit:** class_3bb8c · **Size:** 70 words · **Status:** STALL, 68/70 best
(up from 63/70 this round). ~14 attempts.

> **ROUND 32 (bravo2): closed "Residue 2" entirely, 63/70 -> 68/70.** The
> fix: move `threshold -= 0x800` into the for-loop's own increment-
> expression (`for (; i < 7; i++, threshold -= 0x800)`) instead of a
> trailing body statement. Round 20's "Residue 2" was that `continue` (the
> deep-threshold-check early exit) skipped the trailing
> `threshold -= 0x800;` statement in C while retail applies the decrement
> on EVERY path that reaches the loop-back branch, continue included (it
> lives in that branch's own delay slot). Writing the decrement as part of
> the for-loop's increment-expression makes `continue` apply it too,
> matching retail exactly -- confirmed via `asm-differ`, no more missing/
> extra instructions anywhere in that half of the function. Init statement
> order (`i = 0; tol = 0xA000; threshold = 0;`) had to stay exactly as
> the preserved body already had it -- an earlier variant that folded
> `threshold = 0` into the for-loop's init-clause (`for (i = 0,
> threshold = 0; ...)`) reordered the prologue's `sw $s1`/`sw $s4` stores
> relative to retail and cost 4 words; keeping the three inits as separate
> statements before the loop avoided that entirely.
>
> Remaining residue is now ONLY "Residue 1" (2 words): the commutative
> `addu` register-operand-order class, already confirmed inert (both
> operand-textual-orders tried, identical wrong output) both here and
> independently on `Class866E8__LoadElementResources` this same project. Not re-attempted --
> this is now a stable, twice-confirmed GCC 2.6.3 RTL-canonicalization
> property, not a per-function coincidence. See the near-miss body below
> for the current best C (restored to `INCLUDE_ASM` in `src/`, per
> convention).

> **ROUND 20 (charlie): re-verified, one correction to this report's own
> prose.** Rebuilt the exact preserved body below from a clean
> `INCLUDE_ASM` baseline (one fix needed first: `Unk14Obj::unk18`/`unk20`
> were retyped to `union { s32 w; u16 h; }` by a LATER round's
> `Class866E8__ComputeFootprintDescriptor`/`Class866E8__BuildRateEntries` work after this report was written, so
> the preserved body's plain `r->unk18`/`r->unk20` no longer compiled --
> `.w` added at both sites, a mechanical header-drift fix, not a residue
> change). Confirmed: still 63/70, no drift, identical residue.
>
> **Correction: Residue 1's prose has retail and built SWAPPED.** Reading
> `asm/nonmatchings/class_3bb8c/Class866E8__FindElementForPosition.s` directly (not a
> diff-tool's relabeled address) shows retail's OWN instruction at
> `3CCDC`/`3CD00` is `addu $v0, $v1, $s4` (freshly-loaded field FIRST,
> tolerance SECOND) -- the built object's `addu $v0, $s4, $v1` (verified via
> `mipsel-linux-gnu-objdump` on `build/src/class_3bb8c.c.o`) is the
> REVERSED one, not the other way around as originally written below. The
> underlying FINDING is unaffected and re-confirmed independently this
> round: rewriting the C as `r->unk18.w + tol` (field first, matching
> retail's true order) produced the IDENTICAL wrong `addu $v0, $s4, $v1` --
> source operand order does not drive this compiler's choice either way.
> Left as-is below (not re-edited into the body text) since the residue
> ITSELF, not just its description, is what matters; whoever reads this
> next should trust this correction over the "Residue 1" section's own
> wording.
>
> This same commutative-operand-order class was independently confirmed a
> SECOND time this round, on a different function and different operator
> (`or`, plus another `addu`) -- see `Class866E8__LoadElementResources`'s report. Two
> confirmations across two functions is enough to treat this as a stable
> GCC 2.6.3 property, not a per-function coincidence: **do not spend
> further attempts reordering commutative operands hoping to match a
> register-operand order** -- screen for this shape (same opcode, same
> immediate/operands, only register POSITIONS swapped) and file it under
> this class directly.

## Best body reached, ROUND 32 update (68/70, restored to `INCLUDE_ASM` in `src/`)

```c
#if 0
Elem *Class866E8__FindElementForPosition(Obj866E8 *self, Unk54Struct *arg1) {
    s32 i;
    s32 tol;
    s32 threshold;
    Elem *candidate;
    Unk14Obj *r;

    i = 0;
    tol = 0xA000;
    threshold = 0;
    for (; i < 7; i++, threshold -= 0x800) {
        candidate = self->methods->slot118(self, i);
        r = candidate->unkC->unk14;
        if (arg1->unk0 >= r->unk18.w && arg1->unk0 < tol + r->unk18.w) {
            if (arg1->unk8 >= r->unk20.w && arg1->unk8 < tol + r->unk20.w) {
                if (self->unk68->unk4 == 0) {
                    return candidate;
                }
                if (threshold >= arg1->unk4) {
                    if (threshold - 0x800 >= arg1->unk4) {
                        continue;
                    }
                    return candidate;
                }
            }
        }
    }
    return 0;
}
#endif
```

(`r->unk18`/`r->unk20` are written `.w` here because `Unk14Obj::unk18`/
`unk20` were retyped to `union { s32 w; u16 h; }` by a later round's work
on `Class866E8__ComputeFootprintDescriptor`/`Class866E8__BuildRateEntries`, after this report's original body was
written -- mechanical header-drift fix, not a residue change, same as
round 20 already noted.)

### Pre-round-32 body (63/70, kept for reference -- the `threshold -= 0x800;`
### trailing statement is what round 32 moved into the for-loop clause)

```c
#if 0
Elem *Class866E8__FindElementForPosition(Obj866E8 *self, Unk54Struct *arg1) {
    s32 i;
    s32 tol;
    s32 threshold;
    Elem *candidate;
    Unk14Obj *r;

    i = 0;
    tol = 0xA000;
    threshold = 0;
    for (; i < 7; i++) {
        candidate = self->methods->slot118(self, i);
        r = candidate->unkC->unk14;
        if (arg1->unk0 >= r->unk18.w && arg1->unk0 < tol + r->unk18.w) {
            if (arg1->unk8 >= r->unk20.w && arg1->unk8 < tol + r->unk20.w) {
                if (self->unk68->unk4 == 0) {
                    return candidate;
                }
                if (threshold >= arg1->unk4) {
                    if (threshold - 0x800 >= arg1->unk4) {
                        continue;
                    }
                    return candidate;
                }
            }
        }
        threshold -= 0x800;
    }
    return 0;
}
#endif
```

This scores 63/70 -- correct SIZE (no "differs outside range" warning), correct
control-flow graph (every branch target lines up with retail's), correct
struct layout. The residue is two DIFFERENT-looking but possibly related
cosmetic issues, both confirmed NOT reachable by source reshaping within this
round's attempt budget:

### Residue 1: commutative-add operand order (2 words)

Retail's window-bound computation is `addu v0,s4,v1` (tolerance register
first, freshly-loaded field second); mine consistently compiles to
`addu v0,v1,s4` (reversed) REGARDLESS of which order the two operands are
written in the C (`tol + r->unk18` and `r->unk18 + tol` both produced the
SAME wrong order). This looks like a GCC-internal RTL canonicalization choice
for a commutative op feeding directly into an `slt`, not something the C
source controls.

### Residue 2: increment/decrement scheduling at the loop-continue merge (5 words)

Retail places `s1++` (the loop counter) at the SHARED merge point reached by
every "no match, keep scanning" exit (both window-check failures AND the
final threshold-check failure), and defers `s0 -= 0x800` (the threshold
decrement) to the DELAY SLOT of the loop-back-edge branch specifically --
i.e. the decrement only physically executes on iterations that continue,
even though by C semantics both quantities are equally dead on the exiting
iteration. Every variant tried (moving `threshold -= 0x800` to the for-loop's
own iteration-expression slot with an explicit `i++;` before each `continue`,
reordering `i++`/`threshold -=` as plain body statements either way) either
reproduced the SAME 63/70 residue or scored WORSE (37/70, 53/70) by also
perturbing the ALREADY-CORRECT prologue register assignment (`s0`/`s1`/`s4`
save order) established for the window-check portion of the function.

**Attempts and what each one taught:**
1. Naive nested `if`/`for`, `&&`-combined conditions throughout, threshold
   decrement as the for-loop's own increment-expression semantics (implicit,
   after the body): wrong branch POLARITY entirely (44/70, then discovered
   the `entry->unkC->unk14` field offset was mistyped 0xC instead of 0x14 --
   fixed, see below).
2. Fixed `UnkCObj::unk14` offset (0xC -> 0x14): jumped to 44/70 with correct
   offsets; remaining residue was the addu operand order (2 instances) plus a
   completely different-looking structural issue in the final threshold
   check.
3. Tried `&&`-combined final threshold test
   (`if (threshold >= arg1->unk4 && threshold - 0x800 < arg1->unk4) return
   candidate;`): no change from (2).
4. Split into nested `if`s matching retail's actual branch structure
   (confirmed via reading the ORIGINAL raw asm's label targets, not the
   diff tool's relabeled addresses): jumped to 63/70, no more "differs
   outside range" warning -- CFG and size now correct.
5-8. Various reorderings of `i++` vs `threshold -= 0x800` (for-loop
   increment-expression swaps, explicit statement order swaps, `do`/`while`
   forms): none improved past 63/70; two variants regressed to 37/70 and
   53/70 by additionally breaking the prologue's `s0`/`s1`/`s4` save order,
   which the 63/70 version gets right.

## Proposed learning

**A commutative binary op (`addu`) whose SOURCE OPERAND TEXTUAL ORDER does
not affect the compiled REGISTER OPERAND ORDER is a real, reproducible
residue class distinct from the documented "prologue store order" one** --
confirmed here across two independent instances in the same function (both
window-check bound computations), both immune to source reordering. Treat an
`addu`/`add`/`or`/`and` (or similar commutative op) with correct VALUES and
correct BRANCH TARGET but swapped register operands as a scheduling-class
residue, not a source-level bug, before spending further attempts on it.

**A "shared merge point vs. deferred-into-delay-slot" split between two
loop-tail updates (an index increment always applied vs. a second quantity
only applied on the CONTINUE edge) resisted every reshaping of statement
order and for-loop increment-slot placement tried this round.** Worth a
permuter run if this exact shape recurs: the search space (which of two
loop-tail updates lands in the back-edge branch's delay slot) is narrow and
mechanical enough that decomp-permuter's own scheduling exploration might
close it faster than further manual attempts, but that is unverified --
nobody has run it against this specific function yet.

## ROUND 38 (alpha, salvaged by head): preserved body REBUILT and confirmed at 68/70 words

Runner alpha was staffed onto this unit and died to an infrastructure error
(org API 403) with nothing committed. The head restored the worktree and
re-measured this report's preserved body directly, by flipping its
`#if 0` guard to `#if 1` and commenting out the matching `INCLUDE_ASM`,
one body at a time against the whole-image oracle.

**Result: `68/70 words`, compiling and LINKING cleanly (zero hits on the
compile-error grep).** The recorded figure is accurate and the body is real.

This is the Gate 1b "rebuild before trusting" check, and it matters here
because round 37 measured roughly one inherited body in six carrying a false
drift-free claim, plus one body that could never have linked at all (it
called a symbol since renamed, so its figure had measured nothing). **All
four preserved bodies in `class_3bb8c` were rebuilt this round and all four
are honest** — `Class866E8__FindElementForPosition` 68/70, `ComputeCellWorldOffsets` 58/73,
`Class866E8__BuildRateEntries` 125/140, `Class866E8__LoadElementResources` 130/150. No stale figure and no
never-linked body in this unit.

No new lever was tried — alpha died before attempting one. This is a
confirmation, not a negative result, and the function's permuter status is
unchanged.


## ROUND 38 (alpha + head): 68/70 -> 69/70, and the "immune to source reordering" verdict is CORRECTED

Runner alpha launched a permuter search on this function, then stopped its
turn waiting for it (the documented section 2c wait-loop) and never collected the
result. `SendMessage` was unavailable to the head this round, so the head
collected and translated it. **Alpha is not at fault for the wait-loop
outcome and its setup work was correct** -- the base score it computed (20)
matched this report's 2-instruction/4-register residue exactly.

### The search

Two searches exist in `permuter-work/Class866E8__FindElementForPosition/`: `output-10-1` from the
first (infrastructure-killed) attempt on 2026-09-12 and `output-10-2` from
this round. **Both reached score 10 from a base of 20; neither reached zero.**
This round's ran **89,374 iterations**.

**The exit code was NOT captured and cannot be recovered.** Alpha never
collected it, and the log's tail shows only a multiprocessing shutdown
warning. Per the 124-vs-137 rule this means the run's terminator is unknown --
do not record this as "the bound fired". What IS established is the iteration
count and that the search did not reach zero. Note this is NOT a
permuter-exhausted verdict.

### Translating the lead -- and what it corrects

The score-10 candidate carried the familiar duplicate-alias artifact
(`new_var2 = r->unk18.w; new_var = new_var2;`) plus one real signal: it
rewrote `tol + r->unk18.w` as `new_var + tol`. Translated to idiomatic C and
measured against the whole-image oracle, one variant per build:

| variant | score |
| --- | --- |
| baseline as filed | 68/70 |
| **hoist 1st into `w`, write `w + tol`** | **69/70** |
| hoist 1st, and flip 2nd's operand order too | 69/70 |
| hoist BOTH, 2nd as `h + tol` | 68/70 |
| hoist BOTH, 2nd as `tol + h` | 67/70 |
| flip both operand orders, hoist NEITHER | 68/70 |

**This report has said twice that the `addu` operand order is "immune to
source reordering", and that claim is wrong as stated.** It was tested by
flipping operand order alone -- which is genuinely inert, reconfirmed here at
68/70. The lever is **hoisting the field into a local AND writing `w + tol`,
both together**. Neither half moves anything on its own, which is exactly why
two rounds of testing them separately concluded the residue was immutable.

**And the lever is ASYMMETRIC, which is the part to carry forward.** Applying
the same transform to the second comparison does not double the gain, it
REVERSES it -- 69/70 back to 68/70, and to 67/70 with the other operand
order. Whatever retail's source does here, it does it once.

### Disposition

**69/70, exact length, one word remaining** at vram `0x8004C500` -- the mirror
`addu` in the second bounds comparison, the one the asymmetry protects. The
`INCLUDE_ASM` is restored and the preserved body in `src/class_3bb8c.c` has
been updated to the 69/70 form.

### Proposed learning

**A "commutative operand order is immune to source shape" verdict is only as
strong as the transforms tried WITH it.** Operand order alone was inert here
across two rounds and three attempts; operand order plus a hoist into a named
local moved it immediately. A residue that survives each of two levers
independently has not been shown to survive their combination, and the
combinatorial gap is where this one was hiding.

## ROUND 39 (charlie): re-verified 69/70, four more variants tried, all inert or worse -- disposition unchanged

Rebuilt round 38's 69/70 body directly (flip `#if 0`->`#if 1`, comment out the
`INCLUDE_ASM`) before touching anything, per Gate 1b: **confirmed 69/70,
exact length, single-word diff at vram `0x8004C500`** (retail
`addu $v0,$v1,$s4` / built `addu $v0,$s4,$v1`) -- identical to the round 38
report's own recorded figure and diff site. Objdump of `build/src/class_3bb8c.c.o`
confirms every OTHER instruction in the function byte-matches retail one for
one; the residue is exactly the single operand-order flip on this one `addu`,
nothing else moved.

Four variants tried this round, targeting exactly this word, each rebuilt
against the whole-image oracle:

| variant | result |
| --- | --- |
| flip 2nd comparison's source operand order alone (`r->unk20.w + tol` instead of `tol + r->unk20.w`), 1st comparison unchanged | 69/70, IDENTICAL diff -- confirms (again) operand-order-alone is inert, this time on top of the 69/70 baseline rather than the old 68/70 one |
| swap the two `&&` clauses of the 2nd comparison (`< h+tol` tested before `>= h`) | REGRESSED to 29/70 with 183241 bytes of drift -- the `&&` evaluation order is load-bearing for the branch/CFG shape, not just register choice; reverted immediately |
| hoist 2nd field into a new local `h`, declared and assigned **before the first `if`** (the "hoist BOTH before either is consumed" project lever from this round's brief) | REGRESSED to 20/70 with 183241 bytes of drift -- an extra load now executes unconditionally every loop iteration instead of only when the first window check passes, so this is length-changing, not just a register reshuffle. **The hoist-both-before-either lever does NOT apply here**: unlike `SsUtGetVVol`-class cases, retail does NOT compute both fields before consuming either -- the second field's load is genuinely gated behind the first comparison's outcome (confirmed by the correct-length 69/70 baseline's own disassembly, where the second `lw v1,0x20(a1)` sits after the first branch, not before it) |
| hoist 2nd field into a new local `h`, declared and assigned right after entering the first `if` (guarded, so length-preserving) | 68/70, correct length, but a DIFFERENT diff (`0x8004C4EC`/`0x8004C4F0`, a instruction-order swap between the `lw a0,8(s3)` and `lw v1,0x20(a1)` loads) -- reproduces exactly the report's own recorded "hoist BOTH, 2nd as h+tol" regression from round 38's translated permuter lead, now confirmed independently |
| bare `__asm__("")` scheduling barrier immediately before the 2nd `if`, baseline C otherwise unchanged | 69/70, IDENTICAL diff -- no effect, consistent with project rule 6's own test (a scheduling barrier only reorders, it does not touch register/operand identity) |

**Disposition unchanged: 69/70, exact length, one word at vram `0x8004C500`.**
Restored to the committed 69/70 form (`tol + r->unk20.w`, not hoisted) and
`INCLUDE_ASM` re-enabled; `git diff` against the round-38 commit for this
function is empty. Not re-attempted further this round -- four fresh variants
covering both the "hoist both" lever and an evaluation-order lever, on top of
round 38's own five, is enough combinatorial coverage on a single-word residue
to call this a confirmed stall rather than an unexplored one.

### Proposed learning

**The hoist-both-before-either lever has a precondition worth stating
explicitly: it requires retail to actually LOAD both values before consuming
either.** `Class866E8__FindElementForPosition`'s second field load is gated behind the first
comparison's branch in retail's own disassembly (checked directly, not
inferred), so no source shape that computes it unconditionally can be
length-preserving, let alone byte-exact -- the lever's "adjacent loads,
later consumers" diagnostic from this round's brief is exactly the right
test, and it says no here. Read the retail `.s` for that adjacency before
spending an attempt on this lever, not after.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004C470` | `Class866E8__FindElementForPosition` | B | Occupant of `D_800866E8` +0x11C (`slot11C`), called by `Class866E8__ComputeFootprintDescriptor` with its own `QueryPos866E8` world-position argument. Loops `self->arr[7]` via `slot118`, bounds-testing `arg1->unk0`/`arg1->unk8` against each candidate's `r->unk18`/`r->unk20` within a fixed `0xA000` tolerance (a spatial hit test), with a decreasing `threshold` and `self->unk68->unk4` gating an early-exit shortcut on ties. "Find...ForPosition" names the mechanic (locate the element whose bounds contain/are nearest a world position); the tie-break rule beyond "closer element wins" is not established. |
