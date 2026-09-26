# ModelData__ForwardScanPackets -- MATCHED (15/15 words)

> Renamed from `func_8004497C` on 2026-09-25 (tools/rename.py). Address 0x8004497c.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 15/15 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Forwarder: `return self->unk30->methods->slot78(self->unk30, arg1, arg2);` returning u8 (`andi 0xFF`).

Table slot (`tools/classtable.py`): gModelDataMethods +0x080 and gTriggerWorldMethods +0x080 (include/code_55dd4.h names this slot `getObjectIds` in its Unk5CObj view).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
#include "ModelData.h"

/* gModelDataMethods/gTriggerWorldMethods +0x080: forwarded to slot +0x078 of the object at +0x30. */
u8 ModelData__ForwardScanPackets(ModelData *self, s32 arg1, s32 arg2) {
    return ((s32 (*)())self->todSet->methods->slot78)(self->todSet, arg1, arg2);
}
```

## Notes

- No shared header was edited. `FileResource.h`, `Class6B5CC.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **ModelData__ForwardScanPackets**, tier A. Slot +0x080, shared with TriggerWorld: forwards to slot78 of the tods object at +0x30.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. Slot +0x080 is `scanPackets`, `u8 (*)(ModelData *self, s32 arg1, s32 arg2)`. The forwarded call reads `todSet->methods->slot78` through FileResource's table, still cast, because the TodSet class (gTodSetMethods) is not unified. Image byte-identical.
