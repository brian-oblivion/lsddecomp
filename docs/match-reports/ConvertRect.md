# ConvertRect -- MATCHED (12/12 words), round 81

> Renamed from `func_80020970` on 2026-09-25 (tools/rename.py). Address 0x80020970.

Round 81, runner alpha. Unit `src/DrawSystem.c`. Fresh ground, no prior attempt.

- **Where:** helper (not a table slot) (slots resolved with `tools/classtable.py gDrawSystemMethods`).
- **What:** copies x/y/w from +0/+2/+4 and h from +8 of a source record into a RECT (lhu/sh pairs from plain s16 member copies).
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  12/12 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
void ConvertRect(RECT *dst, Class6C070Rect *src) {
    dst->x = src->x;
    dst->y = src->y;
    dst->w = src->w;
    dst->h = src->h;
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
```

## Naming

`ConvertRect`, tier A. A pure field-by-field copy from the class's own
odd-shaped rect record (`DrawSystemRect`) into LIBGPU.H's `RECT`; mechanics
ARE the purpose. Not a method (no table slot); the source struct's unknown
`+0x6` short is still unidentified.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `gDrawSystem` (rename.py). Byte-identical.
