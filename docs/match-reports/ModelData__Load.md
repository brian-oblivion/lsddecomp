# ModelData__Load -- MATCHED (20/20 words)

> Renamed from `func_80044808` on 2026-09-25 (tools/rename.py). Address 0x80044808.

Round 82, runner echo (graphics_resources session, echo #6), 2026-09-25. Unit `graphics_resources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 20/20 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`GetActiveDataSourceMethods()->setFlag(self); self->methods->slot78(self);`

Table slot (`tools/classtable.py`): gModelDataMethods +0x064.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
#include "ModelData.h"

/* gModelDataMethods +0x064: the active driver's setFlag, then slot +0x078. */
void ModelData__Load(ModelData *self) {
    GetActiveDataSourceMethods()->setFlag((FileResource *)self);
    ((s32 (*)())self->methods->slot78)(self);
}
```

## Notes

- No shared header was edited. `file_resource.h`, `scene_node.h`, `basic_class.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **ModelData__Load**, tier A. Slot +0x064: the active driver's setFlag, then BuildResources (slot78).

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. Slot +0x078 keeps FileResource's name `slot78`: an inherited slot keeps the parent's name. Its occupant here is ModelData__BuildResources, so the call still casts it. Image byte-identical.
