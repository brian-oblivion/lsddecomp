# TaskCore__BeginElementScroll — MATCHED (49/49)

> Renamed from `Obj86B60__BeginElementScroll` on 2026-09-25 (tools/rename.py). Address 0x8003da10.

> Renamed from `func_8003DA10` on 2026-09-24 (tools/rename.py). Address 0x8003da10.

**Unit:** code_2cc8c_b · **Size:** 49 words · **Result:** byte-exact, first attempt

## What it does

`Obj86B60Methods::slot108` (already recorded in `code_2cc8c.h`). Only acts
when `self->unk3C == 1`: notifies a new slot (`slot100`), looks up an
element from a computed doubly-indexed pointer array, hands it an 8-byte-
offset buffer pointer through its own `+0x0B8` slot, then advances
`self->unk3C` to state `2` and fires a fixed notification (`slot60(self,
14)`).

```c
void TaskCore__BeginElementScroll(Obj86B60 *self)
{
    s32 idx;
    Unk64Elem *elem;
    u8 *buf;

    if (self->unk3C != 1) {
        return;
    }
    idx = self->unk58;
    self->methods->slot100(self, self->unk14, 1);
    elem = ((Unk64Elem **)self->unk64[idx])[self->unk60[idx]];
    buf = (u8 *)self->unk4C->unk24[idx] + 8;
    elem->methods->slotB8(elem, buf);
    self->unk3C = 2;
    self->methods->slot60(self, 14);
}
```

## Header additions

`include/code_2cc8c.h`:

- New field `unk14` on `Obj86B60` (`s32`, carved from existing padding
  `0x004`-`0x01C`) — forwarded as an opaque word to `slot100`, never
  dereferenced by this unit.
- New slot `slot100` on `Obj86B60Methods`
  (`void (*)(Obj86B60 *, s32, s32)`), carved from existing padding
  `0xF4`-`0x108`.

No existing declaration's type or offset changed.

## Field-read-order check (per head's request)

`idx` was read from `self->unk58` explicitly BEFORE the `slot100` call in
the C, matching retail's own instruction order (`self->unk58` is loaded
into a callee-saved register ahead of the call, even though the value is
only consumed afterward — consistent with the source itself reading it
early rather than the compiler hoisting it). After the call, `self->unk64`
and `self->unk60` are read in that order (matching this unit's established
`unk58 -> unk64 -> unk5C/unk60` pattern noted in `TaskCore__BroadcastToSlotElements`'s and
`TaskCore__AdvanceSlotCursor`'s reports) — **fifth and sixth confirmed instances**,
counting `TaskCore__ReleaseSlotElements`, `TaskCore__AdvanceSlotCursor`, `TaskCore__RetreatSlotCursor`, `TaskCore__BroadcastToSlotElements`.
No violation found here.

## Residue

None — matched on the first attempt. The `self->unk4C->unk24[idx] + 8`
pointer arithmetic was cast to `u8 *` locally at the point of use rather
than retyping the shared field `Unk4CObj::unk24` (still `void **`, used
elsewhere in the sibling unit `code_2cc8c.c`'s already-matched
`TaskCore__Tick` as a pure null-check) — avoids a shared-header type change
for a computation this unit alone needs.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__BeginElementScroll`. **Tier B**: Gated on `self->unk3C == 1`. Notifies the target (`slot100`), highlights the item at the slot's PERSISTED cursor (`slotCounts[idx]`, the same field TaskCore__AdvanceSlotCursor/RetreatSlotCursor step), advances `unk3C` to 2, fires a closing notification. Opens interactive scrolling of the current slot's item list -- state 1 -> 2. Paired with TaskCore__CommitElementScroll/TaskCore__CancelElementScroll, both gated on state 2 and both returning to state 1; the data flow (DA10 highlights `slotCounts[idx]`, DAD4 later WRITES that same value into `SlotEntry::savedCursor`, DCAC READS `savedCursor` back out) is what grounds 'scroll session that a later step commits or cancels' rather than a guess.

## Proposed field names

Not renamed here -- `self->unk3C` is CROSS-UNIT (`code_2cc8c.c`'s
`TaskCore__OnPadConfirm`/`TaskCore__OnPadCancel`/`TaskCore__OnPadPrev`/`TaskCore__OnPadNext` all gate on
it too, plus the STALL `TaskCore__OnPadEvent`/`TaskCore__SetState`), not attempted as a
compiler-verified rename this round. Proposing for the head to apply at
merge:

- `Obj86B60::unk3C` -> `scrollState` (tier B). In this unit it is exactly
  the 1<->2 state this function/`TaskCore__CommitElementScroll`/
  `TaskCore__CancelElementScroll` open and close (BeginElementScroll:
  1->2; the other two: 2->1). Whether the sibling unit's own
  `TaskCore__OnPadConfirm` etc. use the same two values for the same meaning, or a
  wider range of states unrelated to scrolling, is NOT established from
  this unit alone -- the head or whoever names that unit should confirm
  `unk3C`'s full value range before applying this name tree-wide.


**Head disposition, round 78.** `unk3C` DECLINED this round: echo (`scrollState`) and delta (`notifyMode`) read it differently, and this report itself asks for its full value range to be confirmed first. It stays `unk3C` with both readings on file.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__BeginElementScroll (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (2026-09-27, round 98, bravo)

`(u8 *)target->unk24[idx] + 8` is `&((SlotEntry *)...)->cursorColor` (SlotEntry +0x008, a SpriteRgb; TitleMenu's is (128, 128, 0)). setState(14) -> TASKCORE_STATE_SCROLL_OPENED.
