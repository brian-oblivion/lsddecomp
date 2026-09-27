# GetVariantBlock -- MATCHED (9/9 words)

> Renamed from `func_80048F60` on 2026-09-25 (tools/rename.py). Address 0x80048f60.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
9/9, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Record index arithmetic: `GetStageRecords(index) + 0x70` is four 0x1C-byte records past the record GetStageRecords selects.

## Source

```c
typedef struct Rec1C { u8 data[0x1C]; } Rec1C;
Rec1C *GetStageRecords(s32 index);   /* INCLUDE_ASM in this unit */

Rec1C *GetVariantBlock(s32 index) {
    return &GetStageRecords(index)[4];
}
```

## Notes

- GetRecordTable (already matched) returns gRecordTable and writes 0x230 to
  `*out`; every `+0x70`/`+0xFC`/`+0x3D40`/`+0x3E04`/`+0x3E20` offset in this
  unit is a whole number of 0x1C-byte records into that table, so the unit
  types the table as `Rec1C` (size only). The record's fields are unknown.
- Callers in other units still declare their own prototypes (`s32` returns in
  class_39e08.h / class_3bb8c.h); those are independent declarations and were
  not touched.

## Naming

- **Name:** `GetVariantBlock`
- **Tier:** A
- **Evidence:** pure getter: &GetStageRecords(index)[4].
