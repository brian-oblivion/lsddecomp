# TaskObjF__ClearResourceSlots — MATCH (6/6 words)

> Renamed from `func_8004E3F4` on 2026-09-24 (tools/rename.py). Address 0x8004e3f4.

**Unit:** class_3bb8c_e (round 14, first slice of the new `Node3bb8cE` class --
see `include`-local declarations at the top of `src/class_3bb8c_e.c`; this
class is unrelated to `Obj866E8`/`D_800866E8` in `include/class_3bb8c.h`).

## What it does

`void TaskObjF__ClearResourceSlots(Node3bb8cE *self)`. Zeroes the object's five typed
resource-slot pointers (`unk60`, `unk64`, `unk68`, `unk78`, `unk7C`) — the
same five fields `TaskObjF__RemoveAllChildren` zeroes before calling the base class's
`removeAllChildren`. Almost certainly a constructor/init helper for this
class's own resource-slot state.

## Result

Matched on the first attempt — a straight sequence of five word stores, no
branches, no register-identity ambiguity possible.

```c
void TaskObjF__ClearResourceSlots(Node3bb8cE *self)
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
