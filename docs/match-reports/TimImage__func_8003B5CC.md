# TimImage__func_8003B5CC -- MATCHED (2/2 words), round 81

> Renamed from `func_8003B5CC` on 2026-09-25 (tools/rename.py). Address 0x8003b5cc.

Round 81, runner echo. Unit `src/code_2bb9c.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** gTimImageMethods slot +0x08C (resolved with `tools/classtable.py gTimImageMethods`).
- **What:** empty slot override: `jr $ra; nop`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 2/2,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** kept as the bare `func_` name. The class (gTimImageMethods, a Class6D430
  data-source subclass) has no confirmed name yet, so no class prefix is
  justified.

## Source

```c
void TimImage__func_8003B5CC(void) {
}
```

The unit-local view it needs, from the top of `src/code_2bb9c.c`:

```c
#include "Class6D430.h"

typedef struct D_8006E558Obj {
    CLASS6D430_FIELDS(Class6D430Methods);
    /* +0x02C */ u8 pad2C[0x1C];
    /* +0x048 */ s32 unk48;
} D_8006E558Obj;

typedef struct GsIMAGE GsIMAGE;
void GsGetTimInfo(u32 *im, GsIMAGE *tim);
extern Class6D430Methods gTimImageMethods;
```
