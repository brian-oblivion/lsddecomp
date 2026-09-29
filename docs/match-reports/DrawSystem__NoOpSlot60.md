# DrawSystem__NoOpSlot60 -- MATCHED (2/2 words), round 81

> Renamed from `DrawSystem__func_80020A1C` on 2026-09-28 (tools/rename.py). Address 0x80020a1c.

> Renamed from `func_80020A1C` on 2026-09-25 (tools/rename.py). Address 0x80020a1c.

Round 81, runner bravo. Unit `src/graphics/draw_system.c` (carved from `psyq_10ee0` in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt.

- **Where:** gDrawSystemMethods slot +0x060 (method-table slots resolved with `tools/classtable.py gDrawSystemMethods`).
- **What:** returns 0 (`jr $ra; addu $v0,$zero,$zero`).
- **Result:** byte-exact on the first build; `funcdiff.py` reports 0
  insertions / 0 deletions and the whole-image SHA1 is green
  (`OK: build matches retail`).
- **Name:** kept as the bare `func_` name; the class gDrawSystemMethods has no
  confirmed name yet.

## Source

```c
s32 DrawSystem__NoOpSlot60(Class6C070 *self) {
    return 0;
}
```

The unit-local view it needs, from the top of `src/graphics/draw_system.c`:

```c
#include "basic_class.h"

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
extern Class6C070Methods gDrawSystemMethods;
extern Class6C070 *sDrawSystem;
extern void GsSwapDispBuff(void);
```

## Naming

Kept the tier-C `DrawSystem__NoOpSlot60` form (class known, function
purpose not): the body is `return 0;` with no caller in this unit and no
other evidence of what the constant answers. A guessed name (e.g. "CanX")
would be worse than the placeholder.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/draw_system.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `sDrawSystem` (rename.py). Byte-identical.

## Track 7 naming (2026-09-28, round 101, alpha)

Renamed `DrawSystem__func_80020A1C` -> `DrawSystem__NoOpSlot60`
(tools/rename.py), tier A by the naming rules' leaf clause: the body is
`return 0;` and nothing else, so its mechanics are all there is to name. The
form is the project's existing one for a constant-return empty slot
(`DreamSys__BeforeMoveCommand`, also `s32` returning 0) and for the empty +0x060
overrides of the other tables (`CdStream__NoOpSlot60`). The section above
saying the placeholder was kept predates this rename. The slot keeps the
name `slot60`, the house form for an empty slot (`dream_sys.h`,
`file_resource.h`, `task_core.h`). Nothing in C calls it.
