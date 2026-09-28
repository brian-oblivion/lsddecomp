# GetIntermediateBaseMethods — MATCH (4/4 words)

> Renamed from `Get_vtable_IntermediateBase` on 2026-09-28 (tools/rename.py). Address 0x8003e5c8.

> Renamed from `func_8003E5C8` on 2026-09-19 (tools/rename.py). Address 0x8003e5c8.

**Unit:** task · **Size:** 4 instructions

## What it does

A tiny getter: returns `&gIntermediateBaseMethods`, the "IntermediateBase" shared utility
class's own table. Already declared as an `extern` in
`include/task.h` (added by whoever carved the original `task.c`
unit, back when this function itself was still raw asm in the
then-uncarved remainder of the segment) -- this round carves it into
`task.c`, so this is simply that existing prototype's definition.
Same static table as `task.h`'s `TaskUtilMethods` and
`dream_day.h`'s own `IntermediateBaseMethods` views (each unit keeps its
own independent local view per this project's established convention).

## The C

```c
IntermediateBaseMethods *GetIntermediateBaseMethods(void)
{
    return &gIntermediateBaseMethods;
}
```

## Header note

`include/task.h` already declared
`extern IntermediateBaseMethods *GetIntermediateBaseMethods(void);` with a comment
saying "still raw asm elsewhere in the still-uncarved task portion
of this segment -- not this unit's function to write". That comment is now
stale (this round carved it into `task.c`) and was updated in place
-- flagged in this unit's final summary as a change to an existing
declaration's comment (not its type/signature).

## Provenance

round 12 (2026-09-03), runner alpha, unit task. Matched on the
first build.

## Naming

**GetIntermediateBaseMethods** (renamed from `func_8003E5C8`, round 55,
runner alpha). Tier A: pure leaf getter, returns `&gIntermediateBaseMethods`
(formerly `D_8006E878`), matching the `GetBasicClassMethods` naming
precedent already established in this project for this exact shape. Called
from `task.c`, `dream_day.c` and this unit (`TaskViewport.c`) --
confirming, independently of `IntermediateBase__IntermediateBase`'s own evidence, that this
accessor's table is genuinely shared across unrelated class hierarchies.

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. The getter, returning `IntermediateBaseMethods *`. Its three local declarations (TaskViewport.h, dream_day.h, task.h's TaskUtilMethods) are gone; include/IntermediateBase.h declares it.
