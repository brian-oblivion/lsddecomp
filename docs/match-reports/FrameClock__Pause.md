# FrameClock__Pause -- MATCHED (3/3 words), round 82

> Renamed from `D8006EF50__SetFlag10` on 2026-09-26 (tools/rename.py). Address 0x80042658.

> Renamed from `func_80042658` on 2026-09-25 (tools/rename.py). Address 0x80042658.

Round 82, runner alpha (re-staffed slot). Unit `src/graphics/Sprite.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gFrameClockMethods slot +0x04C (`tools/classtable.py`).
- **What:** sets the class-5 object's +0x010 word to 1.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/scene_node.h` (untouched). The FrameClock and RequestedFile objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gFrameClockMethods slot +0x04C. */
void FrameClock__Pause(D_8006EF50Obj *self) {
    self->flag10 = 1;
}
```

## Naming

- `FrameClock__Pause` -- tier A. Slot +0x04C: sets flag10 = 1. Pure setter.

## Track 4 (2026-09-26, round 88, delta)

Renamed from `D8006EF50__SetFlag10`, tier A: sets `paused` (was `flag10`). Evidence: ObjM__AdvancePauseSetup (src/world/ObjMStyleActor.c) calls +0x04C on its +0x010 FrameClock in the same step as it calls +0x04C on its +0x054 WBgm, which is WBgm__Pause (include/WBgm.h); while set, tick sends event 3 instead of 2 and does not count, and DreamSys__TimerTick does not advance on 3. The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnDrawSystemEvent on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/graphics/Sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
