# GetStageMapChunkRecords -- MATCHED (9/9 words)

> Renamed from `GetGridRecordBase` on 2026-09-27 (tools/rename.py). Address 0x8004903c.

> Renamed from `func_8004903C` on 2026-09-25 (tools/rename.py). Address 0x8004903c.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
9/9, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

`GetStageRecords(index) + 0xFC`, nine 0x1C-byte records past it.

## Source

```c
typedef struct FilePathRecord { u8 data[0x1C]; } FilePathRecord;
FilePathRecord *GetStageRecords(s32 index);   /* INCLUDE_ASM in this unit */

FilePathRecord *GetStageMapChunkRecords(s32 index) {
    return &GetStageRecords(index)[9];
}
```

## Notes

- GetRecordTable (already matched) returns gRecordTable and writes 0x230 to
  `*out`; every `+0x70`/`+0xFC`/`+0x3D40`/`+0x3E04`/`+0x3E20` offset in this
  unit is a whole number of 0x1C-byte records into that table, so the unit
  types the table as `FilePathRecord` (size only). The record's fields are unknown.
- Callers in other units still declare their own prototypes (`s32` returns in
  class_39e08.h / class_3bb8c.h); those are independent declarations and were
  not touched.

## Naming

- **Name:** `GetStageMapChunkRecords`
- **Tier:** A
- **Evidence:** &GetStageRecords(stage)[9]: records 9 on are the stage's Mnnn.LBD files (the record paths are retail's gRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100); LbdFile loads them (include/LbdFile.h).

## Naming history

- Round 100 (bravo, polish): renamed from `GetGridRecordBase` with tools/rename.py, on the record paths and callers above; previous tier A.
