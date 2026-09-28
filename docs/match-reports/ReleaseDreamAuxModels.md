# ReleaseDreamAuxModels -- MATCHED

> Renamed from `TickDreamAuxSlots` on 2026-09-27 (tools/rename.py). Address 0x8005c5e8.

> Renamed from `func_8005C5E8` on 2026-09-21 (tools/rename.py). Address 0x8005c5e8.

Unit `DreamAux` (was `code_4cd08`). 26/26 words, `0x4CDE8`-`0x4CE50`. Whole-image
`build-and-verify.sh` green (`build exit=0`, sha1 OK). First attempt matched.

## What it does

Ticks slot 0 of the `gDreamAuxSlots` object-slot family exactly once: if the slot
holds a live object, call that object's method-table slot 1 (offset `0x4`)
with the object itself as the argument, and overwrite the slot with whatever
the call returns.

```c
void ReleaseDreamAuxModels(void)
{
    DreamAuxSlot *slot = gDreamAuxSlots;
    u32 done;

    for (done = 0; done < 1; done++) {
        DreamAuxObj *obj = slot->obj;

        if (obj != NULL) {
            DreamAuxTickFn tick = (DreamAuxTickFn)obj->vtable[1];
            slot->obj = tick(obj);
        }
        slot++;
    }
}
```

Uses `DreamAuxObj`, `DreamAuxTickFn`, `DreamAuxSlot`, `gDreamAuxSlots` from the new
`include/DreamAux.h` (see below).

## Why the "loop that only runs once" shape

`for (done = 0; done < 1; done++)` is not decorative. GCC 2.6.3 at `-O2`
special-cases a for-loop whose trip count is a compile-time-provable positive
constant: it skips the leading guard (the body starts directly at the
loop-back label) AND, because `done` only ever takes the value `0` on entry
to the exit test, folds the `done < 1` check into `beqz $s1` rather than
emitting a `slti`/`sltiu` + separate branch. Retail's asm shows exactly this:
no leading guard before `.L8005C604`, and `beqz $s1, .L8005C604` as the
back-edge test. Do not "clean this up" to a plain `if` -- the loop shape
*is* what produces the byte-exact instruction sequence.

Same shape recurs at `ReleaseDreamAuxEntities` (below, over `gDreamAuxSlots2`) and, per
splat's asm, in `SetDreamAuxWorld` and `InitDreamAux`'s second loop (both
off-limits/gp-relative or already stalled elsewhere in this unit) -- this
looks like a recurring internal idiom for "process slot 0 of a small
object-slot table", not a one-off.

## Struct derivation

`gDreamAuxSlots` stride confirmed as `0x14` (20 bytes) independently by
`SetDreamAuxWorld` (off-limits here, gp-relative-blocked, but read for context):
it writes a *different* field of the same array, at `+0x4`, with a
`New_Entity()` result, while reading `+0x0` as an argument to that same call
-- meaning slot 0's `obj` field (this function's target) is populated
*before* `SetDreamAuxWorld` runs, and `SetDreamAuxWorld`'s own `+0x4` write in turn
feeds something else. Only `+0x0` (`obj`) and the fact that the struct is
`0x14` bytes are established; the other `0x10` bytes are undocumented
padding (`u8 unk4[0x10]`).

The vtable-call shape (`lw v0,0(a0); lw v0,4(v0); jalr v0`) is exactly
CLAUDE.md's "every object's method table pointer lives at offset 0" pattern.
Slot 1 (table offset `0x4`) is a "tick" method taking the object and
returning a (possibly new) object pointer -- not resolved against
`tools/classtable.py` (the object's concrete class/table is unknown from
this function alone), so the type is deliberately generic
(`DreamAuxObj`/`DreamAuxTickFn`), not a named class.

## Register/goto levers checked (per head broadcasts)

- **Early exit returning a different value from the main path (Lever from
  `New_Pad`):** not applicable. This function is `void`, has one `if`
  with no `else` and no early `return` at all.
- **Prologue callee-save order / "let GCC hoist its own loop invariants"
  (Levers from `Pad__DispatchEvents`):** the second lever *is* what's happening
  here structurally -- `slot` is a pointer named directly at the top
  (`DreamAuxSlot *slot = gDreamAuxSlots;`), not a byte offset hand-maintained
  inside the loop, and it matched first try. Consistent with "let the
  compiler own the induction variable."

## Proposed learning

- **"`for (i = 0; i < 1; i++) { ... }` where `i` is otherwise unused is a
  real, recurring source shape in this codebase, not decompiler noise.**
  GCC 2.6.3 -O2 compiles it to a `do`-style loop with a `beqz $reg` back-edge
  test (no leading guard, no `slti`), and removing the "loop" in favor of a
  plain `if` changes the instruction count. Confirmed independently on two
  functions in `DreamAux` (`ReleaseDreamAuxModels`, `ReleaseDreamAuxEntities`); the same
  shape also appears in `SetDreamAuxWorld` and `InitDreamAux`'s tail (both
  otherwise blocked/stalled). Likely a shared "process the first slot of an
  N-slot table" macro/pattern in the original source where N happened to be
  1 at these call sites.

## Naming

**ReleaseDreamAuxModels** — tier A. A pure leaf over `gDreamAuxSlots`: if slot 0's
`obj` is live, call its vtable slot 1 (the unit's own established
`DreamAuxTickFn` typedef, already named "tick" before this round) and store
the result back. The mechanics ARE the name (a tick pass), so this qualifies
as tier A by FINISHING-PLAN's "pure leaf whose mechanics are its purpose"
rule regardless of why the caller (`DayTask__Finalize`, apparently a destructor)
invokes it once at that point.

## Round 100 (alpha): track 7, moved from src/world/DreamAux.c and include/DreamAux.h

## Naming (round 100)

**ReleaseDreamAuxModels** (was TickDreamAuxSlots) -- tier A. The method
table slot it calls, +0x004, is BasicClass's `release` (finalize, free,
return NULL), not a tick, and the slots hold the ModelData InitDreamAux made.
Its only caller is DayTask__Finalize (src/world/DayTaskStageMap.c), right after the
other releases, mirroring DayTask's ctor calling InitDreamAux. The call is
now `model->methods->release(model)` through ModelData's own table;
DreamAuxObj/DreamAuxTickFn are gone. `done` -> `i`, bound
ARRAY_COUNT(gDreamAuxSlots) (1). Byte-identical. The tier-A argument in the
section above was made for the old name and reading.
