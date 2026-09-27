# Viewport__SetLightMode — MATCHED

> Renamed from `Unk18Obj__SetLightMode` on 2026-09-25 (tools/rename.py). Address 0x8003ea7c.

> Renamed from `Unk18Obj__SetUnk54` on 2026-09-23 (tools/rename.py). Address 0x8003ea7c.

> Renamed from `func_8003EA7C` on 2026-09-23 (tools/rename.py). Address 0x8003ea7c.

Unit: `Task`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetLightMode(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x060` slot occupant.

## What it does

One-instruction field setter.

```c
void Viewport__SetLightMode(Unk18Obj *self, s32 a1) {
    self->unk54 = a1;
}
```

## Header changes

`include/Task.h`: `Unk18Obj` gains `unk54` (`+0x054`).

## Naming

`Unk18Obj__SetLightMode` -- tier A. One-instruction plain field setter for `unk54`, no guard.

**Head review, round 73:** renamed to `Unk18Obj__SetLightMode` at merge, tier A: a plain setter of the `lightMode` field, whose name the runner established from its Sony consumer (`GsSetLightMode` in `Viewport__Update`/`Viewport__Flip`). Any line above saying the function name pre-dates the field rename is superseded.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetLightMode`. Slot +0x060 `setLightMode`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
