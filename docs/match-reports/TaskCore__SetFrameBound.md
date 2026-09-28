# TaskCore__SetFrameBound — MATCH (8/8 words)

> Renamed from `Obj86B60__SetFrameBound` on 2026-09-25 (tools/rename.py). Address 0x8003c794.

> Renamed from `func_8003C794` on 2026-09-24 (tools/rename.py). Address 0x8003c794.

**Unit:** Task · **Size:** 8 instructions

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

round 2026-09-02, runner echo, unit Task (first function of the unit,
established `include/Task.h` and the `Obj86B60` struct name from the
class's base method table gTitleMenuMethods).

## Naming (round 78, delta)

**Tier A** (pure setter -- "a getter, a clamp, a list push" per
FINISHING-PLAN's tier-A-by-definition rule for a pure leaf). `Obj86B60__SetFrameBound`
-> `Obj86B60__SetFrameBound`. Body: `self->unk40 = a1;` then, when `a1 >= 0`,
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
(class_39e08.c, GameApplicationFileResource.c, class_3bb8c_*.c, Task.c, code_179d8_*.c,
Task.c) so this is a PROPOSAL, not a direct rename -- only a
definition-only rename + rebuild can tell which are this same struct.


**Head disposition, round 78.** `unk40` -> `frameBound` DECLINED: the only source of "frame" is this setter's own name, so the field name would restate the function's hypothesis; left for whoever establishes what the bound is compared against.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__SetFrameBound (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 98, alpha)

`bound * 20` is `bound * TASKCORE_FRAMES_PER_SECOND` (tier B, include/TaskCore.h):
DrawSystem__Init sets vsyncCount 3, so the game draws 20 frames a second
and frameCounter counts them, which makes `bound` seconds. A negative bound
is stored as is, and TaskCore__Update's unsigned compare then never fires:
no time limit. StreamTask__SetFrameBound's 15 is a different rate
(proposed as its own constant). Byte-identical.
