# TimImage__func_8003B5E4 -- MATCHED (3/3 words), round 81

> Renamed from `func_8003B5E4` on 2026-09-25 (tools/rename.py). Address 0x8003b5e4.

Round 81, runner echo. Unit `src/code_2bb9c.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x098 (resolved with `tools/classtable.py D_8006E558`).
- **What:** sets the subclass field +0x048 to 1 (`ori $v0,$zero,1; jr $ra; sw $v0,0x48($a0)`). TimImage__TimImage clears the same field. No other function in the unit reads it, and no caller outside the unit reaches this slot directly, so its purpose is not established; it keeps `unk48`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 3/3,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** `TimImage__func_8003B5E4`, tier C (round 81 naming pass, runner
  bravo): the class is confirmed as `TimImage` (see `## Naming` below), but
  the field it sets (`unk48`) is written here and nowhere read within the
  unit, so what the flag MEANS is unknown -- tier-C `Class__func_xxxxx`.

## Source

```c
void TimImage__func_8003B5E4(TimImage *self) {
    self->unk48 = 1;
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

- **`TimImage__func_8003B5E4`**, tier C: class confirmed as `TimImage` (this
  round), slot +0x098. The body sets `unk48 = 1`, and `TimImage__TimImage`
  clears the same field, but nothing in this unit reads `unk48`, and no
  cross-unit caller reaches this slot directly (`TimImage` is always seen as
  an opaque handle typed per call site elsewhere in the project), so what
  the flag gates is unestablished -- `Class__func_xxxxx`.
