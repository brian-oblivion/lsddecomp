# StreamTask__OnPadNext

> Renamed from `StreamTaskObj__func_8003BDAC` on 2026-09-26 (tools/rename.py). Address 0x8003bdac.

> Renamed from `func_8003BDAC` on 2026-09-23 (tools/rename.py). Address 0x8003bdac.

**Unit:** task · **Size:** 14 instructions (0x38 bytes) · **Status:** MATCHED (14/14 words, whole-image SHA1 green), first attempt

## What it does

Identical shape to `StreamTask__OnPadPrev` (see that report for the full
derivation and the return-type discussion) one slot over: forwards to
`GetTaskCoreMethods()`'s (i.e. `gTaskCoreMethods`'s / `LoaderTaskMethods`'s) slot
`+0x084` instead of `+0x080`. Occupies `gStreamTaskMethods` slot `+0x084` itself.

## Derivation

```c
void StreamTask__OnPadNext(StreamTaskObj *self) {
    GetTaskCoreMethods()->slot84(self);
}
```

Matched first attempt, same reasoning as `StreamTask__OnPadPrev`.

## New struct/header knowledge

Added `TaskCoreMethods::slot84` alongside `slot80` in
`include/task.h` (see `StreamTask__OnPadPrev`'s report).

## Proposed learning

None beyond `StreamTask__OnPadPrev`'s -- same shape, same open return-type flag.

## Naming

**StreamTask__OnPadNext** -- tier C. Occupies `gStreamTaskMethods`
slot `+0x084`; identical pure up-call shape to
`StreamTask__OnPadPrev` one slot over. Same reasoning, left
`Class__func_xxxxx`.

## Track 4 (2026-09-25, round 84, alpha)

Its up-calls to TaskCore (include/task_core.h, track 4 round 84) now go through `GetTaskCoreMethods()` with `self` upcast to `TaskCore *` and TaskCore's slot names; byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/stream_task.h): the `Obj` suffix is dropped (track 4 step 2; include/game_application.h already viewed the class as `StreamTask`). Was StreamTaskObj__func_8003BDAC. Occupies +0x084 onPadNext and only up-calls TaskCore's.
