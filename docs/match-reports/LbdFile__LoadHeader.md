# LbdFile__LoadHeader -- MATCHED (51/51 words)

> Renamed from `Class81940__LoadHeader` on 2026-09-26 (tools/rename.py). Address 0x80048aac.

> Renamed from `DataSrc39094__LoadHeader` on 2026-09-26 (tools/rename.py). Address 0x80048aac.

> Renamed from `func_80048AAC` on 2026-09-25 (tools/rename.py). Address 0x80048aac.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the first build; whole-image SHA1 green, funcdiff 51/51.

## What it does

Slot +0x078 of gLbdFileMethods. With a buffer and a non-NULL name: reset (`headerReady
= 0` when idle, else cancelRequests), enter state 9, then close / open(name,
1, 0) / read(buffer, 0xB358) through the object's own (run-time-bound)
FileResource interface slots. State 9 is completed by LbdFile__AdvanceLoadState (setFlag).

## Source

```c
/* slot +0x078 of gLbdFileMethods: start streaming a file into the buffer */
void LbdFile__LoadHeader(DataSrc39094 *self, char *name) {
    if (self->buffer != NULL && name != NULL) {
        if (self->unk2A == 0) {
            self->headerReady = 0;
        } else {
            self->methods->cancelRequests(self);
        }
        self->unk2A = 9;
        self->methods->close(self);
        self->methods->open(self, name, 1, 0);
        self->methods->read(self, self->buffer, 0xB358);
    }
}
```

## Naming

- **Name:** `LbdFile__LoadHeader`
- **Tier:** A
- **Evidence:** matches the unit's own established fact 'state 9 = header load': sets state to 9 and issues close/open/read of the header buffer.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__LoadHeader` -> `LbdFile__LoadHeader` with `rename.py` (class rename only). It occupies +0x078, FileResource's `void *slot78`; the slot keeps the inherited type and its one caller, StageMap__ApplyRateEntries (src/class_3bb8c.c), calls it through `LbdFileLoadHeaderFn` (track 4 step 6). The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.


## Track 6 (2026-09-26, round 92, echo)

Renamed with `python3 tools/renametype.py Class81940 LbdFile` (the whole
class family: object, table `gClass81940Methods` -> `gLbdFileMethods`,
getter, constructors, methods, the header `include/Class81940.h` ->
`include/LbdFile.h` and its typedefs). The tool rewrote every
`Class81940` token in these reports too, so the Track 4 section above now
says the class "was named `LbdFile` for its table address"; what it named
then was `Class81940`.

**Class name `LbdFile`, tier A.** The files the class is handed are the
stage's map chunks, STGnn\Mnnn.LBD: StageMap__ApplyRateEntries calls
loadHeader (+0x078) with each rate entry's `ptr0`, which
StageMap__ComputeRateEntry takes from the grid's callback, and that
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
