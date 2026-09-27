# TaskCore__GetActiveSlotCount — MATCH (7/7 words)

> Renamed from `Obj86B60__GetActiveSlotCount` on 2026-09-25 (tools/rename.py). Address 0x8003dfa0.

> Renamed from `func_8003DFA0` on 2026-09-19 (tools/rename.py). Address 0x8003dfa0.

**Unit:** code_2cc8c_c · **Size:** 7 instructions

## What it does

`Obj86B60Methods::slot120` (verified by reading the raw table bytes at
`gTitleMenuMethods+0x120` in `disk/SLPS_015.56` directly, and cross-checked with
`tools/classtable.py gTitleMenuMethods`). Returns the current ring-buffer slot's
running count: `self->unk60[self->unk58]`.

## The C

```c
s32 TaskCore__GetActiveSlotCount(Obj86B60 *self)
{
    return self->unk60[self->unk58];
}
```

## Struct knowledge established / corrected

`include/code_2cc8c.h`'s `Obj86B60Methods` already had a `slot118` field
whose comment attributed it to `TaskCore__GetActiveSlotCount` (citing `TaskCore__OnPadPrev.md`'s
"Struct knowledge established" section as the source). **That attribution
was wrong.** Reading the retail table bytes directly:

```
gTitleMenuMethods+0x118 = 0x8003DE30  (TaskCore__RetreatSlotCursor, NOT TaskCore__GetActiveSlotCount)
gTitleMenuMethods+0x120 = 0x8003DFA0  (TaskCore__GetActiveSlotCount's real slot)
```

The byte OFFSET `TaskCore__OnPadPrev` compiled against (0x118) was and is
correct -- that function still matches -- but the function pointer VALUE
stored there at runtime is `TaskCore__RetreatSlotCursor`, not `TaskCore__GetActiveSlotCount`. The old
report's error: a discarded/void-typed call site is not evidence of which
function occupies a slot, only of the slot's own signature. I corrected
`slot118`'s comment and added the (previously missing) `slot120` field for
`TaskCore__GetActiveSlotCount` itself, both in `include/code_2cc8c.h`. See this unit's
final summary for the flagged existing-declaration change.

### Proposed learning

When a match report's "struct knowledge established" section names a
function as a slot's occupant, that's a claim about the DATA (the pointer
value at that table offset), not just about the compiled OFFSET -- verify
it independently (`tools/classtable.py <table>` or read the raw bytes)
before trusting it for a DIFFERENT function's signature/attribution,
especially when, as here, the discarding call site gave no signal either
way.

## Provenance

round 12 (2026-09-03), runner alpha, unit code_2cc8c_c. Matched on the
first build.

## Naming

**Obj86B60__GetActiveSlotCount** (renamed from `func_8003DFA0`, round 55, runner alpha).
Tier A: pure leaf getter (a pure leaf whose mechanics ARE its purpose,
CLAUDE.md/track 3's own definition) -- returns `self->unk60[self->unk58]`,
the running count for the currently-active ring-buffer slot (`unk60` is an
array indexed by the `unk58` slot index, established across
`Obj86B60__GetActiveSlotCount`/TaskCore__AdvanceSlotCursor/TaskCore__RetreatSlotCursor, code_2cc8c_b.c). No purpose beyond
the getter itself is claimed.

## Proposed field names

- `Obj86B60::unk58` -> `activeSlot` (tier B). The index into
  `unk4C->unk24[]`/`unk64[]`/`unk60[]`/`unk5C[]`, i.e. which ring-buffer
  slot is currently selected -- established across `TaskCore__Tick`,
  `TaskCore__FindNextFreeSlot`, `TaskCore__BroadcastToSlots`, `TaskCore__ReleaseSlotElements` (code_2cc8c.c/_b.c) and
  this function. NOT renamed directly: heavily shared with `code_2cc8c.c`
  and `code_2cc8c_b.c` (the same class, split by address range). Head
  applies by type scope.
- `Obj86B60::unk60` -> `slotCounts` (tier B). A per-slot running count
  array, indexed by `unk58`/`activeSlot` above, incremented by
  `TaskCore__AdvanceSlotCursor` and decremented by `TaskCore__RetreatSlotCursor` (both code_2cc8c_b.c)
  -- this function is the plain getter for the active slot's own entry.
  NOT renamed directly: shared with `code_2cc8c_b.c`. Head applies by type
  scope.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__GetActiveSlotCount (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Unit banner history (moved here round 98, echo, track 7)

This is the first function of `src/code_2cc8c_c.c`, so the unit banner's
project history lives here. Track 7 rewrote the banner to say only what the
file holds; the banner it replaced, verbatim:

```c
/* code_2cc8c_c -- third slice of the 0x2CC8C block (0x8003DFA0..0x8003E874,
 * 19 functions plus one stall), continuing directly from code_2cc8c_b.
 *
 * After TaskCore's TaskCore__GetActiveSlotCount and the two getters come
 * IntermediateBase's own methods, the ctor through OnState3 and its getter
 * Get_vtable_IntermediateBase (class id 0x30, gIntermediateBaseMethods, the
 * parent of TaskCore and TimedTask). The class is declared once, in
 * include/IntermediateBase.h, whose banner says what it does (track 4,
 * round 82); these functions take `IntermediateBase *self`.
 *
 * The remaining functions are Viewport's (class id 0x7, gViewportMethods,
 * include/Viewport.h; track 4, round 85): New_Viewport, the ctor, finalize
 * and the addChild/removeChild/removeAllChildren overrides, which cache a
 * child by its class-id nibble. code_2cc8c_d.c holds the rest of its table. `Get_vtable_TaskCore`/`GetDefaultStreamTaskInitData`
 * are plain accessors for tables SHARED far more widely (code_2c054.c,
 * class_3bb8c_t.c) that simply happen to live in this unit's address range.
 *
 * Round 55 (runner alpha): full track-3 naming pass. Every definition named;
 * see each function's own match report for the `## Naming` evidence.
 * Unk18Obj (now Viewport, include/Viewport.h) and until round 84 the
 * TaskCore view Obj86B60 (now include/TaskCore.h) were
 * SHARED with one or more of code_2cc8c.c, code_2cc8c_b.c and
 * code_2cc8c_d.c (same classes, split by address range across sibling
 * units), so most field/slot renames on those particular structs are
 * PROPOSALS in this round's report, not direct edits -- only the
 * fields/slots this unit's own functions touch AND no sibling reaches were
 * renamed here.
 */
```

Two things in it no longer hold: the unit has 20 definitions and no stall
(IntermediateBase__SetState matched in round 72), and IntermediateBase's
+0x010/+0x014 fields are still `unk10`/`unk14` because six other units
access them (round 98's proposal: `frameClock`, `lightRig`).
