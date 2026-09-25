# DataSrc39094__CancelRequests -- MATCHED (17/17 words)

> Renamed from `func_80048A68` on 2026-09-25 (tools/rename.py). Address 0x80048a68.

Round 82, runner echo, 2026-09-25. Unit `code_39094` (carved revision 18).
Byte-exact on the FIRST build; whole-image SHA1 green
(`./build-and-verify.sh`: `OK: build matches retail SLPS_015.56`), funcdiff
17/17, 0 insertions / 0 deletions, no out-of-range drift. No levers needed.

## What it does

Method slot +0x074 of D_80081940 (cancelRequests; table word at 0x800819B4). Calls the ACTIVE data source's `cancelRequests(self)` through GetActiveDataSourceMethods() (declared locally as returning `Class6D430Methods *`, which is the header's slot name at +0x074), then clears +0x2C, +0x2E and Class6D430's own `unk2A`, stores in that order.

## Source

```c
/* D_80081940Obj: the local view at the top of src/code_39094.c --
 * CLASS6D430_FIELDS, then u16 unk2C, u16 unk2E, u8 pad30[4], void *unk34, s32 unk38. */
extern Class6D430Methods *GetActiveDataSourceMethods(void);

/* slot +0x074 of D_80081940 (cancelRequests) */
void DataSrc39094__CancelRequests(D_80081940Obj *self) {
    GetActiveDataSourceMethods()->cancelRequests((Class6D430 *)self);
    self->unk2C = 0;
    self->unk2E = 0;
    self->unk2A = 0;
}
```

## Notes

- GetRecordTable (already matched) returns D_80081A04 and writes 0x230 to
  `*out`; every `+0x70`/`+0xFC`/`+0x3D40`/`+0x3E04`/`+0x3E20` offset in this
  unit is a whole number of 0x1C-byte records into that table, so the unit
  types the table as `Rec1C` (size only). The record's fields are unknown.
- Callers in other units still declare their own prototypes (`s32` returns in
  class_39e08.h / class_3bb8c.h); those are independent declarations and were
  not touched.
