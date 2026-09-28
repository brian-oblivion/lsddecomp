# TimedTask__Deinit

> Renamed from `Class86668__Deinit` on 2026-09-26 (tools/rename.py). Address 0x8004a324.

> Renamed from `func_8004A324` on 2026-09-23 (tools/rename.py). Address 0x8004a324.

**Unit:** dream_day · **Size:** 14 words (0x38 bytes) · **Status:** MATCHED (14/14 words)

## What it does

Method-table slot +0x048 of `gTimedTaskMethods` (the sibling class; see
`TimedTask__Finalize.md`). Calls the BASE class's own +0x048 slot (fetched through
`GetIntermediateBaseMethods()`) directly on `self` -- an explicit "call the base
implementation" pattern, not a self-vtable dispatch.

## Derivation

```
jal   GetIntermediateBaseMethods
 move $s0, $a0
lw    $v0, 0x48($v0)
jalr  $v0
 move $a0, $s0
```

Written as:

```c
void TimedTask__Deinit(Obj865C8 *self) {
    GetIntermediateBaseMethods()->slot48(self);
}
```

## Proposed learning

None beyond what's already documented.

## Naming

`TimedTask__Deinit` -- tier B. Occupies +0x048 (the mirror of TimedTask__Init): a thin wrapper forwarding to `GetIntermediateBaseMethods()->slot48(self)`.

## Track 4

2026-09-25, round 84 (bravo): class unified in `include/TimedTask.h`. Not renamed. `self` is `TimedTask *`; DayTask__Deinit and ObjM__DetachTarget call it as `GetTimedTaskMethods()->deinit((TimedTask *)self)`. Image byte-identical.
