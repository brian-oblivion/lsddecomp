# New_DataSrc39094 -- MATCHED (20/20 words)

> Renamed from `func_80048894` on 2026-09-25 (tools/rename.py). Address 0x80048894.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 20/20, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Allocator for the D_80081940 object: BMemPMgrAlloc(0x3C), and when non-NULL runs slot +0x008 (ctor) of the table GetDataSrc39094Methods returns (D_80081940). Delta's round-82 allocator shape (`if (obj != NULL) { ctor; return obj; } return NULL;`) matches as written.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`DataSrc39094`, `DataSrc39094Methods`, `Rec1C`) and `include/Class6D430.h`.

```c
/* allocator: new D_80081940 object */
DataSrc39094 *New_DataSrc39094(void) {
    DataSrc39094 *obj = BMemPMgrAlloc(0x3C);
    if (obj != NULL) {
        ((Class6D430Methods *)GetDataSrc39094Methods())->ctor((Class6D430 *)obj);
        return obj;
    }
    return NULL;
}
```


## Notes

- The unit now has a local `DataSrc39094Methods` view (CLASS6D430_SLOTS plus
  slots +0x07C..+0x084, +0x084 = DataSrc39094__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `SeedAndRandom`'s local definition gained an unused second parameter
  (`s32 unused`): PickWeeklyGroup passes one in `$a1`, as code_1677c's own
  prototype already says. Byte-neutral for SeedAndRandom.
- No shared header was edited. Other units' prototypes for these functions
  (class_39e08.h, class_3bb8c.h, Class6D3C8.h) are independent and untouched.

## Naming

- **Name:** `New_DataSrc39094`
- **Tier:** A
- **Evidence:** the allocator convention (`New_Class`); allocates 0x3C bytes and runs the class's own ctor through GetDataSrc39094Methods().
