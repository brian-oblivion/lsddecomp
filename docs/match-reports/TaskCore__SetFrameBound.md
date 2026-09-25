# TaskCore__SetFrameBound — MATCH (8/8 words)

> Renamed from `Obj86B60__SetFrameBound` on 2026-09-25 (tools/rename.py). Address 0x8003c794.

> Renamed from `func_8003C794` on 2026-09-24 (tools/rename.py). Address 0x8003c794.

**Unit:** code_2cc8c · **Size:** 8 instructions

## What it does

A setter on `Obj86B60 *self`: `self->unk40 = a1;`, then if `a1 >= 0`,
overwrite it with `a1 * 20`. Retail computes `a1 * 20` as `(a1 << 2) + a1`
then `<< 2` (i.e. `a1*5*4`), which is GCC 2.6.3's ordinary strength
reduction for a multiply by 20 -- reproduced automatically by writing the
plain `*`.

```c
void TaskCore__SetFrameBound(Obj86B60 *self, s32 a1)
{
    self->unk40 = a1;
    if (a1 >= 0) {
        self->unk40 = a1 * 20;
    }
}
```

The unconditional `self->unk40 = a1;` and the conditional override are both
load-bearing: the `bltz $a1` guard's delay slot IS the unconditional store,
so the negative-a1 path leaves `self->unk40 == a1` untouched and only the
non-negative path recomputes it as `a1*20`.

## Struct knowledge established

`Obj86B60::unk40` (s32, +0x040) -- OBSERVED here as a setter target; also
read (compared against `unk1C`) by `TaskCore__Update`.

## Provenance

round 2026-09-02, runner echo, unit code_2cc8c (first function of the unit,
established `include/code_2cc8c.h` and the `Obj86B60` struct name from the
class's base method table gClass86B60Methods).

## Naming (round 78, delta)

**Tier A** (pure setter -- "a getter, a clamp, a list push" per
FINISHING-PLAN's tier-A-by-definition rule for a pure leaf). `TaskCore__SetFrameBound`
-> `TaskCore__SetFrameBound`. Body: `self->unk40 = a1;` then, when `a1 >= 0`,
`self->unk40 = a1 * 20`. Occupies slot6C (round 78: this slot was previously
padded over in the header as unoccupied -- corrected against the raw table
bytes, see the header's own comment on that field). Corroborated by
`TaskCore__Update`, which compares `self->unk40` against `frameCounter`
as an upper bound to decide whether to call `SetState(6)` -- "FrameBound"
describes that mechanic (a frame-count threshold) without asserting why the
game sets one.

## Proposed field names (round 78, delta -- NOT applied, cross-unit)

`Obj86B60::unk40` (s32, +0x040) -> `frameBound`, matching the function name
above. Grep shows `unk40` textual hits in many unrelated units
(class_39e08.c, code_171e0.c, class_3bb8c_*.c, code_2c054.c, code_179d8_*.c,
code_2cc8c_d.c) so this is a PROPOSAL, not a direct rename -- only a
definition-only rename + rebuild can tell which are this same struct.


**Head disposition, round 78.** `unk40` -> `frameBound` DECLINED: the only source of "frame" is this setter's own name, so the field name would restate the function's hypothesis; left for whoever establishes what the bound is compared against.
