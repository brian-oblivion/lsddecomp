# DataSrc39094__ReleaseHeader -- MATCHED (18/18 words)

> Renamed from `func_80048B78` on 2026-09-25 (tools/rename.py). Address 0x80048b78.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 18/18, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

D_80081940 method: calls its own `freeBuffer` (slot +0x05C, Class6D430 interface) on self, then clears +0x2C and sets the new s16 field +0x30 to -1.

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`DataSrc39094`, `DataSrc39094Methods`, `Rec1C`) and `include/Class6D430.h`.

```c
void DataSrc39094__ReleaseHeader(DataSrc39094 *self) {
    self->methods->freeBuffer(self);
    self->headerReady = 0;
    self->unk30 = -1;
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

- **Name:** `DataSrc39094__ReleaseHeader`
- **Tier:** B
- **Evidence:** frees the header buffer via the base freeBuffer slot and resets headerReady/the -1 sentinel; mirror of DataSrc39094__ReleaseDataBlock, but not itself vtable-dispatched here.
