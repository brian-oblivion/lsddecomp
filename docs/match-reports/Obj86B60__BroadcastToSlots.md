# Obj86B60__BroadcastToSlots — MATCHED (57/57)

> Renamed from `func_8003D2CC` on 2026-09-24 (tools/rename.py). Address 0x8003d2cc.

**Unit:** code_2cc8c_b · **Size:** 57 words · **Result:** byte-exact

## What it does

Walks every element of `self->unk54[]` (`self->unk50` entries), forwarding
`a1` to each element's own `+0x0B8` slot. For each index whose
`self->unk4C->unk24[i]` is non-null, additionally sets `self->unk58 = i`
and fires `self->methods->slot104(self, a1)`. Restores `self->unk58` to its
original value once the whole walk is done — a save/restore around the
loop, not a permanent index change.

```c
void Obj86B60__BroadcastToSlots(Obj86B60 *self, void *a1)
{
    s32 origIdx;
    Unk64Elem **arr;
    s32 i;

    if (self->unk4C == NULL) {
        return;
    }
    arr = self->unk54;
    origIdx = self->unk58;
    for (i = 0; i < self->unk50;) {
        Unk64Elem *elem = *arr;

        arr++;
        elem->methods->slotB8(elem, a1);
        if (self->unk4C->unk24[i] != NULL) {
            self->unk58 = i;
            self->methods->slot104(self, a1);
        }
        i++;
        __asm__("");
    }
    self->unk58 = origIdx;
}
```

## Header additions

`include/code_2cc8c.h`: new field `unk54` on `Obj86B60`
(`Unk64Elem **`, walked with an incrementing pointer — the established
idiom from `Obj86B60__BroadcastToSlotElements`), carved from what had been 4 bytes of padding
immediately after `unk50`. New slot `slot104` on `Obj86B60Methods`
(`void (*)(Obj86B60 *, void *)`), carved from existing padding between
`slot100` and `slot108`. No existing declaration's type or offset changed.

## Residues, and how each closed

**Residue 1: an extra callee-saved register (9/57, then structural
size mismatch).** First attempt cached `count = self->unk50;` once before
the loop and used it as the loop bound. Retail does NOT cache this value —
it RE-READS `self->unk50` from memory on every iteration (a caller-saved
temp, not a persistent local), which is why retail's frame only needs 5
saved registers (`s0`-`s4`) where my cached version needed 6. **Fix: use
`self->unk50` directly in the loop condition instead of a cached local.**
This is the same family as `Obj86B60__BroadcastToSlotElements`'s "this unit's field reads are
not reliably CSE'd across a call" lesson, but here it is about NOT
introducing a persistent local for a value the source itself re-reads.

**Residue 2: instruction order only (39/57 once residue 1 closed).**
Every register matched; the sole difference was the ORDER of two
independent, data-unrelated instructions at the loop's back edge — the
`i++` and the reload of `self->unk50` for the next iteration's comparison.
Retail orders `addiu s1,s1,1` (increment) THEN `lw v0,0x50(s0)` (reload);
GCC's scheduler, with nothing forcing an order between two unrelated
instructions, picked the opposite order in every C shape tried that let
the increment live in a `for` loop's own increment-clause or in an
implicit loop-back.

Three restructurings were tried and made things WORSE by changing the
function's overall SIZE (each lost a saved register versus the working
39/57 baseline, confirmed via the "differs outside this range" warning
jumping to six figures each time): a `do { } while` wrapped in a separate
`if (count > 0)` guard (15/57), and an explicit `goto`-based loop with the
guard and back-edge written out by hand (32/57). Neither is included here
as a body — both were worse than the eventual fix and are not near-misses
worth preserving.

**Fix: a bare `__asm__("")` as the LAST statement in the loop body**,
immediately after the manual `i++;` (with the `for` loop's own increment
clause left empty so `i++` is a genuine body statement, not
compiler-synthesized). This pins the increment before the barrier and
therefore before whatever the compiler schedules next — the reload — while
touching neither register: removing the barrier only changed instruction
ORDER (both retail and the built code before the barrier already agreed on
which registers held `i` and the reloaded bound; only their sequence
differed), satisfying CLAUDE.md rule 6's test for a legitimate barrier.

### Proposed learning

**This is a genuinely new instance of "an `__asm__("")` barrier fixes a
pure instruction-order residue" — not the previously-documented cross-jump
class from `Obj86B60__FindNextFreeSlot`.** There, the problem was two SOURCE-DISTINCT
statements getting merged into one shared instruction, and a barrier could
not stop it (cross-jump is block-level unification). Here, there was only
ONE physical copy of `i++` in both retail and every C variant tried — the
compiler's local scheduler simply chose a different order for two
independent instructions within the same basic block, which is exactly
the class `__asm__("")` is documented to fix. **The tell that distinguishes
them: check whether the built code is missing/duplicating an instruction
(cross-jump, barrier won't help) or has the SAME instructions in a
different sequence (local scheduling, barrier should help).** Confirmed by
placing the barrier as the loop body's OWN final statement (after a manual
increment, with the `for`'s increment-clause left empty) rather than
relying on the `for` loop's implicit increment slot, which the compiler
treats as ineligible for user-barrier placement.

Also: **caching a value into a local specifically to serve as a loop
bound, when the source re-reads the underlying field every iteration
instead, costs a whole extra callee-saved register** — a stronger and more
diagnosable version of "not every field read is CSE'd" (see
`Obj86B60__BroadcastToSlotElements`), because here the tell is a FRAME SIZE mismatch (6 saved
registers vs. retail's 5), not a register-identity swap.
