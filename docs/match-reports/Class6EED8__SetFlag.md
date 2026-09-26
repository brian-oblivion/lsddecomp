# Class6EED8__SetFlag -- MATCHED (3/3 words), round 82

> Renamed from `D8006EED8__SetFlag2C` on 2026-09-26 (tools/rename.py). Address 0x800423e4.

> Renamed from `func_800423E4` on 2026-09-25 (tools/rename.py). Address 0x800423e4.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gClass6EED8Methods slot +0x064 (a Class6D430-derived table, id 0xB03) (`tools/classtable.py`).
- **What:** sets the object's +0x02C word to 1.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** Class6B5CC-derived methods take `Class6B5CC *` from the UNIFIED
  `include/Class6B5CC.h` (untouched). The D_8006EF50 and Class6EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gClass6EED8Methods slot +0x064. */
void Class6EED8__SetFlag(D_8006EED8Obj *self) {
    self->flag2C = 1;
}
```

## Naming

- `Class6EED8__SetFlag` -- tier A. Slot +0x064: sets flag2C = 1. A pure setter; the deeper game meaning of flag2C is not established (kept as flagNN rather than invented), but the setter's own mechanics ARE its purpose.
