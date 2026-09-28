# ModelData__ReleaseResources -- MATCHED (33/33 words)

> Renamed from `func_800448F8` on 2026-09-25 (tools/rename.py). Address 0x800448f8.

Round 82, runner echo (graphics_resources session, echo #8), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 33/33 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

When +0x34 is nonzero: releases (own slot +0x004) the object at +0x30 if non-NULL, then the object at +0x2C if non-NULL. The release results are discarded (the fields are not cleared).

Table slot (`tools/classtable.py`): gModelDataMethods +0x07C.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/graphics_resources.c`.

```c
#include "model_data.h"

/* gModelDataMethods +0x07C: when +0x34 is set, release the objects at +0x30 and
 * +0x2C (each when there is one). */
void ModelData__ReleaseResources(ModelData *self) {
    if (self->ownsResources != 0) {
        if (self->todSet != NULL) {
            self->todSet->methods->release(self->todSet);
        }
        if (self->linkResource != NULL) {
            self->linkResource->methods->release(self->linkResource);
        }
    }
}
```

## Notes

First build. +0x2C is `s32` in the unit-local DataSrc33808 view (other classes store a count there), so it is cast at the use rather than retyped.

## Naming

- **ModelData__ReleaseResources**, tier A. Slot +0x07C: releases the tmd/tods sub-objects when owned.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/model_data.h`; the unit-shared `DataSrc33808` view no longer types it. Its slot, +0x07C, is named `releaseResources` for this function. Both releases now go through the unified FileResource table (`linkResource->methods->release`, `todSet->methods->release`); before, they went through `DataSrc33808` casts. Image byte-identical.
