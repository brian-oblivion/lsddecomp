# GetStageRecords -- MATCHED (21/21 words)

> Renamed from `GetRecordGroup` on 2026-09-27 (tools/rename.py). Address 0x80048e2c.

> Renamed from `func_80048E2C` on 2026-09-25 (tools/rename.py). Address 0x80048e2c.

Round 82, runner echo (second echo session), 2026-09-25. Unit `GameFiles`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 21/21, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Record lookup: `gStageFirstRecord` is an s16 table of record indices; returns `&table[gStageFirstRecord[index]]` where `table` is GetRecordTable(NULL). The call precedes the index load in retail, which is the natural evaluation order.

## Source

Declarations it needs are the local views at the top of `src/GameFiles.c`
(`D_80081940Obj`, `D_80081940Methods`, `FilePathRecord`) and `include/FileResource.h`.

```c
FilePathRecord *GetStageRecords(s32 index) {
    return &((FilePathRecord *)GetRecordTable(NULL))[gStageFirstRecord[index]];
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

- **Name:** `GetStageRecords`
- **Tier:** A
- **Evidence:** returns &gRecordTable[gStageFirstRecord[stage]] (gStageFirstRecord renamed from gRecordIndexTable); the record paths are retail's gRecordTable data (0x80081A04), read from disk/SLPS_015.56 in round 100: record 0 of each group is STGnn\TEXA.TIX, and gStageFirstRecord[0] = 16 is the first record after the sound banks. Every caller passes ObjM::stage.

## Naming history

- Round 100 (bravo, polish): renamed from `GetRecordGroup` with tools/rename.py, on the record paths and callers above; previous tier A.
