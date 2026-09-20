> Renamed from `func_80040024` on 2026-09-20 (tools/rename.py). Address 0x80040024.

# Class6E99C__StartFadeToIndex -- STALL (scheduling residue, 29/35)

## Round 46 (runner delta): drift-checked fresh, no new attempt -- DELIBERATE SKIP

Re-spliced the exact preserved body (unchanged) in isolation and rebuilt:
**29/35 words match (file 0x30824-0x308B0)**, no outside-range drift --
identical to every prior measurement, not stale. Deliberate skip: this
"literal materialised late" residue has already absorbed ~94,000
unguided permuter iterations (round 20) plus four hand attempts spanning
naming the table address, naming the literal, and a scheduling barrier
(the last of which actively regressed). The two statement POSITIONS a
fresh attempt would naturally try (naming `one = 1` before vs. after the
`idx = ...` dispatch) are both already in that attempt list. No new
lever occurred to me. Restored to `INCLUDE_ASM`; full oracle
re-confirmed green.

## Round 21 (runner delta): re-verified fresh, no new attempt

Restored the exact 29/35 body and rebuilt fresh, isolated (with
`Class6E99C__StartFadeDefault` reverted to `INCLUDE_ASM` at the time, so this
function's own window cannot be contaminated by that sibling's own
drift -- see that function's own report for a real instance of this
project's "one inherited body in six carries a false drift-free claim"
warning). Reproduces `29/35 words match (file 0x30824-0x308B0)` exactly,
**no outside-range drift warning** -- this function's own length really
does match retail, unlike its neighbour. Also re-ran with the
coordinator's corrected oracle grep
(`error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]`, round 21
broadcast) instead of the three-pattern one: zero hits, confirming this
was never a masked compile error.

Round 20 already spent ~94,000 unguided permuter iterations on this
exact residue with zero improvement (see that round's own section
below) and this round's assignment explicitly flagged the class as
confirmed-negative; not re-spending a further unguided search on the
identical mechanism without a new lever. Verdict unchanged: STALL,
29/35, restored to `INCLUDE_ASM`.

## Round 20 (runner delta): permuter search, confirmed negative

Drift check: rebuilt the exact 29/35 body above verbatim, reproduced
cleanly. Ran `permuter.py --debug --stack-diffs` first to confirm the
scaffold scores the reported residue exactly: **0 stack/branch
differences, 5 register differences, 60 reorderings(2), base score
120** -- matches "pure instruction-scheduling difference, zero
insertions/deletions" from the original report precisely.

Ran the real search twice (unguided randomization -- no `PERM_VAR`/
`PERM_LINESWAP` macros used, since the report's own "materialize the
literal in a local" and "reorder the two `self->methods` reads" levers
are exactly what unguided mutation explores anyway): first bounded run
`timeout 280`, ~9,900 iterations; second bounded run `timeout 500`,
~84,438 iterations. **Neither ever beat the base score of 120** -- no
candidate came within any measurable distance of zero across ~94,000
combined iterations. Both runs used `--stack-diffs` throughout (per the
project's own tooling-bug broadcast), so a false zero from the missing
flag is ruled out.

**This closes the "not attempted due to time budget" note in the
original report as a genuine negative**, not merely an unexplored lead.
Restored to `INCLUDE_ASM`; full oracle re-confirmed green
(`build exit=0` on this unit's baseline before and after).

### Proposed learning (round 20)

A "retail materializes a literal argument immediately after an unrelated
dispatch, before computing an index expression" residue does not yield
to ~94,000 unguided permuter iterations either, on top of the four
hand-reshaping attempts the original report already spent. Combined with
`Class6E99C__StartFadeDefault`'s identical-shape residue (see that report), this is now
a **confirmed-negative class**, not merely an untried one -- a future
runner should not re-spend permuter budget on this specific
`slotXX(self, 1, tableEntry)`-after-a-fresh-dispatch shape without a new
lever (e.g. a `PERM_LINESWAP`-guided search, which requires bypassing
`setup-permuter.sh`'s own cc1 sanity-check per `func_8003F764.md`'s
round-19 finding -- not attempted this round either, same tooling
boundary).

> **CROSS-REFERENCE CHECKED, STANDS (round 39, head).** `func_8003F764` is
> **`select_max_param`** (`libgs/gs_131.o`), now a linked Sony object, so
> do not read that report for a game-code residue precedent. The thing
> cited here is different in kind and survives the reclassification: it is
> a finding about `setup-permuter.sh`'s own behaviour, which is a property
> of the TOOL and not of whose code it was pointed at.

Unit `code_2cc8c_e`, carved round 14. `Class6E99CMethods::startFadeToIndex` (`+0x0D4`).

**Correction to an earlier version of this report**, which claimed a full
35/35 match under the stale-build window described in `New_Class6E99C.md`
(same cause, cross-referenced there). Once the build was confirmed genuinely
fresh (every symbol's linked address checked against `build/lsdde.map`),
this function turned out to still have a real residue.

## Best body reached (29/35)

```c
#if 0
void Class6E99C__StartFadeToIndex(Class6E99CObj *self) {
    s32 idx;

    if (self->state != 0) {
        return;
    }
    idx = self->methods->configure(self);
    self->methods->slotB8(self, 1, &D_8006EA90[idx * 3]);
    self->state = 1;
    self->step = -self->step;
}
#endif
```

## Residue

Pure instruction-scheduling difference, zero insertions/deletions. Retail
materialises the `slotB8` call's literal `1` argument (`li $a1,0x1`)
IMMEDIATELY after the `configure` dispatch's own delay slot -- before computing
`idx*3` -- and reloads `self->methods` a second time (fresh, not cached)
right before the `slotB8` dispatch itself. This body's compiled form
computes `idx*3` first and defers the `li $a1,0x1` to just before the call.
Same total instruction count, same registers, purely reordered.

## Attempts (4)

1. Base body above: 29/35, the "li a1,1 late" residue.
2. Materialize the table address into a named local (`entry = &D_8006EA90[...]; slotB8(self, 1, entry);`)
   before the call: no change.
3. Materialize the literal into a named local (`one = 1;`) assigned
   immediately after `idx = ...`, before computing the table address: no
   change -- GCC still floats the `li` to just before the call regardless
   of where the C source puts it.
4. Same as (3) plus a bare `__asm__("")` barrier right after the `one = 1;`
   statement, to physically pin its position: made it WORSE (3/35) --
   the barrier perturbed something else's scheduling elsewhere in the
   function, consistent with the project's documented "`__asm__("")` is
   not a local lever -- it perturbs the WHOLE function's register
   allocation" caution.

## What I did NOT try, and why

- **The permuter.** This is a small (35-word), pure-reordering residue with
  zero structural difference -- close to the ideal permuter target. Not run
  due to time budget in this round; worth a `PERM_VAR`-guided search on a
  re-attempt, seeded from the base body above.
- **Reordering the two `self->methods` reads** (the `configure` dispatch's own
  read vs the `slotB8` dispatch's fresh reload) relative to the `idx*3`
  computation, independently of the literal's position. Only the literal's
  position was varied across all 4 attempts; the reload's position was left
  alone since it already matches retail's own placement in every attempt.

## Proposed learning

A trivial constant argument (`1`) to a call reached immediately after a
DIFFERENT dispatch through the same vtable pointer can resist being pinned
to retail's early position by any combination of source reordering, naming
it in a local, or a bare scheduling barrier (which actively regresses it).
`Class6E99C__StartFadeDefault` in this same unit shows the identical pattern on the exact
same call shape (`slotB8(self, 1, tableEntry)`) -- worth treating as one
class rather than two coincidences; see that function's own report.

## Round 59 (runner charlie): NON_MATCHING body promoted

NON_MATCHING body promoted, round 59. The exact preserved body above (29/35
words, length exact, instruction-scheduling residue on the `li $a1,1`
materialization) is now live in `src/code_2cc8c_e.c` under `#ifdef
NON_MATCHING`, with the verified build still taking the `#else INCLUDE_ASM`
branch. `./build-and-verify.sh` and `tools/check-nonmatching.sh` both green.

## Naming (round 61, track 3)

**`Class6E99C__StartFadeToIndex`** -- tier B (STALL, preserved body
unchanged by this rename). `Class6E99CMethods::startFadeToIndex` (`+0x0D4`). Guards
on `state == 0` (idle), looks up an index via `configure`, dispatches the
`slotB8` color-set slot with `&D_8006EA90[idx * 3]` (an INDEXED table
entry), sets `state = 1`, and negates `step`. Named opposite
`Class6E99C__StartFadeDefault` (`startFadeDefault`, `state = 2`, the FIXED
`D_8006EAA8` table) -- the two are a matched pair distinguished by which
color source they select. "Fade" is inferred from `step` accumulating into
color-channel bytes over time in `Class6E99C__Update`; "index" from this
function's own `idx`-based table lookup versus its sibling's fixed one.
Game-level purpose (what is fading, and why) is not established.
