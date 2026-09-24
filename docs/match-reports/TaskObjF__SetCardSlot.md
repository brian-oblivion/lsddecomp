# TaskObjF__SetCardSlot — MATCH (4/4 words)

> Renamed from `func_8004E5D4` on 2026-09-24 (tools/rename.py). Address 0x8004e5d4.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

## What it does

`void TaskObjF__SetCardSlot(Node3bb8cE *self, s32 val)`. Sets `self->unkC = val`
and `self->unk10 = val << 4`. The only setter in this unit for `unkC`,
which `TaskObjF__FormatCard` later reads as a nonzero-tested flag and
`TaskObjF__OpenAndReadMemcardFile`/`TaskObjF__ProbeCardFreeSpace` forward opaquely as `BuildMemcardPath`'s
2nd argument.

## Result

Matched on the first attempt.

```c
void TaskObjF__SetCardSlot(Node3bb8cE *self, s32 val)
{
    self->unkC = val;
    self->unk10 = val << 4;
}
```

### Proposed learning

None — trivial straight-line store pair.
