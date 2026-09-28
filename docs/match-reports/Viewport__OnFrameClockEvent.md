# Viewport__OnFrameClockEvent — MATCHED

> Renamed from `Viewport__OnNotifyTag5` on 2026-09-28 (tools/rename.py). Address 0x8003ee40.

> Renamed from `Unk18Obj__OnNotifyTag5` on 2026-09-25 (tools/rename.py). Address 0x8003ee40.

> Renamed from `func_8003EE40` on 2026-09-23 (tools/rename.py). Address 0x8003ee40.

Unit: `Task`. Round 14, runner delta. 18/18 words, full match.

## Signature

```c
void Viewport__OnFrameClockEvent(Unk18Obj *self, GenericObj *arg1, s32 arg2);
```

`Unk18ObjMethods`'s own `+0x094` slot occupant (`slot94`, dispatched by
`Viewport__OnNotify`, this round).

## What it does

Increments `self->unk90` unconditionally, and additionally dispatches
`self->methods->slot9C` when `arg2` is 2 or 3.

```c
void Viewport__OnFrameClockEvent(Unk18Obj *self, GenericObj *arg1, s32 arg2) {
    self->unk90 = self->unk90 + 1;
    if (arg2 == 2 || arg2 == 3) {
        self->methods->slot9C(self);
    }
}
```

Note: retail's own `(unsigned)(arg2 - 2) < 2` range-check fold (a single
`sltiu`) is exactly what `arg2 == 2 || arg2 == 3` compiles to here — the
OPPOSITE lesson from this round's `SceneNode__TryAttachNearby`/`SceneNode__NotifyWithHull` (a
different unit), where the same fold was UNWANTED and had to be avoided
with guard clauses. Whether the fold is wanted is purely a property of
what retail's own disassembly shows, not a general rule either way.

## Header changes

`include/Task.h`:
- `Unk18Obj` gains `unk90` (`+0x090`, `s32`).
- `Unk18ObjMethods` gains `slot9C` (`+0x09C`, `void (*)(Unk18Obj*)`),
  splitting the `pad09C` span added alongside `slot94`/`slot98` earlier
  this round.

## Naming

`Unk18Obj__OnNotifyTag5` -- tier B. The `slot94` occupant `Viewport__OnNotify` dispatches to when the sender's dynamic-class tag is 5. Body always increments `unk90` and additionally dispatches `slot9C` (`Viewport__Update`) when `event` is 2 or 3. Named after the dispatch mechanism (which tag reaches it), not after what tag 5 or event codes 2/3 mean in the game -- that is not established.

## Track 4 (2026-09-25, round 85, bravo)

Renamed from `Unk18Obj__OnNotifyTag5`. Slot +0x094 `onNotifyTag5`, onNotify's class-5 (FrameClock) case: counts in unk90 and runs +0x09C `update` on events 2 and 3. The class (id 0x7, table `gViewportMethods`, formerly `D_8006E8E4`) is unified as `Viewport` in `include/Viewport.h`, whose banner gives the evidence for the name: its methods hold a GsRVIEW2 (GsSetRefView2), the projection and near clip, a double-buffered GsOT pair, draw the scene tree into it and flip it; IntermediateBase and TaskCore already called the field holding it `viewport`. Any source block above is the pre-unification spelling; the live body takes the unified types and field and slot names, byte-identical.


## Track 7 (round 95, alpha, polish pass)

Events 2 and 3 are `FRAMECLOCK_EVENT_RUNNING` and `FRAMECLOCK_EVENT_PAUSED`, a new `enum FrameClockEvent` in include/FrameClock.h (values from FrameClock__Tick, the clock's own banner). Byte-identical.

## Proposed field names (round 95, alpha)

Viewport's `unk90` -> `clockEventCount` (tier A: this function increments it on every FrameClock event and InitDefaults zeroes it; nothing in this unit reads it).


## Track 7 (round 100, echo, polish pass)

Round 95's proposal applied: `unk90` -> `clockEventCount` (include/Viewport.h; the accessors are this function and InitDefaults, both in Task.c).
