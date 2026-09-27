# TaskCore__RetreatSlotCursor — MATCHED (27/27)

> Renamed from `Obj86B60__RetreatSlotCursor` on 2026-09-25 (tools/rename.py). Address 0x8003de30.

> Renamed from `func_8003DE30` on 2026-09-24 (tools/rename.py). Address 0x8003de30.

**Unit:** TaskViewport · **Size:** 27 words · **Result:** byte-exact

## What it does

The mirror-image of `TaskCore__AdvanceSlotCursor`: retreats the same ring-buffer index
(`self->unk60[idx]`), wrapping to `capacity - 1` when it goes negative
instead of forward to 0 at the capacity, then reports it through the same
`+0x11C` slot.

```c
void TaskCore__RetreatSlotCursor(Obj86B60 *self)
{
    s32 idx = self->unk58;
    s32 v = self->unk60[idx];

    v--;
    if (v < 0) {
        v = self->unk5C[idx] - 1;
    }
    self->methods->slot11C(self, v, 1);
}
```

## Residue

Same class as `TaskCore__AdvanceSlotCursor`, same fix, applied directly from that
function's report: split the load of `self->unk60[idx]` from the `-1`
into two statements (`s32 v = self->unk60[idx]; v--;`) rather than one
combined initializer. Matched on the first attempt written this way — see
`TaskCore__AdvanceSlotCursor.md`'s report for the full derivation and the proposed
learning; not re-derived here.

## Header

No new header changes beyond what `TaskCore__AdvanceSlotCursor` already added
(`slot11C`, `unk5C`, `unk60`, `unk58`) — this function reuses all of it.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__RetreatSlotCursor`. **Tier B**: The exact mirror of TaskCore__AdvanceSlotCursor, stepping backward with wraparound at 0. Same reasoning.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__RetreatSlotCursor (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
