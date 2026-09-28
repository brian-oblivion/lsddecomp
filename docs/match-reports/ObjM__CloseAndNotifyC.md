# ObjM__CloseAndNotifyC

> Renamed from `func_8005426C` on 2026-09-23 (tools/rename.py). Address 0x8005426c.

**Unit:** class_3bb8c_m · **Size:** 25 instructions · **Status:** MATCHED (25/25 words)

## What this function does

Same shape as `ObjM__CloseAndNotifyD` (this round), differing only in the literal
passed to `slot30` (`0xC` here vs `0xD` there):

```c
void ObjM__CloseAndNotifyC(ObjM *self) {
    if (self->unk84) {
        self->methods->slotD4(self);
        self->methods->slot30(self, 0xC);
    }
}
```

## Residue

None — matched on the first attempt, by copying `ObjM__CloseAndNotifyD`'s already-
confirmed shape and changing the one literal (per DECOMPILATION_LEARNINGS'
"when a function closely resembles an already-matched sibling, copy its
exact idiom before deriving anything").

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`.

## Naming

**ObjM__CloseAndNotifyC** -- tier B. Identical shape to `ObjM__CloseAndNotifyD`, differing only in the notify code (0xC). Same tier and same caveat.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and DayTaskStageMap.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Track 7 (2026-09-27, round 98, delta)

`0xC` is `OBJM_NOTIFY_CLOSE`: DayTask__OnObjMNotify ends the day with
endDay(1) for 12 (EndDay's outcome 1 does neither the day advance nor the
new game). No caller in C. Zero bytes.
