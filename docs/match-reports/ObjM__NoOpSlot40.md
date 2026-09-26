# ObjM__NoOpSlot40

**Unit:** class_3bb8c_l · **Status:** MATCHED (splat-generated, `jr $ra; nop`)

## What it does

```c
void ObjM__NoOpSlot40(void) {
}
```

An empty body -- splat generated this stub itself (`jr $ra; nop`), not
work done in this round. It fills vtable slot `+0x040` of `D_80087034`
(`tools/classtable.py 0x80087034`), the class whose constructor
(`ObjM__ObjM`, slot `+0x008`) and destructor (`ObjM__Dtor`, slot `+0x00C`)
confirm the table is `ObjM`'s own, the same class as sibling unit
class_3bb8c_m's `ObjM`.

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_80052DE0` | `ObjM__NoOpSlot40` | A | empty body, splat-generated stub; slot number is the only real content, matching the project's established `Class__NoOpSlotXX` convention (e.g. `TextRow__NoOpSlotD0`, `Actor__NoOpSlotD8`) |
