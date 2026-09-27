# TaskCore__SetActiveSlot — MATCHED (57/57)

> Renamed from `Obj86B60__SetActiveSlot` on 2026-09-25 (tools/rename.py). Address 0x8003d4dc.

> Renamed from `func_8003D4DC` on 2026-09-24 (tools/rename.py). Address 0x8003d4dc.

**Unit:** code_2cc8c_b · **Size:** 57 words · **Result:** byte-exact

## What it does

Looks up two elements of `self->unk54[]` — one at `self->unk58` (only
dispatched through `slotB8` when that index is non-negative) and one at
the caller-supplied index `a1` (dispatched unconditionally) — feeding each
a different fixed offset into `self->unk4C` as a buffer pointer. Then
unconditionally installs `a1` as the new `self->unk58`, optionally fires a
notification (`slot70`) if `a2` is non-null, and always fires a final
`slot60(self, 9)`.

```c
void TaskCore__SetActiveSlot(Obj86B60 *self, s32 a1, void *a2)
{
    s32 idx;
    Unk64Elem *elemB;
    Unk64Elem *elemA;

    if (self->unk4C == NULL) {
        return;
    }
    idx = self->unk58;
    elemB = self->unk54[idx];
    elemA = self->unk54[a1];
    if (idx >= 0) {
        elemB->methods->slotB8(elemB, self->unk4C->unk10);
    }
    elemA->methods->slotB8(elemA, (u8 *)self->unk4C + 0x13);
    self->unk58 = a1;
    if (a2 != NULL) {
        self->methods->slot70(self, 0);
    }
    self->methods->slot60(self, 9);
}
```

No header changes — reuses `unk54`, `unk58`, `unk4C`, `unk4C->unk10`,
`slot70`, `slot60`, `Unk64Elem`/`slotB8`, all already modelled from earlier
functions in this unit.

## Residues, and how each closed

**Residue 1 (39/57): `elemB` computed only inside the `idx >= 0` guard.**
First attempt declared `elemB` as a block-scoped local inside the `if`,
matching the fact that it is only ever USED there. Retail, however, loads
BOTH `self->unk54[idx]` and `self->unk54[a1]` unconditionally, before the
sign check that decides whether `elemB` gets dispatched — the load
happens regardless of whether the value is used. **Fix: compute both
`elemB` and `elemA` unconditionally at the top, gating only the dispatch
call.** This is a variant of "an unused branch's associated computation
can still be unconditional in the source" — the sign check governs the
CALL, not the LOAD.

**Residue 2 (55/57): `self->unk58 = a1;` written inside `if (a2 != NULL)`.**
The remaining two-word swap was `self->unk58 = a1;` and the setup for the
`slot70` call trading positions. Retail places the store in the delay
slot of the `beqz` that skips the `slot70` call — meaning the store
executes UNCONDITIONALLY, even on the branch that skips the call
entirely. This is semantic evidence, not just a scheduling artifact: the
true source assigns `self->unk58 = a1;` OUTSIDE the `if`, and only the
`slot70` call is actually gated by `a2 != NULL`. Moving the assignment
out of the `if` (unconditional, immediately before it) matched
byte-for-byte. An `__asm__("")` barrier tried first (between the store and
the call, still inside the `if`) made the function one word LONGER
(introduced padding) rather than fixing it — this was never a scheduling
residue, it was a wrong conditional scope for the store.

### Proposed learning

**A value that LOOKS like it only matters inside an `if` (because it is
only READ or only USEFUL there) is not evidence it is only ASSIGNED
there.** Two independent instances of this exact shape in one function:
`elemB`'s load, and `self->unk58`'s store. Both times, retail computes/
stores the value unconditionally and gates only the later STATEMENT that
consumes it (a call). The tell in the disassembly is a value landing in a
branch's DELAY SLOT (always executes) when the naive C reading would put
it inside the branch's taken-only body — check whether the store you
suspect is "inside an if" is actually sitting in a delay slot before
concluding the C guard is right. A barrier will not fix this class (tried
here, made the function longer); only widening the assignment's scope
does.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__SetActiveSlot`. **Tier B**: Switches `self->activeSlot` to `a1`: un-highlights the old slot's representative element, highlights the new one (both via `slotElements[idx]->methods->slotB8`), then notifies (`slot70`) and fires a closing `slot60(self, 9)`. Same old/new-highlight-swap shape as TaskCore__SetSlotCursor one level down (items within a slot instead of slots/tabs themselves) -- cross-confirms the pairing. Mechanics (switch the active slot, with a visible highlight swap) are clear; kept tier B since it has real side effects beyond a plain setter.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__SetActiveSlot (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 98, bravo)

setState(9) -> TASKCORE_STATE_CURSOR_MOVED, playSound(0) -> TASKCORE_TONE_CURSOR. a1 -> slot, a2 -> withSound, elemB/elemA -> prevWidget/nextWidget.
