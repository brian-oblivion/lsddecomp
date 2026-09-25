# DrawSystem__ClearImage -- MATCHED (37/37 words), round 82

> Renamed from `func_80020B74` on 2026-09-25 (tools/rename.py). Address 0x80020b74.

Round 82, runner alpha. Unit `src/code_10ee0.c`. Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x078 (`tools/classtable.py D_8006C070`).
- **What:** clear a rectangle to an RGB colour. With a NULL rect it asks
  slot +0x07C (DrawSystem__GetDims) for the whole-screen dims `{0, 0, w, h*2}` and
  re-dispatches itself through slot +0x078 with that as the rect; otherwise
  `ClearImage(&rect, color[0], color[1], color[2])`.
- **Why the cast is honest:** `Class6C070Dims` is `{s16 x, y; s32 w; s32 h}`,
  and `Class6C070Rect` reads x/y/w at +0/+2/+4 and h at +8, i.e. the low
  halves of the two s32s on little-endian. That is exactly why
  ConvertRect's odd +8 `h` offset exists.
- **Method slots typed:** `slot78` and `slot7C` in the unit-local
  `Class6C070Methods` (previously `void *`). Local view only.
- **Result:** byte-exact on the FIRST build; 37/37 words, 0 insertions /
  0 deletions, whole-image SHA1 green. Declaring `dims` before `rect` gives
  retail's frame (dims at sp+0x10, rect at sp+0x20).

## Source

```c
extern int ClearImage(RECT *rect, u_char r, u_char g, u_char b); /* LIBGPU.H */

/* in Class6C070Methods: */
/* +0x078 */ void (*slot78)(Class6C070 *self, u8 *color, Class6C070Rect *src); /* DrawSystem__ClearImage */
/* +0x07C */ Class6C070Size *(*slot7C)(Class6C070 *self, Class6C070Dims *out); /* DrawSystem__GetDims */

void DrawSystem__ClearImage(Class6C070 *self, u8 *color, Class6C070Rect *src) {
    Class6C070Dims dims;
    RECT rect;

    if (src == NULL) {
        self->methods->slot7C(self, &dims);
        self->methods->slot78(self, color, (Class6C070Rect *)&dims);
    } else {
        ConvertRect(&rect, src);
        ClearImage(&rect, color[0], color[1], color[2]);
    }
}
```

Needs the unit-local `Class6C070`, `Class6C070Methods`, `RECT`,
`Class6C070Rect`, `Class6C070Dims` and `Class6C070Size` view at the top of
`src/code_10ee0.c`.

## Naming

Proposed `Class6C070__ClearImage` (tier B).
