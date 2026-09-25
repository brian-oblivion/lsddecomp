# Viewport__SetUnk44 — MATCHED

> Renamed from `Unk18Obj__SetUnk44` on 2026-09-25 (tools/rename.py). Address 0x8003ea2c.

> Renamed from `func_8003EA2C` on 2026-09-23 (tools/rename.py). Address 0x8003ea2c.

Unit: `code_2cc8c_d`. Round 14, runner delta. 7/7 words, full match.

## Signature

```c
void Viewport__SetUnk44(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x04C` slot occupant.

## What it does

Writes `unk44` only the first time — guarded by `self->unk70`, a latch this
unit's queue never itself sets (its own setter, if any, lies outside this
carve).

```c
void Viewport__SetUnk44(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk44 = a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk44` (`+0x044`) and `unk70`
(`+0x070`, the guard flag; no evidence of its own set site within this
unit). Same carve pass as `Viewport__SetOtLength`/`EA48`/`EA64`/`EA7C`/`EAC4`
(`+0x03C..+0x060` span), all typed `s32` for lack of further evidence
beyond "a stored word".

## Naming

`Unk18Obj__SetUnk44` -- tier A. Plain setter for `unk44`, guarded by the `unk70` one-time-init latch (only writes the first time). The guard is part of the mechanics; the field's real meaning is unestablished.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnk44`. Slot +0x04C `setUnk44`. unk44 * unk48 is each buffer's packet area (InitOt); which of the two is the packet count and which the size is not shown, so the field keeps its offset name. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
