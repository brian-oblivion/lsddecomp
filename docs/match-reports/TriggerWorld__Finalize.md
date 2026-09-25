# TriggerWorld__Finalize -- MATCHED (21/21 words)

> Renamed from `func_80044B04` on 2026-09-25 (tools/rename.py). Address 0x80044b04.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 21/21 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`self->methods->slot7C();` (zero-argument, as ModelData__Finalize) then the parent D_8006F384's finalize through its getter `GetModelDataMethods`, cast to the local methods type. D_8006F40C derives from D_8006F384 (they share +0x080/+0x084).

Table slot (`tools/classtable.py`): D_8006F40C +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* D_8006F40C +0x00C: finalize -- slot +0x07C, then the parent D_8006F384's. */
void TriggerWorld__Finalize(DataSrc33808 *self) {
    self->methods->slot7C();
    ((DataSrc33808Methods *)GetModelDataMethods())->finalize(self);
}
```

## Notes

- No shared header was edited. `Class6D430.h`, `Class6B5CC.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TriggerWorld__Finalize**, tier A. Slot +0x00C: releases parts (slot7C) then the parent ModelData's finalize.
