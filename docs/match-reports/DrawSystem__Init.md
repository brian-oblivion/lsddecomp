# DrawSystem__Init -- MATCHED (22/22 words), round 81

> Renamed from `func_80020784` on 2026-09-25 (tools/rename.py). Address 0x80020784.

Round 81, runner alpha. Unit `src/DrawSystem.c`. Fresh ground, no prior attempt.

- **Where:** gDrawSystemMethods slot +0x040 (init) (slots resolved with `tools/classtable.py gDrawSystemMethods`).
- **What:** clears +0x10, calls slot +0x070 (DrawSystem__SetVSyncCount) with 3 and slot +0x080 (DrawSystem__SetSyncMode) with 1, clears +0x30.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  22/22 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
void DrawSystem__Init(Class6C070 *self) {
    self->unk10 = 0;
    self->methods->slot70(self, 3);
    self->methods->slot80(self, 1);
    self->unk30 = 0;
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

`DrawSystem__Init`, tier B. It occupies the +0x040 slot dispatched by the
ctor and sets up default state (running=0, VSync count=3, sync mode=1,
callback=NULL); "init" describes what it does, but why these particular
defaults isn't established.

## Round 82 note

Round 82 (alpha) retyped +0x030 from `s32 unk30` to `void (*callback)(void)`: DrawSystem__RunLoop calls it (`jalr`) once per VSync. The live source now reads `self->callback = ...`; DrawSystem__SetCallback takes `void (*callback)(void)`. Still byte-exact (whole-image SHA1 green).

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `gDrawSystem` (rename.py). Byte-identical.
