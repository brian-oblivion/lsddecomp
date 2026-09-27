# FrameClock__Reset -- MATCHED (5/5 words), round 82

> Renamed from `D8006EF50__Reset` on 2026-09-26 (tools/rename.py). Address 0x800425d8.

> Renamed from `func_800425D8` on 2026-09-25 (tools/rename.py). Address 0x800425d8.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** gFrameClockMethods slot +0x040 (reset) (`tools/classtable.py`).
- **What:** Stores its argument at +0x0C and clears +0x14, +0x10, +0x18, in that source order (retail order). Added `parentCursor` to the unit-local `D_8006EF50Obj` view.
- **Result:** byte-exact; 5/5 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* gFrameClockMethods slot +0x040 (reset). */
void FrameClock__Reset(D_8006EF50Obj *self, s32 a1) {
    self->count = a1;
    self->flag14 = 0;
    self->flag10 = 0;
    self->parentCursor = 0;
}
```

## Naming

- `FrameClock__Reset` -- tier A. Slot +0x040: sets count from the caller's argument and clears flag14/flag10/parentCursor.

## Track 4 (2026-09-26, round 88, delta)

Renamed from `D8006EF50__Reset`: its own slot +0x040 `reset`. Parameter `a1` named `frameCount` (it is stored there; the ctor passes 0). The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnTag1Notify on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/Sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
