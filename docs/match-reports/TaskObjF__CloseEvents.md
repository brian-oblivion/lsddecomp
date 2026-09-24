# TaskObjF__CloseEvents — MATCH (16/16 words)

> Renamed from `func_8004E678` on 2026-09-24 (tools/rename.py). Address 0x8004e678.

**Unit:** class_3bb8c_e (round 14, `Node3bb8cE` class).

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
Declared locally in `src/class_3bb8c_e.c`, not in the shared
`include/class_3bb8c.h` — see the file-top comment there for why.

### Proposed learning

None new — matches the established "discarded call for side effect, then
unconditional literal return" shape already documented for one-line
wrappers.
