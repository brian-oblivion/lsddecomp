# ObjM__NoOpSlot7C

**Unit:** class_3bb8c_l · **Status:** MATCHED (splat-generated, `jr $ra; nop`)

## What it does

```c
void ObjM__NoOpSlot7C(void) {
}
```

An empty body -- splat generated this stub itself (`jr $ra; nop`), not
work done in this round. It fills vtable slot `+0x07C` of `gObjMMethods`
(`tools/classtable.py 0x80087034`), `ObjM`'s own table (confirmed via the
constructor/destructor slots, see `ObjM__NoOpSlot40.md`).

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_800534C0` | `ObjM__NoOpSlot7C` | A | empty body, splat-generated stub; slot number is the only real content, matching the project's established `Class__NoOpSlotXX` convention |


## Track 4 (2026-09-26, round 89, echo)

The class is unified as ObjM in include/ObjM.h (table gObjMMethods, was D_80087034); the class_3bb8c_k/_l/_m views (ObjM_3bb8c_k, Obj87034_3bb8c_l, ObjM) and DayTaskStageMap.h's Obj4C/SubObjB/EventArg are gone. Byte-identical.
