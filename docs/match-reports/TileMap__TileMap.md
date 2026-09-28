# TileMap__TileMap -- MATCHED (34/34 words)

> Renamed from `GridIndexSrc__GridIndexSrc` on 2026-09-25 (tools/rename.py). Address 0x80044d40.

> Renamed from `func_80044D40` on 2026-09-25 (tools/rename.py). Address 0x80044d40.

Round 82, runner echo (graphics_resources session, echo #8), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 34/34 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: active driver's ctor, install gTileMapMethods (GetTileMapMethods), store the third argument at +0x3C, clear +0x42; when the second argument is 0, set +0x40 = 1, clear +0x2A and call its own +0x064 (TileMap__Load).

Table slot (`tools/classtable.py`): gTileMapMethods +0x008.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/graphics_resources.c`.

```c
typedef struct Obj6F498 {
    FILERESOURCE_FIELDS(DataSrc33808Methods);
    /* +0x02C */ u8 pad2C[0x10];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ u16 unk40;
    /* +0x042 */ u16 unk42;
} Obj6F498;

/* gTileMapMethods +0x008: constructor -- the active driver's, then this table;
 * store `arg2` at +0x3C, clear +0x42, and with no `arg1` set +0x40, clear
 * +0x2A and run its own +0x064. */
void TileMap__TileMap(Obj6F498 *self, s32 arg1, s32 arg2) {
    s32 unused[8];

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTileMapMethods();
    self->unk3C = arg2;
    self->unk42 = 0;
    if (arg1 == 0) {
        self->unk40 = 1;
        self->unk2A = 0;
        self->methods->setFlag((DataSrc33808 *)self);
    }
}
```

## Notes

First build; the gTileAtlasMethods ctor (TileAtlas__TileAtlas) shape with one more stored argument. The 0x40 frame with no stack use is the unused-local-array lever (`s32 unused[8];`). Unit-local `Obj6F498` moved up to precede this function and gained +0x3C (s32) and +0x40 (u16); TileMap__Load uses it unchanged (whole-image oracle green).

## Naming

- **TileMap__TileMap**, tier A. Constructor: stores the companion TileAtlas object at +0x3C and, unowned, runs Load.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/tile_map.h`. `self` is `TileMap *` (was `Obj6F498`); the third parameter is `FileResource *atlas` (was `s32 arg2`); +0x03C `atlas` (was unk3C), +0x040 `defaultGrid` (was unk40: set here only when arg1 == 0, and BuildMap lays out the default grid only when it is set), +0x042 `loaded` (was unk42: cleared here, set by TileMap__Load after BuildMap). setFlag is called with `self` uncast. Byte-identical.

Later the same round (alpha, third class): TileAtlas unified; the `atlas` parameter and TileMap::atlas are `TileAtlas *` (were `FileResource *`). Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `arg1` | `source` | C | as New_TileMap's |

MATCHING line on `s32 unused[8]`: it gives retail's 0x40-byte frame, which holds nothing.

## Round 95 (alpha, track 6: Sony headers)

include/tile_map.h's banner no longer carries the name's history: the class was named TileMap in round 83 (from TileMap__BuildMap's GsMAP), unified in round 88. The header's local GsMAP is gone; the type is <libgs.h>'s (field-for-field the same: cellw, cellh, ncellw, ncellh, base, index), so both includers take Sony's headers first. Byte-identical.
