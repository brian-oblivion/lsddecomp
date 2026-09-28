# Viewport__SetOtLength — MATCHED

> Renamed from `Unk18Obj__SetUnk3C` on 2026-09-25 (tools/rename.py). Address 0x8003ea24.

> Renamed from `func_8003EA24` on 2026-09-23 (tools/rename.py). Address 0x8003ea24.

Unit: `task`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetOtLength(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x048` slot occupant (`tools/classtable.py gViewportMethods`).

## What it does

A one-instruction field setter — the whole body is `sw $a1, 0x3C($a0)`.

```c
void Viewport__SetOtLength(Unk18Obj *self, s32 a1) {
    self->unk3C = a1;
}
```

## Header changes

`include/task.h`: `Unk18Obj` gains `s32 unk3C` at `+0x03C` (splits the
`pad034[0x0AC-0x034]` span). See `Viewport__SetMaxPackets`'s report for the sibling
setters carved from the same span in one pass.

## Naming

`Unk18Obj__SetUnk3C` -- tier A. One-instruction (`sw $a1, 0x3C($a0)`) plain field setter, no guard.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnk3C`. Renamed for the field it stores: +0x03C is `otLength`, the GsOT length (InitOt sizes each OT as 4 << otLength and writes it into the GsOT header's `length`; DrawNode sorts backgrounds at priority (1 << otLength) - 1). Slot +0x048 `setOtLength`; TaskCore__OnInit calls it with TaskCore's unk28. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.
