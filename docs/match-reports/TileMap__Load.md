# TileMap__Load -- MATCHED (21/21 words)

> Renamed from `GridIndexSrc__Load` on 2026-09-25 (tools/rename.py). Address 0x80044e10.

> Renamed from `func_80044E10` on 2026-09-25 (tools/rename.py). Address 0x80044e10.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 21/21 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

When the base field +0x2A is zero, calls slot +0x078 with no arguments ($a0 is never set up for the call) and stores 1 to the u16 at +0x42.

Table slot (`tools/classtable.py`): gTileMapMethods +0x064 (setFlag).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/code_33808.c`.

```c
/* gTileMapMethods +0x064: unless +0x2A is set, slot +0x078 and mark +0x42. */
typedef struct Obj6F498 {
    FILERESOURCE_FIELDS(DataSrc33808Methods);
    /* +0x02C */ u8 pad2C[0x16];
    /* +0x042 */ u16 unk42;
} Obj6F498;

void TileMap__Load(Obj6F498 *self) {
    if (self->unk2A == 0) {
        ((void (*)())self->methods->slot78)();
        self->unk42 = 1;
    }
}
```

## Notes

- Byte-exact on the first build.
- Unit-local view Obj6F498 (FILERESOURCE_FIELDS(DataSrc33808Methods) + u16 at +0x42).
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TileMap__Load**, tier B (head review, round 83: was A). Slot +0x064: unless +0x2A is set, BuildMap and mark +0x42.
  Head review, round 83: the body builds the map (slot +0x078, TileMap__BuildMap) unless +0x2A is set, then sets +0x42; "Load" is its role in the slot protocol, not shown by the body, so tier B. The base slot is FileResource__SetFlag.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileMap.h`. `self` is `TileMap *` (was `Obj6F498`); +0x042 is `loaded`. The no-argument call through FileResource's `void *slot78` is spelled `((TileMapBuildMapFn)self->methods->slot78)()` (a typedef with an empty parameter list, no code). Byte-identical.

## Round 93 polish (charlie, track 7)

MATCHING line on the BuildMap call: retail passes it no argument.
