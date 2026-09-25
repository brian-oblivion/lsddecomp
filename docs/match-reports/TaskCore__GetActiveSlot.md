# TaskCore__GetActiveSlot — MATCHED (3/3)

> Renamed from `Obj86B60__GetActiveSlot` on 2026-09-25 (tools/rename.py). Address 0x8003d5c0.

> Renamed from `func_8003D5C0` on 2026-09-24 (tools/rename.py). Address 0x8003d5c0.

**Unit:** code_2cc8c_b · **Size:** 3 words · **Result:** byte-exact, first attempt

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

Renamed `func_` -> `TaskCore__GetActiveSlot`. **Tier A**: A one-line getter for `self->activeSlot`, an already-modelled field. Distinct from the pre-existing TaskCore__GetActiveSlotCount (which returns `slotCounts[activeSlot]`, a COUNT, not the slot index itself).
