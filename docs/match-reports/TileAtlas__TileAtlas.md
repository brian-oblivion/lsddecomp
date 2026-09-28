# TileAtlas__TileAtlas -- MATCHED (31/31 words)

> Renamed from `func_80044F90` on 2026-09-25 (tools/rename.py). Address 0x80044f90.

Round 82, runner echo (graphics_resources session, echo #8), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 31/31 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: active driver's ctor, install gTileAtlasMethods (GetTileAtlasMethods), clear +0x34 (word) and +0x32 (halfword); when `arg` is 0, set +0x30 = 1, clear +0x2A, and call its own +0x064 slot (TileAtlas__Load) with self.

Table slot (`tools/classtable.py`): gTileAtlasMethods +0x008.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/graphics_resources.c`.

```c
typedef struct Obj6F514 {
    FILERESOURCE_FIELDS(DataSrc33808Methods);
    /* +0x02C */ u8 pad2C[4];
    /* +0x030 */ u16 unk30;
    /* +0x032 */ u16 unk32;
    /* +0x034 */ s32 unk34;
} Obj6F514;

/* gTileAtlasMethods +0x008: constructor -- the active driver's, then this table;
 * clear +0x34/+0x32, and with no `arg` set +0x30, clear +0x2A and run its
 * own +0x064. */
void TileAtlas__TileAtlas(Obj6F514 *self, s32 arg) {
    s32 unused[8];

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTileAtlasMethods();
    self->unk34 = 0;
    self->unk32 = 0;
    if (arg == 0) {
        self->unk30 = 1;
        self->unk2A = 0;
        self->methods->setFlag((DataSrc33808 *)self);
    }
}
```

## Notes

First build. The 0x40 frame with nothing on the stack past ra/s0/s1 is the unused-local-array lever (`s32 unused[8];`, same as TileAtlas__Load in this class). The unit-local `Obj6F514` moved up the file to precede this function and gained +0x30 (u16) and +0x34 (s32); TileAtlas__Load still uses it unchanged (re-verified by the whole-image oracle).

## Naming

- **TileAtlas__TileAtlas**, tier A. Constructor: clears state and, unowned, runs Load.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileAtlas.h` (gTileAtlasMethods, 0x303, a FileResource subclass, 0x38 bytes). `self` is `TileAtlas *` (was the unit-local `Obj6F514`). +0x030 unk30 -> `defaultCells` (set to 1 here when arg1 == 0; TileAtlas__BuildCells builds only when it is set, the counterpart of TileMap's `defaultGrid`), +0x032 unk32 -> `loaded` (0 here, 1 from TileAtlas__Load), +0x034 `unk34` retyped `s32` -> `void *` (zeroed here, freed by Finalize, set by no TileAtlas method). The setFlag call needs no cast. No rename. Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `arg1` | `source` | C | as New_TileMap's |

MATCHING line on `s32 unused[8]`: retail's 0x40-byte frame.

## Round 95 (alpha, track 6: Sony headers)

include/TileAtlas.h's banner no longer carries the name's history: the class was named TileAtlas in round 83 (from TileAtlas__BuildCells's 300 GsCELLs over VRAM), unified in round 88. The header's local GsCELL is gone; the type is <libgs.h>'s (field-for-field the same: u, v, cba, flag, tpage), so both includers (graphics_resources.c, task.c) take common.h, <libgte.h>, <libgpu.h>, <libgs.h> first. Byte-identical.
