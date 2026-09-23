# StreamTaskObj__func_8003BD74

> Renamed from `func_8003BD74` on 2026-09-23 (tools/rename.py). Address 0x8003bd74.

**Unit:** code_2c054 · **Size:** 14 instructions (0x38 bytes) · **Status:** MATCHED (14/14 words, whole-image SHA1 green), first attempt

## What it does

A one-line forwarder: fetches the sibling class's method table via
`Get_vtable_TaskCore()` (returns `&gTaskCoreMethods`, `Class6D3C8.h`'s `LoaderTaskMethods`
-- see `Get_vtable_StreamTaskObj`'s report for how the delegation between the two
sibling classes was established) and calls its slot `+0x080`, passing
`self` straight through. Occupies `gStreamTaskObjMethods` slot `+0x080` itself.

## Derivation

```
addiu $sp, $sp, -0x18
sw    $s0, 0x10($sp)
sw    $ra, 0x14($sp)
jal   Get_vtable_TaskCore
 addu $s0, $a0, $zero        ; self saved across the call
lw    $v0, 0x80($v0)
jalr  $v0
 addu $a0, $s0, $zero        ; self re-passed as arg
...epilogue
```

```c
void StreamTaskObj__func_8003BD74(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot80(self);
}
```

**Return type note (per the runner brief's "one-line wrapper tail-calls a
non-void function" trap):** `self` must survive the call to
`Get_vtable_TaskCore()` (hence the `$s0` save/restore), but nothing after the
inner `jalr` reads or stores `$v0` before the epilogue -- so whatever the
inner call returns is left in `$v0` at exit either way, and typing this
wrapper `void` vs. forwarding a return value compiles to byte-identical
code. Typed `void` here based on the sibling-slot convention: every OTHER
slot of this same table typed so far (`+0x004`/`+0x044`/`+0x06C`/`+0x12C` in
`Class6D3C8.h`'s `StreamTaskMethods`) is void. No caller of `+0x080`
specifically was found to confirm either way -- flagged for whoever adds one.

## New struct/header knowledge

Added `include/code_2c054.h`'s `TaskCoreMethods` (this unit's own local view
of `gTaskCoreMethods`, independent of `Class6D3C8.h`'s `LoaderTaskMethods`, same
precedent as `Get_vtable_StreamTaskObj`'s report) with slot `+0x080` typed
`void (*)(StreamTaskObj *self)`.

## Proposed learning

See `StreamTaskObj__func_8003BDAC`'s report (same shape, slot `+0x084`) and
`Get_vtable_StreamTaskObj`'s (the delegation pattern itself).
