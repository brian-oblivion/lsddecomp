# Tod__Finalize -- MATCHED (14/14 words)

> Renamed from `func_80043F78` on 2026-09-25 (tools/rename.py). Address 0x80043f78.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 14/14 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Finalize straight to the active driver's: `GetActiveDataSourceMethods()->finalize(self)`, the TimImage__Finalize shape.

Table slot (`tools/classtable.py`): gTodMethods +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTodMethods +0x00C: finalize, straight to the active driver's. */
void Tod__Finalize(Class6D430 *self) {
    GetActiveDataSourceMethods()->finalize(self);
}
```

## Notes

- No shared header was edited. `Class6D430.h`, `Class6B5CC.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **Tod__Finalize**, tier A. Class6D430 finalize override, straight to the active driver's.

## Track 4 (2026-09-26, round 86, charlie)

`self` is now `Tod *` (include/Tod.h), no longer `Class6D430 *`; the call to the active driver's finalize upcasts `(Class6D430 *)self` (a pointer cast, no code). Bytes unchanged.
