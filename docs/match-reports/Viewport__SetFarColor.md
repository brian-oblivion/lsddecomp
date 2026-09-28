# Viewport__SetFarColor — MATCHED

> Renamed from `Unk18Obj__SetFarColor` on 2026-09-25 (tools/rename.py). Address 0x8003eaa4.

> Renamed from `Unk18Obj__SetUnk5B` on 2026-09-23 (tools/rename.py). Address 0x8003eaa4.

> Renamed from `func_8003EAA4` on 2026-09-23 (tools/rename.py). Address 0x8003eaa4.

Unit: `task`. Round 14, runner delta. 8/8 words, full match.

## Signature

```c
void Viewport__SetFarColor(Unk18Obj *self, SByte3_d294 *src);
```

`Unk18ObjMethods`'s own `+0x068` slot occupant.

## What it does

Sibling of `Viewport__SetClearColor` (same report has the full derivation — signed
bytes, whole-struct assignment), writing `self->unk5B` instead of
`self->unk58`.

```c
void Viewport__SetFarColor(Unk18Obj *self, SByte3_d294 *src) {
    self->unk5B = *src;
}
```

## Header changes

`include/task.h`: `Unk18Obj` gains `unk5B` (`+0x05B`, `SByte3_d294`).

## Naming

`Unk18Obj__SetFarColor` -- tier A. Sibling of `Viewport__SetClearColor`: same whole-struct-assignment shape, writes `unk5B`.

**Head review, round 73:** renamed to `Unk18Obj__SetFarColor` at merge, tier A: a plain setter of the `farColor` field, whose name the runner established from its Sony consumer (`SetFarColor` in `Viewport__Update`/`Viewport__Flip`). Any line above saying the function name pre-dates the field rename is superseded.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetFarColor`. Slot +0x068 `setFarColor`; the colour is a `ViewportRgb *` (the former SByte3_d294). The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Track 10 (2026-09-28, round 104, echo)

The six per-class aliases of `ColorRgb` (include/draw_system.h) -- BgLayerRgb, BoxFillRgb, FlatLightColor, LightRigRgb, ViewportRgb, TimBlockSrcColor -- are deleted and every use is spelled `ColorRgb`. Byte-identical. Measured for the MATCHING line in TaskCore__SetColors: the whole-struct copy is three `lb` then three `sb`, and rewriting one of the copies byte by byte loads each byte with `lbu` and interleaves the stores (asm-differ on the experiment), so the struct copy stays; the old line's "signed bytes" was wrong (ColorRgb's channels are u8; the lb comes from the block copy, not the type), and the same claim in GraphRoom__BuildGraphPoints' colour comment is corrected.
