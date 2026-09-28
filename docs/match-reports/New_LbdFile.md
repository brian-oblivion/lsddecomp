# New_LbdFile -- MATCHED (20/20 words)

> Renamed from `New_Class81940` on 2026-09-26 (tools/rename.py). Address 0x80048894.

> Renamed from `New_DataSrc39094` on 2026-09-26 (tools/rename.py). Address 0x80048894.

> Renamed from `func_80048894` on 2026-09-25 (tools/rename.py). Address 0x80048894.

Round 82, runner echo (second echo session), 2026-09-25. Unit `game_files`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 20/20, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Allocator for the gLbdFileMethods object: BMemPMgrAlloc(0x3C), and when non-NULL runs slot +0x008 (ctor) of the table GetLbdFileMethods returns (gLbdFileMethods). Delta's round-82 allocator shape (`if (obj != NULL) { ctor; return obj; } return NULL;`) matches as written.

## Source

Declarations it needs are the local views at the top of `src/cd/game_files.c`
(`DataSrc39094`, `DataSrc39094Methods`, `FilePathRecord`) and `include/file_resource.h`.

```c
/* allocator: new gLbdFileMethods object */
DataSrc39094 *New_LbdFile(void) {
    DataSrc39094 *obj = BMemPMgrAlloc(0x3C);
    if (obj != NULL) {
        ((FileResourceMethods *)GetLbdFileMethods())->ctor((FileResource *)obj);
        return obj;
    }
    return NULL;
}
```


## Notes

- The unit now has a local `DataSrc39094Methods` view (FILERESOURCE_SLOTS plus
  slots +0x07C..+0x084, +0x084 = LbdFile__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickSoundBank passes one in `$a1`, as game_shell's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (dream_day.h, class_3bb8c.h, GameApplication.h) are independent and untouched.

## Naming

- **Name:** `New_LbdFile`
- **Tier:** A
- **Evidence:** the allocator convention (`New_Class`); allocates 0x3C bytes and runs the class's own ctor through GetLbdFileMethods().

## Track 4 (2026-09-26, round 87)

Renamed `New_DataSrc39094` -> `New_LbdFile` with `rename.py`: the allocator of LbdFile (BMemPMgrAlloc(0x3C), then the table's +0x008 ctor). Its one caller is StageMap__StageMap, once per grid element. The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as PlacementGrid is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.


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
`&group[9 + chunk]` of sRecordTable in $v0. sRecordTable's 0x1C-byte records
begin with a path, and record 9 of every stage group is its M000.LBD
(stage 0: sStageFirstRecord[0] = 16, record 25 at 0x80081CC0 =
"STG00\M000.LBD"; stage 1: record 39 at 0x80081E48 = "STG01\M000.LBD").
The same index goes into `ownerRate` and is split into column and row
(StageMap__SplitChunkIndex). LBD is the game's own extension, and the
name follows the siblings named for the format they load (TimImage, Tod,
VabStreamObj). What the header block's two regions and the data block hold
beyond what their consumers do with them is not established here.

## History (moved from src/GameFiles.c, comments pass)

The file's banner carried its edge evidence:

> Edges: the file sits between two placed Sony objects, libcd/c_007
> (StFreeRing) and libc2/rand, so neither edge is a choice. Inside it no
> rodata crosses and no jump-table parity forces a boundary; tuboundary.py
> calls every gap "boundary possible" except GetRecordTable..GetSoundEffectDir,
> "boundary unlikely (single-user data)". Content reads as two subjects,
> LbdFile up to GetLbdFileMethods and the path getters from
> GetDefaultDataDirectory, but no tool splits a unit, so it stays one file
> named for both (FINISHING-PLAN track 8, park rule).
