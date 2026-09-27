# GetLinkResourceMethods -- MATCHED (4/4 words)

> Renamed from `func_80043B78` on 2026-09-25 (tools/rename.py). Address 0x80043b78.

Round 82, runner echo (GraphicsResources session), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gLinkResourceMethods`, declared locally as `extern s32 gLinkResourceMethods[];`.

Table slot (`tools/classtable.py`): `gFileResourceMethods` +0x094 (the FileResource `Get...Methods` getter list).

## Source

```c
extern s32 gLinkResourceMethods[];

void *GetLinkResourceMethods(void) {
    return gLinkResourceMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/GraphicsResources.c`.

## Naming

- **GetLinkResourceMethods**, tier A. Table getter.

## Track 4

2026-09-26, round 89 (delta): LinkResource (table `gLinkResourceMethods`,
renamed from D_8006F13C) is unified in `include/LinkResource.h`. The
unit-local views this body used (`DataSrc33808`, `Obj6F13C`, `Buf439EC`,
`Rec6F13C`/`Buf6F13C`, the `extern s32 D_8006F13C[]` array) are gone:
`self` is `LinkResource *`, its +0x02C is `TmdModel **models`, the buffer is
read as `TmdFile *` (include/TmdModel.h), the allocator's descriptor is
`ResourceSource *`, and the getter returns `&gLinkResourceMethods`.
Byte-identical.
