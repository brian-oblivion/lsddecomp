# IntermediateBase__OnNotify — MATCH (52/52 words)

> Renamed from `Obj86B60__OnNotify` on 2026-09-25 (tools/rename.py). Address 0x8003e030.

> Renamed from `func_8003E030` on 2026-09-19 (tools/rename.py). Address 0x8003e030.

**Unit:** Task · **Size:** 52 instructions

## What it does

`gIntermediateBaseMethods+0x038` (the "IntermediateBase" table): forwards to the
BasicClass-level slot38 (`BasicClass__OnNotify`, `TmdRenderer`, signature
`(BasicClass *self, void *arg1, s32 arg2)` per `include/code_8220.h`), then
reads `arg1->target->header & 0xF` and dispatches to one of
`self->methods->slot54/58/5C` (all three called with `(self, arg1, arg2)`)
for header values 1/2/5 respectively; any other value is a no-op.

`gTitleMenuMethods`'s own `+0x038` is this same function (verbatim inherit, no
override) -- confirmed with `tools/classtable.py gTitleMenuMethods`, which is also
how `slot54`/`slot58`/`slot5C`'s occupants (`IntermediateBase__OnTag1Notify`,
`TaskCore__OnPadEvent`, `TaskCore__Update`) were identified.

## The C

```c
void IntermediateBase__OnNotify(Obj86B60 *self, EventArg *arg1, s32 arg2)
{
    s32 header;

    GetBasicClassMethods()->slot38(self, arg1, arg2);
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
  shape `include/DayTaskStageMap.h` independently derived (`arg1->target->header`).
  Only the one field each touches is modelled.
- `Obj86B60Methods`: added `slot10` (BasicClass addChild, inherited,
  `IntermediateBase__Init`), `slot40` (`IntermediateBase__ResetCounters`, `IntermediateBase__IntermediateBase`), `slot48`
  (`IntermediateBase__Deinit`, `IntermediateBase__Init`), `slot4C` (external `TaskCore__OnInit`,
  `IntermediateBase__Init`), `slot54` (`IntermediateBase__OnTag1Notify`), `slot58` (external
  `TaskCore__OnPadEvent`, STALL in unit `Task` -- its own report confirms
  signature `(Obj86B60 *, s32 a1, s32 a2)`; this call site's `a1` is
  genuinely `EventArg *`, an independent local view of the same shared
  slot, not a conflict), `slot5C` (`TaskCore__Update`, already matched
  elsewhere in this unit with `a1` typed `s32` -- same slot, same "each
  call site keeps its own local view" convention).
- `BasicClassMethodsCC8C`: added `slot38` (`BasicClass__OnNotify`).

**One classtable dump resolved this function plus six more of this round's
queue in one pass** -- `gIntermediateBaseMethods` (`tools/classtable.py gIntermediateBaseMethods`) is
literally this unit's queue: `+0x008 IntermediateBase__IntermediateBase`, `+0x038 IntermediateBase__OnNotify`,
`+0x040 IntermediateBase__ResetCounters` (already matched), `+0x044 IntermediateBase__Init`,
`+0x048 IntermediateBase__Deinit`, `+0x054 IntermediateBase__OnTag1Notify`, `+0x05C IntermediateBase__IncrementFrameCounter`
(already matched), `+0x060 IntermediateBase__SetState`, `+0x064 IntermediateBase__OnState2` (already
matched), `+0x068 IntermediateBase__OnState3`. Every one of this round's 12 fresh
functions except `New_Viewport`/`Viewport__Viewport`/`Viewport__Finalize`/
`Viewport__AddChild`/`Viewport__RemoveChild` (a SECOND, unrelated shared table,
`gViewportMethods`, see `New_Viewport.md`) is a slot of this one table.

### Proposed learning

When a unit's queue functions are all small and share a ROM range, run
`tools/classtable.py <addr>` against every nearby vtable found via
`grep -rn func_NAME asm/data/*.s` (or a sibling unit's own header comments
citing the same table, as `Task.h`'s `TaskUtilMethods` did here) before
reading any single function's disassembly in isolation. The table dump
resolves not just the CURRENT function's identity but its callers' and
callees' argument SHAPES (arity, and often a concrete non-`s32` type) for
free -- `slot54`/`slot58`/`slot5C`'s exact signatures came from other units'
own match reports for the SAME shared slots, not from re-deriving them from
this function's own bytes.

## Provenance

round 13 (2026-09-03), runner alpha, unit Task. Matched on the first
build.

## Naming

**IntermediateBase__OnNotify** (renamed from `func_8003E030`, round 55, runner
alpha). Tier A: forwards to `GetBasicClassMethods()->slot38` first (that
slot IS `BasicClass__OnNotify` per `include/Task.h`'s own
`BasicClassMethodsCC8C` struct, offset-verified against
`include/code_8220.h`'s canonical, already-named `BasicClassMethods::slot38`
= `onNotify`), then adds its own dispatch on the incoming `EventArg`'s
dynamic class tag -- the textbook "override calls base first, then does its
own work" shape for a virtual method whose base identity is independently
confirmed. `GetBasicClassMethods()->slot38` is PROPOSED for rename to
`onNotify` in this unit's `## Proposed field names` (shared with
`Task.c`'s own `Viewport__OnNotify`).

## Proposed field names

- `BasicClassMethodsCC8C::slot38` -> `onNotify` (tier A). Offset `+0x038`
  matches `include/code_8220.h`'s own canonical, already-named
  `BasicClassMethods::slot38` = `onNotify` exactly (`IS BasicClass__OnNotify`,
  TmdRenderer, per this header's own comment). NOT renamed directly:
  `Task.c`'s `Viewport__OnNotify` also calls
  `GetBasicClassMethods()->slot38(self, arg1, arg2)`, so this field is
  shared within the Task family. Head applies by type scope (rename
  the field in `BasicClassMethodsCC8C`'s own definition,
  `include/Task.h`, rebuild, fix the compiler-listed accessors in
  both `Task.c` and `code_2cc8c_d.c`, oracle).

## Track 4 (2026-09-25, round 82, charlie)

The class is IntermediateBase (class id 0x30, gIntermediateBaseMethods; `tools/classtable.py gIntermediateBaseMethods` lists this function as one of its own occupants), declared once in include/IntermediateBase.h. `self` is now `IntermediateBase *`, not TaskCore's `Obj86B60` view; byte-identical. Renamed from Obj86B60__OnNotify (class prefix). Occupies +0x038 (BasicClass's onNotify). The argument called `EventArg *arg1` is the SENDER, a BasicClass: `arg1->target->header` was `sender->methods->header`, the class id word, so the parameter is `BasicClass *sender`. The three cases are the sender's root class nibble (typeviews.py --tree): 1 gDrawSystemMethods -> onTag1Notify (+0x054), 2 gPadMethods -> onPadEvent (+0x058, NULL here), 5 gFrameClockMethods -> update (+0x05C), the same split SceneNode's onNotify makes (include/SceneNode.h names its tag-2/5 slots onPadEvent/update).

## Track 7 (round 98, echo)

Local `header` -> `rootClass`: the sender's table word +0x000 masked to
its lowest nibble, `CLASS_ID_ROOT_MASK` (0xF, new in BasicClass.h). The
cases are `DRAWSYSTEM_CLASS_ID` (1, gDrawSystemMethods), `PAD_CLASS_ID`
(2, gPadMethods) and `FRAMECLOCK_CLASS_ID` (5, gFrameClockMethods), each
in its class's header (`plan.py classes` lists those ids). Byte-identical.
