# Unk18Obj__Unk18Obj — MATCH (41/41 words)

> Renamed from `func_8003E628` on 2026-09-19 (tools/rename.py). Address 0x8003e628.

**Unit:** code_2cc8c_c · **Size:** 41 instructions

## What it does

`D_8006E8E4+0x008` -- the ctor of the class `New_Unk18Obj`'s New_X
allocator constructs (`Unk18Obj`, 0xBC bytes). Runs the BasicClass ctor,
installs its own vtable, zeroes two fields (`unkC`/`unk10`), stashes the
return of `New_Class6B5CC` (a `New_Class6B5CC` allocator, already matched
elsewhere as `code_d294.c`) into `unkAC`, constructs a `SubHandleObj` via
`New_Class6E99C` (already known elsewhere as `include/Entity.h`'s own
`Unk100Obj`/`New_Class6E99C`) into `unkB0`, dispatches that object's own
`slot4C` with `(obj, self->unkAC, &D_8008A904)`, then runs its own freshly
installed `slot40`.

## The C

```c
void Unk18Obj__Unk18Obj(Unk18Obj *self)
{
    SubHandleObj *obj;

    Get_vtable_BasicClass()->ctor(self);
    self->methods = GetUnk18ObjMethods();
    self->unkC = 0;
    self->unk10 = 0;
    self->unkAC = New_Class6B5CC();
    obj = New_Class6E99C(D_8008A90C, 0, 0);
    self->unkB0 = obj;
    obj->methods->slot4C(obj, self->unkAC, D_8008A904);
    self->methods->slot40(self);
}
```

Matched on the first build.

## A delay-slot trap worth flagging for the next reader

First read-through of this function's disassembly misattributed
`sw $v0, 0xAC($s0)` (the delay slot of `jal New_Class6E99C`) as storing
`New_Class6E99C`'s OWN return value -- it does not. A `jal`'s delay slot
executes BEFORE the callee runs, using whatever `$v0` held from the
PRECEDING call (`New_Class6B5CC`'s return), not the value about to come
back. Re-reading confirmed: `self->unkAC = New_Class6B5CC()` (stored in the
delay slot of the NEXT call), and `self->unkB0 = New_Class6E99C(...)`'s
actual return (read from `$v0` only AFTER that call, in `addu $a0,$v0,$zero`
one instruction later). Getting this backwards would have produced a
plausible-looking but wrong struct/field derivation with no diagnostic --
worth the explicit callout since nothing in the build would have caught it
before the byte-level score did.

## Struct/table knowledge established

- `Unk18Obj`: added `unkC`, `unk10` (both `s32`, zeroed by the ctor),
  `unkAC` (`void *`, from `New_Class6B5CC`, never dereferenced by this unit),
  `unkB0` (`SubHandleObj *`, from `New_Class6E99C`).
- `Unk18ObjMethods`: added `slot40` -- a DIFFERENT function from
  `Obj86B60Methods::slot40` despite the identical offset; `D_8006E8E4`'s own
  `+0x040` occupant is `Unk18Obj__InitDefaults` (per `tools/classtable.py
  D_8006E8E4`), not `IntermediateBase__ResetCounters`. Two unrelated tables, same offset,
  different occupants -- ordinary vtable-layout coincidence, not evidence
  of a shared ancestor at this slot (contrast with the GENUINELY shared
  slots this unit has documented elsewhere, e.g. `IntermediateBase__OnState2`).
- New type `SubHandleObj`/`SubHandleObjMethods` (`slot4C`) -- this unit's own
  local view of `include/Entity.h`'s `Unk100Obj`/`New_Class6E99C`, per this
  project's independent-local-views convention.
- `New_Class6B5CC`: local view added, returning `void *` (this unit never
  dereferences it) -- `include/code_d294.h`'s own view types it
  `Class6B5CCObj *`, unaffected since it's a separate header.
- `D_8008A90C`/`D_8008A904`: two new address-taken-only globals.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build (after correctly re-reading the delay-slot ordering above).

## Naming

**Unk18Obj__Unk18Obj** (renamed from `func_8003E628`, round 55, runner
alpha). Tier A: the `Class__Class` constructor convention (matching
`BasicClass__BasicClass`) -- confirmed as `Unk18ObjMethods::ctor`'s
occupant (called by `New_Unk18Obj` via `GetUnk18ObjMethods()->ctor(self)`,
`GetUnk18ObjMethods` being this class's own vtable getter, code_2cc8c_d.c).
Chains `Get_vtable_BasicClass()->ctor` first, then installs its own vtable
and sets up `self->unkAC`/`self->unkB0` -- the standard base-then-derived
construction shape.
