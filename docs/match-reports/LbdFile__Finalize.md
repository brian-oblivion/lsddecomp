# LbdFile__Finalize -- MATCHED (21/21 words)

> Renamed from `Class81940__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80048960.

> Renamed from `DataSrc39094__Finalize` on 2026-09-26 (tools/rename.py). Address 0x80048960.

> Renamed from `func_80048960` on 2026-09-25 (tools/rename.py). Address 0x80048960.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 21/21, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Slot +0x00C (finalize) of gLbdFileMethods: calls its own slot +0x084 (LbdFile__ReleaseDataBlock, frees the +0x34 allocation), then the active data source's `finalize(self)` via GetActiveDataSourceMethods().

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`DataSrc39094`, `DataSrc39094Methods`, `Rec1C`) and `include/FileResource.h`.

```c
/* slot +0x00C of gLbdFileMethods (finalize) */
void LbdFile__Finalize(DataSrc39094 *self) {
    self->methods->releaseAlloc(self);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}
```


## Notes

- The unit now has a local `DataSrc39094Methods` view (FILERESOURCE_SLOTS plus
  slots +0x07C..+0x084, +0x084 = LbdFile__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickWeeklyGroup passes one in `$a1`, as code_1677c's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (class_39e08.h, class_3bb8c.h, Class6D3C8.h) are independent and untouched.

## Naming

- **Name:** `LbdFile__Finalize`
- **Tier:** A
- **Evidence:** slot +0x00C (finalize); releases the data-block allocation then chains the active data source's base finalize.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__Finalize` -> `LbdFile__Finalize` with `rename.py` (class rename only; +0x00C finalize occupant). The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.
