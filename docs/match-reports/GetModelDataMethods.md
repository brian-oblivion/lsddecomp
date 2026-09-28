# GetModelDataMethods -- MATCHED (4/4 words)

> Renamed from `func_800449FC` on 2026-09-25 (tools/rename.py). Address 0x800449fc.

Round 82, runner echo (GraphicsResources session, echo #5), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gModelDataMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gModelDataMethods[];`.

Table slot (`tools/classtable.py`): `gFileResourceMethods` +0x0AC.

## Source

```c
#include "ModelData.h"

ModelDataMethods *GetModelDataMethods(void) {
    return &gModelDataMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/graphics/GraphicsResources.c`.

## Naming

- **GetModelDataMethods**, tier A. Table getter.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. The getter returns `ModelDataMethods *` from `&gModelDataMethods`, where it used to return `void *` from a local `extern s32 gModelDataMethods[]` that is now deleted. The code is the same lui/addiu. Image byte-identical.
