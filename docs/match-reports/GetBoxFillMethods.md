# GetBoxFillMethods — MATCHED (4/4 words)

> Renamed from `Obj6EAC0__GetBaseMethods` on 2026-09-25 (tools/rename.py). Address 0x800408bc.

> Renamed from `func_800408BC` on 2026-09-18 (tools/rename.py). Address 0x800408bc.

Unit: `src/code_2cc8c_f.c`. First attempt.

```c
Obj6EAC0Methods *GetBoxFillMethods(void) {
    return &gBoxFillMethods;
}
```

A plain "get the base class table" getter, same shape as
`Get_vtable_IntermediateBase`/`Get_vtable_BasicClass` elsewhere in this project.
`gBoxFillMethods` is the base method table for a previously-unnamed
BasicClass-derived class (see `include/code_2cc8c.h`'s `Obj6EAC0`
comment); this unit's `Obj6EAC0__GetDerivedMethods` is the matching getter for the
override table `D_8006EB90`.

### Proposed learning

None beyond what's already recorded.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `func_800408BC` | `GetBoxFillMethods` | A |

**Evidence.** A pure leaf getter (tier A by definition): returns
`&gBoxFillMethods`, the base method table, with no other logic. Twin of
`Obj6EAC0__GetDerivedMethods` (returns `&D_8006EB90`, the override table);
both are named identically to the project's existing "getter returns a
fixed vtable" precedent (e.g. `Get_vtable_BasicClass`).

## Track 4 (2026-09-25, round 85, charlie)

Class 0x64 (was D_8006EAC0) is unified as BoxFill in include/BoxFill.h: Viewport__DrawNode draws a node whose class-id low byte is 0x64 with GsSortBoxFill over the GsBOXF at +0x058 (pri +0x044, `relative` +0x048, x/y +0x050/+0x054). The body now takes `BoxFill *`; zero bytes changed. Renamed from `Obj6EAC0__GetBaseMethods`: it returns &gBoxFillMethods (ex D_8006EAC0), BoxFill's own table, so it is the class getter (tier A). D_8006EB90 is NOT a BoxFill subclass (id 0x11144, below D_8006EC74/ScreenSprite), so "Base" as opposed to `Obj6EAC0__GetDerivedMethods` was the old reading's.
