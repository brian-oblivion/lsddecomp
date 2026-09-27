# Viewport__NoOpSlot58 — MATCHED

> Renamed from `Viewport__func_8003EA6C` on 2026-09-27 (tools/rename.py). Address 0x8003ea6c.

> Renamed from `func_8003EA6C` on 2026-09-27 (tools/rename.py). Address 0x8003ea6c.

Unit: `code_2cc8c_d`. Round 73, runner charlie (report written retroactively;
the function itself was already byte-exact under `INCLUDE_ASM` before this
round -- an empty body, `jr $ra; nop`, that splat's own extraction produced
without any hand-written C being required).

## Signature

```c
void Viewport__NoOpSlot58(void);
```

Not a `gViewportMethods` vtable slot occupant (`tools/classtable.py gViewportMethods`
lists no entry at any offset resolving to this address) and not a method --
it takes no `self` at all. No caller found anywhere in `src/` or the
remaining `asm/*.s` segments as of this round.

## What it does

Nothing: the whole body is `jr $ra; nop`. CLAUDE.md's own caution applies
verbatim here -- "not every matched function was work."

## Naming

Kept `Viewport__NoOpSlot58`. No evidence of any kind: no `self` parameter to tie it
to `Unk18Obj` or any other class, no caller, no vtable slot. Tier-C's
`Class__func_xxxxx` form does not apply because no class is established.
Nothing to propose.

## Track 4 (2026-09-25, round 85, bravo)

Occupant of Viewport's +0x058 (`slot58` in `include/Viewport.h`; also in gNodeGuardedViewportMethods). Empty and never called, so neither the slot nor the function is named. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.


## Track 7 (round 95, alpha, polish pass)

## Naming

Renamed from `func_8003EA6C` with `tools/rename.py`: tier C, `Class__func_xxxxx`. The class IS established now: it occupies gViewportMethods +0x058 (and gNodeGuardedViewportMethods'), so the round-73 note above that no class applies is superseded. The body is empty and no C calls the slot, so no purpose is shown.


## Track 7 (round 100, echo, polish pass)

## Naming

Renamed from `Viewport__func_8003EA6C` with `tools/rename.py`: tier A. The body is empty (`jr $ra; nop`), and an empty occupant's mechanics are its purpose, so the name says it does nothing and which slot of gViewportMethods (+0x058) holds it. The form follows the project's other empty occupants (`NodeGuardedViewport__NoOpSlotB8`, `MoviePlayer__NoOpSlot5C`, `StreamTask__NoOpSlot88`). An empty body says nothing about the slot's arguments (DECOMPILATION_LEARNINGS), so the prototype keeps `(void)`. The slot keeps its offset name `slot58`: nothing calls it.
