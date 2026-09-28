# TextRow__SetColor -- MATCHED (40/40 words)

> Renamed from `Obj6EAC0__PropagateColor` on 2026-09-26 (tools/rename.py). Address 0x80040d74.

> Renamed from `func_80040D74` on 2026-09-18 (tools/rename.py). Address 0x80040d74.

Unit: `src/ui/screen_widgets.c`.

## Final body

```c
void TextRow__SetColor(Obj6EAC0 *self, s32 a1) {
    Obj6EAC0 **elemp = self->unkB4 + self->unkAC;
    s32 i = self->unkAC;
    s32 bound = i;
    s32 count = i + self->unkAB;
    if (i < count) {
        do {
            Obj6EAC0 *elem = *elemp;
            s32 ab;
            elemp++;
            ab = self->unkAB;
            elem->methods->slotB8(elem, a1);
            i++;
            bound = self->unkAC;
        } while (i < (bound + self->unkAB));
    }
}
```

Loop over `self->unkB4[unkAC..unkAC+unkAB)`, dispatching each child's own
`slotB8(child, a1)`.

## How the prior 38/40 stall closed

The prior best (`ab`/`ac` both declared, assigned in the same statement
pair every iteration, `count` recomputed and used as the loop condition)
was one pure two-instruction reorder short: retail loads `self->unkAC`
BEFORE `self->unkAB` each iteration (`lbu v0,0xac(s1)` then
`lbu v1,0xab(s1)`), every C shape tried loaded them in the opposite
order, with the following `addu` byte-identical either way.

Ran the permuter (bare randomization, no `PERM` macros, `-j 6
--stop-on-zero --best-only`, `timeout 400`, ~26,663 iterations) seeded
from the prior 38/40 body. **Reached a genuine zero** (`output-0-1`).
The winning candidate was heavily permuter-mangled (a function-pointer
extracted into its own local before the call, an `ac = (ab = ...);`
double-assignment, an external `bound`-carrying variable reset every
iteration) -- not committed as found. Simplified it back to ordinary C
one change at a time, re-verifying `--debug` stayed at 0 after each
step:

1. Dropped the function-pointer indirection (`fn = elem->methods->
   slotB8; fn(elem, a1);` -> `elem->methods->slotB8(elem, a1);`):
   still 0.
2. Dropped the double-assignment trick for `ab`/`ac`
   (`ac = (ab = self->unkAB); ... ac = self->unkAC;`) down to a single
   `ab = self->unkAB;` statement, and inlined the second field read
   directly into the loop condition instead of a separate `ac`/`count`
   reassignment: still 0.
3. Removed a provably dead in-loop `count = ...` store (the loop
   condition no longer reads `count` once the field read is inlined):
   still 0.

**The essential, load-bearing fact the mangled candidate was pointing
at**: retail's loop condition re-reads `self->unkAB` a SECOND time
(combined with a fresh `self->unkAC` read into `bound`) directly in the
`while` clause, rather than going through the `count` variable computed
earlier in the loop body at all -- `count` only matters for the
PRE-loop guard. Every prior attempt kept `count` live as the thing the
condition tests, recomputing it each iteration; retail's actual
condition is a fresh expression, not a re-tested cached variable. This
is the structural fix; the `ab` load happening before the call
(matching retail's own instruction order exactly, no barrier needed
once written this way) came along for free.

Verified against the real oracle: `./build-and-verify.sh` exits 0,
whole-image SHA1 matches, `funcdiff.py TextRow__SetColor` reports `40/40
words match`.

## Attempts (7 manual, round 15, preserved from the prior report)

1. `count = i + self->unkAB;` (offset-order operand), field recompute
   as one direct expression -- 34/40.
2. `count = self->unkAB + i;` (load-order operand) -- 36/40.
3. Named `s32 ac, ab;` temps, `ac` first then `ab`, on attempt 1's
   pre-loop order -- 34/40, worse.
4. Same temps, `ab` first then `ac`, combined with attempt 1's pre-loop
   order -- 38/40 (previously reported best).
5-6. Bare `__asm__("");` barrier variants -- both worse (31/40),
   perturbed unrelated register identity.
7. Declaration-list order swap only -- no effect, 38/40.

Plus this round's permuter run and 3-step simplification (above),
closing the function.

### Proposed learning

**The loop condition ITSELF can be a fresh recomputed expression rather
than a re-test of a variable maintained through the loop body** -- every
manual attempt assumed `count` (maintained as a loop-carried variable,
recomputed each iteration and tested in the `while`) was the right
shape, because that is the natural way to write "recompute the bound
each pass." Retail instead lets the PRE-loop guard use `count` once and
writes the back-edge test as its own independent expression
(`bound + self->unkAB`) that does not flow through `count` at all. This
is a variant of the project's "do not transcribe a lowering back into
C" caution, but at the loop-condition level rather than an
expression-lowering level: a maintained loop variable and a
recomputed-inline condition can be semantically identical yet compile
to different code, and only the permuter's blind search (not word-count
diffing) surfaced which one retail used. Cross-reference
`TextRow__SetDisplay.md` and `TextRow__DetachFromParent.md` -- this unit's OTHER
`unkB4[unkAC..unkAC+unkAB)` loop functions -- to check whether this same
"condition is a fresh expression, not the maintained variable" shape
applies to their own remaining residues too.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040D74` | `TextRow__SetColor` | B |

**Evidence.** Derived occupant of `slotB8` (the same slot
`BoxFill__SetColor`, this unit's base occupant, fills with the real
colour-set/blend call): loops over `self->children[self->childStart ..
self->childStart+self->childCount)` calling `elem->methods->slotB8(elem,
a1)` on each -- i.e. forwards the same slot's own call down to every
child. Mechanics fully established (a for-each-child dispatch through the
identical slot); whether the game actually uses this to recolour a whole
row of children (vs. some other meaning `a1` carries) is not independently
confirmed, hence tier B rather than A.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/text_row.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__PropagateColor`: the +0x0B8 setColor occupant, `(TextRow *self, ColorRgb *rgb)`, forwarded to each visible cell. Callers pass TaskCoreTarget's 3-byte selected/unselected colours and ItemList's row colours. Image byte-identical; the current source is src/ui/screen_widgets.c.

## Track 7 (round 99, bravo)

The dead `ab = self->visibleCount` load and the `count` local are gone: the entry test now reads `i < bound + self->visibleCount` like its siblings. Byte-exact (40/40); the permuter-era shape above is no longer needed.
