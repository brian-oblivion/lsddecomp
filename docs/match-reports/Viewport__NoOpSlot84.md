# Viewport__NoOpSlot84 — MATCHED

> Renamed from `Viewport__func_8003ECC0` on 2026-09-27 (tools/rename.py). Address 0x8003ecc0.

> Renamed from `func_8003ECC0` on 2026-09-27 (tools/rename.py). Address 0x8003ecc0.

Unit: `task`. Round 73, runner charlie (report written retroactively;
same shape as `Viewport__NoOpSlot58`/`Viewport__NoOpSlot5C`).

## Signature

```c
void Viewport__NoOpSlot84(void);
```

Not a `gViewportMethods` vtable slot occupant, not a method (no `self`), no caller
found anywhere in `src/` or the remaining `asm/*.s` segments.

## What it does

Nothing: `jr $ra; nop`.

## Naming

Kept `Viewport__NoOpSlot84`. No evidence of any kind.

## Track 4 (2026-09-25, round 85, bravo)

Occupant of Viewport's +0x084 (`slot84` in `include/Viewport.h`; also in gNodeGuardedViewportMethods). Empty and never called, so neither the slot nor the function is named. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.


## Track 7 (round 95, alpha, polish pass)

## Naming

Renamed from `func_8003ECC0` with `tools/rename.py`: tier C, `Class__func_xxxxx`, occupant of gViewportMethods +0x084 (and gNodeGuardedViewportMethods'). Empty body, no C caller of the slot.


## Track 7 (round 100, echo, polish pass)

## Naming

Renamed from `Viewport__func_8003ECC0` with `tools/rename.py`: tier A. The body is empty (`jr $ra; nop`), and an empty occupant's mechanics are its purpose, so the name says it does nothing and which slot of gViewportMethods (+0x084) holds it. The form follows the project's other empty occupants (`NodeGuardedViewport__NoOpSlotB8`, `MoviePlayer__NoOpSlot5C`, `StreamTask__NoOpSlot88`). An empty body says nothing about the slot's arguments (DECOMPILATION_LEARNINGS), so the prototype keeps `(void)`. The slot keeps its offset name `slot84`: nothing calls it.
