# Viewport__SetProjection — MATCHED

> Renamed from `Unk18Obj__SetUnk40` on 2026-09-25 (tools/rename.py). Address 0x8003ea64.

> Renamed from `func_8003EA64` on 2026-09-23 (tools/rename.py). Address 0x8003ea64.

Unit: `task`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetProjection(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x054` slot occupant.

## What it does

One-instruction field setter, the sibling of `Viewport__SetOtLength`.

```c
void Viewport__SetProjection(Unk18Obj *self, s32 a1) {
    self->unk40 = a1;
}
```

## Header changes

`include/task.h`: `Unk18Obj` gains `unk40` (`+0x040`).

## Naming

`Unk18Obj__SetUnk40` -- tier A. One-instruction plain field setter for `unk40`, no guard.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnk40`. Renamed for the field it stores: +0x040 is `projH`, the projection distance Viewport__Update passes to GsSetProjection and DrawNode multiplies sprite positions by (DrawView's `projH`). Slot +0x054 `setProjection`. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
