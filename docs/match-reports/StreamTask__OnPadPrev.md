# StreamTask__OnPadPrev

> Renamed from `StreamTaskObj__func_8003BD74` on 2026-09-26 (tools/rename.py). Address 0x8003bd74.

> Renamed from `func_8003BD74` on 2026-09-23 (tools/rename.py). Address 0x8003bd74.

**Unit:** code_2c054 · **Size:** 14 instructions (0x38 bytes) · **Status:** MATCHED (14/14 words, whole-image SHA1 green), first attempt

## What it does

A one-line forwarder: fetches the sibling class's method table via
`Get_vtable_TaskCore()` (returns `&gTaskCoreMethods`, `GameApplication.h`'s `LoaderTaskMethods`
-- see `Get_vtable_StreamTask`'s report for how the delegation between the two
sibling classes was established) and calls its slot `+0x080`, passing
`self` straight through. Occupies `gStreamTaskMethods` slot `+0x080` itself.

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
void StreamTask__OnPadPrev(StreamTaskObj *self) {
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
`GameApplication.h`'s `StreamTaskMethods`) is void. No caller of `+0x080`
specifically was found to confirm either way -- flagged for whoever adds one.

## New struct/header knowledge

Added `include/code_2c054.h`'s `TaskCoreMethods` (this unit's own local view
of `gTaskCoreMethods`, independent of `GameApplication.h`'s `LoaderTaskMethods`, same
precedent as `Get_vtable_StreamTask`'s report) with slot `+0x080` typed
`void (*)(StreamTaskObj *self)`.

## Proposed learning

See `StreamTask__OnPadNext`'s report (same shape, slot `+0x084`) and
`Get_vtable_StreamTask`'s (the delegation pattern itself).

## Naming

**StreamTask__OnPadPrev** -- tier C. Occupies `gStreamTaskMethods`
slot `+0x080`; a pure one-line up-call to the base slot with no
StreamTaskObj-specific logic at all (see the report's own return-type
discussion). There is nothing here to name beyond "this class's own
override of that slot," so left `Class__func_xxxxx`.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/TaskCore.h, track 4 round 84) now go through `Get_vtable_TaskCore()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Was StreamTaskObj__func_8003BD74. Occupies +0x080 onPadPrev and only up-calls TaskCore's.
