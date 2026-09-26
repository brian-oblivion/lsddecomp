# LinkResource__Finalize -- MATCHED (38/38 words)

> Renamed from `func_80043954` on 2026-09-25 (tools/rename.py). Address 0x80043954.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 38/38 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Finalize: walks the NULL-terminated object-pointer array at +0x2C releasing each (own slot +0x004), frees the array (BMemPMgrFree), then the active driver's finalize.

Table slot (`tools/classtable.py`): gLinkResourceMethods +0x00C.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* gLinkResourceMethods +0x00C: finalize -- release every object in the NULL-ended
 * array at +0x2C, free the array, then the active driver's. */
void LinkResource__Finalize(DataSrc33808 *self) {
    DataSrc33808 **objs = (DataSrc33808 **)self->unk2C;

    while (*objs != NULL) {
        (*objs)->methods->release(*objs);
        objs++;
    }
    BMemPMgrFree((void *)self->unk2C);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}
```

## Notes

First build. A plain `while (*objs != NULL)` is rotated by GCC into the top-test + bottom-test shape retail has.

## Naming

- **LinkResource__Finalize**, tier A. FileResource finalize override: releases every object in the NULL-ended array at +0x2C.

## Track 4

2026-09-26, round 89 (delta): LinkResource (table `gLinkResourceMethods`,
renamed from D_8006F13C) is unified in `include/LinkResource.h`. The
unit-local views this body used (`DataSrc33808`, `Obj6F13C`, `Buf439EC`,
`Rec6F13C`/`Buf6F13C`, the `extern s32 D_8006F13C[]` array) are gone:
`self` is `LinkResource *`, its +0x02C is `TmdModel **models`, the buffer is
read as `TmdFile *` (include/TmdModel.h), the allocator's descriptor is
`Src6F240 *`, and the getter returns `&gLinkResourceMethods`.
Byte-identical.
