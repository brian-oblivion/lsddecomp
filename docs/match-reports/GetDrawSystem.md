# GetDrawSystem -- MATCHED (3/3 words), round 81

> Renamed from `func_80020C5C` on 2026-09-25 (tools/rename.py). Address 0x80020c5c.

Round 81, runner bravo. Unit `src/code_10ee0.c` (carved from `psyq_10ee0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** not a method: the singleton getter (method-table slots resolved with `tools/classtable.py D_8006C070`).
- **What:** returns the gp-relative sdata global gDrawSystem (`lw %gp_rel`).
- **Result:** byte-exact on the first build; `funcdiff.py` reports 0
  insertions / 0 deletions and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Name:** kept as the bare `func_` name; the class D_8006C070 has no
  confirmed name yet.

## Source

```c
Class6C070 *GetDrawSystem(void) {
    return gDrawSystem;
}
```

The unit-local view it needs, from the top of `src/code_10ee0.c`:

```c
#include "BasicClass.h"

typedef struct Class6C070 Class6C070;
typedef struct Class6C070Methods Class6C070Methods;
struct Class6C070Methods {
    BASICCLASS_SLOTS(Class6C070, (Class6C070 *self));
};
struct Class6C070 {
    BASICCLASS_FIELDS(Class6C070Methods);
    /* +0x00C */ u8 pad0C[0x10 - 0x0C];
    /* +0x010 */ s32 unk10;
    /* +0x014 */ u8 pad14[0x20 - 0x14];
    /* +0x020 */ s32 unk20;
    /* +0x024 */ u8 pad24[0x2C - 0x24];
    /* +0x02C */ s32 unk2C;
    /* +0x030 */ s32 unk30;
};
extern Class6C070Methods D_8006C070;
extern Class6C070 *gDrawSystem;
extern void GsSwapDispBuff(void);
```

## Naming

`GetDrawSystem`, tier A. The singleton getter for `gDrawSystem`; three other
units (`code_2bb9c.c`, `code_179d8_q.c`, `code_2a0e0.c`) independently
called this function's return "the draw singleton" in their own comments
before this rename -- convergent naming from callers that never saw each
other's code.
