> Renamed from `func_800408BC` on 2026-09-18 (tools/rename.py). Address 0x800408bc.

# Obj6EAC0__GetBaseMethods — MATCHED (4/4 words)

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
Obj6EAC0Methods *Obj6EAC0__GetBaseMethods(void) {
    return &D_8006EAC0;
}
```

A plain "get the base class table" getter, same shape as
`func_8003E5C8`/`Get_vtable_BasicClass` elsewhere in this project.
`D_8006EAC0` is the base method table for a previously-unnamed
BasicClass-derived class (see `include/code_2cc8c.h`'s `Obj6EAC0`
comment); this unit's `Obj6EAC0__GetDerivedMethods` is the matching getter for the
override table `D_8006EB90`.

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800408BC` | `Obj6EAC0__GetBaseMethods` | A |

**Evidence.** A pure leaf getter (tier A by definition): returns
`&D_8006EAC0`, the base method table, with no other logic. Twin of
`Obj6EAC0__GetDerivedMethods` (returns `&D_8006EB90`, the override table);
both are named identically to the project's existing "getter returns a
fixed vtable" precedent (e.g. `Get_vtable_BasicClass`).
