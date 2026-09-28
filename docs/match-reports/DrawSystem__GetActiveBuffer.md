# DrawSystem__GetActiveBuffer -- MATCHED (8/8 words), round 81

> Renamed from `func_800208D8` on 2026-09-25 (tools/rename.py). Address 0x800208d8.

Round 81, runner alpha. Unit `src/DrawSystem.c`. Fresh ground, no prior attempt.

- **Where:** gDrawSystemMethods slot +0x054 (slots resolved with `tools/classtable.py gDrawSystemMethods`).
- **What:** tail of GsGetActiveBuff (LIBGS.H `int GsGetActiveBuff(void)`); returning its value is a guess -- the bytes cannot tell void from int (tail-call wrapper rule).
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  8/8 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
s32 DrawSystem__GetActiveBuffer(Class6C070 *self) {
    return GsGetActiveBuff();
}
```

The declarations it needs (unit-local view in `src/DrawSystem.c`; the class
structs start with `BASICCLASS_SLOTS`/`BASICCLASS_FIELDS` from
`include/BasicClass.h`, and the SDK externs are local copies of the
LIBGPU.H/LIBGS.H prototypes):

```c
typedef struct Class6C070 Class6C070;
typedef struct Class6C070Methods Class6C070Methods;
typedef struct {
    /* +0x0 */ s32 w;
    /* +0x4 */ s32 h;
} Class6C070Size;

struct Class6C070 {
    BASICCLASS_FIELDS(Class6C070Methods);
    /* +0x00C */ s32 unkC;            /* 80020AF4 sets to 1 once unk24 reaches unk20 */
    /* +0x010 */ s32 unk10;           /* cleared by 8002089C; 80020B4C stores unk20 only while 0 */
    /* +0x014 */ Class6C070Size size; /* 80020C08 returns its address */
    /* +0x01C */ u8 pad1C[0x20 - 0x1C];
    /* +0x020 */ s32 unk20;           /* 80020B4C sets, 80020B68 gets */
    /* +0x024 */ s32 unk24;           /* 80020AF4 counts up to unk20 */
    /* +0x028 */ u8 pad28[0x2C - 0x28];
    /* +0x02C */ s32 unk2C;           /* 80020C3C sets */
    /* +0x030 */ s32 unk30;           /* 80020C44 sets */
};
```

## Naming

`DrawSystem__GetActiveBuffer`, tier B. A tail-call wrapper of LIBGS.H's
`GsGetActiveBuff`; the bytes cannot distinguish `void` from `int` for a
tail call (CLAUDE.md), so the `s32` return is carried from the SDK
prototype, not confirmed as used by any caller.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `gDrawSystem` (rename.py). Byte-identical.
