# ObjM__NoOpSlot7C

**Unit:** class_3bb8c_l · **Status:** MATCHED (splat-generated, `jr $ra; nop`)

## What it does

```c
void ObjM__NoOpSlot7C(void) {
}
```

An empty body -- splat generated this stub itself (`jr $ra; nop`), not
work done in this round. It fills vtable slot `+0x07C` of `D_80087034`
(`tools/classtable.py 0x80087034`), `ObjM`'s own table (confirmed via the
constructor/destructor slots, see `ObjM__NoOpSlot40.md`).

## Naming

Round 78 (charlie), FINISHING-PLAN track 3.

| was | now | tier | evidence |
| --- | --- | --- | --- |
| `func_800534C0` | `ObjM__NoOpSlot7C` | A | empty body, splat-generated stub; slot number is the only real content, matching the project's established `Class__NoOpSlotXX` convention |
