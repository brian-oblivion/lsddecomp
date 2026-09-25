# DrawSystem__GetDims -- MATCHED (13/13 words), round 81

> Renamed from `func_80020C08` on 2026-09-25 (tools/rename.py). Address 0x80020c08.

Round 81, runner alpha. Unit `src/code_10ee0.c`. Fresh ground, no prior attempt.

- **Where:** D_8006C070 slot +0x07C (slots resolved with `tools/classtable.py D_8006C070`).
- **What:** if out is non-NULL writes {0, 0, size.w, size.h * 2}; returns &self->size (+0x14).
- **Result:** byte-exact on the FIRST build, no levers; `funcdiff.py` reports
  13/13 words, 0 insertions / 0 deletions, and the whole-image SHA1 is
  green (`OK: build matches retail`).

## Source

```c
Class6C070Size *DrawSystem__GetDims(Class6C070 *self, Class6C070Dims *out) {
    if (out != NULL) {
        out->x = 0;
        out->y = 0;
        out->w = self->size.w;
        out->h = self->size.h * 2;
    }
    return &self->size;
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
    /* +0x0 */ s16 x;
    /* +0x2 */ s16 y;
    /* +0x4 */ s32 w;
    /* +0x8 */ s32 h;
} Class6C070Dims;

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

Kept func_.
