# StreamTaskObj__func_8003BDAC

> Renamed from `func_8003BDAC` on 2026-09-23 (tools/rename.py). Address 0x8003bdac.

**Unit:** code_2c054 · **Size:** 14 instructions (0x38 bytes) · **Status:** MATCHED (14/14 words, whole-image SHA1 green), first attempt

## What it does

Identical shape to `StreamTaskObj__func_8003BD74` (see that report for the full
derivation and the return-type discussion) one slot over: forwards to
`Get_vtable_TaskCore()`'s (i.e. `gTaskCoreMethods`'s / `LoaderTaskMethods`'s) slot
`+0x084` instead of `+0x080`. Occupies `D_8006E5F8` slot `+0x084` itself.

## Derivation

```c
void StreamTaskObj__func_8003BDAC(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot84(self);
}
```

Matched first attempt, same reasoning as `StreamTaskObj__func_8003BD74`.

## New struct/header knowledge

Added `TaskCoreMethods::slot84` alongside `slot80` in
`include/code_2c054.h` (see `StreamTaskObj__func_8003BD74`'s report).

## Proposed learning

None beyond `StreamTaskObj__func_8003BD74`'s -- same shape, same open return-type flag.
