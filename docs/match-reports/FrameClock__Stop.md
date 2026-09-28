# FrameClock__Stop -- MATCHED (3/3 words), round 82

> Renamed from `FrameClock__SetFlag14` on 2026-09-28 (tools/rename.py). Address 0x80042678.

> Renamed from `D8006EF50__SetFlag14` on 2026-09-26 (tools/rename.py). Address 0x80042678.

> Renamed from `func_80042678` on 2026-09-25 (tools/rename.py). Address 0x80042678.

Round 82, runner alpha (re-staffed slot). Unit `src/graphics/sprite.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gFrameClockMethods slot +0x058 (`tools/classtable.py`).
- **What:** sets the class-5 object's +0x014 word to 1.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/scene_node.h` (untouched). The FrameClock and RequestedFile objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gFrameClockMethods slot +0x058. */
void FrameClock__Stop(D_8006EF50Obj *self) {
    self->flag14 = 1;
}
```

## Naming

- `FrameClock__Stop` -- tier A. Slot +0x058: sets flag14 = 1. Pure setter.

## Track 4 (2026-09-26, round 88, delta)

Kept its tier-C name (renamed only for the class prefix, from `D8006EF50__SetFlag14`): it sets `flag14`, which makes tick send event 4 and not count; TodActor__Update releases itself on 4 and Viewport__OnFrameClockEvent skips its redraw, but no caller of +0x058 is found in C, so what the flag means in the game is not established. The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/frame_clock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnDrawSystemEvent on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/graphics/sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
