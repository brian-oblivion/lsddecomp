# TimImage__func_8003B5BC -- MATCHED (2/2 words), round 81

> Renamed from `func_8003B5BC` on 2026-09-25 (tools/rename.py). Address 0x8003b5bc.

Round 81, runner echo. Unit `src/code_2bb9c.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x084 (resolved with `tools/classtable.py D_8006E558`).
- **What:** empty slot override: `jr $ra; nop`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 2/2,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** `TimImage__func_8003B5BC`, tier C (round 81 naming pass, runner bravo): the
  class is confirmed as `TimImage` (see `## Naming` below), but this slot's
  body is empty (`jr $ra; nop`) and no caller relies on it doing anything, so
  its purpose is unknown -- the tier-C `Class__func_xxxxx` form applies.

## Source

```c
void TimImage__func_8003B5BC(void) {
}
```

The unit-local view it needs, from the top of `src/code_2bb9c.c`:

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

- **`TimImage__func_8003B5BC`**, tier C: class confirmed as `TimImage` (this round; see
  `docs/match-reports/TimImage__TimImage.md`), slot +0x084. Empty body
  (`jr $ra; nop`), no caller overrides it with anything else, so its
  purpose is unknown -- `Class__func_xxxxx` per the tier-C convention for a
  method whose class is known but mechanics are not (an empty slot has no
  mechanics to describe).
