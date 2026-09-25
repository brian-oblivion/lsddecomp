# GetTimImageMethods -- MATCHED (4/4 words), round 81

> Renamed from `func_8003B614` on 2026-09-25 (tools/rename.py). Address 0x8003b614.

Round 81, runner echo. Unit `src/code_2bb9c.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** not a slot: the table getter for gTimImageMethods.
- **What:** the class's method-table getter (`lui/addiu %hi/%lo(gTimImageMethods)`). It is not a table slot. func_8003B39C and TimImage__TimImage call it. The table is declared `extern Class6D430Methods gTimImageMethods;` in the unit, which is a base-class view of a 39-slot table.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 4/4,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** kept as the bare `func_` name. The class (gTimImageMethods, a Class6D430
  data-source subclass) has no confirmed name yet, so no class prefix is
  justified.

## Source

```c
Class6D430Methods *GetTimImageMethods(void) {
    return &gTimImageMethods;
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
