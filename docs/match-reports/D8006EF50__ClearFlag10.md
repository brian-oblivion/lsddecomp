# D8006EF50__ClearFlag10 -- MATCHED (2/2 words), round 82

> Renamed from `func_80042664` on 2026-09-25 (tools/rename.py). Address 0x80042664.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** D_8006EF50 slot +0x050 (`tools/classtable.py`).
- **What:** clears the class-5 object's +0x010 word (`sw $zero, 0x10($a0)` in the delay slot).
- **Result:** byte-exact on the FIRST build; 2/2 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** Class6B5CC-derived methods take `Class6B5CC *` from the UNIFIED
  `include/Class6B5CC.h` (untouched). The D_8006EF50 and Class6EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* D_8006EF50 slot +0x050. */
void D8006EF50__ClearFlag10(D_8006EF50Obj *self) {
    self->flag10 = 0;
}
```

## Naming

- `D8006EF50__ClearFlag10` -- tier A. Slot +0x050: sets flag10 = 0. Pure setter.
