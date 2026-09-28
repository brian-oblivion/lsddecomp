# FrameClock__Resume -- MATCHED (2/2 words), round 82

> Renamed from `D8006EF50__ClearFlag10` on 2026-09-26 (tools/rename.py). Address 0x80042664.

> Renamed from `func_80042664` on 2026-09-25 (tools/rename.py). Address 0x80042664.

Round 82, runner alpha (re-staffed slot). Unit `src/graphics/Sprite.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gFrameClockMethods slot +0x050 (`tools/classtable.py`).
- **What:** clears the class-5 object's +0x010 word (`sw $zero, 0x10($a0)` in the delay slot).
- **Result:** byte-exact on the FIRST build; 2/2 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/SceneNode.h` (untouched). The FrameClock and RequestedFile objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gFrameClockMethods slot +0x050. */
void FrameClock__Resume(D_8006EF50Obj *self) {
    self->flag10 = 0;
}
```

## Naming

- `FrameClock__Resume` -- tier A. Slot +0x050: sets flag10 = 0. Pure setter.

## Track 4 (2026-09-26, round 88, delta)

Renamed from `D8006EF50__ClearFlag10`, tier A: clears `paused`. Evidence: ObjM__TeardownPauseOverlay calls +0x050 on its FrameClock beside +0x050 on its WBgm (WBgm__Resume). The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnDrawSystemEvent on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/graphics/Sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
