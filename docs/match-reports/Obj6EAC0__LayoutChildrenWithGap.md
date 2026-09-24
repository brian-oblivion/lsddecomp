# Obj6EAC0__LayoutChildrenWithGap — MATCHED (70/70), round 19

> Renamed from `func_80040AE8` on 2026-09-18 (tools/rename.py). Address 0x80040ae8.

## Final body (byte-exact, full oracle green)

```c
void Obj6EAC0__LayoutChildrenWithGap(Obj6EAC0 *self, s32 a1, Pair32E99C *a2) {
    Pair32E99C buf;
    s32 i, bound;
    Obj6EAC0 **elemp;

    if (self->unkC != 0) {
        return;
    }
    func_80041C3C()->slot4C(self, a1, a2);
    buf = *a2;
    elemp = self->unkB4 + self->unkAC;
    i = self->unkAC;
    bound = i;
    if (i < bound + self->unkAB) {
        do {
            if (self->unkAA != 0 && i == self->unkAA) {
                buf.a += 0x10;
            }
            (*elemp)->methods->slot4C(*elemp, self, &buf);
            buf.a += self->unkB0;
            bound = self->unkAC;
            elemp++;
            i++;
        } while (i < bound + self->unkAB);
    }
}
```

`build-and-verify.sh` exits 0 (`OK: build matches retail SLPS_015.56`) --
genuinely whole-image byte-exact.

## Round 19: three levers stacked, in order

1. **Whole-struct assignment for the `a2`-copy buffer** (this round's
   recurring lever -- see `Class6E99C__PushPosition`/`Obj6EAC0__SetPosition`/`Obj86B60__SetColors`/
   `Obj6EAC0__ApplyColor`/`Obj6EAC0__LayoutChildren`): retyped the local `s32 buf[2]` to
   the existing `Pair32E99C` and `a2` to `Pair32E99C *`, replaced
   `buf[0]=a2[0]; buf[1]=a2[1];` with `buf = *a2;`. This alone closed
   the ORIGINAL report's +2-word length gap outright and jumped the
   score from 16/70 to 62/70 in one step.
2. **Statement-order swap for the loop increment** (the same lever that
   closed `Obj6EAC0__LayoutChildren` this round): swapping `i++; elemp++;` to
   `elemp++; i++;` fixed a delay-slot-filler placement residue --
   confirmed NOT a register-identity issue (same registers both ways,
   only which of two adjacent slots got the `addiu $s1,$s1,1` vs a
   `nop`).
3. **Matching `Obj6EAC0__PropagateColor`'s own idiom for the recomputed bound**
   (this unit's sibling in the same `unkB4[unkAC..unkAC+unkAB)` loop
   family, already matched): rather than storing `self->unkAC +
   self->unkAB` into a single combined `bound` local each iteration (my
   first attempt after step 2, which left a pure register/operand-order
   residue no amount of `+`-operand reordering closed -- tried both
   `i + self->unkAB` and `self->unkAB + i`, each fixed one of two
   residue instances while breaking the other), split it exactly the
   way `Obj6EAC0__PropagateColor` already does: `bound = self->unkAC;` (JUST the
   base, no addition) with the comparison itself written as
   `i < bound + self->unkAB` at BOTH the pre-loop guard and the
   `while` condition, rather than pre-summing into `bound`. This closed
   the last 4 words immediately.

This confirms the round-2 report's own cross-reference note (a lever
that closes one sibling doesn't reliably transfer to another) needed
one addition: it DOES transfer once applied to BOTH the pre-loop guard
and the loop-exit condition identically, not just the in-loop
recompute alone -- `Obj6EAC0__PropagateColor`'s own body already keeps `bound` as
a bare field read and defers the `+ self->unkAB` to each COMPARISON
site rather than ever materializing the sum into a variable, and that
detail (not merely "use named field reads") is what this function
also needed.

## Round-2 history (superseded, kept for the record)

STALL (best 16/70, +2 words long)

Unit: `src/code_2cc8c_f.c`. Blocker screen clean. Callee-saved count
is 3 (`$s0`,`$s1`,`$s2`) -- below the round-13 saturation threshold, so
not that class either.

Body reached (near-miss, preserved literally):

```c
#if 0
void Obj6EAC0__LayoutChildrenWithGap(Obj6EAC0 *self, s32 a1, s32 *a2) {
    s32 buf[2];
    s32 i, count;
    Obj6EAC0 **elemp;

    if (self->unkC != 0) {
        return;
    }
    func_80041C3C()->slot4C(self, a1, a2);
    buf[0] = a2[0];
    buf[1] = a2[1];
    elemp = self->unkB4 + self->unkAC;
    i = self->unkAC;
    count = i + self->unkAB;
    if (i < count) {
        do {
            if (self->unkAA != 0 && i == self->unkAA) {
                buf[0] += 0x10;
            }
            (*elemp)->methods->slot4C(*elemp, self, buf);
            buf[0] += self->unkB0;
            count = self->unkAC + self->unkAB;
            i++;
            elemp++;
        } while (i < count);
    }
}
#endif
```

## The shape, derived from the disassembly

- Early exit when `self->unkC != 0` (whole function is a no-op then).
- One call BEFORE the loop: `func_80041C3C()->slot4C(self, a1, a2)`,
  where `a2` is a pointer to a 2-word struct.
- Copy `a2[0]`/`a2[1]` into a 2-word LOCAL stack buffer (this local
  buffer is later passed BY ADDRESS to each child, and mutated between
  iterations -- it is not a `const` copy).
- Loop over `self->unkB4[self->unkAC .. self->unkAC+self->unkAB)`:
  for each element `elem`, if `self->unkAA != 0 && i == self->unkAA`
  add `0x10` to the buffer's first word; then call
  `elem->methods->slot4C(elem, self, &buf)` (this unit's PARENT
  `self`, not the original `a1`, is passed as the child's 2nd arg --
  `a1`'s only use in the whole function is the ONE pre-loop call);
  then add `self->unkB0` into the buffer's first word again; recompute
  the loop bound each iteration (matches the project's documented
  "caching a loop bound the source re-reads costs an extra register"
  family, so the bound is deliberately NOT hoisted here).

## Measured residue

At the best form reached, retail is **70 words** (`0x118`, confirmed
against the `.s` file's `nonmatching Obj6EAC0__LayoutChildrenWithGap, 0x118`); my
compiled form is **72 words** (confirmed via
`objdump -d build/src/code_2cc8c_f.c.o`: spans `0x484`-`0x5a4` =
`0x120` bytes = 72 words) -- 2 words LONG, not a same-length pure
rename. The word-level diff also shows retail using `$s1` for BOTH the
short-lived `a1` (only live until the first call) AND the loop index
`i` afterward (register reuse across two non-overlapping live ranges,
naturally following register-allocation order), while `$s2` similarly
serves `a2` then `elemp` -- my compiled form put `a1`'s value into
`$s2` and `elemp` into a different register, so the whole function's
register numbering diverges from word 3 onward.

## Attempts (3)

1. Direct body (as shown above, in ROM/logical statement order) --
   16/70, +2 words, register permutation from word 3.
2. Reordered the three post-branch assignments to `i = ...; elemp =
   ...; count = ...;` (matching the ORDER retail's own registers
   "hand off" -- a1->i first, a2->elemp second) -- WORSE (12/70), and
   the outside-range byte count grew, meaning this also changed the
   LENGTH further away from 70, not just the register numbering.
   Reverted immediately.
3. (Implicit) Re-confirmed attempt 1 is the best reached before
   filing this stall.

## Direction NOT tried, and why

**Did not try:** isolating and fixing the +2-word length gap BEFORE
touching register order. Per
`docs/DECOMPILATION_LEARNINGS.md`'s "one instruction short... shifts
everything after it" family (and its `Class866E8__ApplyRateEntries` example, where a
low score was misread as a distant shape when it was one missing
`addiu`), a 70-vs-72-word gap is small enough that it may be ONE OR TWO
missing/duplicated instructions rather than a wholesale wrong shape --
but with the register identity ALSO scrambled from word 3, isolating
which of the two problems to fix first was not attempted; the register
angle was pursued first because it was more visible in the funcdiff
output, which in hindsight (per the same learnings entry) is the wrong
order to attack a stall with BOTH symptoms present. The next attempt
should find the +2-word source via `objdump` block-by-block comparison
against the `.s` file BEFORE touching declaration order again.

**Did not try:** the permuter. This function is large (70 words) with
a real loop and 2 calls; per `docs/MATCHING-GUIDE.md`'s guidance the
permuter is best seeded from a close manual near-miss, and 16/70 (or
even a hypothetical correct-length rewrite) is not close enough yet to
justify the setup cost here.

### Proposed learning

None promoted yet -- this stall's shape (mixed length-and-register
residue on a large loop body) doesn't cleanly fit an existing named
class, and per `docs/MATCHING-GUIDE.md`'s own caution, a length gap
should be resolved before trusting any register-identity reading on
top of it.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_80040AE8` | `Obj6EAC0__LayoutChildrenWithGap` | B |

**Evidence.** Derived occupant of `slot4C` (same slot `Obj6EAC0__Layout`
fills for the leaf case): the SAME "walk children, advance a position
cursor by `childPitch` per child" shape as `Obj6EAC0__LayoutChildren`
(this unit, `slotBC`'s derived occupant), but with one extra difference:
when the loop index equals `self->gapIndex`, an additional `0x10` is
added to the cursor before that child is placed -- a one-shot extra gap
at one specific child slot. Given this unit's own text/digit-display
hypothesis (see the unit header comment), the most natural reading is a
decimal-point or separator gap in a digit string, but that is not
independently confirmed -- tier B, and `gapIndex`/`childPitch` are named
for their mechanics (which index gets the gap; how far each child
advances), not for that specific guess.
