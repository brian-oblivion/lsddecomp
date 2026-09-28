# TmdModel__InitBoundsCount -- MATCHED (4/4 words), round 82

> Renamed from `MarkTmdModelConstructed` on 2026-09-26 (tools/rename.py). Address 0x8001f394.

> Renamed from `func_8001F394` on 2026-09-25 (tools/rename.py). Address 0x8001f394.

Round 82, runner charlie (matching slot). Unit `src/TmdModel.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called at the tail of `TmdModel__TmdModel` (slot +0x008, the constructor) (`tools/classtable.py gTmdModelMethods`).
- **What:** `gTmdModelBoundsCount = 1;` (`ori v0,1; sw %gp_rel`)
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.


## Source

```c
void TmdModel__InitBoundsCount(void) {
    gTmdModelBoundsCount = 1;
}
```

## Naming

`TmdModel__InitBoundsCount` -- tier B. Free helper (VerbNoun), not vtable-
dispatched. Evidence: called once, at the tail of `TmdModel__TmdModel`, and
its whole body is `gTmdModelBoundsCount = 1;` with `self` ignored. The name
describes exactly the mechanics (a global flag set once a TmdModel finishes
construction); it stops short of claiming why the flag exists or what reads
it besides `TmdModel__GetBoundsCount` (kept `func_` -- see that report), since neither
is established.

## Track 4 (2026-09-26, round 87, delta)

Renamed from `MarkTmdModelConstructed`: tier B. Called only at the tail of `TmdModel__TmdModel`; stores 1 into `gTmdModelBoundsCount`. `gTmdModelBoundsCount` (was `gTmdModelConstructed`) is the NUMBER of bounds boxes a TmdModel exposes, not a constructed flag: `SceneNode__CheckBoundsOverlap` (SceneNode) reads it into `n` and walks `p + n` boxes from `TmdModel__GetBoundsBuffer(model, 0)`, and `SceneNode__RaycastHullAgainstFaces` loops `for (i = 0; i < count; i++) plane = TmdModel__GetBoundsBuffer(model, i)`. The three boolean callers (`SceneNode__NotifyWithHull`, `SceneNode__TryAttachNearby`, `Actor__NotifyMove`) test it non-zero, i.e. "has any box". The only writer is the ctor's `TmdModel__InitBoundsCount`, which stores 1, and `TmdModel__GetHull` likewise writes a hull list whose count word is 1: a TmdModel is one box.
