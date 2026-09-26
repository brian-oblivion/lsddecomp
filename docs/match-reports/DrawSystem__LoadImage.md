# DrawSystem__LoadImage -- MATCHED (30/30 words), round 81

> Renamed from `func_800208F8` on 2026-09-25 (tools/rename.py). Address 0x800208f8.

Round 81, runner alpha. Unit `src/code_10ee0.c`. Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x058 (slots resolved with `tools/classtable.py D_8006C070`).
- **What:** if +0x10 is clear or +0x2C set: builds a RECT on the stack via ConvertRect, LoadImage(&rect, pixels), and DrawSync(0) when +0x2C is set. The `||` in the guard gives the retail two-branch shape directly.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  30/30 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
void DrawSystem__LoadImage(Class6C070 *self, Class6C070Rect *src, u_long *pixels) {
    RECT rect;

    if (self->unk10 == 0 || self->unk2C != 0) {
        ConvertRect(&rect, src);
        LoadImage(&rect, pixels);
        if (self->unk2C != 0) {
            DrawSync(0);
        }
    }
}
```

The declarations it needs (unit-local view in `src/code_10ee0.c`; the class
structs start with `BASICCLASS_SLOTS`/`BASICCLASS_FIELDS` from
`include/BasicClass.h`, and the SDK externs are local copies of the
LIBGPU.H/LIBGS.H prototypes):

```c
typedef struct Class6C070 Class6C070;
typedef struct Class6C070Methods Class6C070Methods;
typedef struct {
    short x, y;
    short w, h;
} RECT;

typedef struct {
    /* +0x0 */ s16 x;
    /* +0x2 */ s16 y;
    /* +0x4 */ s16 w;
    /* +0x6 */ s16 unk6;
    /* +0x8 */ s16 h;
} Class6C070Rect;

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

`DrawSystem__LoadImage`, tier A. Wraps LIBGPU.H's `LoadImage`; confirmed by
CONVERGENT naming from two other units that never saw each other's code --
`code_2bb9c.c` and `code_179d8_q.c` each carry their own independent local
view of this class's method table and both independently named this exact
slot (+0x058) `loadImage`.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `gDrawSystem` (rename.py). Byte-identical.

Callers outside this unit, now through the header: TimImage__Upload (code_2bb9c) and MoviePlayer__DrawStrip (code_33808, rect cast from its own Rect45BC8).
