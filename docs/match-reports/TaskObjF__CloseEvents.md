# TaskObjF__CloseEvents — MATCH (16/16 words)

> Renamed from `func_8004E678` on 2026-09-24 (tools/rename.py). Address 0x8004e678.

**Unit:** class_3bb8c_c (round 14, `Node3bb8cE` class).

## What it does

`s32 TaskObjF__CloseEvents(Node3bb8cE *self)`. Calls the still-INCLUDE_ASM
`TaskObjF__DisableEvents(self)` (class_3bb8c_f) purely for its side effect (return
value discarded), then `TaskObjF__ForEachEvent(self, func_8003902C, 1)` — passing
a function pointer (`func_8003902C`, still raw asm elsewhere) — and
unconditionally returns `1` regardless of either call's outcome.

## Result

Matched on the first attempt.

```c
s32 TaskObjF__CloseEvents(Node3bb8cE *self)
{
    TaskObjF__DisableEvents(self);
    TaskObjF__ForEachEvent(self, func_8003902C, 1);
    return 1;
}
```

### Extern added for a function outside this unit

`TaskObjF__DisableEvents`, `TaskObjF__ForEachEvent` (both still `INCLUDE_ASM` in
`class_3bb8c_f.c`, runner charlie's unit) and `func_8003902C` (uncarved).
Declared locally in `src/class_3bb8c_c.c`, not in the shared
`include/class_3bb8c.h` — see the file-top comment there for why.

### Proposed learning

None new — matches the established "discarded call for side effect, then
unconditional literal return" shape already documented for one-line
wrappers.

## Naming (round 78, track 3)

`func_8004E678` -> `TaskObjF__CloseEvents`. **Tier A.** Sits at `gTaskObjFMethods` +0x048. Disables events (`TaskObjF__DisableEvents`) then closes all 4 via `TaskObjF__ForEachEvent(self, CloseEvent, 1)` -- the exact teardown counterpart of `TaskObjF__OpenEvents`, which this unit's own +0x044 slot opens the same 4 events.

## Declarations (round 98, track 7)

`CloseEvent` now comes from `<kernel.h>` (`long CloseEvent(long)`), so it is
cast to `TaskObjF__ForEachEvent`'s `s32 (*)(s32)` callback type; the
unit's own `s32` extern is gone. Zero bytes changed.
