# TaskCore__SetFadeRate — MATCH (2/2 words)

> Renamed from `Obj86B60__SetFadeRate` on 2026-09-25 (tools/rename.py). Address 0x8003cbb8.

> Renamed from `func_8003CBB8` on 2026-09-24 (tools/rename.py). Address 0x8003cbb8.

**Unit:** task · **Size:** 2 instructions

## What it does

```c
void TaskCore__SetFadeRate(Obj86B60 *self, s32 a1)
{
    self->unk84 = a1;
}
```

The unit's smallest real function (a single `sw`, plus its `jr $ra` delay
slot). Confirms `Obj86B60::unk84` (already established as a multiplier by
`TaskCore__TickFadeIn`) is a plain setter target, not computed internally.

## Provenance

round 2026-09-02, runner echo, unit task. 1 attempt.

## Naming (round 78, delta)

**Tier A** (pure setter). `func_8003CBB8` -> `Obj86B60__SetFadeRate`. Body:
`self->unk84 = a1;`, nothing else. Corroborated by `TaskCore__TickFadeIn`,
the sole reader of `unk84`, which multiplies it against `frameCounter` to
build the per-tick colour delta -- exactly what a "rate" describes
mechanically, without asserting what the fade itself means in the game.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__SetFadeRate (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/task_core.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
