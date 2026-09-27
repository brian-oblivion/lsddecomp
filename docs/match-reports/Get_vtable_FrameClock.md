# Get_vtable_FrameClock -- MATCHED (4/4 words), round 82

> Renamed from `Get_vtable_D8006EF50` on 2026-09-26 (tools/rename.py). Address 0x80042684.

> Renamed from `func_80042684` on 2026-09-25 (tools/rename.py). Address 0x80042684.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py`).
- **What:** Returns the gFrameClockMethods method table.
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* Returns the gFrameClockMethods method table. */
void *Get_vtable_FrameClock(void) {
    return gFrameClockMethods;
}
```

## Naming

- `Get_vtable_FrameClock` -- tier A. Table getter ("return gFrameClockMethods;").

## Track 4 (2026-09-26, round 88, delta)

Renamed from `Get_vtable_D8006EF50`, kept the `Get_vtable_` form the class already had. Now returns `FrameClockMethods *` (`&gFrameClockMethods`) instead of `void *` over an `s32[]` extern. The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnTag1Notify on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/Sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
