# UpdateTmdModelBoundsBuffer -- MATCHED (10/10 words), round 82

> Renamed from `func_8001F4E4` on 2026-09-25 (tools/rename.py). Address 0x8001f4e4.

Round 82, runner charlie (matching slot). Unit `src/code_fa50.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py D_8006BEA0`).
- **What:** `TmdModel__ComputeBounds(self, gTmdModelBoundsBuf)`: forwards its own a0 and passes the static buffer
- **Result:** byte-exact; 10/10 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.


## Source

```c
void UpdateTmdModelBoundsBuffer(void *self) {
    TmdModel__ComputeBounds(self, gTmdModelBoundsBuf);
}
```
