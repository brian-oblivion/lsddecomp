# ObjM__DetachTarget

> Renamed from `func_80052EBC` on 2026-09-24 (tools/rename.py). Address 0x80052ebc.

**Unit:** ObjMStyleActor · **Size:** 21 words (0x54 bytes) ·
**Status: MATCHED 21/21**, whole-image SHA1 green.

## What it does

```c
void ObjM__DetachTarget(Obj87034_3bb8c_l *self) {
    self->methods->slot14(self, self->unk3C);
    GetTimedTaskMethods()->slot48(self);
}
```

Straight-line: dispatch through `self`'s own vtable slot `+0x14`, then
through the shared BasicClass-family base table's slot `+0x48` (via
`GetTimedTaskMethods()`, the same accessor `ObjM__AttachTarget` uses at a different
slot). No branches, matched on the first correctly-typed attempt.

## Notes

- `BaseMethods87034_3bb8c_l::slot48` (the base accessor's `+0x48`) is a
  DIFFERENT table from `Obj87034Methods_3bb8c_l::slot48` (self's own
  `+0x48`, used by `ObjM__TeardownStyle` on a sibling object) — same numeric
  offset, unrelated classes, no naming collision since they're separate
  struct types.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80052EBC` | `ObjM__DetachTarget` | B | see below |

**Evidence.** vtable slot +0x048. Forwards `self->target` through self's own `slot14`, then the shared base accessor's `slot48` -- the structural mirror of `ObjM__AttachTarget` (attach/register at +0x044, teardown counterpart immediately after at +0x048).


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the ObjMStyleActor/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and DayTaskStageMap.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.
