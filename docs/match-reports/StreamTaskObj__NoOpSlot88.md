# StreamTaskObj__NoOpSlot88

> Renamed from `func_8003BDE4` on 2026-09-23 (tools/rename.py). Address 0x8003bde4.

**Unit:** code_2c054 · **Size:** 1 instruction (0x4 bytes, `jr $ra; nop`) · **Status:** MATCHED

## What it does

Nothing: an empty function body (`{ }`), occupying `gStreamTaskObjMethods`
slot `+0x088` (confirmed by `tools/classtable.py gStreamTaskObjMethods`; the
sibling table `gTaskCoreMethods` has a NULL entry at this same offset, so
this is a real StreamTaskObj-level override of an otherwise-unpopulated
slot, not an inherited stub). No report existed for this function before
this round -- it matched trivially (splat's own `jr $ra; nop` shape) and was
never separately written up; folded into this round's naming pass since it
still needed a name.

## Naming

**StreamTaskObj__NoOpSlot88** -- tier A. An intentionally empty vtable-slot
override; the mechanics ARE the whole purpose (do nothing when this slot is
dispatched). Matches the `Class__NoOpSlotNN` convention already established
in this codebase for the identical shape (`DreamSys.h`'s
`DreamSys__NoOpSlotE8Default`/`DreamSys__NoOpSlotD8`, `code_2cc8c.h`'s
`Obj6EAC0__NoOpSetter`/`Obj6EAC0__NoOpSlotD0`).
