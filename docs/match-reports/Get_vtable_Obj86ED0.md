# Get_vtable_Obj86ED0 -- MATCHED (4/4 words)

> Renamed from `func_80051A4C` on 2026-09-24 (tools/rename.py). Address 0x80051a4c.

Unit: `src/class_3bb8c_j.c`. ROUND 75 CORRECTION: this is NOT
`Class86F88_3bb8c_j`'s own getter (an earlier round assumed so, since it was
the only table getter this unit's C had resolved at the time, and named the
whole sibling class after it -- see `Class86F88__Class86F88.md`
and `include/class_3bb8c.h`'s round-15 HEAD NOTEs on D_80086ED0/D_80086F88
for that history). `tools/classtable.py D_80086ED0` places
`Obj86ED0__AdvanceCountdown` .. `Obj86ED0__DispatchLookupValue` (this same
unit's own first six functions) at that table's +0x094..+0x0A8, and
`include/class_3bb8c.h` already types and shares the WHOLE table as
`Obj86ED0Methods`, established independently by class_3bb8c_i from its own
call sites (`func_80050BA8` there is the actual `New_X` for THIS class,
allocating 0x4C bytes and dispatching its ctor through `->ctor(...)` on the
pointer this function returns). So this function is `Obj86ED0`'s own
table getter, simply DEFINED in this unit; `Class86F88_3bb8c_j`'s real
table is D_80086F88, reached instead through `GetClass86F88Methods()`
(class_3bb8c_k).

## Body

```c
Obj86ED0Methods *Get_vtable_Obj86ED0(void)
{
    return &D_80086ED0;
}
```

Plain address-of getter for `Obj86ED0`'s own vtable, same shape as
`GetClass869D8Methods`/`GetClass86AA0Methods`/`GetClass6B5CCMethods` already documented in
`include/class_3bb8c.h`. Matched first try.

## Naming

- `Get_vtable_Obj86ED0` -- tier A. Plain `return &D_80086ED0;` -- a table-getter's purpose IS its mechanics (a pure leaf returning a fixed vtable pointer), same shape as the project's other `Get_vtable_*`/`GetClass*Methods` accessors. Identity of D_80086ED0 as Obj86ED0's table is classtable.py D_80086ED0 (42 slots) cross-checked against class_3bb8c_i's own already-shared struct.
