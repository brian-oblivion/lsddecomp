# LbdFile__ReleaseDataBlock -- MATCHED (16/16 words)

> Renamed from `Class81940__ReleaseDataBlock` on 2026-09-26 (tools/rename.py). Address 0x80048c98.

> Renamed from `DataSrc39094__ReleaseDataBlock` on 2026-09-26 (tools/rename.py). Address 0x80048c98.

> Renamed from `func_80048C98` on 2026-09-25 (tools/rename.py). Address 0x80048c98.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
16/16, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Method slot +0x084 of gLbdFileMethods (table word at 0x800819C4). Clears `dataReady`, then frees the `+0x34` buffer through BMemPMgrFree and stores the RESULT back (BMemPMgrFree returns a value; retail stores `$v0`, presumably NULL). The `sh $zero,0x2E` lands in the `beqz` delay slot with the store written FIRST in source.

## Source

```c
/* DataSrc39094: the local view at the top of src/code_39094.c --
 * FILERESOURCE_FIELDS, then u16 headerReady, u16 dataReady, u8 pad30[4], void *dataBuffer, s32 autoLoadData. */
extern void *BMemPMgrFree(void *ptr);

/* slot +0x084 of gLbdFileMethods */
void LbdFile__ReleaseDataBlock(DataSrc39094 *self) {
    self->dataReady = 0;
    if (self->dataBuffer != NULL) {
        self->dataBuffer = BMemPMgrFree(self->dataBuffer);
    }
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

- **Name:** `LbdFile__ReleaseDataBlock`
- **Tier:** A
- **Evidence:** slot +0x084 (releaseAlloc); frees dataBuffer and clears dataReady -- the mirror LbdFile__LoadDataBlock itself calls before reallocating.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__ReleaseDataBlock` -> `LbdFile__ReleaseDataBlock` with `rename.py` (class rename only; +0x084). Callers: LbdFile__Finalize, LbdFile__LoadDataBlock, StageMap__ResetAllElements and ObjM__CheckAuxTrigger. The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.


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
callback is ObjM__OnRegistrantEvent (ObjM__AttachTarget installs it), whose
tail call GetGridRecordAt(stage, chunk) / GetGridRecordXY leaves
`&group[9 + chunk]` of gRecordTable in $v0. gRecordTable's 0x1C-byte records
begin with a path, and record 9 of every stage group is its M000.LBD
(stage 0: gRecordIndexTable[0] = 16, record 25 at 0x80081CC0 =
"STG00\M000.LBD"; stage 1: record 39 at 0x80081E48 = "STG01\M000.LBD").
The same index goes into `ownerRate` and is split into column and row
(StageMap__SplitChunkIndex). LBD is the game's own extension, and the
name follows the siblings named for the format they load (TimImage, Tod,
VabStreamObj). What the header block's two regions and the data block hold
beyond what their consumers do with them is not established here.
