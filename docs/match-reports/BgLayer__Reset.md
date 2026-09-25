# BgLayer__Reset -- MATCHED (59/59 words)

> Renamed from `func_80044294` on 2026-09-25 (tools/rename.py). Address 0x80044294.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 59/59 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Reset a GsBG embedded at +0x044 of the D_8006F2C4 object over a map source whose +0x2C is a GsMAP (the D_8006F498 object lays one out: cellw/cellh 16, ncellw 20, ncellh 15). Mode 0: attribute 0x1000000, w/h = cell size x cell count; mode 1: attribute 0x2000000, 320 x 240. Then x/y/scroll 0, r,g,b from D_8008A938, map = &src->cellw, scale 0x1000/0x1000, rotate 0, mx/my = w/2, h/2.

Table slot (`tools/classtable.py`): D_8006F2C4 +0x040 (the Class6B5CC `reset` slot, called by BgLayer__BgLayer with its two arguments).

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* D_8006F2C4 +0x040: reset -- lay out the GsBG at +0x044 over a map
 * source: mode 0 sizes it to the map (cell size x cell count), mode 1 to a
 * 320 x 240 screen (with its own attribute); then zero position and
 * scroll, take the colour in D_8008A938, point it at the source's GsMAP
 * (+0x2C), unit scale, no rotation, and centre the pivot. */
typedef struct Map44294 {
    /* +0x00 */ u8 pad0[0x2C];
    /* +0x2C */ u8 cellw;         /* a GsMAP from here */
    /* +0x2D */ u8 cellh;
    /* +0x2E */ u16 ncellw;
    /* +0x30 */ u16 ncellh;
} Map44294;

extern Vec3S8 D_8008A938;

void BgLayer__Reset(Obj6F2C4 *self, Map44294 *src, s32 mode) {
    if (mode == 0) {
        self->bgAttribute = 0x1000000;
        self->w = src->cellw * src->ncellw;
        self->h = src->cellh * src->ncellh;
    } else if (mode == 1) {
        self->bgAttribute = 0x2000000;
        self->w = 320;
        self->h = 240;
    }
    self->x = 0;
    self->y = 0;
    self->scrollx = 0;
    self->scrolly = 0;
    self->unk54 = D_8008A938;
    self->map = &src->cellw;
    self->scalex = 0x1000;
    self->scaley = 0x1000;
    self->unk64 = 0;
    self->mx = self->w / 2;
    self->my = self->h / 2;
}
```

## Notes

First build that compiled (the first attempt named the field `attribute`, which Class6B5CC's own fields already use at +0x00?: `duplicate member`, caught by the `*** [...o]` grep -- the funcdiff 59/59 printed alongside it was stale). The layout +0x044..+0x067 is exactly LIBGS.H's GsBG (attribute, x, y, w, h, scrollx, scrolly, r, g, b, map, mx, my, scalex, scaley, rotate), which also explains the `20.12 fixed point` +0x064 that BgLayer__SetRotation accumulates: it is GsBG.rotate. The unit-local `Obj6F2C4` view was extended in place (pad44[0x10] / pad57[0xD] replaced by the named GsBG fields; +0x054 stays `Vec3S8 unk54` because retail copies r,g,b as a signed three-byte struct -- lb/lb/lb, sb/sb/sb -- which GsBG's three u8 fields would not give, and BgLayer__SetColor already matched on it). The fields are named `bgAttribute`, x, y, w, h, scrollx, scrolly, map, mx, my, scalex, scaley. Whole image green after the struct edit (BgLayer__SetRotation/BgLayer__SetColor still byte-exact). No shared header touched.

### Proposed learning

A Class6B5CC subclass whose fields past +0x044 read (u32, s16 x6, three bytes, a pointer, s16 x4, s32) is a GsBG, and one with a (u8, u8, u16, u16, ptr, ptr) block is a GsMAP; check LIBGS.H's GsBG/GsMAP/GsCELL before inventing field names.

## Naming

- **BgLayer__Reset**, tier B. Slot +0x040: lays out the GsBG over a map source, sized either to the map's cell grid or to a 320x240 screen.
