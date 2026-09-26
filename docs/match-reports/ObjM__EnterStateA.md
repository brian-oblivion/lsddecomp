# ObjM__EnterStateA

> Renamed from `func_80053E00` on 2026-09-23 (tools/rename.py). Address 0x80053e00.

**Unit:** class_3bb8c_m · **Size:** 33 instructions · **Status:** MATCHED (33/33 words)

## What this function does

Sets `self->unk20 = 0xA`, calls `ObjM__StartFadeUp(self, 0, 0, 6, 1)`, then
makes two further calls on `self->unk3C`: slot `0x13C` with argument 2,
then slot `0xF4` (the same slot `ObjM__EnterState8` uses) also with argument 2.

## The C

```c
void ObjM__EnterStateA(ObjM *self) {
    self->unk20 = 0xA;
    ObjM__StartFadeUp(self, 0, 0, 6, 1);
    self->unk3C->methods->slot13C(self->unk3C, 2);
    self->unk3C->methods->slotF4(self->unk3C, 2);
}
```

## Residue

None — matched on the first attempt.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`.

## Naming

**ObjM__EnterStateA** -- tier B. Same shape again for `ObjM::mode = 0xA`, forwarding request code 6, then `dreamSys->selectCallback98(dreamSys, 2)` and `setMoveOverride(dreamSys, 2)`. Tier B for the same reason as its two siblings.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.
