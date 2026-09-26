# DrawSystem__SetCallback -- MATCHED (2/2 words), round 81

> Renamed from `func_80020C44` on 2026-09-25 (tools/rename.py). Address 0x80020c44.

Round 81, runner bravo. Unit `src/code_10ee0.c` (carved from `psyq_10ee0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x084 (method-table slots resolved with `tools/classtable.py D_8006C070`).
- **What:** setter of the s32 at +0x30.
- **Result:** byte-exact on the first build; `funcdiff.py` reports 0
  insertions / 0 deletions and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Name:** kept as the bare `func_` name; the class D_8006C070 has no
  confirmed name yet.

## Source

```c
void DrawSystem__SetCallback(Class6C070 *self, s32 value) {
    self->unk30 = value;
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

## Round 82 note

Round 82 (alpha) retyped +0x030 from `s32 unk30` to `void (*callback)(void)`: DrawSystem__RunLoop calls it (`jalr`) once per VSync. The live source now reads `self->callback = ...`; DrawSystem__SetCallback takes `void (*callback)(void)`. Still byte-exact (whole-image SHA1 green).

## Naming

`DrawSystem__SetCallback`, tier A. A pure setter of the `callback` field
`DrawSystem__RunLoop` invokes once per VSync; mechanics ARE the purpose for
a plain setter.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `gDrawSystem` (rename.py). Byte-identical.

Slot +0x084 `setCallback` was missing from the old local table view; the header has it. Its one C caller is SetCdDriverMode (code_179d8_q), which installs ServiceCdDriver or clears it with 0.
