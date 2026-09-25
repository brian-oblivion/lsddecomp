# TimImage__GetTimInfo -- MATCHED (9/9 words), round 81

> Renamed from `func_8003B5F0` on 2026-09-25 (tools/rename.py). Address 0x8003b5f0.

Round 81, runner echo. Unit `src/code_2bb9c.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** TimImage's table (`gTimImageMethods`) slot +0x09C (resolved with `tools/classtable.py D_8006E558`).
- **What:** calls `GsGetTimInfo(buffer + 4, tim)`: it loads `self->buffer` (Class6D430 +0x010), steps past the TIM id word and passes `a1` through unchanged. That makes the class a data source whose buffer holds a TIM image. `GsIMAGE` is declared opaque in the unit, because the function only passes it through. The prototype is LIBGS.H's own, spelled with `u32 *`.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 9/9,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** `TimImage__GetTimInfo`, tier A (round 81 naming pass, runner
  bravo). See `## Naming` below.

## Source

```c
void TimImage__GetTimInfo(TimImage *self, GsIMAGE *tim) {
    GsGetTimInfo((u32 *)self->buffer + 1, tim);
}
```

The unit-local view it needs, from the top of `src/code_2bb9c.c`:

```c
#include "Class6D430.h"

typedef struct TimImage {
    CLASS6D430_FIELDS(Class6D430Methods);
    /* +0x02C */ u8 pad2C[0x1C];
    /* +0x048 */ s32 unk48;
} TimImage;

typedef struct GsIMAGE GsIMAGE;
void GsGetTimInfo(u32 *im, GsIMAGE *tim);
extern Class6D430Methods gTimImageMethods;
```

## Naming

- **`TimImage__GetTimInfo`** (was `func_8003B5F0`), tier A: slot +0x09C,
  already spelled `getTimInfo` in the vtable declaration before this pass;
  a one-line wrapper around Sony's `GsGetTimInfo` on `buffer + 4` (past the
  TIM id word) -- mechanics are the purpose.
