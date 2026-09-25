# TmdModel__GetObject -- MATCHED (7/7 words), round 82

> Renamed from `func_8001F360` on 2026-09-25 (tools/rename.py). Address 0x8001f360.

Round 82, runner charlie (matching slot). Unit `src/code_fa50.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** slot +0x048 of D_8006BEA0 (`tools/classtable.py D_8006BEA0`).
- **What:** returns `&self->data->recs[i]`: a 28-byte record array at +0x0C of the model data at object +0x0C. `sll 3; subu; sll 2` = i*28, then `addiu 0xC` for the array offset.
- **Result:** byte-exact; 7/7 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.


## Source

```c
TmdObject_fa50 *TmdModel__GetObject(TmdModel *self, s32 i) {
    return &self->data->recs[i];
}
```

## Naming

`TmdModel__GetObject` -- tier A. Slot +0x048: `return &self->data->recs[i];`,
a plain indexed getter over the model's TMD object table
(`TmdObject_fa50 recs[]`).
