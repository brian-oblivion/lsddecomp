# GetRecordGroup -- MATCHED (21/21 words)

> Renamed from `func_80048E2C` on 2026-09-25 (tools/rename.py). Address 0x80048e2c.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 21/21, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Record lookup: `gRecordIndexTable` is an s16 table of record indices; returns `&table[gRecordIndexTable[index]]` where `table` is GetRecordTable(NULL). The call precedes the index load in retail, which is the natural evaluation order.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`D_80081940Obj`, `D_80081940Methods`, `Rec1C`) and `include/FileResource.h`.

```c
Rec1C *GetRecordGroup(s32 index) {
    return &((Rec1C *)GetRecordTable(NULL))[gRecordIndexTable[index]];
}
```


## Notes

- The unit now has a local `D_80081940Methods` view (FILERESOURCE_SLOTS plus
  slots +0x07C..+0x084, +0x084 = LbdFile__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickWeeklyGroup passes one in `$a1`, as code_1677c's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (class_39e08.h, class_3bb8c.h, GameApplication.h) are independent and untouched.

## Naming

- **Name:** `GetRecordGroup`
- **Tier:** A
- **Evidence:** pure getter: &gRecordTable[gRecordIndexTable[index]]; a getter is tier A by the leaf-mechanics rule, purpose of the group itself not established.
