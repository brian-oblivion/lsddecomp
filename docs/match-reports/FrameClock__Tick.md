# FrameClock__Tick -- MATCHED (24/24 words), round 82

> Renamed from `D8006EF50__Tick` on 2026-09-26 (tools/rename.py). Address 0x800425ec.

> Renamed from `func_800425EC` on 2026-09-25 (tools/rename.py). Address 0x800425ec.

Round 82, runner alpha (fourth slot on Sprite). Unit `src/graphics/Sprite.c`. Fresh ground, no prior attempt.

- **Where:** gFrameClockMethods slot +0x044.
- **What:** picks an event -- 4 if `flag14` (set by slot +0x058), else 3 if `flag10` (slot +0x04C), else increments the counter `count` and uses 2 -- and calls the class's own notifyParents (slot +0x030, FrameClock__NotifyParents) with `(self, event)`.
- **Result:** byte-exact, 24/24 words, 0 ins / 0 del, whole-image SHA1 green. First build: an `if / else if / else` with the event in one local; the constants land in the branch delay slots by themselves.
- **Types:** the unit-local `D_8006EF50Methods` gained `notifyParents` at +0x030; no shared header touched.

## Source

```c
struct D_8006EF50Methods {
    u8 pad00[0x30];
    void (*notifyParents)(D_8006EF50Obj *self, s32 event); /* +0x030 = FrameClock__NotifyParents */
    u8 pad34[0x40 - 0x34];
    void (*reset)(D_8006EF50Obj *self, s32 a1);            /* +0x040 = FrameClock__Reset */
};

void FrameClock__Tick(D_8006EF50Obj *self) {
    s32 event;

    if (self->flag14 != 0) {
        event = 4;
    } else if (self->flag10 != 0) {
        event = 3;
    } else {
        self->count++;
        event = 2;
    }
    self->methods->notifyParents(self, event);
}
```

## Naming

- `FrameClock__Tick` -- tier B. Slot +0x044: notify event 4 if flag14, else 3 if flag10, else increment count and notify 2. Mechanics fully described (round-82 broadcast: "notifyParents picks events 4/3/2"); named by analogy with the project's existing per-frame/per-cycle "Tick" methods (IntermediateBase__IncrementFrameCounter, DreamSys__TimerTick), but the game-level meaning of the counter and the two flags is not independently confirmed, hence tier B.

## Track 4 (2026-09-26, round 88, delta)

Renamed from `D8006EF50__Tick`; slot +0x044 `tick`. Its one caller in C is IntermediateBase__OnDrawSystemEvent (src/app/Task.c), on the DrawSystem's (class nibble 1) event 2, which DrawSystem sends once per VSync-loop pass (include/DrawSystem.h). That caller now reaches it through `FrameClock`'s table rather than the `IntermediateBaseLinked` view's `slot44`. Tier A now: the listeners' reactions are read (Viewport__OnNotifyTag5 redraws on 2 and 3, DreamSys__TimerTick advances only on 2, TodActor__Update releases on 4). The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnDrawSystemEvent on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/graphics/Sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, charlie)

Events 4/3/2 spelled with FrameClock.h's existing `enum FrameClockEvent` (FLAG14, PAUSED, RUNNING). Byte-exact.
