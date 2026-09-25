# D8006EFAC__GetChild -- MATCHED (5/5 words), round 82

> Renamed from `func_80042828` on 2026-09-25 (tools/rename.py). Address 0x80042828.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EFAC and D_800866E8 slot +0x0B8 (`getChild` in `include/class_3ac78.h`) (`tools/classtable.py`).
- **What:** `return self->children[index]` over a pointer array at +0x44 (unit-local view `ChildArrayObj_322b4`).
- **Result:** byte-exact; 5/5 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* D_8006EFAC and D_800866E8 slot +0x0B8 (getChild). */
void *D8006EFAC__GetChild(ChildArrayObj_322b4 *self, s32 index) {
    return self->children[index];
}
```

## Naming

- `D8006EFAC__GetChild` -- tier A. Slot +0x0B8: returns children[index]. Shared, unchanged, with D_800866E8's own slot +0x0B8 (include/Class6B5CC.h documents D_8006EFAC as "the base of Class866E8"), i.e. Class866E8 simply inherits this getChild rather than overriding it. Pure getter.
