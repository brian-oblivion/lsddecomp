# ScreenSprite__SetPivotAnchor -- MATCHED (32/32 words), round 82

> Renamed from `D8006ED4C__SetPivotAnchor` on 2026-09-25 (tools/rename.py). Address 0x80041e58.

> Renamed from `func_80041E58` on 2026-09-25 (tools/rename.py). Address 0x80041e58.

Round 82, runner alpha (fifth slot on Sprite). Unit `src/graphics/sprite.c`. Fresh ground, no prior body attempt.

- **Where:** gCharSpriteMethods and gScreenSpriteMethods slot +0x0C0 (`tools/classtable.py`).
- **What:** When `parent` (+0x00C) is set, a five-case switch (owns jtbl_80011290) moves the GsSPRITE pivot: 0 = (w/2, h/2), 1 = mx 0, 2 = mx w, 3 = my 0, 4 = my h. Unsigned anchor (`sltiu`).
- **Result:** byte-exact; 32/32 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** typed through the UNIFIED `Sprite` (sprite.w/h/mx/my), unchanged.

## Source

```c
#include "screen_sprite.h"

/* gCharSpriteMethods and gScreenSpriteMethods slot +0x0C0: when attached, move the sprite's
 * pivot: 0 centre, 1 left, 2 right, 3 top, 4 bottom. */
void ScreenSprite__SetPivotAnchor(ScreenSprite *self, u32 anchor) {
    if (self->parent != NULL) {
        switch (anchor) {
        case 0:
            self->sprite.mx = self->sprite.w >> 1;
            self->sprite.my = self->sprite.h >> 1;
            break;
        case 1:
            self->sprite.mx = 0;
            break;
        case 2:
            self->sprite.mx = self->sprite.w;
            break;
        case 3:
            self->sprite.my = 0;
            break;
        case 4:
            self->sprite.my = self->sprite.h;
            break;
        }
    }
}
```

## Naming

- `D8006ED4C__SetPivotAnchor` -- tier A. New slot +0x0C0: when attached, moves the embedded GsSPRITE's pivot (mx/my) per a 0..4 anchor code (centre/left/right/top/bottom) -- matches the round-82 broadcast note "pivot setter at +0x0C0" and the round-82 header comment. A plain switch whose mechanics are its purpose.

## Track 4

2026-09-25, round 84 (charlie): class unified in `include/screen_sprite.h`. Renamed from `D8006ED4C__SetPivotAnchor`, tier A: the +0x0C0 slot's occupant, named `setPivotAnchor` in include/screen_sprite.h. `self` is `ScreenSprite *` (was `Sprite *`); the accessors are unchanged, `parent` and `sprite.mx/my/w/h`. The Source block above is the unified spelling. Image byte-identical.

## Track 7 (round 99, charlie)

Case labels spelled with `enum ScreenSpriteAnchor` (include/screen_sprite.h, added this round): CENTRE 0, LEFT 1, RIGHT 2, TOP 3, BOTTOM 4, each read off the case body (mx/my set to w/2,h/2; mx 0; mx w; my 0; my h). Byte-exact.
