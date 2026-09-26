# New_LbdFile -- MATCHED (20/20 words)

> Renamed from `New_Class81940` on 2026-09-26 (tools/rename.py). Address 0x80048894.

> Renamed from `New_DataSrc39094` on 2026-09-26 (tools/rename.py). Address 0x80048894.

> Renamed from `func_80048894` on 2026-09-25 (tools/rename.py). Address 0x80048894.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 20/20, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Allocator for the gLbdFileMethods object: BMemPMgrAlloc(0x3C), and when non-NULL runs slot +0x008 (ctor) of the table GetLbdFileMethods returns (gLbdFileMethods). Delta's round-82 allocator shape (`if (obj != NULL) { ctor; return obj; } return NULL;`) matches as written.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`DataSrc39094`, `DataSrc39094Methods`, `Rec1C`) and `include/FileResource.h`.

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
  (`s32 unused`): PickWeeklyGroup passes one in `$a1`, as code_1677c's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (class_39e08.h, class_3bb8c.h, Class6D3C8.h) are independent and untouched.

## Naming

- **Name:** `New_LbdFile`
- **Tier:** A
- **Evidence:** the allocator convention (`New_Class`); allocates 0x3C bytes and runs the class's own ctor through GetLbdFileMethods().

## Track 4 (2026-09-26, round 87)

Renamed `New_DataSrc39094` -> `New_LbdFile` with `rename.py`: the allocator of LbdFile (BMemPMgrAlloc(0x3C), then the table's +0x008 ctor). Its one caller is Class866E8__Class866E8, once per grid element. The class (method table gLbdFileMethods, id 0x903, a FileResource subclass) was named `LbdFile` for its table address, 0x80081940 (renamed from `D_80081940` to `gLbdFileMethods`), as Class6D940 is (FINISHING-PLAN track 4 step 2); the old `DataSrc39094` was the unit's local view name, and dropping its unit suffix leaves `DataSrc`, which every FileResource subclass is. The unified definition is `include/LbdFile.h`.
