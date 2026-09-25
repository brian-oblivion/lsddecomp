# MarkTmdModelConstructed -- MATCHED (4/4 words), round 82

> Renamed from `func_8001F394` on 2026-09-25 (tools/rename.py). Address 0x8001f394.

Round 82, runner charlie (matching slot). Unit `src/code_fa50.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called at the tail of `TmdModel__TmdModel` (slot +0x008, the constructor) (`tools/classtable.py D_8006BEA0`).
- **What:** `gTmdModelConstructed = 1;` (`ori v0,1; sw %gp_rel`)
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.


## Source

```c
void MarkTmdModelConstructed(void) {
    gTmdModelConstructed = 1;
}
```
