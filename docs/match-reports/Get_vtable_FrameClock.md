# Get_vtable_FrameClock -- MATCHED (4/4 words), round 82

> Renamed from `Get_vtable_D8006EF50` on 2026-09-26 (tools/rename.py). Address 0x80042684.

> Renamed from `func_80042684` on 2026-09-25 (tools/rename.py). Address 0x80042684.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

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
