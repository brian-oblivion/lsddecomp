# DataSrc39094__Finalize -- MATCHED (21/21 words)

> Renamed from `func_80048960` on 2026-09-25 (tools/rename.py). Address 0x80048960.

Round 82, runner echo (second echo session), 2026-09-25. Unit `code_39094`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`: `OK: build matches
retail SLPS_015.56`), funcdiff 21/21, 0 insertions / 0 deletions, no
out-of-range drift.

## What it does

Slot +0x00C (finalize) of D_80081940: calls its own slot +0x084 (DataSrc39094__ReleaseDataBlock, frees the +0x34 allocation), then the active data source's `finalize(self)` via GetActiveDataSourceMethods().

## Source

Declarations it needs are the local views at the top of `src/code_39094.c`
(`D_80081940Obj`, `D_80081940Methods`, `Rec1C`) and `include/Class6D430.h`.

```c
/* slot +0x00C of D_80081940 (finalize) */
void DataSrc39094__Finalize(D_80081940Obj *self) {
    self->methods->releaseAlloc(self);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
```


## Notes

- The unit now has a local `D_80081940Methods` view (CLASS6D430_SLOTS plus
  slots +0x07C..+0x084, +0x084 = DataSrc39094__ReleaseDataBlock) and the object's
  `pad30[4]` is split into `s16 unk30` (init -1) and `u16 unk32`. Byte-neutral
  for the ten functions matched earlier this round (whole image green).
- `func_80048CFC`'s local definition gained an unused second parameter
  (`s32 unused`): func_80048D74 passes one in `$a1`, as code_1677c's own
  prototype already says. Byte-neutral for func_80048CFC.
- No shared header was edited. Other units' prototypes for these functions
  (class_39e08.h, class_3bb8c.h, Class6D3C8.h) are independent and untouched.
