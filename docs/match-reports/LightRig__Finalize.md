# LightRig__Finalize -- MATCHED (33/33 words), round 82

> Renamed from `D8006EFAC__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80042790.

> Renamed from `func_80042790` on 2026-09-25 (tools/rename.py). Address 0x80042790.

Round 82, runner alpha (fifth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior body attempt.

- **Where:** gLightRigMethods slot +0x00C (finalize) (`tools/classtable.py`).
- **What:** For i = 0..2, fetches child i through its own slot +0x0B8 (LightRig__GetLight) and calls that child's release (+0x004); then SceneNode's finalize via `GetSceneNodeMethods()`.
- **Result:** byte-exact; 33/33 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** the unit-local `D_8006EFACObj` view (see LightRig__LightRig).

## Source

```c
/* LightRig slot +0x00C (finalize): release the three lights, then the
 * SceneNode finalize. */
void LightRig__Finalize(LightRig *self) {
    s32 i;
    BasicClass *light;

    for (i = 0; i < 3; i++) {
        light = self->methods->getLight(self, i);
        light->methods->release(light);
    }
    GetSceneNodeMethods()->finalize((SceneNode *)self);
}
```

## Naming

- `D8006EFAC__Finalize` -- tier A. Finalize (slot +0x00C): releases the three light children then calls the SceneNode finalize.

## Track 4

2026-09-26, round 86 (delta): class 0x14 unified as LightRig in `include/LightRig.h`. Renamed from `D8006EFAC__Finalize`, tier A: slot +0x00C. `self` is `LightRig *` (was `D_8006EFACObj`); the +0x0B8 call is now `getLight` (was `getChild`). The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

Loop bound `3` -> `ARRAY_COUNT(self->lights)`. Byte-exact.
