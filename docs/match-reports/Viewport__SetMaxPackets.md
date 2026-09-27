# Viewport__SetMaxPackets — MATCHED

> Renamed from `Viewport__SetPacketCount` on 2026-09-27 (tools/rename.py). Address 0x8003ea2c.

> Renamed from `Viewport__SetUnk44` on 2026-09-27 (tools/rename.py). Address 0x8003ea2c.

> Renamed from `Unk18Obj__SetUnk44` on 2026-09-25 (tools/rename.py). Address 0x8003ea2c.

> Renamed from `func_8003EA2C` on 2026-09-23 (tools/rename.py). Address 0x8003ea2c.

Unit: `code_2cc8c`. Round 14, runner delta. 7/7 words, full match.

## Signature

```c
void Viewport__SetMaxPackets(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x04C` slot occupant.

## What it does

Writes `unk44` only the first time — guarded by `self->unk70`, a latch this
unit's queue never itself sets (its own setter, if any, lies outside this
carve).

```c
void Viewport__SetMaxPackets(Unk18Obj *self, s32 a1) {
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

## Proposed field names (round 95, alpha)

Viewport's `unk44` -> `maxPackets`, slot `setUnk44` -> `setMaxPackets`, this function -> `Viewport__SetMaxPackets` (tier B). InitOt sizes each half's packet area as `unk48 * unk44`; the defaults are 2000 and 64, and 64 is a per-primitive byte budget on the scale of Psy-Q's own samples (`PACKETMAX * 24`), while the caller that varies one of them per scene (class_39e08.c, `setUnk44(vp, 1200)`) varies this one. Callers of the slot outside this unit: class_39e08.c, code_2c054.c; the field is accessed only in code_2cc8c.c. Not applied: include/Viewport.h is not this job's.


## Track 7 (round 100, echo, polish pass)

## Naming

Renamed `Viewport__SetUnk44` -> `Viewport__SetPacketCount` -> `Viewport__SetMaxPackets` with `tools/rename.py` (two runs; the second takes round 95's proposal above, whose name says the value is a capacity): tier B. The field is now `maxPackets` (include/Viewport.h, field rule: every accessor is in code_2cc8c.c). Evidence which factor is the count: InitOt sizes each packet area as `packetSize * maxPackets`; the defaults are 2000 and 64, and 64 is a per-primitive byte budget (the largest GPU polygon primitive, POLY_GT4, is 52 bytes) where the other factor is what callers vary per scene (class_39e08.c passes 1200, TaskCore's reset 300); TaskCore (code_2c054.c) already names the value it passes to the other setter `packetSize`; Psy-Q's samples size their packet areas `PACKETMAX * size`. The multiplication itself is symmetric, which is why this stays tier B. Parameter `value` -> `maxPackets`.

## Proposed field names

Slot +0x04C `setUnk44` -> `setMaxPackets` (callers outside this unit: class_39e08.c, code_2c054.c).
