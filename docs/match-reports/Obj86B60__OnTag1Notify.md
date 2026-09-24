# Obj86B60__OnTag1Notify — MATCH (35/35 words)

> Renamed from `func_8003E418` on 2026-09-19 (tools/rename.py). Address 0x8003e418.

**Unit:** code_2cc8c_c · **Size:** 35 instructions

## What it does

`gIntermediateBaseMethods+0x054` (and `gClass86B60Methods`'s own verbatim-inherited `+0x054`):
`self->methods->slot54`'s occupant, and one of `Obj86B60__OnNotify`'s own
3-way `arg1->target->header` dispatch targets. `arg1` is entirely unused in
the body -- only `arg2` is read. Gated on `arg2 == 2`: dispatches `slot44`
on `self->unk10` (reinterpreted as a pointer, same alternate reading
`Obj86B60__Deinit` established), then fetches `self->unkC->unk4` (the
`Obj86B60InitArgs` field `Obj86B60__Init`/`Obj86B60__Deinit` only ever forward
opaquely) and dispatches its OWN `slot44` and `slot48` on it -- the first
function in this unit to dereference that field as a real class instance
rather than an opaque child pointer.

## The C

```c
void Obj86B60__OnTag1Notify(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    Unk4ArgObj *obj4;

    if (arg2 == 2) {
        ((Unk10Obj *)self->unk10)->methods->slot44((Unk10Obj *)self->unk10);
        obj4 = ((Obj86B60InitArgs *)self->unkC)->unk4;
        obj4->methods->slot44(obj4);
        obj4->methods->slot48(obj4);
    }
}
```

Matched on the first build.

## Struct/table knowledge established

- `Unk10ObjMethods`: added `slot44`.
- New type `Unk4ArgObj`/`Unk4ArgObjMethods` (slots `slot44`/`slot48`) --
  `Obj86B60InitArgs->unk4`'s real pointee type. Retyped that field from
  generic `void *` to `Unk4ArgObj *` (an implicit-conversion-to-`void*`
  call site in the already-matched `Obj86B60__Init` is unaffected -- same
  register, same bytes, per this project's established "retyping a field
  to something more specific doesn't reopen an already-matched caller"
  precedent, confirmed unaffected by the unchanged whole-image SHA1).

### Proposed learning

`arg1` (the `EventArg *` `Obj86B60__OnNotify` forwards to `slot54`/`58`/`5C`)
went completely unused here -- a live-but-unconsumed register at this call
site, same shape as `Obj86B60__OnTag5Notify`'s own `$a1` note in
`include/code_2cc8c.h`. Declaring the parameter with its full established
type (`EventArg *`, matching the vtable field) rather than degrading it to
`s32`/`void *` costs nothing and keeps the signature consistent with its
two siblings (`slot58`, `slot5C`) for whoever reads this table's occupants
side by side.

## Head-broadcast levers (round 13): applicability check

- **Lever 1 (`~x + 1` vs `-x`):** does not apply -- no arithmetic at all in
  this function, only dispatch.
- **Lever 2 (dual-based-type array walkers):** does not apply -- no array
  walk.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**Obj86B60__OnTag1Notify** (renamed from `func_8003E418`, round 55, runner
alpha). Tier B: confirmed to be `Obj86B60Methods::onTag1Notify` (`+0x054`,
exclusive to this unit, renamed from `slot54`), the handler
`Obj86B60__OnNotify` dispatches to when the incoming `EventArg`'s target
class tag is 1 -- the "Tag1" in the name records that dispatch condition,
which is directly observed in the caller, rather than a guessed purpose.
On event code 2 it finalizes `self->unk10` and the `initArgs->unk4` helper
object; what event code 2 represents in the game is not established
(tier B).
