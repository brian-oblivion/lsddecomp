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
  (`s32 unused`): PickSoundBank passes one in `$a1`, as code_1677c's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (class_39e08.h, class_3bb8c.h, GameApplication.h) are independent and untouched.

## Naming

- **Name:** `LbdFile__Finalize`
- **Tier:** A
- **Evidence:** slot +0x00C (finalize); releases the data-block allocation then chains the active data source's base finalize.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__Finalize` -> `LbdFile__Finalize` with `rename.py` (class rename only; +0x00C finalize occupant). The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as PlacementGrid is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.


## Track 6 (2026-09-26, round 92, echo)

Renamed with `python3 tools/renametype.py Class81940 LbdFile` (the whole
class family: object, table `gClass81940Methods` -> `gLbdFileMethods`,
getter, constructors, methods, the header `include/Class81940.h` ->
`include/LbdFile.h` and its typedefs). The tool rewrote every
`Class81940` token in these reports too, so the Track 4 section above now
says the class "was named `LbdFile` for its table address"; what it named
then was `Class81940`.

**Class name `LbdFile`, tier A.** The files the class is handed are the
stage's map chunks, STGnn\Mnnn.LBD: StageMap__ApplyChunkLoads calls
loadHeader (+0x078) with each rate entry's `ptr0`, which
StageMap__ComputeChunkLoadEntry takes from the grid's callback, and that
callback is ObjM__GetGridRecord (ObjM__AttachTarget installs it), whose
tail call GetStageMapChunkRecord(stage, chunk) / GetGridRecordXY leaves
`&group[9 + chunk]` of gRecordTable in $v0. gRecordTable's 0x1C-byte records
begin with a path, and record 9 of every stage group is its M000.LBD
(stage 0: gStageFirstRecord[0] = 16, record 25 at 0x80081CC0 =
"STG00\M000.LBD"; stage 1: record 39 at 0x80081E48 = "STG01\M000.LBD").
The same index goes into `ownerRate` and is split into column and row
(StageMap__SplitChunkIndex). LBD is the game's own extension, and the
name follows the siblings named for the format they load (TimImage, Tod,
VabStreamObj). What the header block's two regions and the data block hold
beyond what their consumers do with them is not established here.
