# DrawSystem__MoveImage -- MATCHED (20/20 words), round 81

> Renamed from `func_80020A24` on 2026-09-25 (tools/rename.py). Address 0x80020a24.

Round 81, runner alpha. Unit `src/code_10ee0.c`. Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x064 (slots resolved with `tools/classtable.py D_8006C070`).
- **What:** RECT from ConvertRect, then MoveImage(&rect, x, y). Declaring x/y as `s16` parameters gives retail exactly: the raw args are held in s0/s1 across the call and sign-extended (sll/sra 16) only at the MoveImage call.
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  20/20 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
void DrawSystem__MoveImage(Class6C070 *self, Class6C070Rect *src, s16 x, s16 y) {
    RECT rect;

    ConvertRect(&rect, src);
    MoveImage(&rect, x, y);
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

`DrawSystem__MoveImage`, tier A. Wraps LIBGPU.H's `MoveImage`; confirmed by
convergent naming -- `code_179d8_q.c`'s own independent local view of this
class's method table names this exact slot (+0x064) `moveImage`, and
`code_2bb9c.c`'s `func_8003B624` calls it through a local `moveImage`
function-pointer variable read from the same slot.
