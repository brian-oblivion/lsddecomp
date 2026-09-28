# TimImage__NoOpSlot90 -- MATCHED (2/2 words), round 81

> Renamed from `TimImage__func_8003B5D4` on 2026-09-27 (tools/rename.py). Address 0x8003b5d4.

> Renamed from `func_8003B5D4` on 2026-09-25 (tools/rename.py). Address 0x8003b5d4.

Round 81, runner echo. Unit `src/graphics/TimImage.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x090 (resolved with `tools/classtable.py D_8006E558`).
- **What:** empty slot override: `jr $ra; nop`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 2/2,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** `TimImage__NoOpSlot90`, tier C (round 81 naming pass, runner bravo): the
  class is confirmed as `TimImage` (see `## Naming` below), but this slot's
  body is empty (`jr $ra; nop`) and no caller relies on it doing anything, so
  its purpose is unknown -- the tier-C `Class__func_xxxxx` form applies.

## Source

```c
void TimImage__NoOpSlot90(void) {
}
```

The unit-local view it needs, from the top of `src/graphics/TimImage.c`:

```c
#include "FileResource.h"

typedef struct TimImage {
    FILERESOURCE_FIELDS(FileResourceMethods);
    /* +0x02C */ u8 pad2C[0x1C];
    /* +0x048 */ s32 unk48;
} TimImage;

typedef struct GsIMAGE GsIMAGE;
void GsGetTimInfo(u32 *im, GsIMAGE *tim);
extern FileResourceMethods gTimImageMethods;
```

## Naming

- **`TimImage__NoOpSlot90`**, tier C: class confirmed as `TimImage` (this round; see
  `docs/match-reports/TimImage__TimImage.md`), slot +0x090. Empty body
  (`jr $ra; nop`), no caller overrides it with anything else, so its
  purpose is unknown -- `Class__func_xxxxx` per the tier-C convention for a
  method whose class is known but mechanics are not (an empty slot has no
  mechanics to describe).

## Naming (track 7, round 100, delta)

Renamed `TimImage__func_8003B5D4` -> `TimImage__NoOpSlot90` with `tools/rename.py`
(the tier-C lines above predate it and name the old form). **Tier A**: a pure
leaf whose mechanics are its purpose -- the body is `jr $ra; nop`, it does
nothing -- named by the project's empty-slot convention
(`CdDriver__NoOpSlot40`, `VabStreamObj__NoOpSlot90`, `Viewport__NoOpSlot58`,
`StreamTask__NoOpSlot88`): the class and the table offset it fills
(gTimImageMethods +0x090, `tools/classtable.py gTimImageMethods`). What the
slot is for in the class tree is not established; no C calls it. The table
field stays `slot90`, as the other empty slots' fields do (`include/cd_stream.h`).
