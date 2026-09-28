# Viewport__SetExtraSwap — MATCHED

> Renamed from `Viewport__SetExtraSwapOnBuffer0` on 2026-09-27 (tools/rename.py). Address 0x8003f23c.

> Renamed from `Viewport__SetUnkB4` on 2026-09-27 (tools/rename.py). Address 0x8003f23c.

> Renamed from `Unk18Obj__SetUnkB4` on 2026-09-25 (tools/rename.py). Address 0x8003f23c.

> Renamed from `func_8003F23C` on 2026-09-23 (tools/rename.py). Address 0x8003f23c.

Unit: `task`. Round 14, runner delta. 2/2 words, full match.

## Signature

```c
void Viewport__SetExtraSwap(Unk18Obj *self, s32 a1);
```

`Unk18ObjMethods`'s own `+0x0B0` slot occupant.

## What it does

One-instruction field setter.

```c
void Viewport__SetExtraSwap(Unk18Obj *self, s32 a1) {
    self->unkB4 = a1;
}
```

## Header changes

`include/task.h`: `Unk18Obj` gains `unkB4` (`+0x0B4`), after the
existing `unkB0` field.

## Naming

`Unk18Obj__SetUnkB4` -- tier A. One-instruction plain field setter for `unkB4`, no guard.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__SetUnkB4`. Slot +0x0B0 `setUnkB4`. unkB4 makes Flip swap once more on buffer 0; what that is for is not shown. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.

## Proposed field names (round 95, alpha)

Viewport's `unkB4` -> `extraSwap`, slot `setUnkB4` -> `setExtraSwap`, this function -> `Viewport__SetExtraSwap` (tier B: Flip swaps the DrawSystem's buffers once more on buffer 0, before and after the draw, while it is set; why is not shown). Slot caller outside this unit: ObjMStyleActor.c.


## Track 7 (round 100, echo, polish pass)

## Naming

Renamed `Viewport__SetUnkB4` -> `Viewport__SetExtraSwapOnBuffer0` -> `Viewport__SetExtraSwap` with `tools/rename.py` (the second run takes round 95's proposal above): tier B. The field is now `extraSwap` (include/Viewport.h; every accessor is in task.c): while it is set, Flip calls the DrawSystem's swapBuffers once more before the clear and once more after the draw on buffer 0. What that is for is not shown; the only caller in C (ObjMStyleActor.c) passes 0, as InitDefaults does. Parameter `value` -> `on`.

## Proposed field names

Slot +0x0B0 `setUnkB4` -> `setExtraSwap` (caller outside this unit: ObjMStyleActor.c).
