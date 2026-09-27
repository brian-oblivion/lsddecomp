# IntermediateBase__IntermediateBase — MATCH (21/21 words)

> Renamed from `func_8003DFDC` on 2026-09-19 (tools/rename.py). Address 0x8003dfdc.

**Unit:** Task · **Size:** 21 instructions

## What it does

This IS `gIntermediateBaseMethods`'s own +0x008 slot -- the "IntermediateBase" shared
utility class's constructor (`tools/classtable.py gIntermediateBaseMethods` shows
`+0x008 IntermediateBase__IntermediateBase`). Runs the BasicClass ctor through `Get_vtable_BasicClass()`,
installs this class's own vtable (`&gIntermediateBaseMethods`, via the already-matched
getter `Get_vtable_IntermediateBase`), then dispatches its own freshly-installed slot40
(`IntermediateBase__ResetCounters`, already matched, void-returning) once.

Same self-typing convention as this unit's other already-matched siblings
from the same shared table (`IntermediateBase__ResetCounters`, `IntermediateBase__IncrementFrameCounter`, `IntermediateBase__OnState2`):
`Obj86B60 *self`, even though the class is generically shared across many
unrelated tables (`Task.h`'s `TaskUtilMethods` names the same function
`gIntermediateBaseMethods+0x008`, called there as `Get_vtable_IntermediateBase()->slot08(self)` on a
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
  `IntermediateBase__ResetCounters`, already matched elsewhere in this unit.
- The explicit cast `(Obj86B60Methods *)Get_vtable_IntermediateBase()` mirrors
  `src/class_39e08.c`'s own `self->methods = (DayTaskMethods *)
  GetTimedTaskMethods();` -- assigning a shared/generic table getter's return
  into a locally-typed `methods` field is an established idiom in this
  codebase, not a workaround.

## Provenance

round 13 (2026-09-03), runner alpha, unit Task. Matched on the first
build. Classtable dump of `gIntermediateBaseMethods` (26 slots) resolved this and six
sibling queue functions' exact slot identities in one pass; see
`IntermediateBase__OnNotify.md` for the full table.

## Naming

**IntermediateBase__IntermediateBase** (renamed from `func_8003DFDC`, round 55,
runner alpha). Tier A: `include/Task.h` independently documents
`D_8006E878+0x008 = func_8003DFDC` -- i.e. this function IS the `ctor` slot
occupant of the shared "IntermediateBase" ancestor table (named identically,
independently, in `include/TaskViewport.h`, `include/Task.h` and
`include/class_39e08.h`, per this project's own established convention for
that class). Named `Class__Class` per the constructor convention, matching
the already-established `BasicClass__BasicClass` precedent at the
equivalent slot in `BasicClass`'s own table. It is not `Obj86B60`-specific
despite this local view typing `self` as `Obj86B60 *` -- it is carved here
only because its address (0x8003DFDC) falls in this unit's window; the
function itself is the shared ancestor's own ctor, called from at least
three unrelated class hierarchies (`Task.c`, `class_39e08.c`, and this
unit).

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. The ctor (+0x008). `self->methods = Get_vtable_IntermediateBase()` needs no cast now.

## Unit banner history (round 98, echo, track 7)

The unit banner's history paragraphs (address range, the round 55 naming
pass, the round 82/85 track-4 unification of IntermediateBase) moved to
`TaskCore__GetActiveSlotCount.md`, the unit's first function.
