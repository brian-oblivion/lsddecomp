# StreamTaskObj__NoOpSlot8C

> Renamed from `func_8003BDEC` on 2026-09-23 (tools/rename.py). Address 0x8003bdec.

**Unit:** code_2c054 · **Size:** 1 instruction (0x4 bytes, `jr $ra; nop`) · **Status:** MATCHED

## What it does

Nothing: an empty function body (`{ }`), occupying `gStreamTaskObjMethods`
slot `+0x08C` (confirmed by `tools/classtable.py gStreamTaskObjMethods`),
one slot over from `StreamTaskObj__NoOpSlot88` -- see that report for the
shared context (the sibling table `gTaskCoreMethods` also has a NULL entry
here). No report existed for this function before this round; folded into
the naming pass for the same reason as its neighbor.

## Naming

**StreamTaskObj__NoOpSlot8C** -- tier A. Same reasoning as
`StreamTaskObj__NoOpSlot88`: an intentionally empty vtable-slot override,
`Class__NoOpSlotNN` convention.
