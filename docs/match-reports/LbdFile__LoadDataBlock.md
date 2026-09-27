# LbdFile__LoadDataBlock -- MATCHED (54/54 words)

> Renamed from `Class81940__LoadDataBlock` on 2026-09-26 (tools/rename.py). Address 0x80048bc0.

> Renamed from `DataSrc39094__LoadDataBlock` on 2026-09-26 (tools/rename.py). Address 0x80048bc0.

> Renamed from `func_80048BC0` on 2026-09-25 (tools/rename.py). Address 0x80048bc0.

Round 82, runner echo (third echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the third build; whole-image SHA1 green, funcdiff 54/54.

## What it does

Slot +0x080 of gLbdFileMethods. When the loaded buffer's header says a data block
exists (`+0x02` nonzero) and the object is idle (`unk2A == 0`): release the
previous block (slot +0x084), allocate `header->+0x14` bytes into `dataBuffer`,
enter state 10, seek to `header->+0x10` and read the block. Returns 1 on
start, 0 otherwise. State 10 is completed by LbdFile__AdvanceLoadState.

## Source

Needs the local views at the top of `src/code_39094.c`, with
`DataSrc39094Methods.releaseAlloc` declared UNPROTOTYPED (`void (*releaseAlloc)();`),
plus:

```c
/* The header at the start of gLbdFileMethods's 0xB358 buffer (local view). */
typedef struct StreamHdr {
    /* +0x00 */ u16 unk0;
    /* +0x02 */ u16 hasData;
    /* +0x04 */ u8 pad4[0xC];
    /* +0x10 */ u32 dataOffset;
    /* +0x14 */ s32 dataSize;
} StreamHdr;

/* slot +0x080 of gLbdFileMethods: load the data block the header describes */
s32 LbdFile__LoadDataBlock(DataSrc39094 *self) {
    s32 size;
    if (((StreamHdr *)self->buffer)->hasData == 0) {
        return 0;
    }
    if (self->unk2A != 0) {
        return 0;
    }
    self->methods->releaseAlloc();
    size = ((StreamHdr *)self->buffer)->dataSize;
    self->dataBuffer = BMemPMgrAlloc(size);
    if (self->dataBuffer == NULL) {
        return 0;
    }
    self->unk2A = 10;
    self->methods->seek(self, ((StreamHdr *)self->buffer)->dataOffset, 0);
    self->methods->read(self, self->dataBuffer, size);
    return 1;
}
```

## Levers (3 builds)

1. `releaseAlloc(self)`: 53/54, one extra `move a0,s0` before the `jalr`.
   So GCC 2.6.3 does NOT elide the argument even when `$a0` still holds
   `self` from entry: retail's call had no argument.
2. (build 2 was a helper-script duplicate-typedef compile error; no score.)
3. MATCH: `releaseAlloc` retyped in the local view to the unprototyped
   `void (*)()` so both call sites are legal C (LbdFile__Finalize passes `self`,
   this one passes nothing). Same fix applied to `loadDataBlock` for LbdFile__AdvanceLoadState.

### Proposed learning

A `jalr` through a method slot with `$a0` never set (it happens to still hold
`self` from entry) is a zero-argument call in the source; passing `self`
costs one `move a0,sN`. Declare the slot unprototyped (`T (*slot)();`) in the
local view when another caller passes arguments.

## Naming

- **Name:** `LbdFile__LoadDataBlock`
- **Tier:** A
- **Evidence:** slot +0x080 (loadDataBlock); matches the unit's own established fact 'state 10 = data block load': allocates dataBuffer sized from the header and issues seek/read.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__LoadDataBlock` -> `LbdFile__LoadDataBlock` with `rename.py` (class rename only; +0x080). Its no-argument call of +0x084 is kept through `LbdFileReleaseDataBlockNoArgFn` (include/LbdFile.h). The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as PlacementGrid is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.


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
