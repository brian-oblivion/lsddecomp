# TileMap__BuildMap -- MATCHED (47/47 words)

> Renamed from `GridIndexSrc__BuildIndex` on 2026-09-25 (tools/rename.py). Address 0x80044e64.

> Renamed from `func_80044E64` on 2026-09-25 (tools/rename.py). Address 0x80044e64.

Round 82, runner echo (graphics_resources session, echo #8), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 47/47 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Copies +0x2C of the object at +0x3C into +0x34. When +0x40 is set: +0x2E = 20, +0x2C = +0x2D = 16 (bytes), +0x30 = 15, then allocates +0x2E * +0x30 halfwords into +0x38 and fills them 0..n-1, returning. With +0x40 clear, or when the allocation fails, calls its own freeBuffer (+0x05C).

Table slot (`tools/classtable.py`): gTileMapMethods +0x078 (called by its setFlag override TileMap__Load).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/graphics_resources.c`.

```c
typedef struct Obj6F498 {
    FILERESOURCE_FIELDS(DataSrc33808Methods);
    /* +0x02C */ u8 unk2C;
    /* +0x02D */ u8 unk2D;
    /* +0x02E */ u16 unk2E;
    /* +0x030 */ u16 unk30;
    /* +0x032 */ u8 pad32[2];
    /* +0x034 */ s32 unk34;
    /* +0x038 */ u16 *unk38;
    /* +0x03C */ s32 unk3C;       /* an object: its +0x2C is read */
    /* +0x040 */ u16 unk40;
    /* +0x042 */ u16 unk42;
} Obj6F498;

/* gTileMapMethods +0x078: copy +0x2C of the object at +0x3C to +0x34; when +0x40
 * is set, lay out a 20 x 15 grid (16 x 16 cells) and fill an allocated
 * index table 0..n-1 at +0x38; otherwise, or when the allocation fails,
 * free the buffer (own +0x05C). */
void TileMap__BuildMap(Obj6F498 *self) {
    s32 n;
    s32 i;
    u16 *p;

    self->unk34 = ((DataSrc33808 *)self->unk3C)->unk2C;
    if (self->unk40 != 0) {
        self->unk2E = 20;
        self->unk2C = 16;
        self->unk2D = 16;
        self->unk30 = 15;
        n = self->unk2E * self->unk30;
        self->unk38 = BMemPMgrAlloc(n * 2);
        if (self->unk38 != NULL) {
            p = self->unk38;
            for (i = 0; i < n; i++) {
                *p++ = i;
            }
            return;
        }
    }
    self->methods->freeBuffer((DataSrc33808 *)self);
}
```

## Notes

Matched on build 2. **Lever: `mult` by a register holding a constant the function just stored = multiply by the FIELD, not the literal.** `n = self->unk2E * 15` compiled to sll/subu (GCC expands a literal multiply to shifts) and 1 word short; `n = self->unk2E * self->unk30` right after `self->unk30 = 15` lets CSE substitute the stored constant into a register and keeps the real `mult`. The +0x2E operand is reloaded (`lhu` after its own `sh`) because the two byte stores in between invalidate it, while +0x30 was stored after them. Also: the 0x28 frame (8 bytes above the outgoing-args area) came for free -- an `s32 unused[2]` on top of it overshot to 0x30; do not pad it. The loop guard is `beqz` (not `blez`) with a signed `slt` in the body from a plain `for (i = 0; i < n; i++) *p++ = i;`. Unit-local `Obj6F498` gained +0x2C..+0x38 fields in place of its pad (additive; TileMap__TileMap/TileMap__Load re-verified by the whole-image oracle).

## Naming

- **TileMap__BuildMap**, tier A. Slot +0x078: copies the TileAtlas's cell array pointer, then lays out a 20x15 grid of 16x16 cells (320x240, a full-screen tile map) and fills an index table 0..n-1.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileMap.h`. `self` is `TileMap *` (was `Obj6F498`); +0x02C..+0x03B are the GsMAP `map` (cellw, cellh, ncellw, ncellh, base, index; were unk2C, unk2D, unk2E, unk30, unk34, unk38 -- LIBGS.H's own layout, the pad at +0x032 is its alignment gap), +0x03C `atlas`, +0x040 `defaultGrid`. The atlas's +0x02C is still read through the unit-local `DataSrc33808` cast (TileAtlas, gTileAtlasMethods, is not unified) and cast to `struct GsCELL *`. Byte-identical.

Later the same round (alpha, third class): TileAtlas unified (`include/TileAtlas.h`). TileMap::atlas (+0x03C) is `struct TileAtlas *` (was `FileResource *`), so the base is read as `self->atlas->cells` -- no `DataSrc33808` cast and no `struct GsCELL *` cast. Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `20`, `15`, `16` | `TILEMAP_COLS`, `TILEMAP_ROWS`, `TILE_SIZE` | A | the GsMAP's ncellw, ncellh and cellw/cellh |
| `n * 2` | `n * sizeof(*self->map.index)` | A | the u16 index table |

## Round 95 (alpha, track 6: Sony headers)

`map` is <libgs.h>'s GsMAP (the header's local copy, same layout and field names, deleted); `base` is Sony's `GsCELL *`. No accessor changed. Byte-identical.
