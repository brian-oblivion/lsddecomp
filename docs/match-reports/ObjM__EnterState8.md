# ObjM__EnterState8

> Renamed from `func_80053D9C` on 2026-09-23 (tools/rename.py). Address 0x80053d9c.

**Unit:** class_3bb8c_m · **Size:** 25 instructions · **Status:** MATCHED (25/25 words)

## What this function does

Sets `self->unk20` (the same mode/state field `ObjM__EnterState7` writes) to 8,
calls the shared helper `ObjM__StartFadeUp(self, 0, 0, 6, 1)`, then tells
`self->unk3C` (via vtable slot `0xF4`) to run with argument 1.

## The C

```c
void ObjM__EnterState8(ObjM *self) {
    self->unk20 = 8;
    ObjM__StartFadeUp(self, 0, 0, 6, 1);
    self->unk3C->methods->slotF4(self->unk3C, 1);
}
```

## Residue

None — matched on the first attempt. Initially mis-transcribed by missing
the trailing `slotF4` call entirely (the .s file's tail was easy to miss on
a first read); re-reading the full disassembly before writing the C caught
it before any build was attempted.

## Provenance

round 15 (2026-09-04), runner echo, fresh carve `class_3bb8c_m`.

## Naming

**ObjM__EnterState8** -- tier B. Same shape as `ObjM__EnterState7` for `ObjM::mode = 8`, forwarding request code 6 and then `dreamSys->setMoveOverride(dreamSys, 1)`. Mechanically described, purpose (why 8) not established.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and class_39e08.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Track 7 (2026-09-27, round 98, delta)

`OBJM_STATE_LINK_TUNNEL` for 8, `DREAM_COLOR_BLACK` for the zero channel
mask (FadeBox's mask 0; ObjM__EnterState4 already spells it this way),
`MOVE_OVERRIDE_FORCED` for setMoveOverride's 1 (enum DreamSysMoveOverride,
include/DreamSys.h, added this round from DreamSys__TickMove: 0 runs
tickMoveFree, 1 tickMoveForced, 2 tickMoveHeld). Zero bytes.
