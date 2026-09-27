# GetStageMapChunkRecord -- MATCHED (14/14 words)

> Renamed from `GetGridRecordAt` on 2026-09-27 (tools/rename.py). Address 0x80049060.

> Renamed from `func_80049060` on 2026-09-25 (tools/rename.py). Address 0x80049060.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
14/14, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

`&GetStageMapChunkRecords(index)[sub]`: the `sll 3 / subu / sll 2` is the x28 multiply of a 0x1C-byte record index. `sub` is kept in `$s0` across the call and `index` passes through untouched in `$a0`.

## Source

```c
typedef struct Rec1C { u8 data[0x1C]; } Rec1C;
Rec1C *GetStageMapChunkRecords(s32 index);

Rec1C *GetStageMapChunkRecord(s32 index, s32 sub) {
    return &GetStageMapChunkRecords(index)[sub];
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

- **Name:** `GetStageMapChunkRecord`
- **Tier:** A
- **Evidence:** record `chunk` of GetStageMapChunkRecords: STGnn\Mnnn.LBD, which StageMap hands an LbdFile (include/LbdFile.h).

## Naming history

- Round 100 (bravo, polish): renamed from `GetGridRecordAt` with tools/rename.py, on the record paths and callers above; previous tier A.
