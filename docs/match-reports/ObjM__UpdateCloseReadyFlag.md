# ObjM__UpdateCloseReadyFlag

> Renamed from `func_800541D4` on 2026-09-23 (tools/rename.py). Address 0x800541d4.

**Unit:** dream_scene · **Size:** 11 instructions · **Status:** MATCHED (11/11 words)

## What this function does

No frame at all — `self` is used directly via `$a0` throughout (no calls).
`self->unk80` and `self->unk80`'s field-adjacent `self->unk84` are plain
`s32` fields, not vtable slots.

```c
void ObjM__UpdateCloseReadyFlag(ObjM *self) {
    if (self->unk80 != 0 && self->unk20 == 0) {
        self->unk84 = 1;
    }
}
```

## Residue

None — matched on the first attempt.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `dream_scene`.

## Naming

**ObjM__UpdateCloseReadyFlag** -- tier B. Sets `self->unk84 = 1` (the flag `ObjM__CloseAndNotify`/`ObjM__CloseAndNotifyNewGame` read) when `self->unk80 != 0` (the pause-setup counter is running) and `ObjM::mode == 0` (idle). Named for the mechanical poll; "close-ready" describes the flag's later use, not an established game concept.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/objm.h (table gObjMMethods, was D_80087034); the dream_scene/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and dream_day.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Track 7 (2026-09-27, round 98, delta)

`self->state == 0` is `OBJM_STATE_IDLE`. Zero bytes.
