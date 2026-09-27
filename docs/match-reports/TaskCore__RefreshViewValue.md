# TaskCore__RefreshViewValue — MATCH (22/22 words)

> Renamed from `Obj86B60__RefreshViewValue` on 2026-09-25 (tools/rename.py). Address 0x8003ca94.

> Renamed from `func_8003CA94` on 2026-09-24 (tools/rename.py). Address 0x8003ca94.

**Unit:** TaskViewport · **Size:** 22 instructions

## What it does

```c
void TaskCore__RefreshViewValue(Obj86B60 *self)
{
    if (self->unk9C != NULL) {
        self->unk9C(self->unkA0);
    }
    self->methods->slot60(self, 7);
}
```

`self->unk9C` is a callback taking one opaque argument (`self->unkA0`,
itself passed through unchanged); both are set together by
`TaskCore__SetCallback` (see that report, next in ROM order below this one but
established first since it is the 3-instruction setter). The `slot60(self,
7)` call is unconditional -- happens whether or not the callback fired.

## Struct knowledge established

- `Obj86B60::unk9C` (`void (*)(void*)`, +0x09C) and `::unkA0` (`void *`,
  +0x0A0) -- OBSERVED here as call targets; set by `TaskCore__SetCallback`.

## Provenance

round 2026-09-02, runner echo, unit TaskViewport. 1 attempt.

## Naming (round 78, delta)

**Tier A.** `func_8003CA94` -> `Obj86B60__RefreshViewValue`. Occupies slot94
in `gTaskCoreMethods`; `gTitleMenuMethods` overrides the same slot with
`TitleMenu__RefreshViewValue`, same evidence shape as `Tick`/`SetState`
above. Corroborated independently: this function's own body invokes
`self->unk9C(self->unkA0)` when set, which is exactly the callback+ctx pair
`TaskCore__SetCallback` installs -- see that report.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__RefreshViewValue (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 98, alpha)

setState(7) -> TASKCORE_STATE_FADE_OUT. Byte-identical.
