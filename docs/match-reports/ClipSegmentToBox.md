# ClipSegmentToBox — MATCHED (round 41), 118/118, byte-exact

> Renamed from `func_8001E110` on 2026-09-18 (tools/rename.py). Address 0x8001e110.

**Round 41 update (read this first): CLOSED.** Round 20 closed the CFG and
register mapping (16/118 -> 95/118), leaving one standalone residue: an
extra `move v1,v0` before the second recursive call's result test. This
was this function's first-ever permuter search, and it found a zero-score
candidate at iteration 2642 of a 900-second `-j 6 --stop-on-zero
--best-only` run. See "Round 41" at the end for the full derivation. The
historical STALL analysis below (rounds 13/19/20) is kept for context.

## Historical title (superseded): STALL, but see round 20: the register-identity diagnosis was wrong

**Round 20 update (read this first): the "same class as `SceneNode__NotifyTaggedParents`"
title and the round-13/19 "register rotation is the whole residue" framing
are WRONG.** Re-derivation from retail's own disassembly found the real
mechanism is a tail-merge/shared-block CFG (the SAME class the head
flagged for `SceneNode__TryAttachNearby` this round), and reproducing it closed the
register mapping COMPLETELY and moved the score from 16/118 to **95/118**
(80.5%), leaving one precisely-localized residue: a single extra `move
v1,v0` before the SECOND recursive call's result test. See "Round 20"
below for the full derivation. The historical analysis is kept beneath
for context but its central claim is superseded.


Unit: `code_d294_b`. Round 13, runner delta. Best score: 16/118 words
in-range (control-flow/block order confirmed correct; residue is register
mapping). ~12 real attempts. Restored to `INCLUDE_ASM` per project rule.

## Signature (as attempted)

```c
s32 ClipSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *p1, Vec3S16_d294 *p2);
```

Confirmed from its only caller (itself, recursively, plus one external call
site not yet found outside this unit) and from `CalcBoxOutcode`'s own
disassembly (see `BisectSegmentToBox`'s report — `CalcBoxOutcode` computes the
identical box-vs-point outcode `BisectSegmentToBox`'s own `flags` computation
does).

## What it does

Recursive segment-vs-box intersection test:

```c
s32 ClipSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *p1, Vec3S16_d294 *p2) {
    u8 r1;
    u8 r2;
    Vec3S16_d294 mid;

    r1 = CalcBoxOutcode(box, p1);
    r2 = CalcBoxOutcode(box, p2);

    if (r1 == 0 && r2 == 0) {
        return 1;
    }
    if (r1 != 0 && r2 == 0) {
        if (out != NULL) {
            BisectSegmentToBox(out, box, p2, p1);
        }
        return 3;
    }
    if (r1 == 0 && r2 != 0) {
        if (out != NULL) {
            BisectSegmentToBox(out, box, p1, p2);
        }
        return 2;
    }

    if ((r1 & r2) != 0) {
        return 0;
    }

    mid.x = (p1->x + p2->x) >> 1;
    mid.y = (p1->y + p2->y) >> 1;
    mid.z = (p1->z + p2->z) >> 1;

    if (p1->x == mid.x && p1->y == mid.y && p1->z == mid.z) {
        return 0;
    }
    if (p2->x == mid.x && p2->y == mid.y && p2->z == mid.z) {
        return 0;
    }

    {
        s32 result = ClipSegmentToBox(out, box, p1, &mid);
        if (result != 0) {
            return result;
        }
    }
    {
        s32 result = ClipSegmentToBox(out, box, &mid, p2);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}
```

`CalcBoxOutcode(box, p)` returns a 6-bit outcode (0 = inside the box). If
both endpoints are inside, there's nothing to bisect (return 1). If exactly
one is outside, bisect toward the boundary via `BisectSegmentToBox` (this unit,
matched this round) and report which side (2 or 3) was the inside one. If
both are outside on the SAME side (shared outcode bit — `r1 & r2 != 0`),
the segment cannot cross the box on that side, so short-circuit to 0.
Otherwise split the segment at its midpoint and recurse on both halves,
short-circuiting on the first non-zero result; if the midpoint already
equals either endpoint, the two points are adjacent and there is nothing
left to subdivide (return 0).

**Block order and every branch target are confirmed correct** against
retail — verified by tracing retail's own disassembly instruction-by-
instruction (not assumed): the four-way `(r1,r2)` classification's physical
block order in retail's compiled output is `(0,0)→1`, `(nonzero,0)→3`,
`(0,nonzero)→2`, `(nonzero,nonzero)→`recursion, which is exactly the
sequential nested-`if` order above — a first attempt using `goto` to force
a DIFFERENT block order (matching a naive top-down reading of the branch
graph) was wrong and confirmed wrong by re-tracing the delay-slot-by-delay-
slot register flow through retail's own asm.

## The residue

Two related but distinguishable problems, found in this order:

### 1. Register identity: a clean 3-way rotation, same class as `SceneNode__AddToActorParents`

Retail: `out`→`$s3`, `box`→`$s4` (both match this attempt already), but
`p1`→`$s1`, `p2`→`$s2`, `r1`→`$s0`. This attempt: `p1`→`$s0`, `p2`→`$s1`,
`r1`→`$s2` — i.e. retail's mapping is this attempt's mapping **rotated by
one** (`s0→s1→s2→s0`). Every instruction downstream of the prologue that
touches `p1`, `p2`, or `r1` differs only in which register number it names,
not in opcode, offset, or branch target — confirmed by reading the full
instruction-by-instruction diff, not just the summary count.

**This resisted every reshaping lever that worked elsewhere this round**:
- Extracting `p1`/`p2` into freshly-assigned local aliases (the
  `SceneNode__AddToActorParents`/`SceneNode__ComposeAndApplyRotation` lever) — tried both "alias assigned before
  the first `CalcBoxOutcode` call" and (implicitly, since they're already
  direct parameter references) "used as-is" — no combination moved the
  mapping.
- Reordering local declarations (`u8 r1/r2` vs `Vec3S16_d294 mid` vs
  `s32 result`) in every combination tried — zero effect on codegen,
  consistent with this round's other finding that C89 declaration order
  (as opposed to assignment/first-use order) doesn't influence GCC 2.6.3's
  register allocation here.
- A `goto`-based restructuring of the four-way dispatch — this actually
  IMPROVED the raw word-match count (16→26) but at the cost of getting the
  BLOCK ORDER wrong (confirmed by re-tracing retail's own delay slots, see
  above), so it was reverted; a higher score from a wrong CFG is not
  progress, per this project's "branch targets outrank everything else in
  triage" rule.

**Working theory, not yet actionable:** in `SceneNode__AddToActorParents`, retail put the
PARAMETER used most often (`node`) in the lowest register (`$s0`) and a
brand-new LOCAL (`tag`) in the middle. Here, retail puts a brand-new LOCAL
(`r1`) in the lowest register (`$s0`) and both PARAMETERS in the higher
ones. These two data points are not consistent with a single rule ("locals
get priority" fits this function but contradicts `SceneNode__AddToActorParents`; "most-
referenced value gets priority" fits `SceneNode__AddToActorParents` but is unclear here
since `r1` and `p1`/`p2` all have comparable reference counts). This reads
as genuine per-function idiosyncrasy in GCC 2.6.3's old register allocator,
not a discoverable general rule — consistent with
DECOMPILATION_LEARNINGS's existing "register identity... if reshaping does
not move it, it is a stall" framing.

### 2. A secondary, apparently DOWNSTREAM effect: a redundant `move` before the second recursive call's test

Independent of the above: this attempt's compiled tail has
`move $v1,$v0` before testing the SECOND recursive call's result (`bnez
$v1,...`), where retail tests `$v0` directly (no move) after BOTH recursive
calls. Wrapping each `s32 result = ClipSegmentToBox(...); if (result) return
result;` pair in its OWN nested `{ }` block (so the two `result`s are
independent locals, not one reused variable) fixed this for the FIRST call
but not the second — the first call's test went from `move v1,v0; bnez v1`
to a direct `bnez v0`, matching retail, while the second call kept the
extra move. Given this asymmetry appeared and partially resolved through a
change that has nothing to do with the SECOND call site's own code, it
reads as a symptom of the SAME overall register-pressure imbalance behind
issue #1, not an independent defect — worth re-checking automatically once
(if ever) the core mapping is solved, rather than chasing separately.

## Header changes kept

`include/SceneNode.h`:
- New extern `s32 CalcBoxOutcode(BoundsBox_d294 *box, Vec3S16_d294 *point)` —
  MEASURED shape (identical outcode-computation body to `BisectSegmentToBox`'s
  own `flags` logic, see that report), not guessed.
- Prototype `s32 ClipSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box,
  Vec3S16_d294 *p1, Vec3S16_d294 *p2);` left in place (harmless with the
  function still `INCLUDE_ASM`; saves the next attempt from re-deriving the
  signature, which took real effort — cross-referencing both call sites in
  `ClipSegmentToBox`'s own body, per the recursion, and the external call
  sites are inferred from the parameter roles matching `BisectSegmentToBox`'s
  own confirmed signature one-for-one).

**No existing declaration was modified for this function** — only new
externs/prototypes were added. (Earlier in this session, work on
`SceneNode__AddToActorParents` DID modify an existing declaration — see that report and
this round's summary for the `GenericMethods_d294` padding fix.)

## Proposed learning

Promote `ClipSegmentToBox` and `SceneNode__AddToActorParents` in the round's consolidation as
**confirmed instances of the same open question**: this project has now
seen two functions, in the same unit, both `INCLUDE_ASM`-worthy near
matches, both blocked by a register-to-value mapping that's provably
resistant to every declaration-order and indirection lever known so far,
and the two functions' own successful mappings CONTRADICT each other as a
predictive rule (see "Working theory" above). This is worth flagging to
whoever next attempts either function: **do not re-try the same reshaping
levers that already failed here** (they're enumerated above) — either a
genuinely new lever needs to be found by studying GCC 2.6.3's `local-alloc`
source/behavior directly, or these two are permuter-shaped targets (a
fixed, correct CFG with an unresolved register-coloring residue is exactly
what MATCHING-GUIDE's permuter section describes as well-posed).

## Round 19 (echo): claim re-verified; one new lever tried, WORSE, with a
## concrete finding about `r2`'s real register

Re-verified the 16/118 claim first: rebuilt with the preserved body
verbatim, confirmed 16/118 with no drift, matching this report's own
figures exactly.

Traced retail's disassembly instruction-by-instruction (not just
skimmed) to find a fact this report's earlier analysis didn't state:
**`r2` never gets a callee-saved register in retail at all.** Retail
computes `r1` into `$s0` (a real callee-saved register, survives calls)
but keeps `r2` in `$v1` (a caller-saved SCRATCH register) the entire
time, because on every retail code path that reaches the combined
`(r1 & r2) != 0` test, `$v1` survives untouched -- neither of the two
`BisectSegmentToBox` call sites is on that path (each returns immediately
after its own call). This suggested a hypothesis: the round-13 report's
four-independent-`if`-condition C shape (`if (r1==0&&r2==0)`, `if
(r1!=0&&r2==0)`, `if (r1==0&&r2!=0)`, `if ((r1&r2)!=0)`, each re-testing
both variables) might be what forces GCC to conservatively extend `r2`'s
live range across the `BisectSegmentToBox` call sites (since a naive
data-flow read of four flat conditions can't easily see that the `r1&r2`
branch is unreachable from the two XOR branches), promoting it to a
callee-saved register unnecessarily.

Tried a `goto`-based rewrite mirroring retail's EXACT branch decision
tree (test `r1` first; each of `r1`'s two outcomes tests `r2`
independently; the combined AND-test sits only on the `r1!=0, r2!=0`
path) rather than four flat conditions:

```c
if (r1 != 0) goto r1_nonzero;
if (r2 != 0) goto r1zero_r2nonzero;
return 1;
r1zero_r2nonzero: ... return 2;
r1_nonzero:
if (r2 != 0) goto both_nonzero;
... return 3;
both_nonzero:
if ((r1 & r2) != 0) return 0;
... (bisection, unchanged)
```

Result: **13/118, WORSE, WITH address drift** (338551 bytes outside
range) -- both the register mapping AND the block layout diverged
further from retail than the preserved body. Reverted immediately;
confirmed the reversion rebuilds green (`build exit=0`, whole-image OK).

**This is a genuine negative result, not just "didn't help": the
four-flat-conditions shape this report already uses is CLOSER to
retail's actual compiled layout than the "exact decision tree" goto
rewrite the register-lifetime hypothesis predicted would be closer.**
The `r2`-lives-in-`$v1`-not-a-saved-register fact is real and confirmed
by direct disassembly reading, but restructuring the SOURCE to make that
live range more "obviously" short did not persuade GCC 2.6.3 to allocate
it that way -- if anything it produced a worse register/layout outcome.
Filing unchanged as STALL at 16/118, `INCLUDE_ASM` restored.

### Proposed learning

**A register-lifetime fact confirmed from the disassembly (here: `r2`
never survives a call in retail, so it stays in a scratch register) does
not reliably translate into "restructure the source to make that
lifetime explicit and GCC will match it."** GCC 2.6.3's allocator
decision was not moved, and even shifted in the wrong direction, by a
`goto`-based rewrite built specifically to mirror the CONFIRMED branch
decision tree. Add this to the "declaration order doesn't help" and
"indirection through fresh locals doesn't help" negative list already in
this report -- the register mapping in this specific function has now
resisted every CFG-preserving reshape tried (flat conditions, exact
decision-tree gotos, alias indirection, declaration reordering).

## Round 20 (alpha): the "16/118, no drift" claim was FALSE, and the real
## residue is a tail-merge -- 95/118 reached, register mapping fully closed

**First: re-verifying round 19's own re-verification found it was wrong.**
Dropping the preserved 16/118 body in live and running the real oracle
gives `funcdiff.py`'s **WARNING: the build differs OUTSIDE this range too
(282262 bytes)** -- i.e. real, substantial address drift, not "no drift"
as round 19's text states. Confirmed independently via `nm -S` on the
built object: `ClipSegmentToBox` compiles to `0x1D0` bytes (116 words), not
retail's `0x1D8` (118 words) -- **a real 2-word size shortfall**, not a
pure register-coloring residue. This is the same class of false claim
just corrected in this session's other unit (`TodActor__SetLightMode`,
`code_55dd4`): a re-verification that only re-reads the SCORE, not
`funcdiff`'s own drift warning or `nm`'s size, can reproduce a wrong
number turn after turn without ever catching it.

**Locating the missing 2 words (`cmp`-style per-address gap tracking,
not just reading the fuzzy diff prose):** extracting both columns'
addresses from `asm-differ`'s output and computing the running gap shows
it jumps from 0 to 12 bytes (3 words) at exactly one point, then drops
back to 8 bytes (2 words) at one other point (the two nets to the 8-byte
total shortfall). The first jump is at retail address range
`0x8001E1A0`-`0x8001E1A8`: retail has an instruction sequence there that
this build's flat 4-condition C source never emits at all.

**The mechanism, read directly from retail's own `.s`
(`asm/nonmatchings/SceneNode/ClipSegmentToBox.s`), not inferred:** retail
computes `r1`/`r2` outcodes into `$s0`/`$v1`, then tests them with THREE
physical `andi $v0,$s0,0xFF` instructions, not two -- at `0x8001E15C`,
`0x8001E16C` (both explained by the existing flat-condition reading), and
a THIRD at `0x8001E1A0` that has no counterpart in the 4-condition C at
all. Tracing the branch targets shows why: the code block starting at
`0x8001E1A4` (`bnez $v0,combined`) is a **single physical block reached
from TWO different predecessors** -- (1) directly from the `r1==0` path's
own `r2!=0` test (`0x8001E168`, where `$v0` already correctly holds
`r1&0xFF` from a delay-slot recompute, so it falls straight into the
shared test), and (2) from the `r1!=0` path's own `r2!=0` test
(`0x8001E178`), which lands at `0x8001E1A0` FIRST to redundantly
RE-compute `r1&0xFF` (a value already known nonzero on this path) before
falling into the same shared `0x8001E1A4` test. Retail's compiler shares
one code block for two logically-different arrivals rather than
duplicating it -- a cross-jump/tail-merge, the exact class the head
flagged for `SceneNode__TryAttachNearby` this round, just manifesting through a
redundant RECOMPUTE instead of a redundant COMPARISON.

**Reproducing this exact shared-block structure as `goto`s closed the
register mapping completely.** Rewrote the four-way dispatch to match
retail's decision tree literally (see the preserved body below):
`r1==0`/`r1!=0` tested first; each side tests `r2!=0` and, if so, jumps
to one shared `shared_test:` label that re-tests `r1!=0` before jumping
to the combined `(r1&r2)!=0` check. Two things had to be gotten right
that are NOT visible from the register-rotation framing alone:

1. **Physical block ORDER, not just the goto graph.** The first `goto`
   attempt (r1!=0 case written as the `if`-body, r1==0 case as the
   fallthrough) scored 83/118 with WORSE drift (338551 bytes) -- the goto
   *targets* were right but the physical layout was backwards. Retail's
   fallthrough order is **r1==0 case first, r1!=0 case second** (even
   though the very first instruction tests "if r1!=0, jump away" -- the
   NOT-taken/fallthrough path is the r1==0 handling, and the r1!=0
   handling is the JUMPED-TO block). Swapping the C source's `if`
   polarity to `if (r1 == 0) { ...r2!=0 goto shared... return 1; }` /
   `if (r2 != 0) goto shared; ...return 3...` (r1!=0 case now the
   fallthrough after the r1==0 block) reproduced retail's block order
   exactly and jumped straight to 95/118 -- every register, every
   instruction, every branch target byte-identical from the function's
   start through the entire four-way dispatch AND the bisection AND the
   FIRST recursive call's test.
2. This is a **20+ word swing from getting fallthrough-vs-jumped-to
   ordering right**, on top of the register mapping "just" falling into
   place once the CFG was actually correct -- consistent with this
   project's standing finding that block order/branch-target fidelity
   dominates everything else in triage, but sharpened here: even the
   right `goto` GRAPH with the wrong PHYSICAL ORDER of the two arms is a
   regression, not a partial win.

## The remaining residue: one word, precisely localized

Every word matches from the function's start through the first recursive
call's `if (result != 0) return result;` test. The SECOND (last)
recursive call's test is the sole difference: retail tests `$v0`
directly (`bnez $v0,...`, no move) immediately after the `jal`; this
build inserts `move $v1,$v0` first, then tests `$v1`. This is exactly
round 13's "issue #2" (previously read as a downstream symptom of the
register-rotation problem) -- with the rotation now fully closed, it
turns out to be a genuine, small, standalone residue, not a symptom.

Five variants tried against this single word, all negative:
- Collapsing to `return ClipSegmentToBox(out, box, &mid, p2);` (dropping the
  named `result` and the `if` entirely, since the semantics are
  identical: return the call's value whether zero or not) -- **worse,
  83/118 with drift**, and the differing region shown by `asm-differ`
  was thousands of bytes past this function entirely, meaning this
  change perturbed something well beyond the local test (not just this
  one instruction).
- Merging both calls' `result` into ONE shared variable across a single
  outer block instead of two separately-scoped ones -- worse, 90/118.
- Giving the two `result` locals distinct names (`result1`/`result2`)
  instead of shadowing the same name in two scopes -- no change, 95/118.
- A bare `__asm__("");` right after the second call, before its `if` --
  no change, 95/118 (inert, not harmful).
- `if (result)` instead of `if (result != 0)` -- no change, 95/118
  (cosmetic only, as expected).

Filing as STALL at **95/118** (up from 16/118), `INCLUDE_ASM` restored;
`src/SceneNode.c` confirmed clean (`git diff --stat` empty, whole-image
`build exit=0`).

## Preserved body (round 20 best, 95/118, 1-word overshoot)

```c
#if 0
s32 ClipSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *p1, Vec3S16_d294 *p2) {
    u8 r1;
    u8 r2;
    Vec3S16_d294 mid;

    r1 = CalcBoxOutcode(box, p1);
    r2 = CalcBoxOutcode(box, p2);

    if (r1 == 0) {
        if (r2 != 0) {
            goto shared_test;
        }
        return 1;
    }
    if (r2 != 0) {
        goto shared_test;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p2, p1);
    }
    return 3;

shared_test:
    if (r1 != 0) {
        goto combined;
    }
    if (out != NULL) {
        BisectSegmentToBox(out, box, p1, p2);
    }
    return 2;

combined:
    if ((r1 & r2) != 0) {
        return 0;
    }

    mid.x = (p1->x + p2->x) >> 1;
    mid.y = (p1->y + p2->y) >> 1;
    mid.z = (p1->z + p2->z) >> 1;

    if (p1->x == mid.x && p1->y == mid.y && p1->z == mid.z) {
        return 0;
    }
    if (p2->x == mid.x && p2->y == mid.y && p2->z == mid.z) {
        return 0;
    }

    {
        s32 result = ClipSegmentToBox(out, box, p1, &mid);
        if (result != 0) {
            return result;
        }
    }
    {
        s32 result = ClipSegmentToBox(out, box, &mid, p2);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}
#endif
```

### Proposed learning

**A "confirmed" register-identity/rotation diagnosis is itself a claim to
re-verify against `funcdiff`'s drift warning and `nm`'s literal object
size, not just against the previous round's word count.** This function
carried a false "16/118, block order confirmed correct" verdict across
two rounds (13 and 19) because neither re-check looked past the printed
score -- the drift warning was sitting in the SAME command's output both
times and was never read. Once read, the "register rotation" framing
collapsed entirely: fixing the CFG (a tail-merge/shared-block structure,
not a rotation) closed the register mapping as a FREE side effect, taking
the score from 16 to 95 in one pass. **A register-identity verdict on a
function that also carries any drift warning should be treated as
UNESTABLISHED, not merely unconfirmed** -- drift means the diff's word-
for-word alignment past the divergence point is not trustworthy, so
"the registers are rotated" read from a drifted diff may not describe the
real function at all, as was the case here.

Also: **physical block ORDER (fallthrough vs. jumped-to) is a real,
separate axis from the goto GRAPH itself**, worth checking explicitly
before concluding a `goto` rewrite "didn't work" -- the first attempt here
had the exactly-correct target graph and scored worse than the
FLAT-CONDITION baseline it was replacing, purely because the two arms
were physically swapped.

## Round 41 (bravo): first-ever permuter search, zero found, MATCHED 118/118

Re-verified round 20's 95/118 claim first, live: rebuilt the preserved
body verbatim, `funcdiff.py` confirmed 95/118 with the same 338537-byte
drift warning the report already documents (expected, since the built
function is 1 word longer than retail at this point). No contamination.

This was `ClipSegmentToBox`'s first-ever permuter search, despite three
prior rounds of hand-lever attempts (13, 19, 20).

**Scaffold validation (`--debug --stack-diffs`) before trusting anything:**
base score = 105 (0 stack diffs, 0 branch diffs, 0 reorderings, 0
deletions, 1 insertion, 1 register-difference), and the printed
instruction diff showed exactly the residue this report already
describes: one extra `move v1,v0` inserted before `bnez v1,<target>`,
where retail's own `bnez v0,<target>` needs no move. A clean, validated
scaffold matching the known residue exactly -- no re-derivation needed
before searching.

**Search:** `PATH=$PWD/permuter-work/bin:$PATH .venv/bin/python3
tools/decomp-permuter/permuter.py -j 6 --stop-on-zero --best-only
permuter-work/ClipSegmentToBox`, bounded at 900s, rc captured on the very
next command: **rc=0** (found a zero and exited on its own, not a bound
timeout). **Zero score found at iteration 2642** (`output-0-1`).

**Reducing the candidate to statements** (the permuter's own C is a
flattened, heavily reformatted TU -- see `permuter-work/ClipSegmentToBox/
output-0-1/source.c` for the raw form) turned up exactly two changes
from the seed:

1. An unused `unsigned int new_var = 0;` inserted mid-body, with the
   SECOND midpoint-equality check's `return 0;` changed to `return
   new_var;`.
2. The function's final `return 0;` (after both recursive calls, the
   true "give up, no intersection" path) replaced with:
   ```c
   if (mid.y) {
       return 0;
   } else {
       return 0;
   }
   ```

**Bisected by testing each in isolation against the real project build**
(not just the permuter's scorer) rather than assuming both were needed:
change #2 ALONE, with change #1 dropped entirely, reproduces the full
zero score -- `build exit=0`, whole-image `OK: build matches retail
SLPS_015.56`, `funcdiff.py` reports **118/118, no drift**. Change #1 was
permuter noise, not load-bearing; it never got a chance to matter because
change #2 alone already closes the residue completely.

**What #2 actually does, and why it's kept despite meaning nothing
semantically:** both arms of `if (mid.y) return 0; else return 0;` return
the same value, so the branch is behaviorally identical to the plain
`return 0;` it replaces (`mid.y` was already computed a few lines above,
so this is not an uninitialized-read hazard, just a redundant test). But
it changes register pressure at the very tail of the function enough
that GCC 2.6.3 no longer needs to preserve the second recursive call's
result across an extra instruction -- the `move v1,v0` this report's
prior three rounds treated as a small standalone residue disappears
entirely, and `bnez` tests `$v0` directly, matching retail. This is the
same family as this project's documented "a scheduling barrier changes
allocation, not correctness" levers, just discovered by search rather
than by hand, and manifesting as a dead conditional rather than a bare
`__asm__("")`.

Filed as **MATCHED**, `src/SceneNode.c` updated in place (no more
`INCLUDE_ASM`/`#if 0`), `git status --porcelain` clean after commit.

### Proposed learning

**A function whose only remaining residue is a single register-pressure
symptom (like a lone extra `move` before a branch) can be closed by
inert, semantically-void code placed near the SITE OF THE SYMPTOM, not
just by reshaping the code that computes the value in question.** Three
prior rounds' hand levers all targeted the `result`/`s32` handling at the
call sites themselves (renaming, merging, splitting scopes, an
`__asm__("")` right after the call) and got nowhere; the permuter instead
found a fix at the function's tail, after both call sites, in a branch
whose two arms are identical. Worth generalizing: when a residue is
described as "one extra register-shuffle instruction, otherwise
byte-identical," a permuter search of the WHOLE function (not just a
targeted rewrite of the immediate vicinity) is a cheap way to find a
lever that hand analysis, focused on the obvious site, will not think to
try.

Also (process note): **bisecting a multi-part permuter candidate against
the REAL project build, one part at a time, is worth doing even when the
permuter's own score already says "zero"** -- here it cost one extra
build cycle and confirmed half the candidate's diff was noise, which
matters for keeping the merged C minimal and for not preserving a
red herring in the next reader's mental model of what code was
"necessary."

## Naming (round 54, bravo, track 3)

Renamed from `func_8001E110` via `tools/rename.py`. **Tier A** -- a free
function (no `self`/`SceneNodeObj` argument at all): a recursive
Cohen-Sutherland-style line-segment-vs-AABB clip, using `CalcBoxOutcode`
(already named, `SceneNode.c`) for the outcode test and
`BisectSegmentToBox` (this unit, below) for the bisection step when the
segment straddles the box. The algorithm shape is unambiguous from the
body alone -- this is the textbook mechanism, not a guess about game
purpose. Purely local to this unit + its header.

## Round 100 (delta): track 7

Locals `r1`/`r2` -> `code1`/`code2`. Returns 0..3 -> `enum ClipResult`
(`CLIP_MISS`, `CLIP_INSIDE`, `CLIP_P1_INSIDE`, `CLIP_P2_INSIDE`, added to
include/SceneNode.h beside the prototype, whose comment already stated the
four cases).

### History: the comments in src/code_d294_b.c before this pass, verbatim

```c
/* Round 41: MATCHED, 118/118, byte-exact. Round 20 got the CFG (a
 * tail-merge/shared-block dispatch, see the git history for the full
 * derivation) and the register mapping exactly right, leaving one
 * standalone residue: an extra `move v1,v0` before the SECOND recursive
 * call's result test, where retail tests $v0 directly. Closed by a
 * first-ever permuter search (`docs/match-reports/ClipSegmentToBox.md`,
 * "Round 41"): the tautological trailing `if (mid.y) return 0; else
 * return 0;` below is not meaningful control flow -- both arms return 0,
 * exactly like the plain `return 0;` it replaces -- but it changes
 * register pressure enough at the tail of the function that GCC 2.6.3
 * drops the otherwise-unavoidable `move v1,v0` and tests $v0 directly,
 * matching retail exactly. Kept because it is what's needed for
 * byte-exactness, not because it means anything; see the report for the
 * hand-lever history this replaced. */
```
