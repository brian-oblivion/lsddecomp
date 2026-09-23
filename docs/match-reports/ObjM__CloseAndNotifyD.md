# ObjM__CloseAndNotifyD

> Renamed from `func_80054208` on 2026-09-23 (tools/rename.py). Address 0x80054208.

**Unit:** class_3bb8c_m · **Size:** 25 instructions · **Status:** MATCHED (25/25 words)

## What this function does

If `self->unk84` (the flag `ObjM__UpdateCloseReadyFlag`/`ObjM__ClearCloseReadyFlag` set/clear) is
nonzero, dispatch `self->methods->slotD4(self)` then
`self->methods->slot30(self, 0xD)`.

```c
void ObjM__CloseAndNotifyD(ObjM *self) {
    if (self->unk84) {
        self->methods->slotD4(self);
        self->methods->slot30(self, 0xD);
    }
}
```

Nearly identical to `ObjM__CloseAndNotifyC` (this round), differing only in the
literal passed to `slot30` (`0xD` here, `0xC` there).

## Residue

None — matched on the first attempt.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`.

## Naming

**ObjM__CloseAndNotifyD** -- tier B. If `self->unk84`, dispatches `teardownPauseOverlay` (CONFIRMED as this unit's own `ObjM__TeardownPauseOverlay` via `tools/classtable.py 0x80087034`, +0x0D4) then `notifyParents(self, 0xD)`. Named for the confirmed mechanics; event code 0xD's game meaning is not established.
