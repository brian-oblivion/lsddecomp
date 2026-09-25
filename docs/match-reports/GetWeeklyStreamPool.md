# GetWeeklyStreamPool -- MATCHED (11/11 words)

> Renamed from `func_80049110` on 2026-09-25 (tools/rename.py). Address 0x80049110.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
11/11, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

`if (out) *out = 0; return records + 0x230;` -- the end of GetRecordTable's 0x230-record table (0x3D40 = 0x230 * 0x1C). Siblings GetStreamPool2/GetStreamPool3 are the same shape with counts 7 and 8 at records 0x237 and 0x238: a count plus a pointer to a sub-table that follows the main one.

## Source

```c
typedef struct Rec1C { u8 data[0x1C]; } Rec1C;
void *GetRecordTable(s32 *out);   /* defined earlier in this unit */

Rec1C *GetWeeklyStreamPool(s32 *countOut) {
    if (countOut != NULL) {
        *countOut = 0;
    }
    return &((Rec1C *)GetRecordTable(NULL))[0x230];
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
