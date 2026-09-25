# D8006EFAC__Reset -- MATCHED (3/3 words), round 82

> Renamed from `func_80042814` on 2026-09-25 (tools/rename.py). Address 0x80042814.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** D_8006EFAC slot +0x040 (reset, over Class6B5CC__Reset) (`tools/classtable.py`).
- **What:** `self->coord2->flg = 0`: marks the GsCOORDINATE2 for recompute, the same thing updateRotation/updateScale/attachToParent do.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** Class6B5CC-derived methods take `Class6B5CC *` from the UNIFIED
  `include/Class6B5CC.h` (untouched). The D_8006EF50 and D_8006EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* D_8006EFAC slot +0x040 (reset): mark the coordinate for recompute. */
void D8006EFAC__Reset(Class6B5CC *self) {
    self->coord2->flg = 0;
}
```

## Naming

- `D8006EFAC__Reset` -- tier A. Reset override (slot +0x040): marks the Class6B5CC coordinate dirty (coord2->flg = 0, include/Class6B5CC.h's documented "0 = recompute").
