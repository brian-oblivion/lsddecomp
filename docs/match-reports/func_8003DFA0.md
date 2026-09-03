# func_8003DFA0 — MATCH (7/7 words)

**Unit:** code_2cc8c_c · **Size:** 7 instructions

## What it does

`Obj86B60Methods::slot120` (verified by reading the raw table bytes at
`D_80086B60+0x120` in `disk/SLPS_015.56` directly, and cross-checked with
`tools/classtable.py D_80086B60`). Returns the current ring-buffer slot's
running count: `self->unk60[self->unk58]`.

## The C

```c
s32 func_8003DFA0(Obj86B60 *self)
{
    return self->unk60[self->unk58];
}
```

## Struct knowledge established / corrected

`include/code_2cc8c.h`'s `Obj86B60Methods` already had a `slot118` field
whose comment attributed it to `func_8003DFA0` (citing `func_8003C944.md`'s
"Struct knowledge established" section as the source). **That attribution
was wrong.** Reading the retail table bytes directly:

```
D_80086B60+0x118 = 0x8003DE30  (func_8003DE30, NOT func_8003DFA0)
D_80086B60+0x120 = 0x8003DFA0  (func_8003DFA0's real slot)
```

The byte OFFSET `func_8003C944` compiled against (0x118) was and is
correct -- that function still matches -- but the function pointer VALUE
stored there at runtime is `func_8003DE30`, not `func_8003DFA0`. The old
report's error: a discarded/void-typed call site is not evidence of which
function occupies a slot, only of the slot's own signature. I corrected
`slot118`'s comment and added the (previously missing) `slot120` field for
`func_8003DFA0` itself, both in `include/code_2cc8c.h`. See this unit's
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
