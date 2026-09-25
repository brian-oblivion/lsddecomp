# GetStreamChannelInit -- MATCHED (17/17 words)

> Renamed from `func_800491FC` on 2026-09-25 (tools/rename.py). Address 0x800491fc.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 17/17, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Record accessor: calls GetStreamPool2(&count) (which writes 7 and returns record 0x237 of GetRecordTable's table), forwards the count to `*countOut` when non-NULL, returns the record pointer.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`D_80081940Obj`, `D_80081940Methods`, `Rec1C`) and `include/Class6D430.h`.

```c
Rec1C *GetStreamChannelInit(s32 *countOut) {
    s32 count;
    Rec1C *rec = GetStreamPool2(&count);
    if (countOut != NULL) {
        *countOut = count;
    }
    return rec;
}
```


## Notes

- The unit now has a local `D_80081940Methods` view (CLASS6D430_SLOTS plus
  slots +0x07C..+0x084, +0x084 = DataSrc39094__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickWeeklyGroup passes one in `$a1`, as code_1677c's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (class_39e08.h, class_3bb8c.h, Class6D3C8.h) are independent and untouched.

## Naming

- **Name:** `GetStreamChannelInit`
- **Tier:** A
- **Evidence:** its one caller, Class6D3C8__StartStreamTaskWithInit, uses the return value as the StreamTask's group id directly; matches exactly.
