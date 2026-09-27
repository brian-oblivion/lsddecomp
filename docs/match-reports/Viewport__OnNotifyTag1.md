# Viewport__OnNotifyTag1 — MATCHED

> Renamed from `Unk18Obj__OnNotifyTag1` on 2026-09-25 (tools/rename.py). Address 0x8003ee88.

> Renamed from `func_8003EE88` on 2026-09-23 (tools/rename.py). Address 0x8003ee88.

Unit: `Task`. Round 14, runner delta. 14/14 words, full match.

## Signature

```c
void Viewport__OnNotifyTag1(Unk18Obj *self, GenericObj *arg1, s32 arg2);
```

`Unk18ObjMethods`'s own `+0x098` slot occupant (`slot98`, dispatched by
`Viewport__OnNotify`, this round).

## What it does

```c
void Viewport__OnNotifyTag1(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    if (arg2 == 2) {
        self->methods->slotA4(self);
    }
}
```

## Header changes

`include/Task.h`: `Unk18ObjMethods` gains `slotA4` (`+0x0A4`,
`void (*)(Unk18Obj*)`), splitting the pad between `slot9C` and the
already-typed `slotA8`.

## Naming

`Unk18Obj__OnNotifyTag1` -- tier B. The `slot98` occupant `Viewport__OnNotify` dispatches to when the sender's tag is 1; dispatches `slotA4` (`Viewport__Flip`) only when `event == 2`. Same tier-B reasoning as `Viewport__OnNotifyTag5`: named after the dispatch mechanism, not an unestablished event meaning.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__OnNotifyTag1`. Slot +0x098 `onNotifyTag1`, onNotify's DrawSystem (class 1) case: runs +0x0A4 `flip` on event 2. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.


## Track 7 (round 95, alpha, polish pass)

Event 2 is the DrawSystem's per-VSync event, `DRAWSYSTEM_EVENT_VSYNC` on `main` since this round (include/DrawSystem.h); this branch predates it, so the literal is left and the swap is proposed to the head.
