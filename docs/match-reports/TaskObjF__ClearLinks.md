# TaskObjF__ClearLinks — MATCH (6/6 words)

> Renamed from `TaskObjF__ClearResourceSlots` on 2026-09-27 (tools/rename.py). Address 0x8004e3f4.

> Renamed from `func_8004E3F4` on 2026-09-24 (tools/rename.py). Address 0x8004e3f4.

**Unit:** class_3bb8c_e (round 14, first slice of the new `Node3bb8cE` class --
see `include`-local declarations at the top of `src/class_3bb8c_e.c`; this
class is unrelated to `Obj866E8`/`gStageMapMethods` in `include/class_3bb8c.h`).

## What it does

`void TaskObjF__ClearLinks(Node3bb8cE *self)`. Zeroes the object's five typed
resource-slot pointers (`unk60`, `unk64`, `unk68`, `unk78`, `unk7C`) — the
same five fields `TaskObjF__RemoveAllChildren` zeroes before calling the base class's
`removeAllChildren`. Almost certainly a constructor/init helper for this
class's own resource-slot state.

## Result

Matched on the first attempt — a straight sequence of five word stores, no
branches, no register-identity ambiguity possible.

```c
void TaskObjF__ClearLinks(Node3bb8cE *self)
{
    self->unk60 = NULL;
    self->unk64 = NULL;
    self->unk68 = NULL;
    self->unk78 = NULL;
    self->unk7C = NULL;
}
```

### Proposed learning

None beyond what's already recorded — a pure straight-line store sequence
needs nothing beyond a direct transcription.

## Naming (round 78, track 3)

`func_8004E3F4` -> `TaskObjF__ClearLinks`. **Tier B.** NOT a `gTaskObjFMethods` entry (checked against the full table) -- called once, directly, from `TaskObjF__TaskObjF`'s own ctor (class_3bb8c_d.c), before `self->methods->slot40(self, arg2)` (`TaskObjF__SetCardSlot`). Zeroes the same five resource-slot pointers (`res02`/`res05`/`unk68`/`res10`/`res20`) that `TaskObjF__RemoveAllChildren` zeroes before chaining to the base class. Mechanics are exact (construction-time init of the resource slots); tier B because it is one line of evidence (a single call site) rather than two independent callers agreeing.
