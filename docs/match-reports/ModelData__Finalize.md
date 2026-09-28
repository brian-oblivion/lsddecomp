# ModelData__Finalize -- MATCHED (21/21 words)

> Renamed from `func_800447B4` on 2026-09-25 (tools/rename.py). Address 0x800447b4.

Round 82, runner echo (GraphicsResources session, echo #6), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 21/21 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`self->methods->slot7C(); GetActiveDataSourceMethods()->finalize(self);` The first `jalr` never sets a0 (it only still holds self by accident): a ZERO-argument call through the unprototyped slot, the round-82 broadcast lever. Writing `slot7C(self)` would add a `move a0,s0`.

Table slot (`tools/classtable.py`): gModelDataMethods +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/GraphicsResources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
#include "ModelData.h"

/* gModelDataMethods +0x00C: finalize -- slot +0x07C, then the active driver's. */
void ModelData__Finalize(ModelData *self) {
    self->methods->releaseResources(self);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}
```

## Notes

- No shared header was edited. `FileResource.h`, `scene_node.h`, `basic_class.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **ModelData__Finalize**, tier A. Slot +0x00C: releases resources (slot7C) then the active driver's finalize.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. Slot +0x07C is `releaseResources` (occupant ModelData__ReleaseResources), prototyped `void (*)(ModelData *self)`. The call is now `releaseResources(self)`; before, it was the unprototyped `slot7C()` with no argument. The bytes are the same because self is already in $a0 at the jalr. Image byte-identical.
