# TriggerWorld__Finalize -- MATCHED (21/21 words)

> Renamed from `func_80044B04` on 2026-09-25 (tools/rename.py). Address 0x80044b04.

Round 82, runner echo (graphics_resources session, echo #6), 2026-09-25. Unit `graphics_resources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 21/21 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`self->methods->slot7C();` (zero-argument, as ModelData__Finalize) then the parent gModelDataMethods's finalize through its getter `GetModelDataMethods`, cast to the local methods type. gTriggerWorldMethods derives from gModelDataMethods (they share +0x080/+0x084).

Table slot (`tools/classtable.py`): gTriggerWorldMethods +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTriggerWorldMethods +0x00C: finalize -- slot +0x07C, then the parent gModelDataMethods's. */
void TriggerWorld__Finalize(DataSrc33808 *self) {
    self->methods->slot7C();
    ((DataSrc33808Methods *)GetModelDataMethods())->finalize(self);
}
```

## Notes

- No shared header was edited. `FileResource.h`, `scene_node.h`, `basic_class.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TriggerWorld__Finalize**, tier A. Slot +0x00C: releases parts (slot7C) then the parent ModelData's finalize.

## Track 4 (2026-09-26, round 88, bravo)

Now `void TriggerWorld__Finalize(TriggerWorld *self)`: `self->methods->slot7C()` (argumentless, through the unit-local DataSrc33808 view) is `self->methods->releaseResources(self)`, ModelData's +0x07C, and the parent call is `GetModelDataMethods()->finalize((ModelData *)self)` with no table cast. Passing self emits no code: it is in $a0 on entry, and the argumentless call left it there. Bytes unchanged.
