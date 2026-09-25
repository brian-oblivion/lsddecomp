# Sprite__Update -- MATCHED (2/2 words), round 82

> Renamed from `func_80042294` on 2026-09-25 (tools/rename.py). Address 0x80042294.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** D_8006EB90 and D_8006EC74 slot +0x098 (update, over func_8001D6AC) (`tools/classtable.py`).
- **What:** empty override: `jr $ra; nop`.
- **Result:** byte-exact on the FIRST build; 2/2 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** Class6B5CC-derived methods take `Class6B5CC *` from the UNIFIED
  `include/Class6B5CC.h` (untouched). The D_8006EF50 and D_8006EED8 objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* D_8006EB90 and D_8006EC74 slot +0x098 (update): empty override. */
void Sprite__Update(Class6B5CC *self, void *sender, s32 event) {
}
```
