# Obj86B60__GetActiveSlot — MATCHED (3/3)

> Renamed from `func_8003D5C0` on 2026-09-24 (tools/rename.py). Address 0x8003d5c0.

**Unit:** code_2cc8c_b · **Size:** 3 words · **Result:** byte-exact, first attempt

## What it does

A one-line getter for `self->unk58`, an already-modelled field.

```c
s32 Obj86B60__GetActiveSlot(Obj86B60 *self)
{
    return self->unk58;
}
```

No residue, no header change. Included here per the project rule that every
touched function gets a report, matches included.
