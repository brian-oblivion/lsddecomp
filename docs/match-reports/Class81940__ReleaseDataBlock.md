# Class81940__ReleaseDataBlock -- MATCHED (16/16 words)

> Renamed from `DataSrc39094__ReleaseDataBlock` on 2026-09-26 (tools/rename.py). Address 0x80048c98.

> Renamed from `func_80048C98` on 2026-09-25 (tools/rename.py). Address 0x80048c98.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
16/16, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Method slot +0x084 of gClass81940Methods (table word at 0x800819C4). Clears `dataReady`, then frees the `+0x34` buffer through BMemPMgrFree and stores the RESULT back (BMemPMgrFree returns a value; retail stores `$v0`, presumably NULL). The `sh $zero,0x2E` lands in the `beqz` delay slot with the store written FIRST in source.

## Source

```c
/* DataSrc39094: the local view at the top of src/code_39094.c --
 * CLASS6D430_FIELDS, then u16 headerReady, u16 dataReady, u8 pad30[4], void *dataBuffer, s32 autoLoadData. */
extern void *BMemPMgrFree(void *ptr);

/* slot +0x084 of gClass81940Methods */
void Class81940__ReleaseDataBlock(DataSrc39094 *self) {
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

- **Name:** `Class81940__ReleaseDataBlock`
- **Tier:** A
- **Evidence:** slot +0x084 (releaseAlloc); frees dataBuffer and clears dataReady -- the mirror Class81940__LoadDataBlock itself calls before reallocating.

## Track 4 (2026-09-26, round 87)

Renamed `DataSrc39094__ReleaseDataBlock` -> `Class81940__ReleaseDataBlock` with `rename.py` (class rename only; +0x084). Callers: Class81940__Finalize, Class81940__LoadDataBlock, Class866E8__ResetAllElements and ObjM__CheckAuxTrigger. The class (method table gClass81940Methods, id 0x903, a Class6D430 subclass) was named `Class81940` for its table address, 0x80081940 (renamed from `D_80081940` to `gClass81940Methods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every Class6D430 subclass is. The unified definition is `include/Class81940.h`.
