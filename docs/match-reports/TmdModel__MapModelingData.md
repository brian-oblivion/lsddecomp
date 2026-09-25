# TmdModel__MapModelingData -- MATCHED (9/9 words), round 82

> Renamed from `func_8001F33C` on 2026-09-25 (tools/rename.py). Address 0x8001f33c.

Round 82, runner charlie (matching slot). Unit `src/code_fa50.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x044 of D_8006BEA0 (`tools/classtable.py D_8006BEA0`).
- **What:** `GsMapModelingData(&self->data->head[1])`: maps the TMD held at object +0x0C, skipping its first word (the Psy-Q idiom `GsMapModelingData(tmd + 1)`).
- **Result:** byte-exact; 9/9 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`Class6BEA0`, `ModelData_fa50`, `Quad_fa50`, `Rec28_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.


## Source

```c
void TmdModel__MapModelingData(Class6BEA0 *self) {
    GsMapModelingData((unsigned long *)&self->data->head[1]);
}
```
