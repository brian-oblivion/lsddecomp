# TaskObjF__Finalize — MATCH (14/14 words)

> Renamed from `func_8004E40C` on 2026-09-24 (tools/rename.py). Address 0x8004e40c.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__Finalize(Node3bb8cE *self)`. A one-line base-class forward:
fetches the shared `BasicClass`-style vtable via the no-argument getter
`Get_vtable_BasicClass()` (same base-class framework as `include/code_8220.h`'s
`BasicClassMethods`, this unit's own independent local view named
`BaseMethods3bb8cE`) and calls its `finalize` slot (+0x00C) on `self`.
Almost certainly this class's own destructor/finalize override, chaining
to the base.

## Result

Matched on the first attempt.

```c
void TaskObjF__Finalize(Node3bb8cE *self)
{
    Get_vtable_BasicClass()->finalize(self);
}
```

### Proposed learning

None new — matches the already-established "base ctor/dtor chaining via a
separately-fetched vtable getter" shape.
