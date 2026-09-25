# TaskCore__BroadcastToSlotElements — MATCHED (36/36)

> Renamed from `Obj86B60__BroadcastToSlotElements` on 2026-09-25 (tools/rename.py). Address 0x8003d980.

> Renamed from `func_8003D980` on 2026-09-24 (tools/rename.py). Address 0x8003d980.

**Unit:** code_2cc8c_b · **Size:** 36 words · **Result:** byte-exact

## What it does

`Obj86B60Methods` vtable data (`asm/data/76DC8.data.s`, `asm/data/57070.data.s`)
confirms this is a real method-table entry, though nothing in this unit
dispatches through the specific slot it occupies (still undifferentiated
padding in `include/code_2cc8c.h`, left alone). Walks `self->unk5C[idx]`
elements of the pointer array `self->unk64[idx]`, calling each element's own
`+0x0B8` vtable slot with the function's own second argument forwarded
unchanged.

```c
void TaskCore__BroadcastToSlotElements(Obj86B60 *self, void *a1)
{
    s32 idx = self->unk58;
    Unk64Elem **arr = (Unk64Elem **)self->unk64[idx];
    s32 count = self->unk5C[idx];
    s32 i;

    for (i = 0; i < count; i++) {
        Unk64Elem *elem = *arr;
        arr++;
        elem->methods->slotB8(elem, a1);
    }
}
```

## Header addition, and a field with two readings

`include/code_2cc8c.h`: new type `Unk64Elem`/`Unk64ElemMethods` (a `+0x0B8`
slot, `void (*)(Unk64Elem *, void *)`) — this is a DIFFERENT class from the
existing `Unk78Obj`, which happens to share the same slot offset with a
different signature (3 args vs. 2).

This function reinterprets `self->unk64[idx]` (already typed `void **` from
`TaskCore__ReleaseSlotElements`, where it is used as an opaque resource handle passed
directly to `ReleaseBasicClassArray`/`BMemPMgrFree`) as `Unk64Elem **` — the SAME
field, two different readings depending on which function touches it. The
field itself stays `void **self->unk64` in the struct; only this function's
local variable casts it, so neither reading contaminates the other. Both are
consistent with the same underlying fact: `self->unk64[idx]` is a raw
allocated block whose size is `self->unk5C[idx]` (a count/capacity, also
independently established from `TaskCore__AdvanceSlotCursor`/`TaskCore__RetreatSlotCursor`'s ring-buffer
usage) — as a resource handle it is opaque, and as this function sees it, it
is exactly an array of that many object pointers.

## The one residue, and how it closed

First attempt read the two fields in the "obvious" order for a loop —
compute the bound first (`self->unk5C[idx]`), then the array
(`self->unk64[idx]`) — and scored 30/36: a register swap between `$v0`/`$v1`/
`$a0` across the whole prologue, propagating through every instruction up to
the loop body.

Retail's raw field-read order is `unk58` (idx), THEN `unk64`, THEN `unk5C` —
matching the exact order already established for `TaskCore__ReleaseSlotElements` and
`TaskCore__AdvanceSlotCursor`/`TaskCore__RetreatSlotCursor` in this same unit (idx first, `unk64`
second, `unk5C` third). Reordering the two local declarations to match —
`arr` (from `unk64`) before `count` (from `unk5C`) — matched immediately.

### Proposed learning

**This unit has a STANDING field-read order for `Obj86B60`: `unk58` (index),
then `unk64`, then `unk5C`, regardless of which of those three values the
function actually uses first in its own logic.** Four functions
(`TaskCore__ReleaseSlotElements`, `TaskCore__AdvanceSlotCursor`, `TaskCore__RetreatSlotCursor`, `TaskCore__BroadcastToSlotElements`) now
confirm it. When a function in this unit touches more than one of these
three fields, declare/read them in that order even if the function's own
control flow would naturally read them in a different sequence — the
compiler reproduces retail's register assignment from source STATEMENT
order, not from logical necessity.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__BroadcastToSlotElements`. **Tier B**: Forwards `a1` through `slotB8` of every element in the CURRENT slot's own item list (`self->itemLists[activeSlot]`) -- narrower in scope than TaskCore__BroadcastToSlots (which walks every SLOT, not one slot's items). Confirmed as a real vtable slot (Obj86B60Methods, slot104) by `asm/data/76DC8.data.s`/`asm/data/57070.data.s` even though nothing in this unit dispatches through this exact slot.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__BroadcastToSlotElements (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
