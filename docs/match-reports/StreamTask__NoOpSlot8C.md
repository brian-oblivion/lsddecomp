# StreamTask__NoOpSlot8C

> Renamed from `StreamTaskObj__NoOpSlot8C` on 2026-09-26 (tools/rename.py). Address 0x8003bdec.

> Renamed from `func_8003BDEC` on 2026-09-23 (tools/rename.py). Address 0x8003bdec.

**Unit:** task · **Size:** 1 instruction (0x4 bytes, `jr $ra; nop`) · **Status:** MATCHED

## What it does

Nothing: an empty function body (`{ }`), occupying `gStreamTaskMethods`
slot `+0x08C` (confirmed by `tools/classtable.py gStreamTaskMethods`),
one slot over from `StreamTask__NoOpSlot88` -- see that report for the
shared context (the sibling table `gTaskCoreMethods` also has a NULL entry
here). No report existed for this function before this round; folded into
the naming pass for the same reason as its neighbor.

## Naming

**StreamTask__NoOpSlot8C** -- tier A. Same reasoning as
`StreamTask__NoOpSlot88`: an intentionally empty vtable-slot override,
`Class__NoOpSlotNN` convention.

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Occupies +0x08C (NULL in TaskCore's table).
