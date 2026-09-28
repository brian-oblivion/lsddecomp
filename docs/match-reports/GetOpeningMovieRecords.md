# GetOpeningMovieRecords -- MATCHED (11/11 words)

> Renamed from `GetWeeklyStreamPool` on 2026-09-27 (tools/rename.py). Address 0x80049110.

> Renamed from `func_80049110` on 2026-09-25 (tools/rename.py). Address 0x80049110.

Round 82, runner echo, 2026-09-25. Unit `GameFiles` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
11/11, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

`if (out) *out = 0; return records + 0x230;` -- the end of GetRecordTable's 0x230-record table (0x3D40 = 0x230 * 0x1C). Siblings GetEndingMovieRecord/GetEventMovieRecords are the same shape with counts 7 and 8 at records 0x237 and 0x238: a count plus a pointer to a sub-table that follows the main one.

## Source

```c
typedef struct FilePathRecord { u8 data[0x1C]; } FilePathRecord;
void *GetRecordTable(s32 *out);   /* defined earlier in this unit */

FilePathRecord *GetOpeningMovieRecords(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 0;
    }
    return &((FilePathRecord *)GetRecordTable(NULL))[0x230];
}
```

## Notes

- GetRecordTable (already matched) returns sRecordTable and writes 0x230 to
  `*out`; every `+0x70`/`+0xFC`/`+0x3D40`/`+0x3E04`/`+0x3E20` offset in this
  unit is a whole number of 0x1C-byte records into that table, so the unit
  types the table as `FilePathRecord` (size only). The record's fields are unknown.
- Callers in other units still declare their own prototypes (`s32` returns in
  DayTaskStageMap.h / class_3bb8c.h); those are independent declarations and were
  not touched.

## Naming

- **Name:** `GetOpeningMovieRecords`
- **Tier:** A
- **Evidence:** &sRecordTable[560] and movie id 0: records 560..566 are ETC\OPENINGA..G.STR (the record paths are retail's sRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100).

## Naming history

- Round 100 (bravo, polish): renamed from `GetWeeklyStreamPool` with tools/rename.py, on the record paths and callers above; previous tier B.

## Track 10 (2026-09-28, round 104, echo)

GameFiles.h's `FilePathRecord` (an opaque `u8 data[0x1C]`) merged into CdDriver.h's `CdFileEntry` ({name[0x14], CdlLOC pos, u32 size}, 0x1C): sRecordTable goes GetRecordTable -> RegisterRecordTableFiles -> RegisterFileTableEntries -> SetFileTable, so its records are the CD driver's file-table entries. GameFiles.h includes CdDriver.h; sRecordTable is declared `CdFileEntry[]` and GetRecordTable returns `CdFileEntry *`, dropping the casts at its callers, and a record used as a path is spelled `record->name` instead of a `(char *)`/`(const char *)` cast. Byte-identical.
