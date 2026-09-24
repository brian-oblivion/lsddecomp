# Class86F88__HandleInputCode -- MATCHED (69/69 words, first attempt)

> Renamed from `func_800522DC` on 2026-09-24 (tools/rename.py). Address 0x800522dc.

Unit `src/class_3bb8c_k.c`. Round 26, runner delta.

## What it is

An event/code dispatcher for this unit's own local view of method table
`D_80087034` (`Obj87034_3bb8c_k` / `Class87034Methods_3bb8c_k`, both
already declared earlier in this same file for `func_80052B70`/
`func_80052C10`/`Class86F88__SetState`/`Class86F88__TickClosing`). Owns `jtbl_800116F4`, a
sparse 22-entry jump table for codes `4..25`, six of which have real
handlers and the rest fall through doing nothing:

```c
void Class86F88__HandleInputCode(Obj87034_3bb8c_k *self, void *arg1, s32 code) {
    switch (code) {
    case 25:
        self->methods->slot60(self, 0x10);
        self->methods->slot54(self, 2);
        break;
    case 23:
        self->methods->slot60(self, 0x10);
        self->methods->slot54(self, 3);
        break;
    case 5:
        self->methods->slot7C(self);
        break;
    case 4:
        self->methods->slot80(self);
        break;
    case 18:
        self->methods->slot84(self);
        break;
    case 19:
        self->methods->slot88(self);
        break;
    }
}
```

`arg1` (the incoming second register argument) is never referenced in the
body -- matches this project's established event-dispatcher signature
shape (`self`, an unused/opaque second argument, an `s32` code), the same
one `class_3bb8c_l`'s `func_80053984`/`func_80053358` use.

## HEAD BROADCAST 1 (source-declaration-order case layout) DOES apply here

Unlike the sibling `func_80053984` (`class_3bb8c_l`, same round), whose
jump-table entry order already coincided with ascending case-value order,
THIS function's case bodies are laid out in the file in the order
**25, 23, 5, 4, 18, 19** -- neither ascending nor descending by value, and
clearly the SOURCE's own declaration order rather than anything the
compiler would choose on its own. The `switch` above lists the cases in
that exact order and matched on the first attempt with no further
reshaping. Between this function and `func_80053984`, the two ends of the
broadcast's claim are both now directly confirmed in this project: some
dense switches need the reorder, some don't, and the only way to tell is
to read the jump table's own body layout off the `.s` before writing the
`switch`.

## Struct edit (unit-local, NOT the shared header)

`Class87034Methods_3bb8c_k` is declared directly in
`src/class_3bb8c_k.c` (not `include/class_3bb8c.h`) -- this unit's own
independent, disjoint-slot view of table `D_80087034`, per the header's
own standing note that `class_3bb8c_l` reaches a different, non-overlapping
slot set on the identical table. Added six new slots, all inside the
existing `pad044[0x090-0x044]` (0x4C = 76 bytes):

```
pad044[0x10] + slot54(4) + pad058[8] + slot60(4) + pad064[0x18]
  + slot7C(4) + slot80(4) + slot84(4) + slot88(4) + pad08C[4]
= 16+4+8+4+24+4+4+4+4+4 = 76  (matches the original span exactly)
```

`slot40` (already used by `func_80052C10`) and `slot90`/`slotB0`/`slotB4`
(already used by `func_80052D10`) keep their original offsets --
confirmed by rebuilding (whole-image SHA1 green) after the struct edit,
before writing this function's body. No cross-unit prototype and no new
type went into either shared header (`class_3bb8c.h` or `class_39e08.h`);
this unit already includes both, per the coordinator's specific caution
for this unit, and neither was touched.

## Proposed learning

Two back-to-back same-round data points on HEAD BROADCAST 1
(`func_80053984`: table order == ascending value order, lever not needed;
`Class86F88__HandleInputCode`: table order == source declaration order, lever
essential) make the discriminator concrete: **check the jump table's own
label order against sorted case-value order before writing the switch,
every time** -- neither "always reorder" nor "never reorder" is safe, and
the check costs nothing (the labels are right there in the `.s`).
