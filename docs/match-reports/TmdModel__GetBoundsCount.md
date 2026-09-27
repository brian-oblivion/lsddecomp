# TmdModel__GetBoundsCount -- MATCHED (3/3 words), round 82

> Renamed from `IsTmdModelConstructed` on 2026-09-26 (tools/rename.py). Address 0x8001f3a4.

> Renamed from `func_8001F3A4` on 2026-09-25 (tools/rename.py). Address 0x8001f3a4.

Round 82, runner charlie (matching slot). Unit `src/TmdModel.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table; called from SceneNode (code_d294_b.c) and class_3bb8c_o.c as `TmdModel__GetBoundsCount(model)` (`tools/classtable.py gTmdModelMethods`).
- **What:** returns the sbss flag `gTmdModelBoundsCount` (reached `%gp_rel`, via `--gp-symbols`); its argument is ignored. Callers use the result both as a truth value and as a count.
- **Result:** byte-exact; 3/3 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views (`TmdModel`, `ModelData_fa50`, `Quad_fa50`, `TmdObject_fa50`, `Outer_fa50`/`Inner_fa50`/`Target_fa50`) and prototypes live in the unit; no shared header was touched.


## Source

```c
s32 TmdModel__GetBoundsCount(void *self) {
    return gTmdModelBoundsCount;
}
```

## Naming

`TmdModel__GetBoundsCount` -- KEPT (not renamed this round). Tier C: mechanics fully
known (returns `gTmdModelBoundsCount`, ignoring its argument), but renaming
would touch `src/class_3bb8c_o.c`, a live types-runner unit this round
(FINISHING-PLAN track 4); `tools/rename.py` rewrites every caller
tree-wide, so this rename is deferred to avoid the collision.

## Proposed name

`TmdModel__GetBoundsCount` (or `TmdModel__IsConstructed` if a caller confirms
it always takes an actual `TmdModel*`) -- tier B. The head should apply this
with `tools/rename.py` once `class_3bb8c_o.c` is not live, then re-verify.
Posted to the broadcast.

## Track 4 (2026-09-26, round 87, delta)

Renamed from `IsTmdModelConstructed`: tier B. It returns `gTmdModelBoundsCount` and ignores `self`. `gTmdModelBoundsCount` (was `gTmdModelConstructed`) is the NUMBER of bounds boxes a TmdModel exposes, not a constructed flag: `SceneNode__CheckBoundsOverlap` (code_d294_b) reads it into `n` and walks `p + n` boxes from `TmdModel__GetBoundsBuffer(model, 0)`, and `SceneNode__ClassifyAgainstPlanes` loops `for (i = 0; i < count; i++) plane = TmdModel__GetBoundsBuffer(model, i)`. The three boolean callers (`SceneNode__NotifyWithHull`, `SceneNode__TryAttachNearby`, `Actor__NotifyMove`) test it non-zero, i.e. "has any box". The only writer is the ctor's `TmdModel__InitBoundsCount`, which stores 1, and `TmdModel__GetHull` likewise writes a hull list whose count word is 1: a TmdModel is one box.
