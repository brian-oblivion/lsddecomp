# GetStreamPool3Channel -- MATCHED (24/24 words)

> Renamed from `func_80049270` on 2026-09-25 (tools/rename.py). Address 0x80049270.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 24/24, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Record accessor: calls GetStreamPool3(&count) (writes 8, returns record 0x238), writes `sub + count` to `*countOut` when non-NULL, returns `&rec[sub]`.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`D_80081940Obj`, `D_80081940Methods`, `Rec1C`) and `include/Class6D430.h`.

```c
Rec1C *GetStreamPool3Channel(s32 *countOut, s32 sub) {
    s32 count;
    Rec1C *rec = GetStreamPool3(&count);
    if (countOut != NULL) {
        *countOut = sub + count;
    }
    return &rec[sub];
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

- **Name:** `GetStreamPool3Channel`
- **Tier:** B
- **Evidence:** pure getter: index into GetStreamPool3(); no cross-unit caller found (only reached internally via ResolveCinematicChannel's negative-group fallback).
