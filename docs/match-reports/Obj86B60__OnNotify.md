> Renamed from `func_8003E030` on 2026-09-19 (tools/rename.py). Address 0x8003e030.

# Obj86B60__OnNotify — MATCH (52/52 words)

**Unit:** code_2cc8c_c · **Size:** 52 instructions

## What it does

`D_8006E878+0x038` (the "IntermediateBase" table): forwards to the
BasicClass-level slot38 (`BasicClass__OnNotify`, `code_8220_b`, signature
`(BasicClass *self, void *arg1, s32 arg2)` per `include/code_8220.h`), then
reads `arg1->target->header & 0xF` and dispatches to one of
`self->methods->slot54/58/5C` (all three called with `(self, arg1, arg2)`)
for header values 1/2/5 respectively; any other value is a no-op.

`D_80086B60`'s own `+0x038` is this same function (verbatim inherit, no
override) -- confirmed with `tools/classtable.py D_80086B60`, which is also
how `slot54`/`slot58`/`slot5C`'s occupants (`Obj86B60__OnTag1Notify`,
`func_8003C48C`, `func_8003C51C`) were identified.

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
  (`Obj86B60__Deinit`, `Obj86B60__Init`), `slot4C` (external `func_8003C238`,
  `Obj86B60__Init`), `slot54` (`Obj86B60__OnTag1Notify`), `slot58` (external
  `func_8003C48C`, STALL in unit `code_2cc8c` -- its own report confirms
  signature `(Obj86B60 *, s32 a1, s32 a2)`; this call site's `a1` is
  genuinely `EventArg *`, an independent local view of the same shared
  slot, not a conflict), `slot5C` (`func_8003C51C`, already matched
  elsewhere in this unit with `a1` typed `s32` -- same slot, same "each
  call site keeps its own local view" convention).
- `BasicClassMethodsCC8C`: added `slot38` (`BasicClass__OnNotify`).

**One classtable dump resolved this function plus six more of this round's
queue in one pass** -- `D_8006E878` (`tools/classtable.py D_8006E878`) is
literally this unit's queue: `+0x008 IntermediateBase__IntermediateBase`, `+0x038 Obj86B60__OnNotify`,
`+0x040 Obj86B60__ResetCounters` (already matched), `+0x044 Obj86B60__Init`,
`+0x048 Obj86B60__Deinit`, `+0x054 Obj86B60__OnTag1Notify`, `+0x05C func_8003E4A4`
(already matched), `+0x060 func_8003E4B8`, `+0x064 func_8003E538` (already
matched), `+0x068 func_8003E578`. Every one of this round's 12 fresh
functions except `func_8003E5D8`/`func_8003E628`/`func_8003E6CC`/
`func_8003E770`/`func_8003E7F4` (a SECOND, unrelated shared table,
`D_8006E8E4`, see `func_8003E5D8.md`) is a slot of this one table.

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
