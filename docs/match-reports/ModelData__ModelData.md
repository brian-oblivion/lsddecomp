# ModelData__ModelData -- MATCHED (46/46 words)

> Renamed from `func_800446FC` on 2026-09-25 (tools/rename.py). Address 0x800446fc.

Round 82, runner echo (GraphicsResources session, echo #8), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 46/46 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: active driver's ctor, install gModelDataMethods (GetModelDataMethods), store the third argument at +0x34 (the flag that ModelData__BuildResources tests before building the two sub-sources and ModelData__ReleaseResources before releasing them; named `owns` as a reading, not evidence); then adopt the descriptor's buffer (size 0) and call its own +0x064 (ModelData__Load), returning NULL on a nonzero result, or request the descriptor's file.

Table slot (`tools/classtable.py`): gModelDataMethods +0x008 (the allocator New_ModelData passes 1 as the third argument; the subclass gTriggerWorldMethods's ctor TriggerWorld__TriggerWorld passes 0).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/GraphicsResources.c`.

```c
#include "ModelData.h"

/* gModelDataMethods +0x008: constructor -- the active driver's, then this table,
 * `owns` at +0x34; adopt the descriptor's buffer (size 0) and run its own
 * +0x064, whose nonzero result fails the construction (NULL), or else
 * request its file. */
void *ModelData__ModelData(ModelData *self, ResourceSource *src, s32 owns) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetModelDataMethods();
    self->ownsResources = owns;
    if (src->buffer != NULL) {
        self->buffer = src->buffer;
        self->bufferSize = 0;
        if (((s32 (*)())self->methods->setFlag)(self)) {
            goto fail;
        }
    } else {
        self->methods->requestLoadFile(self, src->name);
    }
    return self;
fail:
    return NULL;
}
```

## Notes

First build, using the `goto fail` lever just found on LinkResource__LinkResource (fail label after the final `return self`). Unlike LinkResource__LinkResource the descriptor is not NULL-tested.

## Naming

- **ModelData__ModelData**, tier A. Constructor: adopts a buffer or requests a file, `owns` stored at +0x34.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. +0x034 is `ownsResources` (was `unk34`). Callers settle what it means: New_ModelData passes 1 and TriggerWorld__TriggerWorld passes 0, and only while it is set do BuildResources build, and ReleaseResources release, the two sub-sources. The ctor's first call is `GetActiveDataSourceMethods()->ctor`, the same call TimBlockSrc__TimBlockSrc makes, which is the evidence that the class sits under FileResource and not under TimBlockSrc (whose id, 0xF03, 0x5F03 extends). Image byte-identical.

## History (moved from include/ModelData.h, round 102)

Comment text moved verbatim out of the header, which now says only
what the code is.

```c
 * The name is round 83's, kept on this evidence: its own methods build a
```
