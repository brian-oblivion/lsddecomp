# TileMap__Finalize -- MATCHED (18/18 words)

> Renamed from `GridIndexSrc__Finalize` on 2026-09-25 (tools/rename.py). Address 0x80044dc8.

> Renamed from `func_80044DC8` on 2026-09-25 (tools/rename.py). Address 0x80044dc8.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 18/18 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`BMemPMgrFree((void *)self->unk38); GetActiveDataSourceMethods()->finalize(self);`

Table slot (`tools/classtable.py`): gTileMapMethods +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTileMapMethods +0x00C: finalize -- free +0x38, then the active driver's. */
void TileMap__Finalize(DataSrc33808 *self) {
    BMemPMgrFree((void *)self->unk38);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}
```

## Notes

- No shared header was edited. `FileResource.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TileMap__Finalize**, tier A. Slot +0x00C: frees the index table and the cell array, then the active driver's finalize.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileMap.h`. `self` is `TileMap *` (was the generic `DataSrc33808`); the freed field is `map.index` (was `unk38`, s32), the GsMAP index table BuildMap allocates. Byte-identical.
