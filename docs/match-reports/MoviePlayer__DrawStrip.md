# MoviePlayer__DrawStrip -- MATCHED (51/51 words)

> Renamed from `func_80045BC8` on 2026-09-25 (tools/rename.py). Address 0x80045bc8.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 51/51 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Uploads the strip buffer at +0x1C into the rectangle at +0x2C through the DrawSystem singleton's +0x058 (LoadImage per bravo's round-82 DrawSystem naming), advances rect.x by rect.w; while rect.x is still below +0x20 + +0x24, decodes the next strip (DecDCTout(+0x1C, +0x38), preceded by DrawSync(0) when +0x34 < 0x80); otherwise sets +0x4C = 1, rewinds the rectangle to (+0x20, +0x22), and sets +0x44 when +0x48 is set. The MDEC movie-strip pump of D_8006F614.

Table slot (`tools/classtable.py`): D_8006F614 +0x060.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
typedef struct DrawSysMethods45BC8 {
    /* +0x000 */ u8 pad0[0x58];
    /* +0x058 */ void (*loadImage)();
} DrawSysMethods45BC8;

typedef struct DrawSys45BC8 {
    /* +0x000 */ DrawSysMethods45BC8 *methods;
} DrawSys45BC8;

typedef struct Rect45BC8 {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect45BC8;

typedef struct Obj45BC8 {
    /* +0x000 */ u8 pad0[0x1C];
    /* +0x01C */ u32 *strip;
    /* +0x020 */ s16 x0;
    /* +0x022 */ s16 y0;
    /* +0x024 */ s32 width;
    /* +0x028 */ u8 pad28[4];
    /* +0x02C */ Rect45BC8 rect;
    /* +0x034 */ s32 unk34;
    /* +0x038 */ s32 stripSize;
    /* +0x03C */ u8 pad3C[8];
    /* +0x044 */ s32 unk44;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
} Obj45BC8;

void MoviePlayer__DrawStrip(Obj45BC8 *self) {
    DrawSys45BC8 *ds = GetDrawSystem();

    ds->methods->loadImage(ds, &self->rect, self->strip);
    self->rect.x += self->rect.w;
    if (self->rect.x < self->x0 + self->width) {
        if (self->unk34 < 0x80) {
            DrawSync(0);
        }
        DecDCTout(self->strip, self->stripSize);
    } else {
        self->unk4C = 1;
        self->rect.x = self->x0;
        self->rect.y = self->y0;
        if (self->unk48 != 0) {
            self->unk44 = 1;
        }
    }
}
```

## Notes

First build. DrawSystem is reached through a per-function methods view (the type lives in code_10ee0.c, not a header); DrawSync/DecDCTout declared locally with the Psy-Q prototypes. Field names (strip, x0, y0, width, stripSize) are readings of this one function.

## Naming

- **MoviePlayer__DrawStrip**, tier A. Slot +0x060: uploads the decoded strip, steps the draw rectangle, and decodes the next strip while still inside the frame; marks the frame done at the end.
