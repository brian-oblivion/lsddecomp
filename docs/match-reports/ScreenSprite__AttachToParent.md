# ScreenSprite__AttachToParent -- MATCHED (32/32 words), round 82

> Renamed from `D8006ED4C__AttachToParent` on 2026-09-25 (tools/rename.py). Address 0x80041dac.

> Renamed from `func_80041DAC` on 2026-09-25 (tools/rename.py). Address 0x80041dac.

Round 82, runner alpha (fourth slot on Sprite). Unit `src/graphics/sprite.c`. Fresh ground, no prior attempt.

- **Where:** gCharSpriteMethods and gScreenSpriteMethods slot +0x04C (attachToParent override).
- **What:** if `parent` (+0x00C) is still NULL, calls Sprite's attachToParent (`GetSpriteMethods()->attachToParent`, i.e. the inherited SceneNode one) with the zero offset `sVec3Zero` (three zero words in .data), then calls the object's own slot +0x0BC (ScreenSprite__SetPosition) with the caller's third argument. No return value is produced on the skip path (`$v0` holds the loaded parent), so it is written `void` although the SceneNode slot type returns a pointer.
- **Result:** byte-exact, 32/32 words, 0 ins / 0 del, whole-image SHA1 green. First build.
- **Types:** self is the unit-local `SpriteView_322b4` (it gained `setPosition` at +0x0BC in its local method view, and `parent` at +0x00C typed `SceneNode *`, the field scene_node.h documents at that offset); the base call casts to `Sprite *`. `extern LongVec3 sVec3Zero;` in the unit. No shared header touched.

## Source

```c
#include "ScreenSprite.h"

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x04C (attachToParent): when not yet
 * attached, attach through Sprite's with a zero offset, then hand the
 * caller's third argument to slot +0x0BC. */
void ScreenSprite__AttachToParent(ScreenSprite *self, SceneNode *parent, ScreenSpritePos *pos) {
    if (self->parent == NULL) {
        GetSpriteMethods()->attachToParent((Sprite *)self, parent, &sVec3Zero);
        self->methods->setPosition(self, pos);
    }
}
```

## Naming

- `D8006ED4C__AttachToParent` -- tier A. Override of SceneNode's inherited attachToParent (slot +0x04C, include/scene_node.h): only when self->parent is still NULL, attaches through the base (Sprite's, i.e. inherited SceneNode__AttachToParent) with a zero offset, then forwards the caller's position to the class's own setPosition slot. Confirmed self->unkC is SceneNode's own +0x00C `parent` field by offset match against include/scene_node.h.
- `sVec3Zero` (renamed from `D_8006EE10`) -- tier A. The three all-zero words this function passes as `attachToParent`'s offset argument; only referenced from this unit. A pure constant, mechanics is the purpose.

## Track 4

2026-09-25, round 84 (charlie): class unified in `include/ScreenSprite.h`. Renamed from `D8006ED4C__AttachToParent`, tier A: the attachToParent slot (+0x04C). The name is the slot's: the body does attach (through the base, with a zero 3-D offset), and its one addition is handing the third argument to setPosition, recorded here rather than in the name. `self` is `ScreenSprite *`, the third argument a `ScreenSpritePos *` (SceneNode's slot types it `LongVec3 *offset`; include/ScreenSprite.h, "Not settled"). The callers' pairs are percentages: (-70, -60) at sCardIconPos and sTextEntryPanelPos, (-100, -60) at sItemListPanelPos. The Source block above is the unified spelling. Image byte-identical.
