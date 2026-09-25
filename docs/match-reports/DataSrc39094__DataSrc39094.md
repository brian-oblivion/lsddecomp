# DataSrc39094__DataSrc39094 -- MATCHED (31/31 words)

> Renamed from `func_800488E4` on 2026-09-25 (tools/rename.py). Address 0x800488e4.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 31/31, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Slot +0x008 (ctor) of D_80081940: runs the active data source's ctor on self, installs its own table (GetDataSrc39094Methods), initialises +0x30=-1, +0x2C/+0x2E/+0x32=0, +0x34=NULL, +0x38=1, then allocates a 0xB358-byte file buffer into Class6D430's `buffer`, setting `bufferSize` only on success. Written in natural order; the scheduler produced retail's store order.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`DataSrc39094`, `DataSrc39094Methods`, `Rec1C`) and `include/Class6D430.h`.

```c
/* slot +0x008 of D_80081940 (ctor) */
void DataSrc39094__DataSrc39094(DataSrc39094 *self) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetDataSrc39094Methods();
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

- **Name:** `DataSrc39094__DataSrc39094`
- **Tier:** A
- **Evidence:** slot +0x008 (ctor); chains the active data source's base ctor, installs this class's own table, and allocates the 0xB358 header buffer -- textbook Class6D430 subclass constructor.
