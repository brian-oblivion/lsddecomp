# GetTmdModelMethods -- MATCHED (4/4 words), round 82

> Renamed from `Get_vtable_TmdModel` on 2026-09-28 (tools/rename.py). Address 0x8001f384.

> Renamed from `func_8001F384` on 2026-09-25 (tools/rename.py). Address 0x8001f384.

Round 82, runner charlie (matching slot). Unit `src/graphics/TmdModel.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; the gTmdModelMethods table getter, called by `New_TmdModel` and `TmdModel__TmdModel` (`tools/classtable.py gTmdModelMethods`).
- **What:** returns `gTmdModelMethods`
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.
- **Lever:** Table-getter shape `void *f(void) { return D_X; }` with a local `extern s32 D_X[];` (broadcast lever).

## Source

```c
void *GetTmdModelMethods(void) {
    return gTmdModelMethods;
}
```

## Naming

`GetTmdModelMethods` -- tier B. Convention: `Get_vtable_<Class>` (matches
`GetBasicClassMethods`, `GetCdStreamMethods`, `GetDrawSystemMethods`).
Class name `TmdModel`: gTmdModelMethods is class tag 9, the object
`SceneNode__LinkModel` (src/graphics/scene_node.c) links as `self->model` -- that
unit's own `ModelObj_d294` local view (pad to +0xC, `tmdFile` at +0xC, `tmd`
at +0x10) lines up field-for-field with this class's own `data`/`unk10` at
the same offsets, and this class's own methods (`TmdModel__MapModelingData`,
`TmdModel__GetObject`, `TmdModel__NextPrimitive`, `TmdModel__RaycastFaces`)
all operate on a TMD file + its object table. Tier B, not A: what wraps a TMD
is certain; why the game needs a standalone "model" object distinct from
SceneNode itself is not established. Only the getter is renamed; the table
symbol gTmdModelMethods is kept, matching the WBgm/DrawSystem precedent (round 82
broadcast, bravo).
