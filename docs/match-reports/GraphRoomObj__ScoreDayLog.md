# GraphRoomObj__ScoreDayLog -- MATCHED (56/56 words, byte-exact)

> Renamed from `func_800585B4` on 2026-09-24 (tools/rename.py). Address 0x800585b4.

Unit: `src/class_3bb8c_t.c` · Size: 56 words (file 0x48DB4-0x48E94) · Round 41
(2026-09-14), runner echo, worktree `lsddecomp2-wt-echo`.

Inherited from round 24's stall: length **1 word SHORT at 55/56**, 15/56 raw
word-match, first real diff at word 15. Closed via a **first-ever permuter
search**, iterated three times as each search shrank the residue, ending in
a zero-score candidate that was then hand-minimized while re-verifying
byte-exactness after every cut.

## The function

A backwards scan of a 365-entry day ring, run once for each of the four
halfword targets in `D_80087BD4`, recording for each target the most recent
day at which it occurred. Bails returning 0 the first time a target is not
found at all; on success marks the log scored, resets the object's cursor and
returns 1. (Unchanged from round 24's derivation -- see that history for the
day-log struct and the table's provenance, both still live in-source.)

## Root cause, now fully characterised

Round 24 established the residue as: retail forms `&D_80087BD4[i]` **inside
the inner loop**, every iteration, as cc1's indexed-global pseudo-op, with
`$t3 = i * 2` recomputed once per OUTER iteration by an `sll`; the build
instead hoisted the address into an induction variable pointer bumped by 2
per outer iteration -- net **-1 word**, and three reshapes of the array
*access expression* (sized bound, unsized, explicit byte offset) all left
the score at exactly 15/56, an admittedly clean negative on that axis.

**The lever that worked was never the access expression -- it was
defeating cc1's decision to treat the address as invariant at all.** cc1
2.6.3's old loop optimizer will strength-reduce `D_80087BD4[i]` into a
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

1. **Cache `D_80087BD4` into a local `s16 *p` and re-derive it inside the
   inner loop.** `p = D_80087BD4;` as the first statement of the inner
   loop body (instead of indexing the global directly) took the length
   from 1-word-short to EXACT and the raw score from 15/56 to 45/56. This
   alone stopped cc1 from hoisting the address computation out past the
   OUTER loop -- it still hoisted it to once-per-outer-iteration, but no
   longer folded it into a whole-function-lifetime pointer.
2. **Duplicate that assignment across both arms of the `idx` wraparound
   `if`.** Splitting `p = D_80087BD4;` into
   `if (idx<0) { idx=0x16C; p=D_80087BD4; } else { p=D_80087BD4; }` --
   semantically a no-op, since both arms assign the identical value -- took
   the score to 48/56 and made the entire tail of the function
   (0x48e2c onward) byte-identical to retail. The remaining residue was a
   single 9-word block (the wraparound branch plus both address
   computations) with matched LENGTH but every word inside it still wrong.
3. **Add a second local cache `s16 *days` for `log->days`, chained off the
   first.** The final permuter hit (score 0) additionally cached
   `log->days` into a same-shaped local, written as
   `p = (days = D_80087BD4); days = log->days;` right after the `if/else`.
   That specific SHAPE -- a chained assignment through `days` before
   overwriting it with the other pointer -- is what flips the one
   remaining wrong instruction (`addu v0,a1,v0` vs `addu v0,v0,a1`, the
   `log->days[idx]` base-plus-offset operand order) to match. This is
   round 23's `&arr[i+j]` vs `arr+i+j` operand-order class, and it responded
   to the SAME local-pointer-caching lever as the D_80087BD4 residue, not
   to any rewrite of the access expression itself (an explicit
   `*(s16*)((u8*)log+0x18+idx*2)` was tried standalone in round 24 and
   again here; both times it left this instruction unchanged).

**Two of the resulting constructs are semantically inert and load-bearing
anyway, which is unusual enough to be worth stating plainly.** With `p`
unconditionally overwritten immediately after the `if/else`, that whole
branch computes nothing the program observes -- and the same is true of the
inner `days = D_80087BD4` half of the chained assignment, since `days` is
overwritten by `log->days` on the very next line. Both were tried removed,
independently, after the byte-exact candidate was found:

| removed | result |
| --- | --- |
| the `else { p = D_80087BD4; }` branch (reverting to unconditional `p = D_80087BD4;` before the `if`) | regresses to 15/56, 1 word short -- this is the round-24 shape again |
| the chained form, i.e. `p = D_80087BD4; days = log->days;` instead of `p = (days = D_80087BD4); days = log->days;` | regresses to 11/56, length wrong by ~95KB (whole-image drift) |

Both are now commented in-source as **do not simplify without
re-verifying** (`src/class_3bb8c_t.c`, directly above the function). This
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

Live in `src/class_3bb8c_t.c`, immediately following the `D_80087BD4`
declaration:

```c
extern s16 D_80087BD4[4];

/* Round 41 (2026-09-14): matched from a permuter-found lead. `p` and `days`
 * are LOCAL pointer caches of D_80087BD4 and log->days respectively -- not
 * because retail's semantics need them (both globals are re-derivable
 * without a temporary), but because caching them THIS WAY is what makes
 * cc1 2.6.3 stop strength-reducing D_80087BD4[i] into a pointer induction
 * variable hoisted across the outer loop (see the match report for the
 * full derivation). The `else { p = D_80087BD4; }` branch below and the
 * `p = (days = D_80087BD4);` chained assignment are BOTH semantically
 * inert -- p is unconditionally overwritten with the same value either
 * way -- but removing either one measurably regresses the codegen (round
 * 41 confirmed both empirically, byte-exact with them, off by dozens of
 * words without). Do not "simplify" this without re-running
 * ./build-and-verify.sh. */
s32 GraphRoomObj__ScoreDayLog(D_80087AACObj *self, D_80087AACUnkA4Result *log)
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
                p = D_80087BD4;
            }
            p = (days = D_80087BD4);
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
independent reshapes of `D_80087BD4[i]`'s own spelling (sized/unsized
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
