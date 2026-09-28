# GetStageMapChunkRecordXY -- MATCHED (23/23 words)

> Renamed from `GetGridRecordXY` on 2026-09-27 (tools/rename.py). Address 0x80049098.

> Renamed from `func_80049098` on 2026-09-25 (tools/rename.py). Address 0x80049098.

Round 82, runner echo (second echo session), 2026-09-25. Unit `game_files`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 23/23, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Grid-cell record: `GetStageMapChunkRecord(index, x + GetStageGridDimensions(index)->columns * y)`. Uses `stage_grid.h`'s `StageGridDimensions` (first field `s16 columns`); the header is now included by this unit (additive).

## Source

Declarations it needs are the local views at the top of `src/cd/game_files.c`
(`D_80081940Obj`, `D_80081940Methods`, `FilePathRecord`) and `include/file_resource.h`.

```c
FilePathRecord *GetStageMapChunkRecordXY(s32 index, s32 x, s32 y) {
    return GetStageMapChunkRecord(index, x + GetStageGridDimensions(index)->columns * y);
}
```


## Notes

- The unit now has a local `D_80081940Methods` view (FILERESOURCE_SLOTS plus
  slots +0x07C..+0x084, +0x084 = LbdFile__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickSoundBank passes one in `$a1`, as game_shell's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (dream_day.h, class_3bb8c.h, GameApplication.h) are independent and untouched.

## Naming

- **Name:** `GetStageMapChunkRecordXY`
- **Tier:** A
- **Evidence:** GetStageMapChunkRecord(stage, x + columns * y), columns from GetStageGridDimensions(stage): the chunk at grid cell (x, y).

## Naming history

- Round 100 (bravo, polish): renamed from `GetGridRecordXY` with tools/rename.py, on the record paths and callers above; previous tier B.

## Track 10 (2026-09-28, round 104, echo)

game_files.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. game_files.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
