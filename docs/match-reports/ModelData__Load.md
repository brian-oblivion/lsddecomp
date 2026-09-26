# ModelData__Load -- MATCHED (20/20 words)

> Renamed from `func_80044808` on 2026-09-25 (tools/rename.py). Address 0x80044808.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 20/20 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`GetActiveDataSourceMethods()->setFlag(self); self->methods->slot78(self);`

Table slot (`tools/classtable.py`): D_8006F384 +0x064.

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
#include "ModelData.h"

/* D_8006F384 +0x064: the active driver's setFlag, then slot +0x078. */
void ModelData__Load(ModelData *self) {
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
    ((s32 (*)())self->methods->slot78)(self);
}
```

## Notes

- No shared header was edited. `Class6D430.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **ModelData__Load**, tier A. Slot +0x064: the active driver's setFlag, then BuildResources (slot78).

## Track 4

2026-09-25, round 84 (delta): ModelData (D_8006F384) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. Slot +0x078 keeps Class6D430's name `slot78`: an inherited slot keeps the parent's name. Its occupant here is ModelData__BuildResources, so the call still casts it. Image byte-identical.
