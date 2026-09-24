# Obj86B60__OnNotify — MATCH (52/52 words)

> Renamed from `func_8003E030` on 2026-09-19 (tools/rename.py). Address 0x8003e030.

**Unit:** code_2cc8c_c · **Size:** 52 instructions

## What it does

`gIntermediateBaseMethods+0x038` (the "IntermediateBase" table): forwards to the
BasicClass-level slot38 (`BasicClass__OnNotify`, `code_8220_b`, signature
`(BasicClass *self, void *arg1, s32 arg2)` per `include/code_8220.h`), then
reads `arg1->target->header & 0xF` and dispatches to one of
`self->methods->slot54/58/5C` (all three called with `(self, arg1, arg2)`)
for header values 1/2/5 respectively; any other value is a no-op.

`gClass86B60Methods`'s own `+0x038` is this same function (verbatim inherit, no
override) -- confirmed with `tools/classtable.py gClass86B60Methods`, which is also
how `slot54`/`slot58`/`slot5C`'s occupants (`Obj86B60__OnTag1Notify`,
`Obj86B60__OnTag2Notify`, `Obj86B60__OnTag5Notify`) were identified.

## The C

```c
void Obj86B60__OnNotify(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    s32 header;

    Get_vtable_BasicClass()->slot38(self, arg1, arg2);
    header = arg1->target->header & 0xF;
    if (header == 1) {
        self->methods->slot54(self, arg1, arg2);
    } else if (header == 2) {
        self->methods->slot58(self, arg1, arg2);
    } else if (header == 5) {
        self->methods->slot5C(self, arg1, arg2);
    }
}
```

## Struct/table knowledge established

- `EventArg`/`HeaderObj`: this unit's own local view of the same two-type
  shape `include/class_39e08.h` independently derived (`arg1->target->header`).
  Only the one field each touches is modelled.
- `Obj86B60Methods`: added `slot10` (BasicClass addChild, inherited,
  `Obj86B60__Init`), `slot40` (`Obj86B60__ResetCounters`, `IntermediateBase__IntermediateBase`), `slot48`
  (`Obj86B60__Deinit`, `Obj86B60__Init`), `slot4C` (external `TaskCoreObj__func_8003C238`,
  `Obj86B60__Init`), `slot54` (`Obj86B60__OnTag1Notify`), `slot58` (external
  `Obj86B60__OnTag2Notify`, STALL in unit `code_2cc8c` -- its own report confirms
  signature `(Obj86B60 *, s32 a1, s32 a2)`; this call site's `a1` is
  genuinely `EventArg *`, an independent local view of the same shared
  slot, not a conflict), `slot5C` (`Obj86B60__OnTag5Notify`, already matched
  elsewhere in this unit with `a1` typed `s32` -- same slot, same "each
  call site keeps its own local view" convention).
- `BasicClassMethodsCC8C`: added `slot38` (`BasicClass__OnNotify`).

**One classtable dump resolved this function plus six more of this round's
queue in one pass** -- `gIntermediateBaseMethods` (`tools/classtable.py gIntermediateBaseMethods`) is
literally this unit's queue: `+0x008 IntermediateBase__IntermediateBase`, `+0x038 Obj86B60__OnNotify`,
`+0x040 Obj86B60__ResetCounters` (already matched), `+0x044 Obj86B60__Init`,
`+0x048 Obj86B60__Deinit`, `+0x054 Obj86B60__OnTag1Notify`, `+0x05C Obj86B60__IncrementFrameCounter`
(already matched), `+0x060 Obj86B60__NotifyParents`, `+0x064 Obj86B60__NotifyTargetReset` (already
matched), `+0x068 Obj86B60__NotifyChildReset`. Every one of this round's 12 fresh
functions except `New_Unk18Obj`/`Unk18Obj__Unk18Obj`/`Unk18Obj__Finalize`/
`Unk18Obj__AddChild`/`Unk18Obj__RemoveChild` (a SECOND, unrelated shared table,
`D_8006E8E4`, see `New_Unk18Obj.md`) is a slot of this one table.

### Proposed learning

When a unit's queue functions are all small and share a ROM range, run
`tools/classtable.py <addr>` against every nearby vtable found via
`grep -rn func_NAME asm/data/*.s` (or a sibling unit's own header comments
citing the same table, as `code_2c054.h`'s `TaskUtilMethods` did here) before
reading any single function's disassembly in isolation. The table dump
resolves not just the CURRENT function's identity but its callers' and
callees' argument SHAPES (arity, and often a concrete non-`s32` type) for
free -- `slot54`/`slot58`/`slot5C`'s exact signatures came from other units'
own match reports for the SAME shared slots, not from re-deriving them from
this function's own bytes.

## Provenance

round 13 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the first
build.

## Naming

**Obj86B60__OnNotify** (renamed from `func_8003E030`, round 55, runner
alpha). Tier A: forwards to `Get_vtable_BasicClass()->slot38` first (that
slot IS `BasicClass__OnNotify` per `include/code_2cc8c.h`'s own
`BasicClassMethodsCC8C` struct, offset-verified against
`include/code_8220.h`'s canonical, already-named `BasicClassMethods::slot38`
= `onNotify`), then adds its own dispatch on the incoming `EventArg`'s
dynamic class tag -- the textbook "override calls base first, then does its
own work" shape for a virtual method whose base identity is independently
confirmed. `Get_vtable_BasicClass()->slot38` is PROPOSED for rename to
`onNotify` in this unit's `## Proposed field names` (shared with
`code_2cc8c_d.c`'s own `Unk18Obj__OnNotify`).

## Proposed field names

- `BasicClassMethodsCC8C::slot38` -> `onNotify` (tier A). Offset `+0x038`
  matches `include/code_8220.h`'s own canonical, already-named
  `BasicClassMethods::slot38` = `onNotify` exactly (`IS BasicClass__OnNotify`,
  code_8220_b, per this header's own comment). NOT renamed directly:
  `code_2cc8c_d.c`'s `Unk18Obj__OnNotify` also calls
  `Get_vtable_BasicClass()->slot38(self, arg1, arg2)`, so this field is
  shared within the code_2cc8c family. Head applies by type scope (rename
  the field in `BasicClassMethodsCC8C`'s own definition,
  `include/code_2cc8c.h`, rebuild, fix the compiler-listed accessors in
  both `code_2cc8c_c.c` and `code_2cc8c_d.c`, oracle).
