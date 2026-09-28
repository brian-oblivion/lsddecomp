# TmdModel__GetBoundsBuffer -- MATCHED (4/4 words), round 82

> Renamed from `GetTmdModelBoundsBuffer` on 2026-09-26 (tools/rename.py). Address 0x8001f50c.

> Renamed from `func_8001F50C` on 2026-09-25 (tools/rename.py). Address 0x8001f50c.

Round 82, runner charlie (matching slot). Unit `src/graphics/TmdModel.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called from SceneNode (code_d294.c) as `TmdModel__GetBoundsBuffer(model, i)` (`tools/classtable.py gTmdModelMethods`).
- **What:** ignores both arguments and returns `sTmdModelBoundsBuf` (the buffer `TmdModel__UpdateBoundsBuffer` fills through `TmdModel__ComputeBounds`)
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.
- **Lever:** Parameters are kept in the definition to document the call shape callers use; they do not affect the bytes.

## Source

```c
void *TmdModel__GetBoundsBuffer(void *self, s32 i) {
    return sTmdModelBoundsBuf;
}
```

## Naming

`TmdModel__GetBoundsBuffer` -- tier A. Free helper: ignores both its
arguments and returns `&sTmdModelBoundsBuf`, the buffer
`TmdModel__UpdateBoundsBuffer` fills. Parameters are kept in the definition
only to document the call shape callers use (per the function's own
`## Source` note); they do not affect the name.

## Track 4 (2026-09-26, round 87, delta)

Renamed from `GetTmdModelBoundsBuffer`: tier A, convention only (`Class__Method`). Returns `sTmdModelBoundsBuf` whatever its arguments; the index is the box number the callers loop over (`sTmdModelBoundsCount` (was `gTmdModelConstructed`) is the NUMBER of bounds boxes a TmdModel exposes, not a constructed flag: `SceneNode__CheckBoundsOverlap` (SceneNode) reads it into `n` and walks `p + n` boxes from `TmdModel__GetBoundsBuffer(model, 0)`, and `SceneNode__RaycastHullAgainstFaces` loops `for (i = 0; i < count; i++) plane = TmdModel__GetBoundsBuffer(model, i)`. The three boolean callers (`SceneNode__NotifyWithHull`, `SceneNode__TryAttachNearby`, `Actor__NotifyMove`) test it non-zero, i.e. "has any box". The only writer is the ctor's `TmdModel__InitBoundsCount`, which stores 1, and `TmdModel__GetHull` likewise writes a hull list whose count word is 1: a TmdModel is one box.)
