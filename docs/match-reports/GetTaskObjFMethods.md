# GetTaskObjFMethods -- MATCH

> Renamed from `GetClass86E00Methods` on 2026-09-23 (tools/rename.py). Address 0x800507e8.

> Renamed from `func_800507E8` on 2026-09-23 (tools/rename.py). Address 0x800507e8.

Unit `TitleMenuTaskObjF`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py GetTaskObjFMethods`: 4/4 words match.

## Source

```c
GenericCtorTable_3bb8c_d *GetTaskObjFMethods(void)
{
    return &gTaskObjFMethods;
}
```

## Notes

This function was already forward-declared, opaquely, in
`include/class_3bb8c.h` as `GenericCtorTable_3bb8c_d *GetTaskObjFMethods(void)`
while deriving `TitleMenuTaskObjF`'s own `New_TaskObjF` (a `New_X` allocator
that calls `GetTaskObjFMethods()->ctor(...)`) last round -- that declaration
predicted exactly this shape (a trivial vtable-getter, same pattern as
`GetTitleMenuMethods`/`gTitleMenuMethods` and `GetTaskCoreMethods`/`gTaskCoreMethods` elsewhere in
this header) before this function's own body was ever read. Confirmed
correct on the first attempt: the real global is `gTaskObjFMethods`, added here
as `extern GenericCtorTable_3bb8c_d gTaskObjFMethods;` right next to the getter's
own declaration.

First attempt, byte-exact.

## Naming

`GetTaskObjFMethods` (was `func_800507E8`), tier A: a bare
`return &gTaskObjFMethods;` vtable-getter, the exact `GetClass<X>Methods` shape
already used tree-wide for this pattern (`GetTimedTaskMethods`,
`GetNodeGuardedViewportMethods`, `GetSceneNodeMethods`) -- mechanics fully IS the
name for a pure getter.
