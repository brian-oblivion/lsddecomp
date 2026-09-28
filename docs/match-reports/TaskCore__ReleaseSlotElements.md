# TaskCore__ReleaseSlotElements — MATCHED (26/26)

> Renamed from `Obj86B60__ReleaseSlotElements` on 2026-09-25 (tools/rename.py). Address 0x8003d6d4.

> Renamed from `func_8003D6D4` on 2026-09-24 (tools/rename.py). Address 0x8003d6d4.

**Unit:** task · **Size:** 26 words · **Result:** byte-exact, first attempt

## What it does

Looks up two per-index resource pointers via `self->unk58` as an index into
two parallel arrays, and passes them to a pair of already-matched (in other
units) helper functions.

```c
void TaskCore__ReleaseSlotElements(Obj86B60 *self)
{
    ReleaseBasicClassArray(self->unk64[self->unk58], self->unk5C[self->unk58]);
    BMemPMgrFree(self->unk64[self->unk58]);
}
```

## Header additions

`include/task.h`:

- New fields `unk5C`/`unk64` on `Obj86B60`, both `void **`, carved out of
  existing padding (`0x05C`-`0x070`, previously undifferentiated). No
  existing field's type or offset changed.
- `extern void *BMemPMgrAlloc(s32 size);` and
  `extern void BMemPMgrFree(void *ptr);` — both already confirmed
  elsewhere in the project (many units use the allocator; `data_source.h`
  and `entity.h` both type the release call `void`-returning, and this
  unit's call site discards any return too, consistent with that).
- `extern void ReleaseBasicClassArray(void *a0, void *a1);` — not previously seen in
  this project. Typed purely from this call site: both argument registers
  are loaded from the two arrays with no further use, and the return value
  (if any) is never consulted, so `void` is the only assumption this call
  site itself supports.

No residue — matched first attempt from a direct reading of the
disassembly, no m2c seed needed.

## Naming (round 78, naming runner echo)

Renamed `func_` -> `Obj86B60__ReleaseSlotElements`. **Tier B**: Releases and frees `self->itemLists[activeSlot]` through two already-matched helpers -- the exact teardown counterpart of TaskCore__CreateSlotElements.

## Track 4 (2026-09-25, round 84, alpha)

Renamed from Obj86B60__ReleaseSlotElements (tools/rename.py): the class prefix. Occupant of its gTaskCoreMethods slot, named for it in TASKCORE_SLOTS (`classtable.py gTaskCoreMethods`). The class (id 0x130, table gTaskCoreMethods) is unified as `TaskCore` in `include/TaskCore.h`; `self` is `TaskCore *` (it was the `Obj86B60` or `StreamTaskObj` view). Any source block above is the pre-unification spelling; the live body takes the unified types and slot names, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
