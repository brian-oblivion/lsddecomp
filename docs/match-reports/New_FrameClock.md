# New_FrameClock -- MATCHED (20/20 words), round 82

> Renamed from `New_D8006EF50` on 2026-09-26 (tools/rename.py). Address 0x80042400.

> Renamed from `func_80042400` on 2026-09-25 (tools/rename.py). Address 0x80042400.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/graphics/sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (allocator) (`tools/classtable.py`).
- **What:** `BMemPMgrAlloc(0x1C)`; if non-NULL, calls slot +0x008 of `GetFrameClockMethods()` (the gFrameClockMethods table) on it and returns it, else NULL. Delta's round-82 allocator shape (`if (obj != NULL) { ctor; return obj; } return NULL;`) matched first build.
- **Result:** byte-exact; 20/20 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
/* Allocate and construct a FrameClock object (0x1C bytes). */
void *New_FrameClock(void) {
    void *obj = BMemPMgrAlloc(0x1C);

    if (obj != NULL) {
        ((Slot08Methods_322b4 *)GetFrameClockMethods())->init(obj);
        return obj;
    }
    return NULL;
}
```

## Naming

- `New_FrameClock` -- tier A. Allocator: BMemPMgrAlloc(0x1C) then the ctor slot.

## Track 4 (2026-09-26, round 88, delta)

Renamed from `New_D8006EF50`: the allocator (BMemPMgrAlloc(0x1C), then the table's ctor). Now returns `FrameClock *` and calls `GetFrameClockMethods()->ctor(obj)` instead of the unit-local `Slot08Methods_322b4` view. Its two callers (IntermediateBase__Init, DayTask__DayTask) upcast the result into their own `BasicClass *` / `SubObjG *` fields. The class (id 0x5, table `gFrameClockMethods`, formerly `D_8006EF50`) is unified as `FrameClock` in `include/FrameClock.h`, whose banner holds the evidence for the class name: its tick (+0x044) is called by IntermediateBase__OnDrawSystemEvent on the DrawSystem's per-VSync event 2, counts one frame and tells its parents event 2, or 3 while paused, or 4 while flag14 is set. Fields renamed: `count` -> `frameCount`, `flag10` -> `paused`. Any source block above is the pre-unification spelling (`D_8006EF50Obj`); the live body in `src/graphics/sprite.c` takes `FrameClock *`, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Track 7 (round 99, charlie)

Allocation size spelled `sizeof(FrameClock)` (0x1C). Byte-exact.
