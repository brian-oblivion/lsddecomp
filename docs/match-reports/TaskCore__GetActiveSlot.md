# TaskCore__GetActiveSlot — MATCHED (3/3)

> Renamed from `Obj86B60__GetActiveSlot` on 2026-09-25 (tools/rename.py). Address 0x8003d5c0.

> Renamed from `func_8003D5C0` on 2026-09-24 (tools/rename.py). Address 0x8003d5c0.

**Unit:** code_2cc8c · **Size:** 3 words · **Result:** byte-exact, first attempt

## What it does

A one-line getter for `self->unk58`, an already-modelled field.

```c
s32 TaskCore__GetActiveSlot(Obj86B60 *self)
{
    return self->unk58;
}
```

No residue, no header change. Included here per the project rule that every
touched function gets a report, matches included.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__GetActiveSlot`. **Tier A**: A one-line getter for `self->activeSlot`, an already-modelled field. Distinct from the pre-existing Obj86B60__GetActiveSlotCount (which returns `slotCounts[activeSlot]`, a COUNT, not the slot index itself).

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__GetActiveSlot (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
