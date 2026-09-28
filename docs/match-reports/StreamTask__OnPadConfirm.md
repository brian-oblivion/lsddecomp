# StreamTask__OnPadConfirm

> Renamed from `StreamTaskObj__func_8003BD10` on 2026-09-26 (tools/rename.py). Address 0x8003bd10.

> Renamed from `func_8003BD10` on 2026-09-23 (tools/rename.py). Address 0x8003bd10.

**Unit:** task · **Size:** 25 words · **Status:** MATCHED (25/25)

## Summary

Straight-line forward: call the shared `TaskCoreMethods` singleton's slot
`+0x078` on `self`, then if `self->unkCC` is set, mark `self->unk38 = 2` and
call `self->methods->slot60(self, 0x12)`.

```c
void StreamTask__OnPadConfirm(StreamTaskObj *self) {
    GetTaskCoreMethods()->slot78(self);
    if (self->unkCC != 0) {
        self->unk38 = 2;
        self->methods->slot60(self, 0x12);
    }
}
```

## Evidence

- `GetTaskCoreMethods()` returns `&gTaskCoreMethods` (`TaskCoreMethods`, established
  elsewhere in `task.h`). `tools/classtable.py gTaskCoreMethods` shows slot
  `+0x078 = TaskCore__OnPadConfirm` (a different unit, not touched here — only the
  slot's existence and signature matter for this call site).
- `self->methods` is `StreamTaskObjMethods*` (`gStreamTaskMethods`).
  `tools/classtable.py gStreamTaskMethods` shows slot `+0x060 = StreamTask__SetState`, which
  is this unit's own queued `StreamTask__SetState` — confirms the slot exists and
  that its signature is `(StreamTaskObj *self, s32 a1)`.
- Both call results are discarded in the disassembly, so both slots are typed
  `void` here (no counter-evidence).

## Proposed learning

`tools/classtable.py <table>` cross-referenced against the OTHER known table
(`gStreamTaskMethods` vs `gTaskCoreMethods`) is a fast way to confirm a newly-added vtable
slot's existence and arity before writing the call: the callee occupying that
slot is often another function already queued (sometimes in the same unit,
sometimes not), and its own parameter list is direct evidence for the slot's
signature — cheaper than deriving the slot purely from the caller's register
setup.

## Naming

**StreamTask__OnPadConfirm** -- tier C. Occupies `gStreamTaskMethods`
slot `+0x078`; up-calls the base slot, then if `unkCC` is set, marks
`unk38 = 2` and re-enters this class's own state-transition slot with code
`0x12`. Neither `unk38` nor "state `0x12`" has a confirmed game meaning
(see `TaskCore__Init`'s report for the same `unk38` field from
the other side), so left `Class__func_xxxxx`.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green). StreamTaskObj now expands TASKCORE_FIELDS: +0x038 is `result`.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/stream_task.h): the `Obj` suffix is dropped (track 4 step 2; include/game_application.h already viewed the class as `StreamTask`). Was StreamTaskObj__func_8003BD10. Occupies +0x078 onPadConfirm, up-calls TaskCore's, then with `skipOnConfirm` set: result = 2, setState(0x12).
