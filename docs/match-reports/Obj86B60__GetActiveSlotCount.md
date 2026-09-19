> Renamed from `func_8003DFA0` on 2026-09-19 (tools/rename.py). Address 0x8003dfa0.

# Obj86B60__GetActiveSlotCount — MATCH (7/7 words)

**Unit:** code_2cc8c_c · **Size:** 7 instructions

## What it does

`Obj86B60Methods::slot120` (verified by reading the raw table bytes at
`D_80086B60+0x120` in `disk/SLPS_015.56` directly, and cross-checked with
`tools/classtable.py D_80086B60`). Returns the current ring-buffer slot's
running count: `self->unk60[self->unk58]`.

## The C

```c
s32 Obj86B60__GetActiveSlotCount(Obj86B60 *self)
{
    return self->unk60[self->unk58];
}
```

## Struct knowledge established / corrected

`include/code_2cc8c.h`'s `Obj86B60Methods` already had a `slot118` field
whose comment attributed it to `Obj86B60__GetActiveSlotCount` (citing `func_8003C944.md`'s
"Struct knowledge established" section as the source). **That attribution
was wrong.** Reading the retail table bytes directly:

```
D_80086B60+0x118 = 0x8003DE30  (func_8003DE30, NOT Obj86B60__GetActiveSlotCount)
D_80086B60+0x120 = 0x8003DFA0  (Obj86B60__GetActiveSlotCount's real slot)
```

The byte OFFSET `func_8003C944` compiled against (0x118) was and is
correct -- that function still matches -- but the function pointer VALUE
stored there at runtime is `func_8003DE30`, not `Obj86B60__GetActiveSlotCount`. The old
report's error: a discarded/void-typed call site is not evidence of which
function occupies a slot, only of the slot's own signature. I corrected
`slot118`'s comment and added the (previously missing) `slot120` field for
`Obj86B60__GetActiveSlotCount` itself, both in `include/code_2cc8c.h`. See this unit's
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
`Obj86B60__GetActiveSlotCount`/func_8003DDC8/func_8003DE30, code_2cc8c_b.c). No purpose beyond
the getter itself is claimed.
