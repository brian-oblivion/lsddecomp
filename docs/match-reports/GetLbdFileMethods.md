# GetLbdFileMethods -- MATCHED (4/4 words), round 81

> Renamed from `GetClass81940Methods` on 2026-09-26 (tools/rename.py). Address 0x80048ce0.

> Renamed from `GetDataSrc39094Methods` on 2026-09-26 (tools/rename.py). Address 0x80048ce0.

> Renamed from `func_80048CE0` on 2026-09-25 (tools/rename.py). Address 0x80048ce0.

Round 81, runner echo. Unit `src/code_39094.c` (carved from psyq_39094 in
FINISHING-PLAN revision 18). Fresh ground, no prior attempt. Byte-exact on
the first build; whole-image SHA1 green.

- **Where:** not a slot; the table getter of gLbdFileMethods.
- **What:** returns the method table address `gLbdFileMethods` (lui/addiu).
- **Levers:** none needed.
- **Name:** kept `func_`; role not yet identified beyond the above.

## Source

```c
void *GetLbdFileMethods(void) {
    return gLbdFileMethods;
}
```

## Naming

- **Name:** `GetLbdFileMethods`
- **Tier:** A
- **Evidence:** table getter: returns gLbdFileMethods, matches the project's 'table getter' convention exactly.

## Track 4 (2026-09-26, round 87)

Renamed `GetDataSrc39094Methods` -> `GetLbdFileMethods` with `rename.py`: the table getter, returns &gLbdFileMethods. It is gFileResourceMethods's gDataSourceClientGetters entry at +0x0A0, so SetActiveDataSource rebinds this class's interface slots. The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as PlacementGrid is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.


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
