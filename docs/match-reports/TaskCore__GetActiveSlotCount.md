# TaskCore__GetActiveSlotCount — MATCH (7/7 words)

> Renamed from `Obj86B60__GetActiveSlotCount` on 2026-09-25 (tools/rename.py). Address 0x8003dfa0.

> Renamed from `func_8003DFA0` on 2026-09-19 (tools/rename.py). Address 0x8003dfa0.

**Unit:** code_2cc8c_c · **Size:** 7 instructions

## What it does

`Obj86B60Methods::slot120` (verified by reading the raw table bytes at
`gClass86B60Methods+0x120` in `disk/SLPS_015.56` directly, and cross-checked with
`tools/classtable.py gClass86B60Methods`). Returns the current ring-buffer slot's
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
gClass86B60Methods+0x118 = 0x8003DE30  (TaskCore__RetreatSlotCursor, NOT TaskCore__GetActiveSlotCount)
gClass86B60Methods+0x120 = 0x8003DFA0  (TaskCore__GetActiveSlotCount's real slot)
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

**TaskCore__GetActiveSlotCount** (renamed from `func_8003DFA0`, round 55, runner alpha).
Tier A: pure leaf getter (a pure leaf whose mechanics ARE its purpose,
CLAUDE.md/track 3's own definition) -- returns `self->unk60[self->unk58]`,
the running count for the currently-active ring-buffer slot (`unk60` is an
array indexed by the `unk58` slot index, established across
`TaskCore__GetActiveSlotCount`/TaskCore__AdvanceSlotCursor/TaskCore__RetreatSlotCursor, code_2cc8c_b.c). No purpose beyond
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
