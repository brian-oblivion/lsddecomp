# D8006EF50__Tick -- MATCHED (24/24 words), round 82

> Renamed from `func_800425EC` on 2026-09-25 (tools/rename.py). Address 0x800425ec.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** D_8006EF50 slot +0x044.
- **What:** picks an event -- 4 if `flag14` (set by slot +0x058), else 3 if `flag10` (slot +0x04C), else increments the counter `count` and uses 2 -- and calls the class's own notifyParents (slot +0x030, D8006EF50__NotifyParents) with `(self, event)`.
- **Result:** byte-exact, 24/24 words, 0 ins / 0 del, whole-image SHA1 green. First build: an `if / else if / else` with the event in one local; the constants land in the branch delay slots by themselves.
- **Types:** the unit-local `D_8006EF50Methods` gained `notifyParents` at +0x030; no shared header touched.

## Source

```c
struct D_8006EF50Methods {
    u8 pad00[0x30];
    void (*notifyParents)(D_8006EF50Obj *self, s32 event); /* +0x030 = D8006EF50__NotifyParents */
    u8 pad34[0x40 - 0x34];
    void (*reset)(D_8006EF50Obj *self, s32 a1);            /* +0x040 = D8006EF50__Reset */
};

void D8006EF50__Tick(D_8006EF50Obj *self) {
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

- `D8006EF50__Tick` -- tier B. Slot +0x044: notify event 4 if flag14, else 3 if flag10, else increment count and notify 2. Mechanics fully described (round-82 broadcast: "notifyParents picks events 4/3/2"); named by analogy with the project's existing per-frame/per-cycle "Tick" methods (IntermediateBase__IncrementFrameCounter, DreamSys__TimerTick), but the game-level meaning of the counter and the two flags is not independently confirmed, hence tier B.
