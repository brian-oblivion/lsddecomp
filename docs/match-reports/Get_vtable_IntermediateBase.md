> Renamed from `func_8003E5C8` on 2026-09-19 (tools/rename.py). Address 0x8003e5c8.

# Get_vtable_IntermediateBase — MATCH (4/4 words)

**Unit:** code_2cc8c_c · **Size:** 4 instructions

## What it does

A tiny getter: returns `&gIntermediateBaseMethods`, the "IntermediateBase" shared utility
class's own table. Already declared as an `extern` in
`include/code_2cc8c.h` (added by whoever carved the original `code_2cc8c.c`
unit, back when this function itself was still raw asm in the
then-uncarved remainder of the segment) -- this round carves it into
`code_2cc8c_c.c`, so this is simply that existing prototype's definition.
Same static table as `code_2c054.h`'s `TaskUtilMethods` and
`class_39e08.h`'s own `IntermediateBaseMethods` views (each unit keeps its
own independent local view per this project's established convention).

## The C

```c
IntermediateBaseMethods *Get_vtable_IntermediateBase(void)
{
    return &gIntermediateBaseMethods;
}
```

## Header note

`include/code_2cc8c.h` already declared
`extern IntermediateBaseMethods *Get_vtable_IntermediateBase(void);` with a comment
saying "still raw asm elsewhere in the still-uncarved code_2cc8c_b portion
of this segment -- not this unit's function to write". That comment is now
stale (this round carved it into `code_2cc8c_c.c`) and was updated in place
-- flagged in this unit's final summary as a change to an existing
declaration's comment (not its type/signature).

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**Get_vtable_IntermediateBase** (renamed from `func_8003E5C8`, round 55,
runner alpha). Tier A: pure leaf getter, returns `&gIntermediateBaseMethods`
(formerly `D_8006E878`), matching the `Get_vtable_BasicClass` naming
precedent already established in this project for this exact shape. Called
from `code_2c054.c`, `class_39e08.c` and this unit (`code_2cc8c_c.c`) --
confirming, independently of `IntermediateBase__IntermediateBase`'s own evidence, that this
accessor's table is genuinely shared across unrelated class hierarchies.
