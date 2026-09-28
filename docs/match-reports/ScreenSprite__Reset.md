# ScreenSprite__Reset -- MATCHED (2/2 words), round 82

> Renamed from `D8006ED4C__Reset` on 2026-09-25 (tools/rename.py). Address 0x80041da4.

> Renamed from `func_80041DA4` on 2026-09-25 (tools/rename.py). Address 0x80041da4.

Round 82, runner alpha (re-staffed slot). Unit `src/graphics/Sprite.c`. Fresh
ground (carved in FINISHING-PLAN revision 18), no prior attempt.

- **Where:** gScreenSpriteMethods slot +0x040 (reset, over SceneNode__Reset) (`tools/classtable.py`).
- **What:** empty override: `jr $ra; nop`.
- **Result:** byte-exact on the FIRST build; 2/2 words, 0 insertions /
  0 deletions, whole-image SHA1 green. No levers needed.
- **Types:** SceneNode-derived methods take `SceneNode *` from the UNIFIED
  `include/scene_node.h` (untouched). The FrameClock and RequestedFile objects use
  unit-local views (`D_8006EF50Obj`, `D_8006EED8Obj`) declared at the top of
  the unit; nothing was added to a shared header.

## Source

```c
#include "ScreenSprite.h"

/* gScreenSpriteMethods slot +0x040 (reset): empty override. */
void ScreenSprite__Reset(ScreenSprite *self) {
}
```

## Naming

- `D8006ED4C__Reset` -- tier A. Reset override (slot +0x040): empty body. A pure leaf whose mechanics (do nothing) ARE its purpose.

## Track 4

2026-09-25, round 84 (charlie): class unified in `include/ScreenSprite.h`. Renamed from `D8006ED4C__Reset`, tier A: the reset slot (+0x040), empty. `self` is `ScreenSprite *`. The Source block above is the unified spelling. Image byte-identical.
