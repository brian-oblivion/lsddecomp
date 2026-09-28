# GetTimImageMethods -- MATCHED (4/4 words), round 81

> Renamed from `func_8003B614` on 2026-09-25 (tools/rename.py). Address 0x8003b614.

Round 81, runner echo. Unit `src/graphics/TimImage.c` (carved from `psyq_2bb9c` in
FINISHING-PLAN revision 18). This was fresh ground with no prior attempt.

- **Where:** not a slot: the table getter for TimImage's table `gTimImageMethods`.
- **What:** the class's method-table getter (`lui/addiu %hi/%lo(gTimImageMethods)`). It is not a table slot. `New_TimImage` and `TimImage__TimImage` call it. The table is declared `extern FileResourceMethods gTimImageMethods;` in this report's own local view, which is a base-class view of a 39-slot table; `src/graphics/TimImage.c` itself now declares the full `TimImageMethods` view.
- **Result:** byte-exact on the first build. `funcdiff.py` reports 4/4,
  and the whole-image SHA1 is green (`OK: build matches retail`).
- **Name:** `GetTimImageMethods`, tier A (round 81 naming pass, runner
  bravo): matches the project's established `Get<Class>Methods` convention
  for table getters. See `## Naming` below.

## Source

```c
TimImageMethods *GetTimImageMethods(void) {
    return &gTimImageMethods;
}
```

The unit-local view it needs, from the top of `src/graphics/TimImage.c`:

```c
#include "FileResource.h"

typedef struct D_8006E558Obj {
    FILERESOURCE_FIELDS(FileResourceMethods);
    /* +0x02C */ u8 pad2C[0x1C];
    /* +0x048 */ s32 unk48;
} D_8006E558Obj;

typedef struct GsIMAGE GsIMAGE;
void GsGetTimInfo(u32 *im, GsIMAGE *tim);
extern FileResourceMethods gTimImageMethods;
```

## Naming

- **`GetTimImageMethods`** (was `func_8003B614`), tier A: matches the
  project's established `Get<Class>Methods` table-getter convention
  (`GetFileResourceMethods`, `GetDayTaskMethods`, ...); the body is exactly
  `return &gTimImageMethods;`.
