# GraphRoom__ScoreDayLog -- MATCHED (56/56 words, byte-exact)

> Renamed from `GraphRoomObj__ScoreDayLog` on 2026-09-26 (tools/rename.py). Address 0x800585b4.

> Renamed from `func_800585B4` on 2026-09-24 (tools/rename.py). Address 0x800585b4.

Unit: `src/ObjMStyleActor.c` · Size: 56 words (file 0x48DB4-0x48E94) · Round 41
(2026-09-14), runner echo, worktree `lsddecomp2-wt-echo`.

Inherited from round 24's stall: length **1 word SHORT at 55/56**, 15/56 raw
word-match, first real diff at word 15. Closed via a **first-ever permuter
search**, iterated three times as each search shrank the residue, ending in
a zero-score candidate that was then hand-minimized while re-verifying
byte-exactness after every cut.

## The function

A backwards scan of a 365-entry day ring, run once for each of the four
halfword targets in `gGraphScoreMoods`, recording for each target the most recent
day at which it occurred. Bails returning 0 the first time a target is not
found at all; on success marks the log scored, resets the object's cursor and
returns 1. (Unchanged from round 24's derivation -- see that history for the
day-log struct and the table's provenance, both still live in-source.)

## Root cause, now fully characterised

Round 24 established the residue as: retail forms `&gGraphScoreMoods[i]` **inside
the inner loop**, every iteration, as cc1's indexed-global pseudo-op, with
`$t3 = i * 2` recomputed once per OUTER iteration by an `sll`; the build
instead hoisted the address into an induction variable pointer bumped by 2
per outer iteration -- net **-1 word**, and three reshapes of the array
*access expression* (sized bound, unsized, explicit byte offset) all left
the score at exactly 15/56, an admittedly clean negative on that axis.

**The lever that worked was never the access expression -- it was
defeating cc1's decision to treat the address as invariant at all.** cc1
2.6.3's old loop optimizer will strength-reduce `gGraphScoreMoods[i]` into a
pointer induction variable hoisted clear out of the loop nest once it
recognises the expression as a linear function of an enclosing loop's
basic induction variable, REGARDLESS of how the array indexing is spelled
in C -- which is exactly why the three access-expression reshapes were a
clean negative. What changes the decision is whether the value assigned to
a LOCAL POINTER looks, to cc1, like something that must be recomputed on
every path through the loop body. Three permuter-found refinements,
applied in sequence (each verified against the WHOLE-IMAGE oracle, not
just the per-function score, since every one of these was found via a
scaffold and needed re-validation against `build-and-verify.sh`):

1. **Cache `gGraphScoreMoods` into a local `s16 *p` and re-derive it inside the
   inner loop.** `p = gGraphScoreMoods;` as the first statement of the inner
   loop body (instead of indexing the global directly) took the length
   from 1-word-short to EXACT and the raw score from 15/56 to 45/56. This
   alone stopped cc1 from hoisting the address computation out past the
   OUTER loop -- it still hoisted it to once-per-outer-iteration, but no
   longer folded it into a whole-function-lifetime pointer.
2. **Duplicate that assignment across both arms of the `idx` wraparound
   `if`.** Splitting `p = gGraphScoreMoods;` into
   `if (idx<0) { idx=0x16C; p=gGraphScoreMoods; } else { p=gGraphScoreMoods; }` --
   semantically a no-op, since both arms assign the identical value -- took
   the score to 48/56 and made the entire tail of the function
   (0x48e2c onward) byte-identical to retail. The remaining residue was a
   single 9-word block (the wraparound branch plus both address
   computations) with matched LENGTH but every word inside it still wrong.
3. **Add a second local cache `s16 *days` for `log->days`, chained off the
   first.** The final permuter hit (score 0) additionally cached
   `log->days` into a same-shaped local, written as
   `p = (days = gGraphScoreMoods); days = log->days;` right after the `if/else`.
   That specific SHAPE -- a chained assignment through `days` before
   overwriting it with the other pointer -- is what flips the one
   remaining wrong instruction (`addu v0,a1,v0` vs `addu v0,v0,a1`, the
   `log->days[idx]` base-plus-offset operand order) to match. This is
   round 23's `&arr[i+j]` vs `arr+i+j` operand-order class, and it responded
   to the SAME local-pointer-caching lever as the gGraphScoreMoods residue, not
   to any rewrite of the access expression itself (an explicit
   `*(s16*)((u8*)log+0x18+idx*2)` was tried standalone in round 24 and
   again here; both times it left this instruction unchanged).

**Two of the resulting constructs are semantically inert and load-bearing
anyway, which is unusual enough to be worth stating plainly.** With `p`
unconditionally overwritten immediately after the `if/else`, that whole
branch computes nothing the program observes -- and the same is true of the
inner `days = gGraphScoreMoods` half of the chained assignment, since `days` is
overwritten by `log->days` on the very next line. Both were tried removed,
independently, after the byte-exact candidate was found:

| removed | result |
| --- | --- |
| the `else { p = gGraphScoreMoods; }` branch (reverting to unconditional `p = gGraphScoreMoods;` before the `if`) | regresses to 15/56, 1 word short -- this is the round-24 shape again |
| the chained form, i.e. `p = gGraphScoreMoods; days = log->days;` instead of `p = (days = gGraphScoreMoods); days = log->days;` | regresses to 11/56, length wrong by ~95KB (whole-image drift) |

Both are now commented in-source as **do not simplify without
re-verifying** (`src/ObjMStyleActor.c`, directly above the function). This
is not a case of dead code surviving by accident; it is dead code that
cc1's scheduler treats differently depending on its presence, which is
exactly the kind of compiler-idiosyncrasy the permuter is for.

**What DID simplify cleanly, also verified by rebuilding after each cut:**
an intermediate `s32 dayCount = log->unk_0x8; idx = dayCount - 1;` from the
zero-score candidate reduced losslessly back to `idx = log->unk_0x8 - 1;`,
and a `(long)` cast on that same expression and a `dummy = (found = 0);`
double-assignment (both permuter noise from randomizing existing
statements) dropped with no effect. An empty `if ((!log) && (!log)) {}`
guard the permuter inserted before the `goto fail;` also dropped cleanly.
None of these four affected codegen; they are permuter artifacts of
mutating already-irrelevant statement shapes, not part of the real fix.

## Final matched body (56/56, byte-exact)

Live in `src/ObjMStyleActor.c`, immediately following the `gGraphScoreMoods`
declaration:

```c
extern s16 gGraphScoreMoods[4];

/* Round 41 (2026-09-14): matched from a permuter-found lead. `p` and `days`
 * are LOCAL pointer caches of gGraphScoreMoods and log->days respectively -- not
 * because retail's semantics need them (both globals are re-derivable
 * without a temporary), but because caching them THIS WAY is what makes
 * cc1 2.6.3 stop strength-reducing gGraphScoreMoods[i] into a pointer induction
 * variable hoisted across the outer loop (see the match report for the
 * full derivation). The `else { p = gGraphScoreMoods; }` branch below and the
 * `p = (days = gGraphScoreMoods);` chained assignment are BOTH semantically
 * inert -- p is unconditionally overwritten with the same value either
 * way -- but removing either one measurably regresses the codegen (round
 * 41 confirmed both empirically, byte-exact with them, off by dozens of
 * words without). Do not "simplify" this without re-running
 * ./build-and-verify.sh. */
s32 GraphRoom__ScoreDayLog(D_80087AACObj *self, D_80087AACUnkA4Result *log)
{
    u32 i;
    s16 *days;
    s32 j;
    s16 *p;
    s32 idx;
    s32 found;
    s32 limit;

    if (log->scored != 0) {
        goto fail;
    }

    if (log->unk_0x4 != 0) {
        limit = 100;
    } else {
        limit = log->unk_0x8;
        if (limit > 100) {
            limit = 100;
        }
    }

    for (i = 0; i < 4; i++) {
        found = 0;
        idx = log->unk_0x8 - 1;
        for (j = 0; j < limit; j++) {
            if (idx < 0) {
                idx = 0x16C;
            } else {
                p = gGraphScoreMoods;
            }
            p = (days = gGraphScoreMoods);
            days = log->days;
            if (p[i] == days[idx]) {
                self->unk_0x240[i] = j;
                found++;
            }
            idx--;
        }
        if (found == 0) {
            goto fail;
        }
    }

    log->scored = 1;
    self->unk_0x23C = 0;
    return 1;

fail:
    return 0;
}
```

`./build-and-verify.sh` output: `build exit=0`, `OK: build matches retail
SLPS_015.56` -- the whole-image SHA1, not just the per-function score.

## Process notes

- The permuter was run THREE times in sequence against progressively
  better hand-improved seeds (base scores 920 -> 625 -> 495 -> 10), each
  time translating the best non-zero candidate into idiomatic project C,
  rebuilding, re-measuring, and re-seeding the next round from the
  improved source -- rather than running one long search against the
  original stalled body. The zero was found on the fourth scaffold
  (score-10 base), 33772 iterations in, under `-j 4` with two other
  runners active on the same machine.
- Every intermediate candidate was validated against the WHOLE-IMAGE
  oracle (`./build-and-verify.sh`), not just `funcdiff.py`'s per-function
  window, because several of the intermediate lengths were still wrong and
  a per-function score under a length mismatch is explicitly untrustworthy
  per CLAUDE.md's "four ways a score lies".
- No register was pinned with `asm("$N")` or an operand constraint at any
  point; every lever here is ordinary C (local variable introduction,
  statement duplication, assignment chaining) that changes what cc1's
  1990s-vintage loop optimizer perceives as invariant.

### Proposed learning

**When a stalled function's residue is "cc1 strength-reduces an array
access that retail does not," the lever is not the array-indexing
*expression* -- it is whatever makes the *value assigned to a local
pointer* look non-invariant to cc1's old loop optimizer.** Three
independent reshapes of `gGraphScoreMoods[i]`'s own spelling (sized/unsized
bound, explicit byte offset) were a clean negative in round 24 precisely
because they don't touch what the optimizer keys on. What worked was: (1)
introduce a local pointer cache and re-assign it inside the loop rather
than reading the global directly, (2) duplicate that assignment across
both arms of an unrelated branch even when semantically a no-op, and (3)
chain a second local pointer's assignment through the first. All three
are permuter-discoverable but not obviously hand-discoverable, since (2)
and (3) look like textbook redundant code right up until you rebuild
without them. If the report's title says "cc1 hoists/strength-reduces
this and retail doesn't," reach for local-pointer-caching mutations before
spending more attempts on the access expression's spelling -- and always
re-verify with the whole-image oracle before believing a per-function
score, since two of the four confirmed regressions here only showed up
as `build exit=2` plus a length/drift warning, not as a wrong per-function
byte count.

## Naming (round 75, track 3)

**`GraphRoom__ScoreDayLog`** -- tier B (name carried over verbatim from
its round-41 provenance, `func_800585B4` -> `GraphRoom__ScoreDayLog` via
`tools/rename.py` this round). Scans the day-log's 365-entry ring for four
fixed day-type targets (`gGraphScoreMoods`) and records, per target, the most
recent matching day index into `matchedDayIndices` -- exactly "scoring"
the log against those four targets, feeding `GraphRoom__TickHighlight`.

## Track 4 (2026-09-26, round 87, alpha): renamed `GraphRoomObj__ScoreDayLog` -> `GraphRoom__ScoreDayLog`

The class is unified in `include/GraphRoom.h` (class id 0x2F130, table `gGraphRoomMethods`, parent TaskCore; `tools/classtable.py gGraphRoomMethods --vs gTaskCoreMethods`). The class name drops round 75's `Obj` suffix (FINISHING-PLAN track 4 step 2); every function prefix moved with it. The unit's GraphRoomObj/GraphRoomMethods views are deleted and `self` is `GraphRoom *`; inherited fields and slots carry TaskCore's and IntermediateBase's names. Zero bytes changed (build-and-verify OK, typeviews --warnings 0 new). Prefix only; not a slot (called by PopulateGraphPoints alone). The log is the DreamSys save block: `days` is moodPreviousDays, `scored` (+0x467 of the block, DreamSys +0x5DF, the last byte of DreamSys's unknown_values_0x5d8) is renamed graphScored in the unit's record. The round-24 "not DreamSys" note compared the offsets against DreamSys's start rather than saveMagic (+0x178).

## Track 7 (2026-09-27, round 97, delta)

- **Naming: `D_80087BD4` -> `gGraphScoreMoods`** (tier B: the mechanics are certain, what the game makes of these four moods is not). Four `.data` halfwords `0x01FF, 0x0101, 0x0000, 0xFD00`, i.e. MoodGraphPoint `(dynamic, upper)` = (-1, 1), (1, 1), (0, 0), (0, -3). This function is its only user. Declared `MoodGraphPoint[GRAPH_SCORE_MOOD_COUNT]` and compared by `.value`; `targets`/`days` are `MoodGraphPoint *`. Zero bytes changed.
- `GRAPH_SCORE_MOOD_COUNT` (4, unit-local): the table's length, this loop's bound, matchedDayIndices' allocation and TickHighlight's bound.
- `limit`'s 100s are `ARRAY_COUNT(self->points)`; the wrap index 0x16C is `DAYS_PER_YEAR - 1` (DreamSys.h). Locals: `p` -> `targets`, `idx` -> `day`, `j` -> `dot`, `found` -> `matches`.
- The source comments below were replaced by a function comment and a one-line `MATCHING:` note. Verbatim as they stood (with the step-1/3 renames already applied):

```c
/* Four halfword targets, 0x01FF/0x0101/0x0000/0xFD00 -- exactly the i < 4
 * bound below, which is why the loop count is the table's length and not a
 * coincidence. */
extern MoodGraphPoint gGraphScoreMoods[GRAPH_SCORE_MOOD_COUNT];

/* Round 41 (2026-09-14): matched from a permuter-found lead. `targets` and `days`
 * are LOCAL pointer caches of gGraphScoreMoods and log->moodPreviousDays respectively -- not
 * because retail's semantics need them (both globals are re-derivable
 * without a temporary), but because caching them THIS WAY is what makes
 * cc1 2.6.3 stop strength-reducing gGraphScoreMoods[i] into a pointer induction
 * variable hoisted across the outer loop (see the match report for the
 * full derivation). The `else { targets = gGraphScoreMoods; }` branch below and the
 * `targets = (days = gGraphScoreMoods);` chained assignment are BOTH semantically
 * inert -- targets is unconditionally overwritten with the same value either
 * way -- but removing either one measurably regresses the codegen (round
 * 41 confirmed both empirically, byte-exact with them, off by dozens of
 * words without). Do not "simplify" this without re-running
 * ./build-and-verify.sh. */
```
