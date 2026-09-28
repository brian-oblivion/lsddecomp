# GetSpecialDayRecords -- MATCHED (25/25 words)

> Renamed from `GetCinematicBank` on 2026-09-27 (tools/rename.py). Address 0x800492d0.

> Renamed from `func_800492D0` on 2026-09-25 (tools/rename.py). Address 0x800492d0.

Round 82, runner echo (second echo session), 2026-09-25. Unit `game_files`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 25/25, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Record accessor: base is record 0x23E (byte +0x3EC8) of GetRecordTable's table, stride 6 records (0xA8 bytes, the `*21*8` shift chain); writes `n*2 + 0xE` to `*countOut` when non-NULL, returns `&base[n*6]`.

## Source

Declarations it needs are the local views at the top of `src/cd/game_files.c`
(`D_80081940Obj`, `D_80081940Methods`, `FilePathRecord`) and `include/FileResource.h`.

```c
FilePathRecord *GetSpecialDayRecords(s32 *countOut, s32 n) {
    FilePathRecord *rec = &((FilePathRecord *)GetRecordTable(NULL))[0x23E];
    if (countOut != NULL) {
        *countOut = n * 2 + 0xE;
    }
    return &rec[n * 6];
}
```


## Notes

- The unit now has a local `D_80081940Methods` view (FILERESOURCE_SLOTS plus
  slots +0x07C..+0x084, +0x084 = LbdFile__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickSoundBank passes one in `$a1`, as GameApplicationFileResource's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (DayTaskStageMap.h, class_3bb8c.h, GameApplication.h) are independent and untouched.

## Naming

- **Name:** `GetSpecialDayRecords`
- **Tier:** A
- **Evidence:** &sRecordTable[574 + 6 * day], the six SPDAYnn records of special day `day` (FILM\SPDAYnnA/B.STR, IMG1\SPDAYnnC..F.TIM; the record paths are retail's sRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100), and movie id 14 + 2 * day, its A movie's. `Special day` is the files' own SPDAY.

## Naming history

- Round 100 (bravo, polish): renamed from `GetCinematicBank` with tools/rename.py, on the record paths and callers above; previous tier B (head review, round 82: was A. The mechanics are this body's; the purpose word comes from the callers' inherited names in code_1677c.c / GameApplication.h, which are themselves hypotheses, so the name is consistent but not established).

## Track 10 (2026-09-28, round 104, echo)

game_files.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into cd_driver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. game_files.h includes cd_driver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
