# ObjM__EnterStateA

> Renamed from `func_80053E00` on 2026-09-23 (tools/rename.py). Address 0x80053e00.

**Unit:** class_3bb8c_m · **Size:** 33 instructions · **Status:** MATCHED (33/33 words)

## What this function does

Sets `self->unk20 = 0xA`, calls `ObjM__ForwardToSubChild(self, 0, 0, 6, 1)`, then
makes two further calls on `self->unk3C`: slot `0x13C` with argument 2,
then slot `0xF4` (the same slot `ObjM__EnterState8` uses) also with argument 2.

## The C

```c
void ObjM__EnterStateA(ObjM *self) {
    self->unk20 = 0xA;
    ObjM__ForwardToSubChild(self, 0, 0, 6, 1);
    self->unk3C->methods->slot13C(self->unk3C, 2);
    self->unk3C->methods->slotF4(self->unk3C, 2);
}
```

## Residue

None — matched on the first attempt.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`.
