# GetStageBgmRecords -- MATCHED (9/9 words)

> Renamed from `GetVariantBlock` on 2026-09-27 (tools/rename.py). Address 0x80048f60.

> Renamed from `func_80048F60` on 2026-09-25 (tools/rename.py). Address 0x80048f60.

Round 82, runner echo, 2026-09-25. Unit `GameFiles` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
9/9, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Record index arithmetic: `GetStageRecords(index) + 0x70` is four 0x1C-byte records past the record GetStageRecords selects.

## Source

```c
typedef struct FilePathRecord { u8 data[0x1C]; } FilePathRecord;
FilePathRecord *GetStageRecords(s32 index);   /* INCLUDE_ASM in this unit */

FilePathRecord *GetStageBgmRecords(s32 index) {
    return &GetStageRecords(index)[4];
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

- **Name:** `GetStageBgmRecords`
- **Tier:** A
- **Evidence:** &GetStageRecords(stage)[4]: records 4..8 of every group are BGA..BGE.SEQ (the record paths are retail's sRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100).

## Naming history

- Round 100 (bravo, polish): renamed from `GetVariantBlock` with tools/rename.py, on the record paths and callers above; previous tier A.
