# TaskObjF__RemoveAllChildren — MATCH (19/19 words)

> Renamed from `func_8004E588` on 2026-09-24 (tools/rename.py). Address 0x8004e588.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__RemoveAllChildren(Node3bb8cE *self)`. Zeroes the same five resource-slot
fields `TaskObjF__ClearResourceSlots` zeroes, then chains to the base class's
`removeAllChildren` (+0x018 on the `Get_vtable_BasicClass()` vtable). Reads as a
finalize/reset helper distinct from `TaskObjF__Finalize` (which only chains
`finalize`, +0x00C, without touching the resource slots).

## Result

Matched on the first attempt.

```c
void TaskObjF__RemoveAllChildren(Node3bb8cE *self)
{
    self->unk60 = NULL;
    self->unk64 = NULL;
    self->unk68 = NULL;
    self->unk78 = NULL;
    self->unk7C = NULL;
    Get_vtable_BasicClass()->removeAllChildren(self);
}
```

### Proposed learning

None new.
