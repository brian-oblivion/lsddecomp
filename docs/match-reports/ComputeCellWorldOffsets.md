# ComputeCellWorldOffsets

> Renamed from `func_8004B44C` on 2026-09-24 (tools/rename.py). Address 0x8004b44c.

**Unit:** class_3bb8c · **Size:** 73 words · **Status:** MATCH (73/73), round 40.

## ROUND 40 (bravo): MATCHED -- first-ever permuter search on this function, zero at iteration 1838

This function was staffed for a permuter-first round specifically because it
had NEVER been searched despite 5 prior rounds (~15 attempts) of manual
reshaping, all of which targeted the `outBuf[0]`/`outBuf[2]` store-vs-load
scheduling residue directly (see the pre-round-40 history below) and all of
which topped out at 58/73.

**Gate 1b first:** rebuilt the exact preserved 58/73 body from a clean
`INCLUDE_ASM` baseline before touching anything -- confirmed 58/73, no
drift, identical residue to what this report already documented. The
recorded figure was honest.

**Scaffold:** `tools/setup-permuter.sh ComputeCellWorldOffsets <seed>` built cleanly;
`--debug --stack-diffs` reported base score 415 (7 register differences, 3
reorderings, 1 insertion, 1 deletion) -- matching the report's own
description of the residue (a deferred store past independent loads), so
the scaffold was trusted.

**Search:** `timeout 900 ... permuter.py -j 4 --stack-diffs --stop-on-zero
--best-only`, run in the background. **Found a ZERO at iteration 1838,
`rc=0`** (the search's own `--stop-on-zero` exit, not the timeout bound).

**The lead, and why it is a genuinely different axis than anything tried
before:** every prior attempt (this report's manual variants) targeted
WHERE the `outBuf[0]`/`outBuf[2]` reload sat relative to the stores. The
permuter instead touched something nobody had: the literal constant
`0x400`, used identically in BOTH `arg0[0]`'s and `arg0[2]`'s tail addend
(`arg4->h4 + 0x400` and `arg4->h8 + 0x400`). Hoisting it into a named local
(`s32 off = 0x400;`), declared between the `outBuf[1]` and `outBuf[2]`
assignment statements (the same position several prior attempts already
used for a DIFFERENT local, `ob0`, without effect), changed which register
the compiler chose for the surrounding computation and closed **all 15
remaining words at once** -- not just the constant's own two uses, the
whole store-scheduling residue this report spent 5 rounds on.

**Translated as found -- the permuter's candidate already was idiomatic C**,
no UB, no duplicate-arm artifact to clean up. Re-verified through the full
oracle: `./build-and-verify.sh` -- `OK: build matches retail SLPS_015.56`;
`funcdiff.py ComputeCellWorldOffsets` -- `73/73 words match`. Byte-exact.

```c
s32 ComputeCellWorldOffsets(s32 *arg0, s32 *outBuf, Unk68Struct *arg2, Unk54Struct *arg3, Descriptor10 *arg4) {
    s32 idx;
    s32 factor;
    s32 sum;
    s32 v1;
    s32 a0v;
    s32 off;

    if (arg2->unk4 == 0) {
        idx = arg4->b1;
        factor = arg2->count;
        sum = arg4->b0 + arg2->divisor * idx;
    } else {
        factor = 1;
        idx = 0;
        sum = 0;
    }
    v1 = (arg3->unk0 - arg2->divisor * 0x5000) + arg4->b0 * 0xA000;
    a0v = arg3->unk8 - factor * 0x5000;
    outBuf[0] = v1;
    if (idx & 1) {
        outBuf[0] = v1 - 0x5000;
    }
    outBuf[1] = arg3->unk4;
    off = 0x400;
    outBuf[2] = a0v + idx * 0xA000;
    arg0[0] = (arg4->b2 << 11) + outBuf[0] + (arg4->h4 + off);
    arg0[1] = arg4->h6 + outBuf[1];
    arg0[2] = (arg4->b3 << 11) + outBuf[2] + (arg4->h8 + off);
    outBuf[0] += 0x5000;
    outBuf[2] = outBuf[2] + 0x5000;
    return sum;
}
```

### Proposed learning

**A residue that survives many manual attempts along one axis (here: store
scheduling / reload placement) can still fall to the permuter finding a
COMPLETELY DIFFERENT axis (here: a shared literal constant used twice,
hoisted into one named local).** Five rounds of this function's own history
kept re-testing variations of "where does the `outBuf[0]`/`outBuf[2]`
reload sit"; nobody tried naming the repeated `0x400` constant, because
nothing about the residue's own description (a load/store scheduling
choice) pointed at a literal appearing twice in unrelated-looking
expressions. This is the strongest argument yet for "run the permuter
before declaring a class exhausted by hand-reasoning alone" -- the
project's own staffing note for this round (function was NEVER searched
despite ~15 manual attempts) is exactly what caught it.

---

## Pre-round-40 history (STALL, 58/73 best, ~15 attempts)

> **ROUND 32 (bravo2): re-verified, two new attempts on the report's own
> suggested next lever, both negative.** Rebuilt the exact preserved body:
> confirmed 58/73, no drift, identical residue. Tried:
> 1. Naming BOTH `outBuf[2]`'s value and the `outBuf[0]` reload as locals,
>    computed before either store (closer to retail's actual load-then-
>    store batching) -- REGRESSED hard: 44/73 with 186603 bytes of
>    address drift, one whole word longer than retail (the extra named
>    local costs a genuine spurious instruction, not just reallocated
>    registers).
> 2. Naming ONLY the `outBuf[0]` reload (`ob0 = outBuf[0];`), placed
>    immediately after the `outBuf[2]` assignment statement (much closer to
>    retail's real reload point than round 20's attempt 5) -- no drift,
>    but IDENTICAL 58/73, same residue. The reload's register choice is
>    insensitive to exactly where in the statement stream its C-level
>    access sits, as long as `outBuf[2]`/`outBuf[1]` remain direct
>    (unlocalized) store statements.
>
> Confirms round 20's diagnosis stands: this is a genuine GCC 2.6.3
> list-scheduler choice (batching independent loads ahead of a store),
> not reachable through source-level reordering or reload-caching
> placement. Not attempted further this round; time went to `StageMap__FindElementForPosition`
> (matched) and a permuter run on `StageMap__ComputeChunkLoadEntry` instead.

> **ROUND 20 (charlie): re-verified, residue mechanism pinned down more
> precisely via direct `.o` disassembly (not just `funcdiff`'s summary).**
> Rebuilt the exact preserved body below from a clean `INCLUDE_ASM`
> baseline: still 58/73, no drift, identical residue. Compiled BOTH
> retail's `.s` and the built `.o` side by side
> (`mipsel-linux-gnu-objdump -d build/src/class_3bb8c.c.o`) for the
> `outBuf[1]`/`outBuf[2]`/`arg0[0]` triad this report already names, to see
> the exact instruction-level shape rather than just the byte-count
> residue:
>
> Retail's real order (confirmed): compute `outBuf[2]`'s value fully ->
> `lw` `arg3->unk4` (independent, for `outBuf[1]`) -> `lw` a RELOAD of
> `outBuf[0]` (for `arg0[0]`'s calc, into `$a0`) -> `sw outBuf[2]` -> `sw
> outBuf[1]` -> (start `arg0[0]`'s own field reads) `lb`/`lh` -> `addu`
> using the ALREADY-RELOADED `$a0`. Mine's order: compute `outBuf[2]`'s
> value, but with the independent `arg3->unk4` load moved EARLIER (before
> the value's own final `addu` completes, not after) -> `sw outBuf[2]` ->
> `sw outBuf[1]` -> `lb`/`lh` for `arg0[0]` -> reload `outBuf[0]` (into
> `$v1`, not `$a0`, and much LATER than retail's position) -> `addu` using
> that late reload.
>
> This confirms the report's own diagnosis exactly (retail defers a STORE
> past independent later LOADS; mine stores immediately), and additionally
> shows this function shares the SAME idiom `StageMap__LoadElementResources` closed this
> round in one of its own three-field groups: **an independent load can
> fill a delay slot a nearby dependent load would otherwise need a `nop`
> for, and WHICH load fills it is a real, GCC 2.6.3 list-scheduling choice
> that isn't obviously steerable by statement order alone** (five
> variations already tried here, per the attempts log below, all converged
> on 58/73 or worse). Not re-attempted further this round beyond this
> confirmation -- the two functions differ in one relevant way
> (`StageMap__LoadElementResources`'s fix worked by giving the LATE-reloaded value its own
> named local positioned to match retail's exact delay-slot filler;
> attempt 5 below already tried the equivalent for THIS function's
> `outBuf[0]` reload and regressed to 44/73, so the lever that worked
> there does not transfer here as-is). Worth one more attempt for whoever
> picks this back up: try caching `outBuf[0]`'s reload only far enough
> ahead to fill retail's specific slot (immediately after the `outBuf[2]`
> value's own last arithmetic op, not before it), rather than before the
> whole `outBuf[2]`/`outBuf[1]` computation as attempt 5 did.

## Best body reached (restored to `INCLUDE_ASM` in `src/`)

```c
#if 0
s32 ComputeCellWorldOffsets(s32 *arg0, s32 *outBuf, Unk68Struct *arg2, Unk54Struct *arg3, Descriptor10 *arg4) {
    s32 idx;
    s32 factor;
    s32 sum;
    s32 v1;
    s32 a0v;

    if (arg2->unk4 == 0) {
        idx = arg4->b1;
        factor = arg2->count;
        sum = arg4->b0 + arg2->divisor * idx;
    } else {
        factor = 1;
        idx = 0;
        sum = 0;
    }
    v1 = (arg3->unk0 - arg2->divisor * 0x5000) + arg4->b0 * 0xA000;
    a0v = arg3->unk8 - factor * 0x5000;
    outBuf[0] = v1;
    if (idx & 1) {
        outBuf[0] = v1 - 0x5000;
    }
    outBuf[1] = arg3->unk4;
    outBuf[2] = a0v + idx * 0xA000;
    arg0[0] = (arg4->b2 << 11) + outBuf[0] + (arg4->h4 + 0x400);
    arg0[1] = arg4->h6 + outBuf[1];
    arg0[2] = (arg4->b3 << 11) + outBuf[2] + (arg4->h8 + 0x400);
    outBuf[0] += 0x5000;
    outBuf[2] = outBuf[2] + 0x5000;
    return sum;
}
#endif
```

This is 58/73, correct SIZE (no "differs outside range" warning past the first
attempt's fix), correct control-flow graph, correct field offsets and arities
throughout (m2c-seeded, and the m2c seed's own control flow and field
offsets required zero correction). The residue is confined to two
STRUCTURALLY IDENTICAL, symmetric blocks (`arg0[0]`'s and `arg0[2]`'s
computation, each of shape `(byteN << 11) + outBufN + (halfM + 0x400)`).

## The residue: store-vs-load SCHEDULING, not values or control flow

In both symmetric blocks, retail computes a value fully, then issues one or
two INDEPENDENT loads (for the *next* statement's operands) BEFORE finally
storing the just-computed value -- i.e. the store is scheduling-deferred past
following loads it has no dependency on. My code's natural compilation stores
each value immediately after computing it, one statement at a time, which is
a shorter/more direct instruction sequence but not retail's.

Concretely, for the `outBuf[1]`/`outBuf[2]`/`arg0[0]` block: retail's order is
compute-outBuf2-value -> load-arg3.unk4 -> reload-outBuf[0] -> store-outBuf2
-> store-outBuf1 -> (`arg0[0]` calc reuses the already-loaded outBuf[0] in
the SAME register it was reloaded into). Mine (matching statement order
`outBuf[1] = ...; outBuf[2] = ...;`) computes and stores each in turn, with
the `outBuf[0]` reload for `arg0[0]` happening in a DIFFERENT register
(`v1` instead of retail's `a0`) at a later point.

## Attempts and what each taught

1. Naive direct transcription from the m2c seed (`outBuf[2]` before
   `outBuf[1]`, values computed inline at point of use): 33/73, with a
   "differs outside range" warning -- the `a0v` (`arg3->unk8 - factor *
   0x5000`) computation was placed too late (after the `idx & 1` branch)
   relative to retail, which computes it EARLY (before that branch, matching
   the `v1` computation's position).
2. Hoisted `a0v`'s computation to alongside `v1`'s (both before the `idx & 1`
   branch): jumped to 44/73, still a small size warning from residual
   ordering deeper in the function.
3. Swapped `outBuf[1]`/`outBuf[2]` statement order (assign `outBuf[1]`
   first): 58/73, no more size warning -- this is the version kept above.
4. Wrapped the `outBuf[2]`/`outBuf[1]`/`arg0[0]` triad in an explicit block
   with named locals for each value/reload, storing all three loads before
   any of the stores (mimicking retail's apparent load-then-store batching):
   IDENTICAL 58/73 output -- GCC's own optimizer collapsed the scaffolding
   back to the same code, confirming the residue is not reachable by
   introducing intermediate locals in this shape.
5. Cached `outBuf[0]`'s reload into a named local BEFORE computing
   `outBuf[2]`/`outBuf[1]` (attempting to force retail's early-reload
   ordering): regressed to 44/73 with a size warning, worse than (3).

Both directions of statement reordering top out at 58/73; neither fully
reproduces retail's precise interleaving of loads before stores.

## Proposed learning

**A residue where retail defers a STORE past one or more INDEPENDENT
subsequent LOADS (while the computed value and control flow are already
correct) resisted every statement-reordering and explicit-local-caching
variant tried.** This reads as a genuine GCC 2.6.3 instruction-scheduling
choice (batching loads ahead of stores when nothing depends on the store
completing first) rather than something the C source directly dictates --
similar in kind to the documented "prologue callee-save store order is not
reachable from C" residue, but for ordinary mid-function stores rather than
the prologue. Two DIFFERENT statement orderings converged on the exact same
58/73 output, which is itself evidence the compiler is choosing this
schedule independent of the C-level order fed to it (at least for the orders
tried). Worth a permuter run if this shape recurs elsewhere; not attempted
this round.

## ROUND 38 (alpha, salvaged by head): preserved body REBUILT and confirmed at 58/73 words

Runner alpha was staffed onto this unit and died to an infrastructure error
(org API 403) with nothing committed. The head restored the worktree and
re-measured this report's preserved body directly, by flipping its
`#if 0` guard to `#if 1` and commenting out the matching `INCLUDE_ASM`,
one body at a time against the whole-image oracle.

**Result: `58/73 words`, compiling and LINKING cleanly (zero hits on the
compile-error grep).** The recorded figure is accurate and the body is real.

This is the Gate 1b "rebuild before trusting" check, and it matters here
because round 37 measured roughly one inherited body in six carrying a false
drift-free claim, plus one body that could never have linked at all (it
called a symbol since renamed, so its figure had measured nothing). **All
four preserved bodies in `class_3bb8c` were rebuilt this round and all four
are honest** — `StageMap__FindElementForPosition` 68/70, `ComputeCellWorldOffsets` 58/73,
`StageMap__BuildRateEntries` 125/140, `StageMap__LoadElementResources` 130/150. No stale figure and no
never-linked body in this unit.

No new lever was tried — alpha died before attempting one. This is a
confirmation, not a negative result, and the function's permuter status is
unchanged.

## ROUND 39 (charlie): re-verified 58/73, two more precise placements tried, both negative

Rebuilt the preserved body per Gate 1b before touching anything: confirmed
58/73, no drift, identical residue at both symmetric blocks
(`arg0[0]`/outBuf[0] at vram `0x8004B4EC`-`0x8004B510`, `arg0[2]`/outBuf[2] at
`0x8004B534`-`0x8004B544`). Disassembled both retail's `.s` and
`build/src/class_3bb8c.c.o` side by side to pin the exact mechanism rather
than trust the byte-count: **retail reuses the register `a0` (freed the
moment `a0v`'s last consumer, the `outBuf[2]` addu, executes) to hold the
`outBuf[0]` reload, and schedules that reload immediately after `arg3->unk4`'s
load and before either store.** The built code reloads into `v1` instead, and
schedules it much later (after both stores and the `arg4->b2` load). Same
shape, same distance, at the symmetric `outBuf[2]`/`arg0[2]` site.

This round targeted the report's own open suggestion ("cache the reload only
far enough ahead to fill retail's specific slot, immediately after the
`outBuf[2]` value's own last arithmetic op") with the ONE placement not yet
tried -- between attempt 5's "before both stores" (regressed) and the
inherited "after both stores" (inert):

1. **Mid-placement**: `ob0 = outBuf[0];` inserted between the `outBuf[1] = ...;`
   statement and the `outBuf[2] = ...;` statement, wrapped in its own block
   scope (mirroring attempt 4's block-scoping, but with only ONE value named,
   not all three) -- **IDENTICAL 58/73**, byte-for-byte the same residue as
   the unhoisted baseline. GCC's scheduler produces the same code regardless
   of where between the two stores the C-level read sits.
2. **Statement-order swap**: compute `outBuf[2]` before `outBuf[1]` in source
   (opposite of attempt 3's fix, closer to retail's own VALUE-computation
   order even though attempt 3 established retail's STORE order is
   `outBuf[2]` then `outBuf[1]` regardless of source order) -- **REGRESSED to
   44/73 with 186545 bytes of drift**, one word longer, reproducing attempt
   5's exact failure signature. Reverted immediately.

**This closes out the "where in the statement stream" axis: every position
from immediately-before-both-stores through immediately-after-both-stores has
now been tried (attempt 5, this round's mid-placement, and the inherited
after-both placement), and the result is bimodal, not a gradient** --
before-both always regresses (extra live range, one word longer), and
anywhere at-or-after-both is byte-identical to never hoisting it at all.
There is no middle ground where the placement alone changes the register
choice. Combined with attempt 4's finding (grouping all three values in a
block, stored together, is ALSO identical to the ungrouped baseline), the
evidence now covers every source-level way to phrase "read this value early"
without changing net register pressure, and none of them move the needle.

**Round 39 did not find a combinatorial lever here analogous to
`StageMap__FindElementForPosition`'s.** That function's fix combined a hoist with an operand-order
flip on a commutative op; this function's residue is a straight
reload-register-and-position choice with no analogous second axis to combine
against (there is no commutative operator here to flip -- `outBuf[0]`/`outBuf[2]`
appear as a single operand each in their respective `addu` chains, not as one
of two swappable ones). Not re-attempted further this round.

### Proposed learning

**"Try the placement between the extremes already tried" is a real, cheap
next step for a scheduling-only residue, but it is not guaranteed to be a
gradient** -- here the space is binary (regress-if-too-early,
inert-otherwise) rather than continuous, so one mid-point probe closes off
the whole axis rather than narrowing it. Worth stating so the next runner
does not re-try five more placements expecting a smooth transition.

## Naming

Round 78 (track 3, naming pass, bravo).

| symbol | name | tier | evidence |
| --- | --- | --- | --- |
| `func_8004B44C` | `ComputeCellWorldOffsets` | B | Free function (no `self` parameter, per the naming convention's `VerbNoun` form for non-methods). Round 40's permuter-found fix hoisted a literal `0x400` used identically in both symmetric output blocks; both blocks compute `(byteN << 11) + outBufN + (halfM + 0x400)`, and `0x800`/`0xA000`/`0x5000` (all powers of the grid's own `0x800` lattice unit and `gDefaultGridSpan`, per `src/class_3ac78.c`'s unit header) recur throughout -- consistent with converting a `Descriptor10` grid-cell descriptor plus a base `Unk54Struct` into world-space offsets. The `sum` return value's own meaning is NOT established (no caller-agreed name for it beyond "also returns a scalar derived from the same divisor arithmetic"), so the name covers only the `arg0[]`/`outBuf[]` side, which is the function's dominant, better-evidenced behaviour. |
