# ScreenSprite__ScreenSprite -- MATCHED (35/35 words), round 82

> Renamed from `D8006ED4C__D8006ED4C` on 2026-09-25 (tools/rename.py). Address 0x80041d18.

> Renamed from `func_80041D18` on 2026-09-25 (tools/rename.py). Address 0x80041d18.

Round 82, runner alpha (fifth slot on Sprite). Unit `src/graphics/sprite.c`. Fresh ground, no prior body attempt.

- **Where:** gScreenSpriteMethods slot +0x008 (ctor) (`tools/classtable.py`).
- **What:** Sprite's ctor through `GetSpriteMethods()` with (self, texture, abr 0, rect, arg4 NULL, arg3) -- the two stack words are 0 and the caller's fourth argument -- then installs `GetScreenSpriteMethods()`'s table and calls its reset (+0x040, the empty ScreenSprite__Reset) with self only.
- **Result:** byte-exact; 35/35 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** typed through the UNIFIED `Sprite`/`SpriteMethods` (include/sprite.h), unchanged.

## Source

```c
#include "screen_sprite.h"

/* gScreenSpriteMethods slot +0x008 (ctor): the Sprite ctor with abr 0 and arg4 NULL,
 * install the table, then reset. */
void ScreenSprite__ScreenSprite(ScreenSprite *self, void *texture, SpriteRect *rect, s32 arg3) {
    GetSpriteMethods()->ctor((Sprite *)self, texture, 0, rect, NULL, arg3);
    self->methods = GetScreenSpriteMethods();
    self->methods->reset(self);
}
```

## Naming

- `D8006ED4C__D8006ED4C` -- tier A. Ctor (slot +0x008): the Sprite ctor with abr 0 and a NULL fourth argument, installs the D_8006ED4C table, then resets.

## Track 4

2026-09-25, round 84 (charlie): class unified in `include/screen_sprite.h`. Renamed from `D8006ED4C__D8006ED4C`, tier A: the ctor slot (+0x008), `Class__Class`. `self` is `ScreenSprite *`; the base call upcasts, `GetSpriteMethods()->ctor((Sprite *)self, texture, 0, rect, NULL, arg3)`. This is the ctor chain that confirms the id tree: Sprite's ctor first, and CharSprite__CharSprite calls this one first. The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

`arg3` is `resetWord`, forwarded to the Sprite ctor's last argument. Byte-exact.
