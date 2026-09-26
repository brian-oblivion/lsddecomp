# TextRow__SetDisplay — MATCHED (41/41), round 19

> Renamed from `Obj6EAC0__QueryChildren` on 2026-09-26 (tools/rename.py). Address 0x80040cd0.

> Renamed from `func_80040CD0` on 2026-09-18 (tools/rename.py). Address 0x80040cd0.

## Final body (byte-exact, full oracle green)

```c
s32 TextRow__SetDisplay(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 **elemp = self->unkB4 + self->unkAC;
    s32 i = self->unkAC;
    s32 bound = i;
    if (i < bound + self->unkAB) {
        do {
            Obj6EAC0 *elem = *elemp;
            s32 result;
            elemp++;
            i++;
            result = elem->methods->slot60(elem, a1);
            bound = self->unkAC;
            a2 = result;
        } while (i < bound + self->unkAB);
    }
    return a2;
}
```

`build-and-verify.sh` exits 0 -- genuinely whole-image byte-exact.

## Round 19: `TextRow__AttachToParent`'s bound-recompute idiom transferred cleanly

Ran the permuter first (seeded from this report's attempt-2 body, the
recommended untried direction): `--debug --stack-diffs` confirmed base
170 (10 register differences, 2 reorderings), bounded search (`-j 6
--stack-diffs --stop-on-zero --best-only`, `timeout 300`) ran ~21,000
iterations and found no zero -- best was 165, barely under the base and
never translated to anything idiomatic. Not adopted; recorded as a
negative permuter result, not a lever.

**What closed it instead was applying `TextRow__AttachToParent`'s fix (matched
earlier this same round) precisely, not just "the same family idiom
loosely applied":** the report's own cross-reference note warned that a
lever closing one `unkB4[unkAC..unkAC+unkAB)`-loop sibling doesn't
reliably transfer to another (attempts 6-9 tried `TextRow__SetColor`'s
named-`ab`/`ac`-temp idiom here and got a DIFFERENT residue triplet each
time, never an improvement over attempt 2's 33/41). The detail that
made `TextRow__AttachToParent` succeed where those attempts didn't: never
pre-summing `self->unkAC + self->unkAB` into a single stored `bound`/
`count` variable at all -- keep `bound` as a bare `self->unkAC` field
re-read, and write the comparison itself as `i < bound + self->unkAB`
at BOTH the pre-loop guard and the loop's own `while` condition. Applied
that way (not the "count = a + b, stored once" shape every earlier
attempt here used, including attempt 2's best-of-4 original attempts
and attempts 6-9's temp-split variants), it closed BOTH of this
function's remaining residues (the pre-loop operand/register choice AND
the deferred-store-into-delay-slot) in one change -- the earlier
"three interacting sub-residues" framing turned out to be one
underlying shape choice, not three independent things to fix
separately.

### Proposed learning

Strengthens `TextRow__AttachToParent`'s own new learning with a second, harder
instance (this function's residue was rated "more tangled" than
`TextRow__SetColor`'s in the original report, and it still yielded to the
same precise idiom): for this unit's whole `unkB4[unkAC..unkAC+unkAB)`
loop family, the transferable lever is specifically **"never store the
summed bound in a variable; keep the recomputed field bare and add the
second field at each comparison site"** -- not the broader and, it turns
out, less precise idea of "use named temps for the two fields" that
several earlier attempts (here and in the cross-referenced siblings)
tried and found did not reliably transfer.

## Round-2 history (superseded, kept for the record)

STALL (best 33/41 words, correct length)

Unit: `src/code_2cc8c_f.c`. Blocker screen clean. Callee-saved count 4
(`$s0`-`$s3`) -- below saturation.

Body reached (near-miss, preserved literally):

```c
#if 0
s32 TextRow__SetDisplay(Obj6EAC0 *self, s32 a1, s32 a2) {
    Obj6EAC0 **elemp = self->unkB4 + self->unkAC;
    s32 i = self->unkAC;
    s32 count = self->unkAB + i;
    if (i < count) {
        do {
            Obj6EAC0 *elem = *elemp;
            s32 result;
            elemp++;
            i++;
            result = elem->methods->slot60(elem, a1);
            count = self->unkAC + self->unkAB;
            a2 = result;
        } while (i < count);
    }
    return a2;
}
#endif
```

`a2` is an "unused parameter in the caller shows up as genuinely
uninitialised" case: it is only ever used as the fallback return value
when the child array is empty, and gets overwritten with each
iteration's call result otherwise (last iteration wins). This is the
same shape as `TextRow__AttachToParent`/`TextRow__DetachFromParent` (loop over
`self->unkB4[unkAC..unkAC+unkAB)`) but this time `self` correctly
lands in `$s1` with `$s0`=`i`, `$s2`=cursor, `$s3`=`a1` -- so THIS
function is NOT an instance of that register-permutation class; the
residue here is two much smaller, more local scheduling questions.

## The residue, precisely (via `tools/asm-differ/diff.py`)

1. **Pre-loop bound, operand register choice.** Retail: `lbu v0,0xab`
   then `addu v0,s0,v0` (reuses `v0` as the sum's destination, `s0`
   first operand). Every operand-order variant of
   `i + self->unkAB`/`self->unkAB + i` tried reproduces the VALUE but
   picks `a0` or leaves the operand order swapped inside the `addu`
   itself (`v0,s0,v0` vs `v0,v0,s0`) rather than reusing `v0` correctly
   with `s0` as the first source register.
2. **Deferred store into the branch delay slot.** Retail computes the
   two `lbu`s for the loop-continuation bound, THEN the `addu`/`slt`,
   THEN defers `move $a2,$v0` (saving the CALL's return into `a2`) all
   the way into the `bnez`'s own delay slot -- the LAST possible
   position. Every C shape tried materialises that store earlier
   (right after the two `lbu`s, before the add/compare), even with an
   explicit separate `result` local and a same-block `__asm__("")`
   immediately before the assignment (which INSTEAD perturbed the
   loop's exit-value store, `move $v0,$a2`, and cost a whole word).

## Attempts (4)

1. Direct body, `count = self->unkAC + self->unkAB` (field order
   matching the field's own OFFSET order) -- 32/41.
2. Swapped the PRE-loop bound to `self->unkAB + i` (matching the LOAD
   order retail shows, `unkAB` loaded most recently before the add) --
   33/41, the current best; fixes one of the two-instruction pre-loop
   residue but not both directions of it.
3. Introduced an explicit `s32 result;` for the call's return, assigned
   to `a2` as a separate trailing statement -- no net change from
   attempt 2 (same 33/41), confirming the deferred-store residue is a
   SCHEDULING choice, not something an intermediate named value moves.
4. Added `__asm__("");` immediately before the `a2 = result;` statement
   (from attempt 3) AND reverted attempt 2's operand swap in the same
   trial -- WORSE and one word longer: the barrier reordered the wrong
   thing (the LOOP-EXIT `move $v0,$a2` epilogue store, not the
   in-loop delay-slot store it was aimed at). Reverted immediately.

## Round 15 update: 5 more attempts, all negative; still 33/41

Re-attempted per coordinator request, both to close the "not tried"
direction above and to test whether `TextRow__SetColor`'s newly-found
"split the in-loop recompute into named `ab`/`ac` temps, in a specific
order, entangled with the pre-loop operand choice" lever transfers to
this near-identical sibling function. It does NOT transfer cleanly:

5. The previously-flagged untried direction -- attempt 4's
   `__asm__("");` placed alone (attempt 2's correct pre-loop operand
   order KEPT, not reverted) -- WORSE than attempt 4 itself: 20/41 with
   a whole-function length regression (the "differs outside range"
   warning fires). Closes the "not tried" note from the original
   report with a firm negative; the barrier is not viable anywhere in
   this function regardless of what else is held fixed.
6. `TextRow__SetColor`'s exact winning combination (`i + self->unkAB` for
   the pre-loop bound, PLUS named `ab`,`ac` temps for the in-loop
   recompute, `ab` assigned first) -- 33/41, no improvement, and a
   DIFFERENT residue triplet than attempt 2's (the pre-loop `addu` and
   one `lbu` register differ from before, the deferred-store residue
   persists unchanged). Confirms the two functions' apparently-
   identical loop skeletons do NOT share one fix.
7. Same named-temp split with `ac` assigned first, on top of the
   `i + self->unkAB` pre-loop order -- 32/41, worse.
8. Same named-temp split (`ab` first) on top of attempt 2's ORIGINAL
   `self->unkAB + i` pre-loop order -- still 33/41, a third distinct
   residue triplet, no net improvement.
9. Direct, un-split in-loop expression reverted to
   `self->unkAC + self->unkAB` with the `i + self->unkAB` pre-loop
   order (i.e. attempt 1's in-loop form with the opposite pre-loop
   order from attempt 1) -- 32/41.

None of attempts 5-9 beat attempt 2's original 33/41, which remains
the reported best. Unlike `TextRow__SetColor`, splitting the field reads
into named temps here shifts the residue around (sometimes fixing the
`addu` operand order, sometimes not, always leaving at least 8 words
wrong) without ever closing more than attempt 2 already closed --
this function's THREE-way residue (pre-loop operand order, in-loop
register crossing, deferred delay-slot store) appears to need a
different combination than its sibling, if one exists at all via
manual reshaping.

## Direction NOT tried, and why

**Did not try:** the permuter. Given `TextRow__SetColor`'s sibling
residue (from the identical loop skeleton) turned out to be a clean
permuter target once isolated, and this function's remaining residue
is more tangled (three interacting sub-residues rather than one), a
permuter run seeded from attempt 2's body is the most promising
untried direction -- more promising than further hand reshaping, which
9 total attempts (4 original + 5 this round) have shown moves the
residue around without shrinking it.

### Proposed learning

The "defer a value's STORE into a later delay slot while the value
itself is computed earlier" shape (already documented for a
tail-call's delay slot, `Class866E8__SetFootprintFromCell`) also applies to a loop's OWN
back-edge branch, not just a tail call -- worth generalising that
entry once a lever is found here. Separately: **a lever that closes
one residue in a near-identical sibling function does not reliably
transfer**, even when the two functions share the same loop skeleton
almost verbatim (`TextRow__SetColor` vs this function) -- cross-reference
both reports before assuming a fix generalises across this unit's
`unkB4[unkAC..unkAC+unkAB)` loop family.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040CD0` | `TextRow__SetDisplay` | B |

**Evidence.** Derived occupant of `slot60`: loops over
`self->children[self->childStart..self->childStart+self->childCount)`
calling `elem->methods->slot60(elem, a1)` on each, threading the return
value through as its own return (last child's result wins, or the
incoming `a2` if the slice is empty) -- a for-each-child dispatch that
returns a value, distinct from the void-returning
`TextRow__SetColor`/`TextRow__SetPosition` family. Named for
this mechanic (query across children, not a specific field); the exact
value being queried is not established -- tier B.

## Track 4 (2026-09-26, round 88, charlie)

2026-09-26, round 88 (charlie): class 0x11144 unified as TextRow in `include/TextRow.h` (a row of CharSprite cells: the ctor makes `count` New_CharSprite cells, setText hands each the next byte of a string, the layout slots step `cellPitch` along x). The view `Obj6EAC0` (named after BoxFill's old table address) is gone; its +0x00C `hasChildren` is SceneNode's `parent` (--merge CONFLICT s32 vs pointer: only tested against 0, bytes unchanged), `children` (+0x0B4) is `CharSprite **cells`, and the per-cell calls go through CharSprite's slots by name. Renamed from `Obj6EAC0__QueryChildren`: the +0x060 setDisplay occupant. It forwards setDisplay(on) to each visible cell and returns the last result; with no visible cell it returns its third parameter, the caller's untouched $a2 (kept as `s32 result` for the bytes). Callers in code_2cc8c_b pass 0/1 (show or hide a slot's items). Image byte-identical; the current source is src/code_2cc8c_f.c.
