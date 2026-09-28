# TaskObjF__Finalize — MATCH (14/14 words)

> Renamed from `func_8004E40C` on 2026-09-24 (tools/rename.py). Address 0x8004e40c.

**Unit:** title_menu (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__Finalize(Node3bb8cE *self)`. A one-line base-class forward:
fetches the shared `BasicClass`-style vtable via the no-argument getter
`GetBasicClassMethods()` (same base-class framework as `include/code_8220.h`'s
`BasicClassMethods`, this unit's own independent local view named
`BaseMethods3bb8cE`) and calls its `finalize` slot (+0x00C) on `self`.
Almost certainly this class's own destructor/finalize override, chaining
to the base.

## Result

Matched on the first attempt.

```c
void TaskObjF__Finalize(Node3bb8cE *self)
{
    GetBasicClassMethods()->finalize(self);
}
```

### Proposed learning

None new — matches the already-established "base ctor/dtor chaining via a
separately-fetched vtable getter" shape.

## Naming (round 78, track 3)

`func_8004E40C` -> `TaskObjF__Finalize`. **Tier A.** A one-line forward to
`GetBasicClassMethods()->finalize(self)`, sitting at `gTaskObjFMethods`
+0x00C (asm/data/76DC8.data.s) -- the exact offset this unit's own
`BaseMethods3bb8cE` view had already named `finalize`. `TaskObjF__` prefix:
round 78 cross-checked the whole `gTaskObjFMethods` table and confirmed
`Node3bb8cE` (this unit's independent local view) is `TaskObjF`
(include/class_3bb8c.h) -- see src/ui/title_menu.c's unit header comment.
