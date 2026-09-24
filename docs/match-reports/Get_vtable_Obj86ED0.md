# Get_vtable_Obj86ED0 -- MATCHED (4/4 words)

> Renamed from `func_80051A4C` on 2026-09-24 (tools/rename.py). Address 0x80051a4c.

Unit: `src/class_3bb8c_j.c`. Not `Obj866E8` -- a SEPARATE, much smaller
sibling class discovered this round, `Class86ED0` (local to this unit; see
the file header comment and New_Class86F88_3bb8c_j.md for the class-identity
evidence).

## Body

```c
Class86ED0Methods *Get_vtable_Obj86ED0(void)
{
    return &D_80086ED0;
}
```

Plain address-of getter for `Class86ED0`'s own vtable, same shape as
`GetClass869D8Methods`/`GetClass86AA0Methods`/`GetClass6B5CCMethods` already documented in
`include/class_3bb8c.h`. Matched first try.
