> Renamed from `func_8003DFDC` on 2026-09-19 (tools/rename.py). Address 0x8003dfdc.

# IntermediateBase__IntermediateBase — MATCH (21/21 words)

**Unit:** code_2cc8c_c · **Size:** 21 instructions

## What it does

This IS `D_8006E878`'s own +0x008 slot -- the "IntermediateBase" shared
utility class's constructor (`tools/classtable.py D_8006E878` shows
`+0x008 IntermediateBase__IntermediateBase`). Runs the BasicClass ctor through `Get_vtable_BasicClass()`,
installs this class's own vtable (`&D_8006E878`, via the already-matched
getter `Get_vtable_IntermediateBase`), then dispatches its own freshly-installed slot40
(`Obj86B60__ResetCounters`, already matched, void-returning) once.

Same self-typing convention as this unit's other already-matched siblings
from the same shared table (`Obj86B60__ResetCounters`, `Obj86B60__IncrementFrameCounter`, `Obj86B60__NotifyTargetReset`):
`Obj86B60 *self`, even though the class is generically shared across many
unrelated tables (`code_2c054.h`'s `TaskUtilMethods` names the same function
`D_8006E878+0x008`, called there as `Get_vtable_IntermediateBase()->slot08(self)` on a
`StreamTaskObj *self`).

## The C

```c
void IntermediateBase__IntermediateBase(Obj86B60 *self)
{
    Get_vtable_BasicClass()->ctor(self);
    self->methods = (Obj86B60Methods *)Get_vtable_IntermediateBase();
    self->methods->slot40(self);
}
```

## Header notes

- Added `ctor` (+0x008) to this unit's local `BasicClassMethodsCC8C`, typed
  `void (*ctor)(void *self)` from `include/code_8220.h`'s own authoritative
  `BasicClassMethods` (`BasicClass__BasicClass`, confirmed void-returning,
  already matched there).
- Added `slot40` to `Obj86B60Methods` (`Obj86B60 *self`), the slot this
  function calls through after installing its own vtable -- it IS
  `Obj86B60__ResetCounters`, already matched elsewhere in this unit.
- The explicit cast `(Obj86B60Methods *)Get_vtable_IntermediateBase()` mirrors
  `src/class_39e08.c`'s own `self->methods = (Class865C8Methods *)
  func_8004A4B8();` -- assigning a shared/generic table getter's return
  into a locally-typed `methods` field is an established idiom in this
  codebase, not a workaround.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the first
build. Classtable dump of `D_8006E878` (26 slots) resolved this and six
sibling queue functions' exact slot identities in one pass; see
`Obj86B60__OnNotify.md` for the full table.
