# Viewport__SetDrawEnabled — MATCHED

> Renamed from `Unk18Obj__SetUnkB8` on 2026-09-25 (tools/rename.py). Address 0x8003f244.

> Renamed from `func_8003F244` on 2026-09-23 (tools/rename.py). Address 0x8003f244.

Unit: `TaskViewport`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetDrawEnabled(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x0B4` slot occupant.

## What it does

One-instruction field setter, sibling of `Viewport__SetExtraSwap`.

```c
void Viewport__SetDrawEnabled(Unk18Obj *self, s32 a1) {
    self->unkB8 = a1;
}
```

## Header changes

`include/TaskViewport.h`: `Unk18Obj` gains `unkB8` (`+0x0B8`).

## Naming

`Unk18Obj__SetUnkB8` -- tier A. One-instruction plain field setter for `unkB8`, no guard.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnkB8`. Renamed for the field it stores: +0x0B8 is `drawEnabled`, which Viewport__Flip tests before resetting, clearing and drawing (0 skips them). Default 1 (InitDefaults). Slot +0x0B4 `setDrawEnabled`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
