# ScreenSprite__Reset -- MATCHED (2/2 words), round 82

> Renamed from `D8006ED4C__Reset` on 2026-09-25 (tools/rename.py). Address 0x80041da4.

> Renamed from `func_80041DA4` on 2026-09-25 (tools/rename.py). Address 0x80041da4.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gScreenSpriteMethods slot +0x040 (reset, over Class6B5CC__Reset) (`tools/classtable.py`).
- **What:** empty override: `jr $ra; nop`.
- **Result:** byte-exact on the FIRST build; 2/2 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** Class6B5CC-derived methods take `Class6B5CC *` from the UNIFIED
  `include/Class6B5CC.h` (untouched). The D_8006EF50 and D_8006EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* gScreenSpriteMethods slot +0x040 (reset): empty override. */
void ScreenSprite__Reset(Class6B5CC *self) {
}
```

## Naming

- `D8006ED4C__Reset` -- tier A. Reset override (slot +0x040): empty body. A pure leaf whose mechanics (do nothing) ARE its purpose.
