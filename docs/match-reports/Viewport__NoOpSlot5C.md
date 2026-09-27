# Viewport__NoOpSlot5C — MATCHED

> Renamed from `Viewport__func_8003EA74` on 2026-09-27 (tools/rename.py). Address 0x8003ea74.

> Renamed from `func_8003EA74` on 2026-09-27 (tools/rename.py). Address 0x8003ea74.

Unit: `Task`. Round 73, runner charlie (report written retroactively;
same shape as `Viewport__NoOpSlot58` immediately above it in ROM order).

## Signature

```c
void Viewport__NoOpSlot5C(void);
```

Not a `gViewportMethods` vtable slot occupant, not a method (no `self`), no caller
found anywhere in `src/` or the remaining `asm/*.s` segments.

## What it does

Nothing: `jr $ra; nop`.

## Naming

Kept `Viewport__NoOpSlot5C`. Same reasoning as `Viewport__NoOpSlot58`: no evidence ties it
to any class or caller.

## Track 4 (2026-09-25, round 85, bravo)

Occupant of Viewport's +0x05C (`slot5C` in `include/Viewport.h`; also in gNodeGuardedViewportMethods). Empty and never called, so neither the slot nor the function is named. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.


## Track 7 (round 95, alpha, polish pass)

## Naming

Renamed from `func_8003EA74` with `tools/rename.py`: tier C, `Class__func_xxxxx`, occupant of gViewportMethods +0x05C (and gNodeGuardedViewportMethods'). Empty body, no C caller of the slot.


## Track 7 (round 100, echo, polish pass)

## Naming

Renamed from `Viewport__func_8003EA74` with `tools/rename.py`: tier A. The body is empty (`jr $ra; nop`), and an empty occupant's mechanics are its purpose, so the name says it does nothing and which slot of gViewportMethods (+0x05C) holds it. The form follows the project's other empty occupants (`NodeGuardedViewport__NoOpSlotB8`, `MoviePlayer__NoOpSlot5C`, `StreamTask__NoOpSlot88`). An empty body says nothing about the slot's arguments (DECOMPILATION_LEARNINGS), so the prototype keeps `(void)`. The slot keeps its offset name `slot5C`: nothing calls it.
