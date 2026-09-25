# Class6E4F0__SetScreenDims

> Renamed from `func_8003B02C` on 2026-09-25 (tools/rename.py). Address 0x8003b02c.

**Round 81 (delta)** · **Unit:** code_2b78c · **Size:** 6 words · **Status:** MATCHED (6/6 words, whole-image SHA1 green)

## What it does

Slot `+0x040` of D_8006E4F0. Copies an 8-byte pair into `self+0x0C` and
stores the third argument at `self+0x14`. The ctor (Class6E4F0__Class6E4F0) calls it
as `setDims(self, &D_8008A8E0, 0)`, and D_8008A8E0 in sdata is
`{0x140, 0xF0}` = {320, 240}: a screen size. func_8003B044 later hands
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

Kept. Suggested `Class6E4F0__SetScreenDims` (tier B: the {320,240}
default is the evidence; what consumes the pair is not yet known).
