# LinkResource__NoOp -- MATCHED (2/2 words)

> Renamed from `func_80043B70` on 2026-09-25 (tools/rename.py). Address 0x80043b70.

Round 82, runner echo (GraphicsResources session), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 2/2, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Empty method (`jr ra; nop`).

Table slot (`tools/classtable.py`): `gLinkResourceMethods` +0x084.

## Source

```c
void LinkResource__NoOp(void) {
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/GraphicsResources.c`.

## Naming

- **LinkResource__NoOp**, tier B. Empty body filling LinkResource's own slot +0x084 (`slot84`; the slot LinkResource__BuildModels calls is +0x078, LinkResource__MapModel -- corrected round 89); mechanics-only name, no further evidence of purpose.

## Track 4

2026-09-26, round 89 (delta): LinkResource (table `gLinkResourceMethods`,
renamed from D_8006F13C) is unified in `include/LinkResource.h`. The
unit-local views this body used (`DataSrc33808`, `Obj6F13C`, `Buf439EC`,
`Rec6F13C`/`Buf6F13C`, the `extern s32 D_8006F13C[]` array) are gone:
`self` is `LinkResource *`, its +0x02C is `TmdModel **models`, the buffer is
read as `TmdFile *` (include/TmdModel.h), the allocator's descriptor is
`ResourceSource *`, and the getter returns `&gLinkResourceMethods`.
Byte-identical.
