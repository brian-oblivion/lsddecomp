# BoxFill__SetSize — MATCHED (12/12 words)

> Renamed from `func_80040824` on 2026-09-25 (tools/rename.py). Address 0x80040824.

Unit: `src/code_2cc8c_f.c`. 3 attempts.

```c
void BoxFill__SetSize(Obj6EAC0 *self, s32 *a1) {
    if (self->unkC != 0) {
        self->unk60 = ((u16 *)a1)[0];
        self->unk62 = ((u16 *)&a1[1])[0];
    }
}
```

Base-table occupant of `Obj6EAC0Methods::slotC0`. `a1` points at the
SAME 2-word struct shape `BoxFill__SetPosition` (slotBC) reads as `{s32 x, s32
y}` -- this occupant reads a `u16` out of each word's LOW half (offset
0 and offset 4, not offset 0 and 2), which is what took 2 attempts to
find: my first cut typed `a1` as `u16 *` and read `a1[0]`/`a1[1]`
(offsets 0/2), scoring 9/12 with only the SECOND load's immediate
wrong (`0x2` vs retail's `0x4`).

First attempt (before this) also caught a real struct bug: I had
declared `unk60`/`unk62` immediately after `unk58` (a `u32` ending at
`+0x05C`) with no padding, so they silently landed at `+0x05C`/`+0x05E`
instead of the intended `+0x060`/`+0x062` -- `include/code_2cc8c.h` now
has an explicit `pad05C[0x060 - 0x05C]` gap. This is exactly the
"field inserted without its leading padNN" trap CLAUDE.md warns about;
caught immediately here because the SAME struct's `BoxFill__SetSize` was
being written in the same sitting, but it could easily have silently
corrupted a different, already-matched function's view of this struct
in a way that wouldn't show up until much later.

### Proposed learning

When adding sibling `s16`/`u16` fields right after a `u32` field in a
freshly-modelled struct, double check the OFFSET comment against the
actual C layout before writing the occupant -- a missing pad gap here
scored a wrong-but-plausible 9/12 (only the immediate differed, not the
whole shape), which could be mistaken for a small residue rather than a
struct bug.

## Naming

Round 54 (alpha), FINISHING-PLAN track 3.

| was | now | tier |
| --- | --- | --- |
| `BoxFill__SetSize` | (kept `BoxFill__SetSize`) | C |

**What is known.** Base occupant of `slotC0`: `self->unk60 =
((u16*)a1)[0]; self->unk62 = ((u16*)&a1[1])[0];` -- copies two `u16`
halfwords out of a caller-supplied 2-halfword struct. Plausibly a
width/height or offset pair (this unit's own layout-heavy neighbourhood
makes that a live guess), but nothing in this unit ever READS `unk60`/
`unk62` back, so there is no mechanical evidence to anchor a name on --
kept `func_`/`unk60`/`unk62` unrenamed rather than assert "size" or
"offset" from a single write-only call site.
