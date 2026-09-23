# GetClass86E00Methods -- MATCH

> Renamed from `func_800507E8` on 2026-09-23 (tools/rename.py). Address 0x800507e8.

Unit `class_3bb8c_g`, round 14. `./build-and-verify.sh` exit 0; whole-image
SHA1 matches retail. `funcdiff.py GetClass86E00Methods`: 4/4 words match.

## Source

```c
GenericCtorTable_3bb8c_d *GetClass86E00Methods(void)
{
    return &D_80086DC4;
}
```

## Notes

This function was already forward-declared, opaquely, in
`include/class_3bb8c.h` as `GenericCtorTable_3bb8c_d *GetClass86E00Methods(void)`
while deriving `class_3bb8c_d`'s own `func_8004E2E0` (a `New_X` allocator
that calls `GetClass86E00Methods()->ctor(...)`) last round -- that declaration
predicted exactly this shape (a trivial vtable-getter, same pattern as
`func_8004E2D0`/`gClass86B60Methods` and `Get_vtable_TaskCore`/`gTaskCoreMethods` elsewhere in
this header) before this function's own body was ever read. Confirmed
correct on the first attempt: the real global is `D_80086DC4`, added here
as `extern GenericCtorTable_3bb8c_d D_80086DC4;` right next to the getter's
own declaration.

First attempt, byte-exact.

## Naming

`GetClass86E00Methods` (was `func_800507E8`), tier A: a bare
`return &D_80086DC4;` vtable-getter, the exact `GetClass<X>Methods` shape
already used tree-wide for this pattern (`GetClass86668Methods`,
`GetClass869D8Methods`, `GetClass6B5CCMethods`) -- mechanics fully IS the
name for a pure getter.
