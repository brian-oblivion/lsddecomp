# LightRig__Reset -- MATCHED (3/3 words), round 82

> Renamed from `D8006EFAC__Reset` on 2026-09-26 (tools/rename.py). Address 0x80042814.

> Renamed from `func_80042814` on 2026-09-25 (tools/rename.py). Address 0x80042814.

Round 82, runner alpha (re-staffed slot). Unit `src/code_322b4.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gLightRigMethods slot +0x040 (reset, over SceneNode__Reset) (`tools/classtable.py`).
- **What:** `self->coord2->flg = 0`: marks the GsCOORDINATE2 for recompute, the same thing updateRotation/updateScale/attachToParent do.
- **Result:** byte-exact on the FIRST build; 3/3 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/SceneNode.h` (untouched). The FrameClock and RequestedFile objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
/* LightRig slot +0x040 (reset): mark the coordinate for recompute. */
void LightRig__Reset(LightRig *self) {
    self->coord2->flg = 0;
}
```

## Naming

- `D8006EFAC__Reset` -- tier A. Reset override (slot +0x040): marks the SceneNode coordinate dirty (coord2->flg = 0, include/SceneNode.h's documented "0 = recompute").

## Track 4

2026-09-26, round 86 (delta): class 0x14 unified as LightRig in `include/LightRig.h`. Renamed from `D8006EFAC__Reset`, tier A: slot +0x040. `self` is `LightRig *` (was `SceneNode *`; `coord2` is inherited, so the access is unchanged). The Source block above is the unified spelling. Image byte-identical.
