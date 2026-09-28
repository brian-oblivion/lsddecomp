# GetEventMovie -- MATCHED (24/24 words)

> Renamed from `GetStreamPool3Channel` on 2026-09-27 (tools/rename.py). Address 0x80049270.

> Renamed from `func_80049270` on 2026-09-25 (tools/rename.py). Address 0x80049270.

Round 82, runner echo (second echo session), 2026-09-25. Unit `GameFiles`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 24/24, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Record accessor: calls GetEventMovieRecords(&count) (writes 8, returns record 0x238), writes `sub + count` to `*countOut` when non-NULL, returns `&rec[sub]`.

## Source

Declarations it needs are the local views at the top of `src/GameFiles.c`
(`D_80081940Obj`, `D_80081940Methods`, `FilePathRecord`) and `include/FileResource.h`.

```c
FilePathRecord *GetEventMovie(s32 *countOut, s32 sub) {
    s32 count;
    FilePathRecord *rec = GetEventMovieRecords(&count);
    if (countOut != NULL) {
        *countOut = sub + count;
    }
    return &rec[sub];
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
  (class_39e08.h, class_3bb8c.h, GameApplication.h) are independent and untouched.

## Naming

- **Name:** `GetEventMovie`
- **Tier:** A
- **Evidence:** record `event` of GetEventMovieRecords, with movie id 8 + event.

## Naming history

- Round 100 (bravo, polish): renamed from `GetStreamPool3Channel` with tools/rename.py, on the record paths and callers above; previous tier B.
