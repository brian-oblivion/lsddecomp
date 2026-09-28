# DrawSystem__DrawSystem -- MATCHED (21/21 words), round 81

> Renamed from `func_80020730` on 2026-09-25 (tools/rename.py). Address 0x80020730.

Round 81, runner alpha. Unit `src/graphics/DrawSystem.c`. Fresh ground, no prior attempt.

- **Where:** gDrawSystemMethods slot +0x008 (ctor) (slots resolved with `tools/classtable.py gDrawSystemMethods`).
- **What:** base ctor through GetBasicClassMethods()->ctor, then installs the table from GetDrawSystemMethods() and calls slot +0x040 (DrawSystem__Init). The `sw v0,0(s0); lw v0,0x40(v0)` reuse falls out of plain sequential C.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  21/21 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
void DrawSystem__DrawSystem(Class6C070 *self) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetDrawSystemMethods();
    self->methods->init(self);
}
```

The declarations it needs (unit-local view in `src/graphics/DrawSystem.c`; the class
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

struct Class6C070Methods {
    BASICCLASS_SLOTS(Class6C070, (Class6C070 *self));
    /* +0x040 */ void (*init)(Class6C070 *self);                 /* DrawSystem__Init */
    /* +0x044 */ void *slot44;
    /* +0x048 */ void *slot48;
    /* +0x04C */ void *slot4C;
    /* +0x050 */ void *slot50;
    /* +0x054 */ void *slot54;
    /* +0x058 */ void *slot58;
    /* +0x05C */ void *slot5C;
    /* +0x060 */ void *slot60;
    /* +0x064 */ void *slot64;
    /* +0x068 */ void (*slot68)(Class6C070 *self);               /* DrawSystem__RunLoop */
    /* +0x06C */ void *slot6C;
    /* +0x070 */ void (*slot70)(Class6C070 *self, s32 value);    /* DrawSystem__SetVSyncCount */
    /* +0x074 */ void *slot74;
    /* +0x078 */ void *slot78;
    /* +0x07C */ void *slot7C;
    /* +0x080 */ void (*slot80)(Class6C070 *self, s32 value);    /* DrawSystem__SetSyncMode */
};
```

## Naming

`DrawSystem__DrawSystem`, tier A. The class is named `DrawSystem` this round
(see `src/graphics/DrawSystem.c`'s header comment); this function occupies the +0x008
ctor slot, and `Class__Class` is the project's ctor-naming convention
(`BasicClass.h`).

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `sDrawSystem` (rename.py). Byte-identical.
