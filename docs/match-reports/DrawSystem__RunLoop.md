# DrawSystem__RunLoop -- MATCHED (32/32 words), round 82

> Renamed from `func_80020A74` on 2026-09-25 (tools/rename.py). Address 0x80020a74.

Round 82, runner alpha. Unit `src/graphics/DrawSystem.c`. Fresh ground, no prior attempt.

- **Where:** gDrawSystemMethods slot +0x068 (`tools/classtable.py gDrawSystemMethods`);
  DrawSystem__Start calls it through `methods->slot68` after setting +0x10.
- **What:** the frame loop. While +0x10 is set: `VSync(self->unk20)`, call the
  optional callback at +0x30, then `notifyParents(self, 2)` (BasicClass slot
  +0x030).
- **Struct change:** +0x030 retyped from `s32 unk30` to
  `void (*callback)(void)` (the `jalr $v0` on it is the evidence).
  DrawSystem__SetCallback's parameter and DrawSystem__Init's `= 0` store follow; both stay
  byte-exact. Local view only.
- **Result:** byte-exact on the FIRST build; 32/32 words, 0 insertions /
  0 deletions, whole-image SHA1 green. A plain `while` gives the retail
  top-test + bottom-test rotated loop.

## Source

```c
extern int VSync(int mode);                         /* LIBETC.H */

void DrawSystem__RunLoop(Class6C070 *self) {
    while (self->unk10 != 0) {
        VSync(self->unk20);
        if (self->callback != NULL) {
            self->callback();
        }
        self->methods->notifyParents(self, 2);
    }
}
```

Needs the unit-local `Class6C070` view at the top of `src/graphics/DrawSystem.c`, with
`/* +0x030 */ void (*callback)(void);` and the `BASICCLASS_SLOTS` method table
(`notifyParents` at +0x030).

## Naming

`DrawSystem__RunLoop`, tier B. The VSync-synced frame loop: while `running`,
`VSync(unk20)`, an optional per-frame callback, then `notifyParents(self, 2)`
(BasicClass's own tick-broadcast slot) -- so every BasicClass object that
`addChild`s this singleton gets event 2 once per VSync (`WBgm.c`'s
`WBgm__WBgm`, `DayTaskStageMap.c`'s `StageMap__StageMap`, both do). +0x10 is
named `running`: `DrawSystem__Start` sets it and enters this loop,
`DrawSystem__Stop` clears it.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `sDrawSystem` (rename.py). Byte-identical.
