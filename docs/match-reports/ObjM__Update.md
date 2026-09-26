# ObjM__Update

> Renamed from `ObjM__TickTarget` on 2026-09-26 (tools/rename.py). Address 0x800533f0.

> Renamed from `func_800533F0` on 2026-09-24 (tools/rename.py). Address 0x800533f0.

**Unit:** class_3bb8c_l · **Size:** 26 words (0x68 bytes) ·
**Status: MATCHED 26/26**, whole-image SHA1 green.

## What it does

```c
void ObjM__Update(Obj87034_3bb8c_l *self) {
    void (*fn)(Obj87034_3bb8c_l *);

    if (self->unk68 != 0) {
        self->unk1C++;
        if (self->unk80 != 0) {
            fn = self->methods->slotD0;
        } else {
            fn = self->methods->slot8C;
        }
        fn(self);
    }
}
```

Matched on the first attempt once `Obj87034Methods_3bb8c_l::slot8C` was
added to the header (this is its only call site in this unit's queue).
`self->unk1C++` is a delay-slot store in the original (`sw` in the branch's
delay slot) that is nonetheless UNCONDITIONAL relative to the `unk80`
check — it happens on every call that gets past the `unk68` guard, which a
plain post-increment statement placed before the nested `if` reproduces
directly.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_800533F0` | `ObjM__Update` | B | see below |

**Evidence.** vtable slot +0x05C. Gated on `self->unk68`: increments the `unk1C` counter, then dispatches one of two slots (`slotD0`/`slot8C`) depending on `self->unk80`.


## Track 4 (2026-09-26, round 89, echo)

Renamed from `ObjM__TickTarget` (rename.py): it occupies IntermediateBase's `update` (+0x05C, onNotify's FrameClock case) and nothing in it touches the DreamSys. While `inSession` it adds one to IntermediateBase::frameCounter (the base update's own work, inlined) and runs advancePauseSetup while the pause overlay is being built, else tickStyle. Tier A for the slot.
