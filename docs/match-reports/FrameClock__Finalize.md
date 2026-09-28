# FrameClock__Finalize -- MATCHED (14/14 words), round 82

> Renamed from `D8006EF50__Finalize` on 2026-09-26 (tools/rename.py). Address 0x800424a8.

> Renamed from `func_800424A8` on 2026-09-25 (tools/rename.py). Address 0x800424a8.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/graphics/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** gFrameClockMethods slot +0x00C (finalize) (`tools/classtable.py`).
- **What:** `GetBasicClassMethods()->finalize(self)`, typed through the UNIFIED `include/BasicClass.h` (reached via `SceneNode.h`).
- **Result:** byte-exact; 14/14 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* gFrameClockMethods slot +0x00C (finalize): the BasicClass finalize. */
void FrameClock__Finalize(BasicClass *self) {
    GetBasicClassMethods()->finalize(self);
}
```

## Naming

- `FrameClock__Finalize` -- tier A. Finalize (slot +0x00C): the BasicClass finalize, no local state to release.

## Track 4 (2026-09-26, round 88, delta)

Renamed from `D8006EF50__Finalize`: override of +0x00C, named for its slot. `self` is now `FrameClock *`, upcast for the base call. The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnDrawSystemEvent on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/graphics/Sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
