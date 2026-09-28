# ObjM__EnterLinkWall

> Renamed from `ObjM__EnterState6` on 2026-09-28 (tools/rename.py). Address 0x80053c94.

> Renamed from `func_80053C94` on 2026-09-24 (tools/rename.py). Address 0x80053c94.

**Unit:** ObjMStyleActor · **Size:** 33 words (0x84 bytes) ·
**Status: MATCHED 33/33**, whole-image SHA1 green.

## What it does

```c
void ObjM__EnterLinkWall(Obj87034_3bb8c_l *self) {
    s32 color;

    self->unk20 = 6;
    color = self->unk3C->methods->slot200(self->unk3C);
    ObjM__StartFadeUp(self, color, 0, 0x1E, 1);
    self->unk3C->methods->slotFC(self->unk3C);
}
```

Matched on the first attempt. `self->unk3C->methods->slot200` is a
`DreamSys` slot resolved via `tools/classtable.py 0x80087BDC` to
`DreamSys__GetDreamColor` (`include/dream_sys.h`) — that named C function
CANNOT be called directly here (it would compile to a plain `jal` by
symbol, not the `jalr` through the vtable pointer retail actually uses),
so this unit's own `DreamSysMethods_3bb8c_l::slot200` field is dispatched
through instead. `ObjM__StartFadeUp` is this unit's OWN sibling slice
`ObjMStyleActor` (still `INCLUDE_ASM` there); its 5th argument (`1`) is
passed on the stack past the four register argument slots, matching a
plain `extern s32 ObjM__StartFadeUp(Obj87034_3bb8c_l*, s32, s32, s32, s32);`
declaration with no special handling needed.

### Proposed learning

A vtable slot resolving (via `tools/classtable.py`) to an already-NAMED,
already-declared C function elsewhere (like `DreamSys__GetDreamColor`) is
still not safe to call by that name if the call site in question is an
INDIRECT (`self->methods->slotN(...)`) dispatch in the disassembly — the
named function and the vtable slot occupant are the same address, but
`jal <symbol>` and `jalr <register loaded from the vtable>` are different
bytes. Use the name only to confirm/cross-check the field's identity and
signature; the actual C call must still go through the struct's function
pointer field.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80053C94` | `ObjM__EnterLinkWall` | B | see below |

**Evidence.** vtable slot +0x09C. Sets `self->phase = 6`; same evidence as `ObjM__EnterTimeUp`.


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and dream_day.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.

## Round 95 (track 7, echo)

The fade step `0x1E` is written 30. Byte-identical.
