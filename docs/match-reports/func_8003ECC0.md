# func_8003ECC0 — MATCHED

Unit: `code_2cc8c_d`. Round 73, runner charlie (report written retroactively;
same shape as `func_8003EA6C`/`func_8003EA74`).

## Signature

```c
void func_8003ECC0(void);
```

Not a `gViewportMethods` vtable slot occupant, not a method (no `self`), no caller
found anywhere in `src/` or the remaining `asm/*.s` segments.

## What it does

Nothing: `jr $ra; nop`.

## Naming

Kept `func_8003ECC0`. No evidence of any kind.

## Track 4 (2026-09-25, round 85, bravo)

Occupant of Viewport's +0x084 (`slot84` in `include/Viewport.h`; also in gClass869D8Methods). Empty and never called, so neither the slot nor the function is named. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
