# New_DrawSystem -- MATCHED (20/20 words), round 81

> Renamed from `new_class_6c078` on 2026-09-25 (tools/rename.py). Address 0x800206e0.

Round 81, runner alpha. Unit `src/DrawSystem.c`. Fresh ground, no prior attempt.

- **Where:** allocator (slots resolved with `tools/classtable.py gDrawSystemMethods`).
- **What:** allocates 0x34 bytes with BMemPMgrAlloc and runs the ctor through the class table (Get_vtable_DrawSystem()->ctor); the broadcast alloc-then-ctor shape `if (p != NULL) { ctor; return p; } return NULL;`.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  20/20 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
Class6C070 *New_DrawSystem(void) {
    Class6C070 *p = BMemPMgrAlloc(0x34);

    if (p != NULL) {
        Get_vtable_DrawSystem()->ctor(p);
        return p;
    }
    return NULL;
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

`New_DrawSystem`, tier A. The class is named `DrawSystem` this round (see
`src/DrawSystem.c`'s header comment for the cross-unit evidence); the
`New_<Class>` allocator shape (alloc + call the ctor slot) is a pure
mechanic, so once the class has a name the allocator's name follows by the
project's own convention (`BasicClass.h`; matches `New_WBgm`, round 81).

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `gDrawSystem` (rename.py). Byte-identical.

## History: the `src/code_10ee0.c` unit banner (moved here in round 101, track 7)

The unit's banner carried the carve and matching history below until the
track 7 pass rewrote it as documentation of what the file holds. It is kept
here verbatim, as the unit's first function's report.

code_10ee0 -- GAME code carved from the head of psyq_10ee0 on 2026-09-25
(FINISHING-PLAN revision 18). 0x10EE0..0x11474 (vram
0x800206E0..0x80020C74). It was counted as Psy-Q SDK by segment name;
tools/gameinsdk.py measured it as game (a call into game code, a method-
table entry beside game methods, or contiguity with those, and no Sony
fingerprint). What it holds: the 19 methods of gDrawSystemMethods, the game's
screen/graphics singleton, matched as DrawSystem this round. `main.c`
builds the one instance (`New_DrawSystem`) and hands it into the game's
startup chain, which lands it in `code_2b78c.c`'s `Application__InitSystems`
as its `source` argument -- that unit dispatches `source`'s own +0x044
slot, the address this unit's table lists as `initGraph`
(`DrawSystem__InitGraph`, GsInitGraph setup), confirming the two units see
the same object. Three OTHER units independently called
`GetDrawSystem()`'s return "the draw singleton" in their own comments
before this rename, and two of them (`TimImage.c`, `code_179d8_q.c`)
independently chose the names `loadImage`/`moveImage` for the exact same
slots this unit matched as LoadImage/MoveImage -- three-way convergent
naming evidence, not a guess. libgpu/sys starts right after, at
ResetGraph (now psyq_11474).

Round 81 (bravo) matched the ten small methods/accessors; round 81
(alpha) matched ten more (the allocator, ctor, init and the RECT/VRAM
helpers). Round 82 (alpha) matched the last four (DrawSystem__InitGraph,
DrawSystem__StoreImage, DrawSystem__RunLoop, DrawSystem__ClearImage); the
unit is complete. Round 82 (bravo): naming pass -- class named DrawSystem,
every function and both gp-variable accessors renamed via
tools/rename.py, method-table slots named for the methods they hold. See
each function's report `## Naming` for tier and evidence.

Round 87 (bravo, track 4): the class is unified. Its one definition is
include/DrawSystem.h (object, table, both value types); the SDK types and
prototypes it uses come from Sony's <libgpu.h>, <libgs.h> and <libetc.h>.
