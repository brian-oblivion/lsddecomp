# DrawSystem__InitGraph -- MATCHED (32/32 words), round 82

> Renamed from `func_800207DC` on 2026-09-25 (tools/rename.py). Address 0x800207dc.

Round 82, runner alpha. Unit `src/graphics/DrawSystem.c`. Fresh ground, no prior attempt.

- **Where:** gDrawSystemMethods slot +0x044 (`tools/classtable.py gDrawSystemMethods`).
- **What:** graphics setup. `GsInitGraph(w, h, 0, 1, vramMode)`,
  `GsDefDispBuff(0, 0, 0, h)`, then copies the 8-byte `{w, h}` size into
  self+0x14 and stores the vram mode at self+0x1C (a new field `unk1C`
  replacing the unit-local `pad1C`; local struct only, whole image green).
- **Result:** byte-exact on the FIRST build, no levers; 32/32 words,
  0 insertions / 0 deletions, whole-image SHA1 green.
- **Shape notes:** the `lhu 0x0`/`lhu 0x4` loads of the s32 fields fall out of
  the `unsigned short` prototypes (cc1 narrows the load itself on
  little-endian); the fifth argument's `andi 0xFFFF` goes to the stack slot
  while the unmasked `$s1` is what reaches `self->unk1C`, so the parameter is
  a full-width `s32`. The `lw`/`lw` + `sw`/`sw` is a plain struct assignment.

## Source

```c
extern void GsInitGraph(unsigned short x_res, unsigned short y_res,
                        unsigned short intmode, unsigned short dith,
                        unsigned short varmmode);   /* LIBGS.H */
extern void GsDefDispBuff(unsigned short x0, unsigned short y0,
                          unsigned short x1, unsigned short y1); /* LIBGS.H */

void DrawSystem__InitGraph(Class6C070 *self, Class6C070Size *size, s32 vramMode) {
    GsInitGraph(size->w, size->h, 0, 1, vramMode);
    GsDefDispBuff(0, 0, 0, size->h);
    self->size = *size;
    self->unk1C = vramMode;
}
```

Needs the unit-local view at the top of `src/graphics/DrawSystem.c`
(`Class6C070Size` is `{ s32 w; s32 h; }`; `Class6C070` gains
`/* +0x01C */ s32 unk1C;`).

## Naming

`DrawSystem__InitGraph`, tier B. Wraps GsInitGraph/GsDefDispBuff; confirmed
as the object's own +0x044 slot from OUTSIDE the unit too --
`Application.c`'s `Application__InitSystems` receives this same object as its
`source` argument and dispatches `source->methods->slot44(source, &self->dims,
self->dimsArg)`, the identical offset.

## Track 4 (2026-09-26, round 87, bravo)

The class is unified: its one definition is `include/DrawSystem.h` (object, method table, `ScreenDims`, `DrawRect`); the unit-local view the declarations above quote is gone. Field renames, settled by this unit's accessors (the only ones): `unk20` -> `vsyncCount` (SetVSyncCount stores, GetVSyncCount returns, RunLoop passes it to VSync, CountFrames compares against it), `unk24` -> `frameCount` (CountFrames counts it). `DrawSystemRect`/`DrawSystemDims` are one type, `DrawRect` {s16 x, y; s32 w, h} (ConvertRect's halfword loads at +0/+2/+4/+8 compile identically from it), and `DrawSystemSize` is `ScreenDims`. The singleton `D_8008A83C` is `sDrawSystem` (rename.py). Byte-identical.

Its dispatching caller, Application__InitSystems (code_2b78c), now takes a `DrawSystem *` (include/Application.h's ApplicationSource view deleted) and passes its own `ScreenDims`.
