# TileAtlas__Load -- MATCHED (21/21 words)

> Renamed from `func_80045060` on 2026-09-25 (tools/rename.py). Address 0x80045060.

Round 82, runner echo (graphics_resources session, echo #7), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 21/21 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

As TileMap__Load but marks the u16 at +0x32. Retail has a 0x38-byte frame although nothing but $ra/$s0 touches the stack.

Table slot (`tools/classtable.py`): gTileAtlasMethods +0x064 (setFlag).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.

```c
/* gTileAtlasMethods +0x064: unless +0x2A is set, slot +0x078 and mark +0x32. */
typedef struct Obj6F514 {
    FILERESOURCE_FIELDS(DataSrc33808Methods);
    /* +0x02C */ u8 pad2C[6];
    /* +0x032 */ u16 unk32;
} Obj6F514;

void TileAtlas__Load(Obj6F514 *self) {
    s32 unused[8];

    if (self->unk2A == 0) {
        ((void (*)())self->methods->slot78)();
        self->unk32 = 1;
    }
}
```

## Notes

- The 0x38 frame (vs 0x18 in TileMap__Load) is reproduced by an unused local `s32 unused[8];`. Without it the frame is 0x18. Whether the original held an unused buffer or something dead-code-eliminated is not recoverable; the bytes are.
- Unit-local view Obj6F514 (u16 at +0x32).
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TileAtlas__Load**, tier A. Slot +0x064: unless +0x2A is set, BuildCells and mark +0x32.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/tile_atlas.h` (gTileAtlasMethods, 0x303, a FileResource subclass, 0x38 bytes). `self` is `TileAtlas *` (was `Obj6F514`); +0x032 is `loaded`. The no-argument call through +0x078 (retail never sets $a0 before the `jalr`, confirmed in the built object) goes through the header's `TileAtlasBuildCellsFn` typedef instead of an inline `void (*)()` cast. No rename. Byte-identical.

## Round 93 polish (charlie, track 7)

MATCHING lines on `s32 unused[8]` (retail's 0x38-byte frame) and on the BuildCells call (no argument).

## History (source comments moved in track 12, round 106)

From the TileAtlas class banner in include/tile_atlas.h (SLOTS, +0x064):

> TileAtlas__Load: unless loadState is set, +0x078 and loaded = 1. It calls
> +0x078 with NO argument ($a0 is never set up, as in TileMap__Load),
> through TileAtlasBuildCellsFn;

Track 12 checked the built object: `self` arrives in $a0 and Load only
copies it (`move s0,a0`), so BuildCells in practice receives the atlas. The
header now says so without the register.
