# Class6E4F0__SetScreenDims

> Renamed from `func_8003B02C` on 2026-09-25 (tools/rename.py). Address 0x8003b02c.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 6 words · **Status:** MATCHED (6/6 words, whole-image SHA1 green)

## What it does

Slot `+0x040` of D_8006E4F0. Copies an 8-byte pair into `self+0x0C` and
stores the third argument at `self+0x14`. The ctor (Class6E4F0__Class6E4F0) calls it
as `setDims(self, &gDefaultScreenDims, 0)`, and gDefaultScreenDims in sdata is
`{0x140, 0xF0}` = {320, 240}: a screen size. Class6E4F0__InitSystems later hands
`&self->dims` and `self->dimsArg` to its source object's `+0x044` slot.

```c
typedef struct ScreenDims { s32 w; s32 h; } ScreenDims;

void Class6E4F0__SetScreenDims(Class6E4F0 *self, ScreenDims *dims, s32 arg) {
    self->dims = *dims;
    self->dimsArg = arg;
}
```

Whole-struct assignment of a 4-aligned 8-byte struct gives the
`lw,lw,sw,sw` retail shows; the third store fills the `jr` delay slot.

## Naming

**Round 81 (delta), track 3.** Renamed `func_8003B02C` -> `Class6E4F0__SetScreenDims`.
**Tier B**: the mechanics (copy an 8-byte pair + a third word into `self`)
are a plain setter, but the name also claims what the pair MEANS. That claim
rests on the ctor's default argument being `gDefaultScreenDims` =
`{0x140, 0xF0}` = {320, 240}, the PS1's standard NTSC/PAL frame-buffer
resolution -- strong but single-source evidence, not two agreeing callers,
so kept at B rather than A. What the stored pair is later used FOR (it is
forwarded to a `source` object's own +0x044 slot in
`Class6E4F0__InitSystems`) is still not established.

## Track 4

**2026-09-25, round 84 (echo).** The class is declared once, in
`include/Class6E4F0.h`. `dimsArg` (+0x014) is now `vramMode`, and this
function's `arg` likewise: InitSystems passes it as the third argument of the
draw system's +0x044 slot, `DrawSystem__InitGraph(self, size, vramMode)`
(code_10ee0), which hands it to GsInitGraph as the vram mode. Bytes unchanged.
