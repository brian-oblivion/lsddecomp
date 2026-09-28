# GetEventMovieRecords -- MATCHED (12/12 words)

> Renamed from `GetStreamPool3` on 2026-09-27 (tools/rename.py). Address 0x80049240.

> Renamed from `func_80049240` on 2026-09-25 (tools/rename.py). Address 0x80049240.

Round 82, runner echo, 2026-09-25. Unit `GameFiles` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
12/12, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Same shape as GetOpeningMovieRecords: `*out = 8`, returns record 0x238 (offset 0x3E20).

## Source

```c
typedef struct FilePathRecord { u8 data[0x1C]; } FilePathRecord;
void *GetRecordTable(s32 *out);   /* defined earlier in this unit */

FilePathRecord *GetEventMovieRecords(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 8;
    }
    return &((FilePathRecord *)GetRecordTable(NULL))[0x238];
}
```

## Notes

- GetRecordTable (already matched) returns gRecordTable and writes 0x230 to
  `*out`; every `+0x70`/`+0xFC`/`+0x3D40`/`+0x3E04`/`+0x3E20` offset in this
  unit is a whole number of 0x1C-byte records into that table, so the unit
  types the table as `FilePathRecord` (size only). The record's fields are unknown.
- Callers in other units still declare their own prototypes (`s32` returns in
  DayTaskStageMap.h / class_3bb8c.h); those are independent declarations and were
  not touched.

## Naming

- **Name:** `GetEventMovieRecords`
- **Tier:** A
- **Evidence:** &gRecordTable[568], FILM\EVENT1..6.STR (the record paths are retail's gRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100), and movie id 8, the first event movie's.

## Naming history

- Round 100 (bravo, polish): renamed from `GetStreamPool3` with tools/rename.py, on the record paths and callers above; previous tier B.
