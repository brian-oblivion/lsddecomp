# TmdModel__ComputeBounds -- MATCHED (77/77 words), round 82

> Renamed from `func_8001F3B0` on 2026-09-25 (tools/rename.py). Address 0x8001f3b0.

Round 82, runner charlie (matching slot, second pass on the unit). Unit `src/code_fa50.c`. Fresh ground, no prior attempt.

- **What:** axis-aligned bounding box of a TMD object's vertex list. `self->unk10` is the TMD object-table entry (the 28-byte `Rec28_fa50`, now given its first two fields `verts`/`nverts`; +0x0C in `ModelData_fa50` is exactly a TMD's 12-byte header, so `recs[]` is the TMD object table). Box = `{s16 x,y,z} min, max` (12 bytes); vertices are 8-byte SVECTORs. Called by `func_8001F4E4` (into `gTmdModelBoundsBuf`) and `func_8001F51C` (into a stack buffer).
- **Result:** byte-exact; 77/77 words, whole-image SHA1 green. Build 8.
- **Builds / levers, measured:**
  1. natural body (field compares through `box->...`) + `u8 pad[0x38]`: frame 0x70, and retail's five hoisted field pointers (`addiu t6,a3,2` ... `addiu t2,a3,0xA`) absent. The 0x38 frame is NOT a pad: removing the pad gave 0x38 by itself (a pad of 0x1C gave 0x58).
  2. `static __inline__` SetMin/SetMax helpers taking `&box->min.y` etc.: identical to the plain body; the inline's pointer args fold back into offsets.
  3. five explicit pointer locals (`&box->min.y`, `min.z`, `max.x`, `max.y`, `max.z`; `min.x` goes through `box` itself) initialised in the declarations: 24/77, structure identical, every register shifted by one (count in `t6` instead of `a0`).
  4. loop bound `i < rec->nverts - 1`: 18/77, size changed.
  5. declaration reorder: no change.
  6. `n = nverts; ... n--;` -> `n = nverts - 1;` in one expression: **77/77**.
  7. same, WITHOUT the pointer locals: 0/77 (size change) -- the pointer locals are load-bearing.
- **Types:** `Rec28_fa50` got `SVec_fa50 *verts; s32 nverts;` (same 28-byte size); `Class6BEA0::unk10` retyped `void *` -> `Rec28_fa50 *`; `func_8001F4E4` now takes `Class6BEA0 *` and casts `gTmdModelBoundsBuf` to `Box_fa50 *`; the local `extern s32 TmdModel__ComputeBounds(void *, void *)` was dropped (the definition precedes its callers). All in this unit; no shared header.

## Source

```c
typedef struct SVec_fa50 { s16 x, y, z, pad; } SVec_fa50;
typedef struct Vec3_fa50 { s16 x, y, z; } Vec3_fa50;
typedef struct Box_fa50 { Vec3_fa50 min; Vec3_fa50 max; } Box_fa50;
typedef struct Rec28_fa50 { SVec_fa50 *verts; s32 nverts; u8 pad8[0x14]; } Rec28_fa50;
/* Class6BEA0: +0x010 Rec28_fa50 *unk10 (see New_TmdModel.md) */

void TmdModel__ComputeBounds(Class6BEA0 *self, Box_fa50 *box) {
    s32 i;
    s32 n;
    SVec_fa50 *v;
    s16 *miny = &box->min.y;
    s16 *minz = &box->min.z;
    s16 *maxx = &box->max.x;
    s16 *maxy = &box->max.y;
    s16 *maxz = &box->max.z;

    v = self->unk10->verts;
    n = self->unk10->nverts - 1;
    box->min.x = v->x;
    box->min.y = v->y;
    box->min.z = v->z;
    box->max = box->min;
    for (i = 0; i < n; i++) {
        v++;
        if (v->x < box->min.x) box->min.x = v->x;
        if (v->y < *miny) *miny = v->y;
        if (v->z < *minz) *minz = v->z;
        if (*maxx < v->x) *maxx = v->x;
        if (*maxy < v->y) *maxy = v->y;
        if (*maxz < v->z) *maxz = v->z;
    }
}
```

### Proposed learning

A leaf that computes `addiu tN, box, K` for several struct fields at entry and then compares/stores through `0(tN)` inside a loop (instead of `K(box)`) was written with explicit pointer locals to those fields; GCC 2.6.3 does not hoist field addresses on its own, and a `static __inline__` helper taking `&box->f` folds them back. And a loop count kept in the dead first-argument register (`$a0`) came from `n = x - 1;` in one expression, not `n = x; n--;` (which shifted every register by one).
