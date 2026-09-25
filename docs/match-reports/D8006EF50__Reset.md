# D8006EF50__Reset -- MATCHED (5/5 words), round 82

> Renamed from `func_800425D8` on 2026-09-25 (tools/rename.py). Address 0x800425d8.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EF50 slot +0x040 (reset) (`tools/classtable.py`).
- **What:** Stores its argument at +0x0C and clears +0x14, +0x10, +0x18, in that source order (retail order). Added `parentCursor` to the unit-local `D_8006EF50Obj` view.
- **Result:** byte-exact; 5/5 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* D_8006EF50 slot +0x040 (reset). */
void D8006EF50__Reset(D_8006EF50Obj *self, s32 a1) {
    self->count = a1;
    self->flag14 = 0;
    self->flag10 = 0;
    self->parentCursor = 0;
}
```

## Naming

- `D8006EF50__Reset` -- tier A. Slot +0x040: sets count from the caller's argument and clears flag14/flag10/parentCursor.
