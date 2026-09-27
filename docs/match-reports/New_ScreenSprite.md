# New_ScreenSprite -- MATCHED (31/31 words), round 82

> Renamed from `New_D8006ED4C` on 2026-09-25 (tools/rename.py). Address 0x80041c9c.

> Renamed from `func_80041C9C` on 2026-09-25 (tools/rename.py). Address 0x80041c9c.

Round 82, runner alpha (fourth slot on code_322b4). Unit `src/code_322b4.c`. Fresh ground, no prior attempt.

- **Where:** not in any method table (allocator for ScreenSprite (gScreenSpriteMethods), the screen-space sprite, 0xA8 bytes).
- **What:** `BMemPMgrAlloc(0xA8)`; if non-NULL, calls slot +0x008 (ctor, ScreenSprite__ScreenSprite) of `GetScreenSpriteMethods()` (the gScreenSpriteMethods table) with `(obj, a1, a2, a3)` and returns obj, else NULL.
- **Result:** byte-exact, 31/31 words, 0 ins / 0 del, whole-image SHA1 green. First build (round-82 allocator shape, three pass-through args in `$s1..$s3`).
- **Types:** the three arguments are left `void *` (their meaning belongs to ScreenSprite__ScreenSprite, still INCLUDE_ASM). Unit-local `CtorArg3Methods_322b4` and a prototype for `GetScreenSpriteMethods`; no shared header touched.

## Source

```c
#include "ScreenSprite.h"

/* Allocate and construct a ScreenSprite (0xA8 bytes). */
ScreenSprite *New_ScreenSprite(void *texture, SpriteRect *rect, s32 arg3) {
    ScreenSprite *obj = BMemPMgrAlloc(0xA8);

    if (obj != NULL) {
        GetScreenSpriteMethods()->ctor(obj, texture, rect, arg3);
        return obj;
    }
    return NULL;
}
```

## Naming

- `New_D8006ED4C` -- tier A. Allocator: BMemPMgrAlloc(0xA8) then the ctor slot. Same New_<Class> convention.

## Track 4

2026-09-25, round 84 (charlie): class unified in `include/ScreenSprite.h`. Renamed from `New_D8006ED4C`, tier A: `New_<Class>`, BMemPMgrAlloc(0xA8) (the object size SCREENSPRITE_FIELDS ends at) then the ctor slot. Signature `ScreenSprite *New_ScreenSprite(void *texture, SpriteRect *rect, s32 arg3)`, the ctor's parameters; the unit-local `CtorArg3Methods_322b4` view is gone. Callers checked: TaskObjF__LoadCardIcon (class_3bb8c_g), TextEntry__LoadCardResources (class_3bb8c_i) and ItemList__LoadResources (class_3bb8c_j) all pass a TimImage handle, a 3-word rect (gCardIconRect {0,0,160,120}, gTextEntryPanelRect {0,0,224,120}, D_80087028 {0,0,256,160}) and 0; their local externs are deleted and each casts the return to its own field's type. The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

Allocation size spelled `sizeof(ScreenSprite)` (0xA8). The last parameter `arg3` is `resetWord`: it is passed through the ScreenSprite and Sprite ctors to Sprite's reset, which does not read it; all three C callers pass 0. Byte-exact.
