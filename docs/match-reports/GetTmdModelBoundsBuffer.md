# GetTmdModelBoundsBuffer -- MATCHED (4/4 words), round 82

> Renamed from `func_8001F50C` on 2026-09-25 (tools/rename.py). Address 0x8001f50c.

Round 82, runner charlie (matching slot). Unit `src/code_fa50.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called from Class6B5CC (code_d294_b.c) as `GetTmdModelBoundsBuffer(model, i)` (`tools/classtable.py D_8006BEA0`).
- **What:** ignores both arguments and returns `gTmdModelBoundsBuf` (the buffer `UpdateTmdModelBoundsBuffer` fills through `TmdModel__ComputeBounds`)
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`Class6BEA0`, `ModelData_fa50`, `Quad_fa50`, `Rec28_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.
- **Lever:** Parameters are kept in the definition to document the call shape callers use; they do not affect the bytes.

## Source

```c
void *GetTmdModelBoundsBuffer(void *self, s32 i) {
    return gTmdModelBoundsBuf;
}
```
