# D8006EF50__GetFlag10 -- MATCHED (3/3 words), round 82

> Renamed from `func_8004266C` on 2026-09-25 (tools/rename.py). Address 0x8004266c.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** D_8006EF50 slot +0x054 (`tools/classtable.py`).
- **What:** returns the class-5 object's +0x010 word.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** Class6B5CC-derived methods take `Class6B5CC *` from the UNIFIED
  `include/Class6B5CC.h` (untouched). The D_8006EF50 and D_8006EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* D_8006EF50 slot +0x054. */
s32 D8006EF50__GetFlag10(D_8006EF50Obj *self) {
    return self->unk10;
}
```
