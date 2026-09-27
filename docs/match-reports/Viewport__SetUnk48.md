# Viewport__SetUnk48 — MATCHED

> Renamed from `Unk18Obj__SetUnk48` on 2026-09-25 (tools/rename.py). Address 0x8003ea48.

> Renamed from `func_8003EA48` on 2026-09-23 (tools/rename.py). Address 0x8003ea48.

Unit: `code_2cc8c_d`. Round 14, runner delta. 7/7 words, full match.

## Signature

```c
void Viewport__SetUnk48(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x050` slot occupant.

## What it does

Same guarded-setter shape as `Viewport__SetPacketCount`, writing `unk48` instead of
`unk44`, under the same `self->unk70` latch.

```c
void Viewport__SetUnk48(Unk18Obj *self, s32 a1) {
    if (self->unk70 == 0) {
        self->unk48 = a1;
    }
}
```

## Header changes

`include/code_2cc8c.h`: `Unk18Obj` gains `unk48` (`+0x048`), part of the
same carve pass as `Viewport__SetPacketCount`.

## Naming

`Unk18Obj__SetUnk48` -- tier A. Sibling of `Viewport__SetPacketCount`: same `unk70`-guarded setter shape, writes `unk48`.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnk48`. Slot +0x050 `setUnk48`; see Viewport__SetPacketCount. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Proposed field names (round 95, alpha)

Viewport's `unk48` -> `packetSize`, slot `setUnk48` -> `setPacketSize`, this function -> `Viewport__SetPacketSize` (tier B; see Viewport__SetPacketCount.md for the evidence and which factor is which). Slot caller outside this unit: code_2c054.c.
