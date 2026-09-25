# TaskCore__CancelElementScroll — MATCHED (71/71)

> Renamed from `Obj86B60__CancelElementScroll` on 2026-09-25 (tools/rename.py). Address 0x8003dcac.

> Renamed from `func_8003DCAC` on 2026-09-24 (tools/rename.py). Address 0x8003dcac.

**Unit:** code_2cc8c_b · **Size:** 71 words · **Result:** byte-exact, first attempt

## What it does

`Obj86B60Methods::slot110` (already recorded in `code_2cc8c.h`). The
state-2 counterpart to `TaskCore__BeginElementScroll`'s state-1 handler: notifies
`slot100` (with `a2=0` this time, vs `TaskCore__BeginElementScroll`'s `a2=1`), dispatches
the CURRENT ring element through its `+0x0B8` slot with the same
`self->unk4C->unk10` buffer, then reads a NEW ring index out of
`self->unk4C->unk24[idx]` (treating that pointer as a small word array and
taking its element at index 1, i.e. `+4` bytes in), installs it as the new
`self->unk60[idx]`, dispatches THAT element through its own `+0x060` slot,
and finally reverts `self->unk3C` from `2` back to `1` before firing a
closing `slot60(self, 17)`.

```c
void TaskCore__CancelElementScroll(Obj86B60 *self)
{
    s32 idx;
    s32 counter;
    Unk64Elem **arr;
    Unk64Elem *elem1;
    Unk64Elem *elem2;
    s32 newVal;

    if (self->unk3C != 2) {
        return;
    }
    idx = self->unk58;
    counter = self->unk60[idx];
    self->methods->slot100(self, self->unk14, 0);
    arr = (Unk64Elem **)self->unk64[idx];
    elem1 = arr[counter];
    elem1->methods->slotB8(elem1, self->unk4C->unk10);
    newVal = ((s32 *)self->unk4C->unk24[idx])[1];
    self->unk60[idx] = newVal;
    elem2 = arr[newVal];
    elem2->methods->slot60(elem2, 1);
    self->unk3C = 1;
    self->methods->slot60(self, 17);
}
```

## Header addition

`include/code_2cc8c.h`: new slot `slot60` on `Unk64ElemMethods`
(`void (*)(Unk64Elem *, s32)`), carved from what had been the leading
padding before `slotB8`. No existing declaration changed.

## Residue

None — matched on the first attempt. This is the third function in this
unit built directly on top of `TaskCore__BeginElementScroll`'s established shape (state
gate, `slot100` notification, ring-indexed `Unk64Elem` dispatch through
`self->unk4C->unk10`/`unk24[idx]`); reusing that derivation wholesale
(down to the field-read order — `unk58` then `unk60[idx]` BEFORE the
`slot100` call, matching this function's own actual need to know the
current ring index early) avoided re-deriving anything.

One new piece of evidence: `self->unk4C->unk24[idx]` is not just a bare
pointer here — it is read through as a small array of at least two `s32`
words (`[1]` used for the new ring index), which is more than any prior
function in this unit needed from it. Left unmodelled as a named struct
field for now (accessed via a local `(s32 *)` cast at the point of use)
since only this one access is confirmed; a future function establishing
`[0]`'s meaning would be the natural point to give it a real name.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `TaskCore__CancelElementScroll`. **Tier B**: Also gated on `self->unk3C == 2` (state 2 -> 1), but reads a NEW cursor value out of `SlotEntry::savedCursor` (the SAME field TaskCore__CommitElementScroll just wrote) instead of keeping whatever the interactive scroll left in `slotCounts[idx]`, then installs THAT value as the new `slotCounts[idx]` -- i.e. it discards the in-progress scroll and reverts to the last-committed position. 'Cancel' follows directly from reading back the persisted value rather than keeping the live one.
