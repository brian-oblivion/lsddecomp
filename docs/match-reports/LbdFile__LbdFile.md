# LbdFile__LbdFile -- MATCHED (31/31 words)

> Renamed from `Class81940__Class81940` on 2026-09-26 (tools/rename.py). Address 0x800488e4.

> Renamed from `DataSrc39094__DataSrc39094` on 2026-09-26 (tools/rename.py). Address 0x800488e4.

> Renamed from `func_800488E4` on 2026-09-25 (tools/rename.py). Address 0x800488e4.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 31/31, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Slot +0x008 (ctor) of gLbdFileMethods: runs the active data source's ctor on self, installs its own table (GetLbdFileMethods), initialises +0x30=-1, +0x2C/+0x2E/+0x32=0, +0x34=NULL, +0x38=1, then allocates a 0xB358-byte file buffer into FileResource's `buffer`, setting `bufferSize` only on success. Written in natural order; the scheduler produced retail's store order.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`DataSrc39094`, `DataSrc39094Methods`, `Rec1C`) and `include/FileResource.h`.

```c
/* slot +0x008 of gLbdFileMethods (ctor) */
void LbdFile__LbdFile(DataSrc39094 *self) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetLbdFileMethods();
    self->unk30 = -1;
    self->headerReady = 0;
    self->dataReady = 0;
    self->unk32 = 0;
    self->dataBuffer = NULL;
    self->autoLoadData = 1;
    self->buffer = BMemPMgrAlloc(0xB358);
    if (self->buffer != NULL) {
        self->bufferSize = 0xB358;
    }
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

- **Name:** `LbdFile__LbdFile`
- **Tier:** A
- **Evidence:** slot +0x008 (ctor); chains the active data source's base ctor, installs this class's own table, and allocates the 0xB358 header buffer -- textbook FileResource subclass constructor.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__DataSrc39094` -> `LbdFile__LbdFile` with `rename.py`: the +0x008 ctor occupant, named for the class. The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as PlacementGrid is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.


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
tail call GetStageMapChunkRecord(stage, chunk) / GetStageMapChunkRecordXY leaves
`&group[9 + chunk]` of gRecordTable in $v0. gRecordTable's 0x1C-byte records
begin with a path, and record 9 of every stage group is its M000.LBD
(stage 0: gStageFirstRecord[0] = 16, record 25 at 0x80081CC0 =
"STG00\M000.LBD"; stage 1: record 39 at 0x80081E48 = "STG01\M000.LBD").
The same index goes into `ownerRate` and is split into column and row
(StageMap__SplitChunkIndex). LBD is the game's own extension, and the
name follows the siblings named for the format they load (TimImage, Tod,
VabStreamObj). What the header block's two regions and the data block hold
beyond what their consumers do with them is not established here.

The unit banner of `src/code_39094.c` was rewritten as documentation in
the same pass; its history is kept here verbatim (with the class tokens as
renamed):

```
/*
 * code_39094 -- GAME code carved from psyq_39094 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x39094..0x39C80 (vram 0x80048894..0x80049480). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). All 38 functions matched round 82 (three
 * echo sessions); named round 82 (bravo). Two independent groups of code:
 *
 * 1. LbdFile (method table gLbdFileMethods; include/LbdFile.h): a
 *    FileResource data source that streams a header block into its own 0xB358
 *    buffer (state 9, LbdFile__LoadHeader), then, once the read completes
 *    (LbdFile__AdvanceLoadState), an optional data block the header
 *    describes into a second allocation (state 10, LbdFile__LoadDataBlock/
 *    dataBuffer), started automatically unless LbdFile__SetAutoLoadData
 *    turned that off.
 * 2. Free functions over gRecordTable, a table of 0x230+ fixed 0x1C-byte
 *    records (Rec1C): random-or-forced pickers (SeedAndRandom,
 *    SetPickOverrides/gForcedSoundBank/gForcedStageBgm), record-group
 *    accessors indexed by gStageFirstRecord and, for GetStageMapChunkRecordXY, by
 *    StageGrid.h's cell columns, and a family of "stream channel" lookups
 *    (GetAsmkMovie, PickOpeningMovie, GetEndingMovie,
 *    GetSpecialDayOrEventRecord, GetSpecialDayMovieSpan) whose shapes match
 *    their exact call sites in code_1677c.c one for one. The records' own
 *    fields and the channels' in-game meaning are not established.
 */
```

## Proposed field names

Not applied: every accessor outside `src/code_39094.c` is in StageMap's
units (class_3bb8c.c, class_3bb8c_b.c), outside this job. For the head to
apply by type scope.

- `LbdFile.ownerRate` (+0x030) -> `chunkIndex`. StageMap__ApplyChunkLoads
  stores in it the same entry value it passed to the grid callback to get
  the file's record (GetStageMapChunkRecord(stage, value)); it is -1 exactly when
  no chunk is held (this ctor, LbdFile__ReleaseHeader; ResetElementCells
  tests `>= 0`); StageMap__SplitChunkIndex splits it into column
  (`% divisor`) and row (`/ divisor`); StageMap__FindSlotIndexByChunk
  finds the element holding a given chunk by it. Accessors: code_39094.c
  (LbdFile__LbdFile, LbdFile__ReleaseHeader), class_3bb8c.c
  (ApplyRateEntries, ResetElementCells, ComputeFootprintDescriptor,
  GetLastTargetRateSplit), class_3bb8c_b.c (FindElemIndexByUnk30).
- `LbdFile.ownerKey` (+0x032) -> `elemKey`. StageMap's ctor writes the
  element's index, BuildRateEntries copies each element's `key` in, and
  FindElemByUnk32 / UpdateFootprintTracking / class_3bb8c_p read it back to
  find an element. Accessors: class_3ac78.c, class_3bb8c.c, class_3bb8c_b.c,
  class_3bb8c_p.c (none in code_39094.c; the ctor zeroes it).
- `LbdFileHeader.gridOffset` / `gridSize` (+0x04 / +0x08) ->
  `placementsOffset` / `placementsSize`. StageMap__PopulateSlotCells
  points the element's PlacementGrid (a 20x20 grid of placement records) at
  header + gridOffset and builds its LinkResource from header + gridOffset +
  gridSize, i.e. right after the placements. Accessor: class_3bb8c.c only.
