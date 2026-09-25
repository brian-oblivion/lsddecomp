# StreamTaskObj__Destroy

> Renamed from `func_8003B9DC` on 2026-09-23 (tools/rename.py). Address 0x8003b9dc.

**Unit:** code_2c054 · **Size:** 23 instructions (0x5C bytes) · **Status:** MATCHED (23/23 words, whole-image SHA1 green), first attempt

## What it does

Two dispatches in a row, both discarding/forwarding through `self`, no other
side effect. First, a genuine virtual call through `self->unkB4`'s own
1-slot vtable (a small object type distinct from `StreamTaskObj`, discovered
here for the first time in this unit); second, the same
`Get_vtable_TaskCore()`-mediated delegation to the sibling class `gTaskCoreMethods`
(`LoaderTaskMethods`) used by `StreamTaskObj__func_8003BD74`/`StreamTaskObj__func_8003BDAC`, this time
slot `+0x00C`. Occupies `gStreamTaskObjMethods` slot `+0x00C` itself.

## Derivation

```
lw   $a0, 0xB4($s0)          ; a0 = self->unkB4
lw   $v0, 0x0($a0)            ; v0 = a0->methods
lw   $v0, 0x4($v0)             ; v0 = methods->slot04
jalr $v0                          ; a0 (still the sub-object) unchanged
jal  Get_vtable_TaskCore
lw   $v0, 0xC($v0)                ; v0 = table->slot0C
jalr $v0
 addu $a0, $s0, zero               ; a0 = self, explicitly reloaded
...epilogue
```

```c
void StreamTaskObj__Destroy(StreamTaskObj *self) {
    self->unkB4->methods->slot04(self->unkB4);
    Get_vtable_TaskCore()->slot0C(self);
}
```

Matched first attempt. The first call's argument register (`$a0`) is never
reloaded between the two loads and the `jalr` -- confirming the call target
is `self->unkB4` itself (a virtual self-call on the sub-object), not `self`.

## New struct/header knowledge

Added `include/code_2c054.h`'s `StreamTaskUnkB4Obj`/`StreamTaskUnkB4Methods`
(a new, previously-unseen 1-slot-vtable object reached through
`StreamTaskObj::unkB4`, `+0x0B4`) and `TaskCoreMethods::slot0C` (this unit's
local view of `gTaskCoreMethods`, see `StreamTaskObj__func_8003BD74`'s report).

## Proposed learning

Same open return-type question as `StreamTaskObj__func_8003BD74`/`StreamTaskObj__func_8003BDAC` for the
tail call through `slot0C` -- typed `void` on the same sibling-slot-
convention basis, unconfirmed by any found caller.

## Naming

**StreamTaskObj__Destroy** -- tier A. Occupies `gStreamTaskObjMethods`'s dtor
slot `+0x00C` (a base-class layout convention independently confirmed in
`include/class_39e08.h`'s own `ctor`/`dtor` pair at `+0x008`/`+0x00C`, and in
`include/code_171e0.h`'s `Class6D430__Finalize`). Tears down the private
`unkB4` sub-object, then up-calls `TaskCore__Finalize` at the same slot --
the "override, do extra work, call the base" shape this whole unit's slot
comparison confirms (see the unit header comment).
