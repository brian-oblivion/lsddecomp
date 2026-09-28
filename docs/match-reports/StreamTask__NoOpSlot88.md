# StreamTask__NoOpSlot88

> Renamed from `StreamTaskObj__NoOpSlot88` on 2026-09-26 (tools/rename.py). Address 0x8003bde4.

> Renamed from `func_8003BDE4` on 2026-09-23 (tools/rename.py). Address 0x8003bde4.

**Unit:** task · **Size:** 1 instruction (0x4 bytes, `jr $ra; nop`) · **Status:** MATCHED

## What it does

Nothing: an empty function body (`{ }`), occupying `gStreamTaskMethods`
slot `+0x088` (confirmed by `tools/classtable.py gStreamTaskMethods`; the
sibling table `gTaskCoreMethods` has a NULL entry at this same offset, so
this is a real StreamTaskObj-level override of an otherwise-unpopulated
slot, not an inherited stub). No report existed for this function before
this round -- it matched trivially (splat's own `jr $ra; nop` shape) and was
never separately written up; folded into this round's naming pass since it
still needed a name.

## Naming

**StreamTask__NoOpSlot88** -- tier A. An intentionally empty vtable-slot
override; the mechanics ARE the whole purpose (do nothing when this slot is
dispatched). Matches the `Class__NoOpSlotNN` convention already established
in this codebase for the identical shape (`dream_sys.h`'s
`DreamSys__NoOpSlotE8Default`/`Actor__NoOpSlotD8`, `task.h`'s
`TextRow__NoOpGetCell`/`TextRow__NoOpSlotD0`).

## Track 4 (2026-09-26, round 87)

Renamed with the class unification (gStreamTaskObjMethods -> class StreamTask, include/StreamTask.h): the `Obj` suffix is dropped (track 4 step 2; include/GameApplication.h already viewed the class as `StreamTask`). Occupies +0x088 (NULL in TaskCore's table).
