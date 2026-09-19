# func_8003BDAC

**Unit:** code_2c054 · **Size:** 14 instructions (0x38 bytes) · **Status:** MATCHED (14/14 words, whole-image SHA1 green), first attempt

## What it does

Identical shape to `func_8003BD74` (see that report for the full
derivation and the return-type discussion) one slot over: forwards to
`Get_vtable_TaskCore()`'s (i.e. `D_8006E730`'s / `LoaderTaskMethods`'s) slot
`+0x084` instead of `+0x080`. Occupies `D_8006E5F8` slot `+0x084` itself.

## Derivation

```c
void func_8003BDAC(StreamTaskObj *self) {
    Get_vtable_TaskCore()->slot84(self);
}
```

Matched first attempt, same reasoning as `func_8003BD74`.

## New struct/header knowledge

Added `TaskCoreMethods::slot84` alongside `slot80` in
`include/code_2c054.h` (see `func_8003BD74`'s report).

## Proposed learning

None beyond `func_8003BD74`'s -- same shape, same open return-type flag.
