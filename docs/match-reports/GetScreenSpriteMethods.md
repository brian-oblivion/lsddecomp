# GetScreenSpriteMethods -- MATCHED (4/4 words), round 82

> Renamed from `Get_vtable_D8006ED4C` on 2026-09-25 (tools/rename.py). Address 0x80041ed8.

> Renamed from `func_80041ED8` on 2026-09-25 (tools/rename.py). Address 0x80041ed8.

Round 82, runner alpha (second re-staffed slot of the round). Unit `src/graphics/Sprite.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** not in any method table (`tools/classtable.py`).
- **What:** Returns the gScreenSpriteMethods method table.
- **Result:** byte-exact; 4/4 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and `extern s32 D_XXXXXXXX[];` table declarations live in the unit; no shared header was touched.

## Source

```c
#include "ScreenSprite.h"

/* Returns the gScreenSpriteMethods method table. */
ScreenSpriteMethods *GetScreenSpriteMethods(void) {
    return &gScreenSpriteMethods;
}
```

## Naming

- `Get_vtable_D8006ED4C` -- tier A. Table getter ("return D_8006ED4C;").

## Track 4

2026-09-25, round 84 (charlie): class unified in `include/ScreenSprite.h`. Renamed from `Get_vtable_D8006ED4C`, tier A: it returns the class's table, and the name is the class's getter convention (GetSpriteMethods, GetSceneNodeMethods). Declared once, in include/ScreenSprite.h, returning `ScreenSpriteMethods *` and `&gScreenSpriteMethods` (formerly `D_8006ED4C`): the same lui/addiu as the array-decay spelling; the unit's `extern s32 D_8006ED4C[]` is gone. Callers: New_ScreenSprite, ScreenSprite__ScreenSprite and CharSprite__CharSprite (the subclass ctor, now `GetScreenSpriteMethods()->ctor((ScreenSprite *)self, ...)`). The Source block above is the unified spelling. Image byte-identical.
