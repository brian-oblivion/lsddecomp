# TileAtlas__Finalize -- MATCHED (21/21 words)

> Renamed from `func_8004500C` on 2026-09-25 (tools/rename.py). Address 0x8004500c.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 21/21 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Frees +0x34 then +0x2C with BMemPMgrFree, then the active driver's finalize.

Table slot (`tools/classtable.py`): gTileAtlasMethods +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* gTileAtlasMethods +0x00C: finalize -- free +0x34 and +0x2C, then the active
 * driver's. */
void TileAtlas__Finalize(DataSrc33808 *self) {
    BMemPMgrFree((void *)self->unk34);
    BMemPMgrFree((void *)self->unk2C);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}
```

## Notes

- Byte-exact on the first build.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TileAtlas__Finalize**, tier A. Slot +0x00C: frees the cell array, then the active driver's finalize.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileAtlas.h` (gTileAtlasMethods, 0x303, a FileResource subclass, 0x38 bytes). `self` is `TileAtlas *` (was the generic unit-local `DataSrc33808` view): frees `unk34` (now `void *`) and `cells` without casts. No rename. Byte-identical.
