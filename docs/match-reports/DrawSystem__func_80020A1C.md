# DrawSystem__func_80020A1C -- MATCHED (2/2 words), round 81

> Renamed from `func_80020A1C` on 2026-09-25 (tools/rename.py). Address 0x80020a1c.

Round 81, runner bravo. Unit `src/code_10ee0.c` (carved from `psyq_10ee0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x060 (method-table slots resolved with `tools/classtable.py D_8006C070`).
- **What:** returns 0 (`jr $ra; addu $v0,$zero,$zero`).
- **Result:** byte-exact on the first build; `funcdiff.py` reports 0
  insertions / 0 deletions and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Name:** kept as the bare `func_` name; the class D_8006C070 has no
  confirmed name yet.

## Source

```c
s32 DrawSystem__func_80020A1C(Class6C070 *self) {
    return 0;
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
extern Class6C070 *D_8008A83C;
extern void GsSwapDispBuff(void);
```
