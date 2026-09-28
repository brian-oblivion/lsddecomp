# LightRig__GetLight -- MATCHED (5/5 words), round 82

> Renamed from `D8006EFAC__GetChild` on 2026-09-26 (tools/rename.py). Address 0x80042828.

> Renamed from `func_80042828` on 2026-09-25 (tools/rename.py). Address 0x80042828.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/graphics/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** gLightRigMethods and gStageMapMethods slot +0x0B8 (`getChild` in `include/DayTaskStageMap.h`) (`tools/classtable.py`).
- **What:** `return self->children[index]` over a pointer array at +0x44 (unit-local view `ChildArrayObj_322b4`).
- **Result:** byte-exact; 5/5 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
/* LightRig slot +0x0B8 (getLight), inherited unchanged by gStageMapMethods. */
BasicClass *LightRig__GetLight(LightRig *self, s32 index) {
    return self->lights[index];
}
```

## Naming

- `D8006EFAC__GetChild` -- tier A. Slot +0x0B8: returns children[index]. Shared, unchanged, with gStageMapMethods's own slot +0x0B8 (include/SceneNode.h documents D_8006EFAC as "the base of StageMap"), i.e. StageMap simply inherits this getChild rather than overriding it. Pure getter.

## Track 4

2026-09-26, round 86 (delta): class 0x14 unified as LightRig in `include/LightRig.h`. Renamed from `D8006EFAC__GetChild`, tier A: slot +0x0B8, named `getLight` in the header. It returns `lights[index]`, the three FlatLightObj the ctor made, not an entry of BasicClass's `children` list (that is getNextChild, +0x01C), so `GetChild` described the wrong thing. Its callers agree: LightRig__Finalize releases getLight(0..2), and StageMap__SetChildParams calls each result's FlatLightObj setColor (+0x044) and setDirection (+0x048). `self` is `LightRig *` and it returns `BasicClass *` (was `ChildArrayObj_322b4 *`, `void *children[1]` at +0x044, and `void *`). StageMap's own view (include/DayTaskStageMap.h) still calls the slot `getChild`; that is the subclass's to rename. The Source block above is the unified spelling. Image byte-identical.
