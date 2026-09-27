# TaskCore__BroadcastToSlots — MATCHED (57/57)

> Renamed from `Obj86B60__BroadcastToSlots` on 2026-09-25 (tools/rename.py). Address 0x8003d2cc.

> Renamed from `func_8003D2CC` on 2026-09-24 (tools/rename.py). Address 0x8003d2cc.

**Unit:** Task · **Size:** 57 words · **Result:** byte-exact

## What it does

Walks every element of `self->unk54[]` (`self->unk50` entries), forwarding
`a1` to each element's own `+0x0B8` slot. For each index whose
`self->unk4C->unk24[i]` is non-null, additionally sets `self->unk58 = i`
and fires `self->methods->slot104(self, a1)`. Restores `self->unk58` to its
original value once the whole walk is done — a save/restore around the
loop, not a permanent index change.

```c
void TaskCore__BroadcastToSlots(Obj86B60 *self, void *a1)
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

`include/Task.h`: new field `unk54` on `Obj86B60`
(`Unk64Elem **`, walked with an incrementing pointer — the established
idiom from `TaskCore__BroadcastToSlotElements`), carved from what had been 4 bytes of padding
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
This is the same family as `TaskCore__BroadcastToSlotElements`'s "this unit's field reads are
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
class from `TaskCore__FindNextFreeSlot`.** There, the problem was two SOURCE-DISTINCT
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
`TaskCore__BroadcastToSlotElements`), because here the tell is a FRAME SIZE mismatch (6 saved
registers vs. retail's 5), not a register-identity swap.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__BroadcastToSlots`. **Tier B**: Forwards `a1` through every slot element's own `slotB8`, plus a secondary per-slot callback for slots with a target entry, saving/restoring `activeSlot` around the walk -- a broadcast to every slot, as opposed to TaskCore__BroadcastToSlotElements which broadcasts only within the CURRENT slot's own item list.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__BroadcastToSlots (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## asm sites

Round 89 (runner delta, track 5 `asm-sites`): the bare `__asm__("")` after
`i++` at the bottom of the loop is **justified** and now commented at the site.
Measured by deleting it alone: the image went red (233504 bytes: the function
came out one word shorter and everything after drifted), `funcdiff` 39/56, and
asm-differ shows `addiu s1,s1,1` (`i++`) moved from before
`lw v0,0x50(s0)` (the `self->slotCount` reload for the loop test) into that
load's delay slot, replacing the `nop` retail keeps there. Instruction order.

## Track 7 (2026-09-27, round 98, bravo)

The comment on the loop's `__asm__("")` is now one line, `/* MATCHING: without it GCC moves i++ into the slotCount load's delay slot. */`; the derivation stays above. Locals: origIdx -> savedSlot, arr -> widget, elem -> row, a1 -> color.
