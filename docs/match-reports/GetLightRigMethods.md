# GetLightRigMethods -- MATCHED (4/4 words), round 82

> Renamed from `Get_vtable_D8006EFAC` on 2026-09-26 (tools/rename.py). Address 0x800428e4.

> Renamed from `func_800428E4` on 2026-09-25 (tools/rename.py). Address 0x800428e4.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py`).
- **What:** Returns the gLightRigMethods method table (`class_39e08.c` declares it `BaseCtorTable_3ac78 *`).
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* Returns the LightRig method table. */
LightRigMethods *GetLightRigMethods(void) {
    return &gLightRigMethods;
}
```

## Naming

- `Get_vtable_D8006EFAC` -- tier A. Table getter ("return D_8006EFAC;").

## Track 4

2026-09-26, round 86 (delta): class 0x14 unified as LightRig in `include/LightRig.h`. Renamed from `Get_vtable_D8006EFAC`, tier A: the table getter, spelled like every unified class's (`Get<Class>Methods`). Returns `LightRigMethods *` and `&gLightRigMethods` (was `void *` over a local `extern s32 D_8006EFAC[]`). The Source block above is the unified spelling. Image byte-identical.
