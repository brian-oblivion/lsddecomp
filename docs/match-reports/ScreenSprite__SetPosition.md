# ScreenSprite__SetPosition -- MATCHED (11/11 words), round 82

> Renamed from `D8006ED4C__SetPosition` on 2026-09-25 (tools/rename.py). Address 0x80041e2c.

> Renamed from `func_80041E2C` on 2026-09-25 (tools/rename.py). Address 0x80041e2c.

Round 82, runner alpha (third re-staffed slot of the round). Unit `src/code_322b4.c`. Fresh ground (carved in FINISHING-PLAN revision 18), no prior attempt, no report before this one.

- **Where:** D_8006EC74 and gScreenSpriteMethods slot +0x0BC (`tools/classtable.py`).
- **What:** If `parent` (+0x00C, Class6B5CC's own field) is non-NULL, copies two words from the argument to +0x0A0/+0x0A4. Retail moves both args to `$a2`/`$a3` first and does `lw,lw,sw,sw`: a whole-struct assignment of an 8-byte `{s32 x,y;}` (`Pair_322b4`), stored in `screenPos`. Uses the unit-local `SpriteView_322b4` view (methods at +0x000, `parent` +0x00C, GsSPRITE attribute +0x064, u/v +0x072/+0x073, 8-byte `Pair_322b4 screenPos` +0x0A0, `cellIndex` byte +0x0A8), added this session.
- **Result:** byte-exact; 11/11 words, 0 insertions / 0 deletions, whole-image SHA1 green (`./build-and-verify.sh` OK). First build.
- **Types:** local views and prototypes live in the unit; no shared header was touched.

## Source

```c
#include "ScreenSprite.h"

/* D_8006EC74 and gScreenSpriteMethods slot +0x0BC. */
void ScreenSprite__SetPosition(ScreenSprite *self, ScreenSpritePos *pos) {
    if (self->parent != NULL) {
        self->screenPos = *pos;
    }
}
```

## Naming

- `D8006ED4C__SetPosition` -- tier B. New slot +0x0BC this class introduces: stores the caller's pair into screenPos, but only once attached (self->parent != NULL). Mechanics clear (a guarded position setter); which on-screen quantity screenPos represents (D_8006ED4C's own first field, per Sprite.h: "D_8006ED4C's own fields start at +0x0A0") is not independently confirmed, hence tier B.

## Track 4

2026-09-25, round 84 (charlie): class unified in `include/ScreenSprite.h`. Renamed from `D8006ED4C__SetPosition`, and the tier is now A: the +0x0BC slot's occupant, named `setPosition` in include/ScreenSprite.h. What `screenPos` is is settled by its reader, Viewport__DrawNode (code_2864.c): for `(tag & 0xFFF) == 0x144` it sets the GsSPRITE's x/y to `(half screen size * 100) / (10000 / screenPos)`, i.e. percent of half the screen, plus the pivot. `self` is `ScreenSprite *`, `pos` a `ScreenSpritePos *` (the former Pair_322b4, the same 8-byte two-word struct, so the lw,lw,sw,sw copy is unchanged). The Source block above is the unified spelling. Image byte-identical.
