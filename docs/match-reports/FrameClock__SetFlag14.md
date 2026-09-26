# FrameClock__SetFlag14 -- MATCHED (3/3 words), round 82

> Renamed from `D8006EF50__SetFlag14` on 2026-09-26 (tools/rename.py). Address 0x80042678.

> Renamed from `func_80042678` on 2026-09-25 (tools/rename.py). Address 0x80042678.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gFrameClockMethods slot +0x058 (`tools/classtable.py`).
- **What:** sets the class-5 object's +0x014 word to 1.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** Class6B5CC-derived methods take `Class6B5CC *` from the UNIFIED
  `include/Class6B5CC.h` (untouched). The FrameClock and Class6EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gFrameClockMethods slot +0x058. */
void FrameClock__SetFlag14(D_8006EF50Obj *self) {
    self->flag14 = 1;
}
```

## Naming

- `FrameClock__SetFlag14` -- tier A. Slot +0x058: sets flag14 = 1. Pure setter.
