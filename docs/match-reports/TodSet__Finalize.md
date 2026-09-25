# TodSet__Finalize -- MATCHED (20/20 words)

> Renamed from `func_800452AC` on 2026-09-25 (tools/rename.py). Address 0x800452ac.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 20/20 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`ReleaseBasicClassArray(buf->entries, buf->count)` over the buffer's counted array, then the PARENT's finalize through its getter: `((DataSrc33808Methods *)GetTodMethods())->finalize(self)` -- GetTodMethods returns D_8006F240, so D_8006F590 derives from D_8006F240 (consistent with the shared +0x07C/+0x080 slots).

Table slot (`tools/classtable.py`): D_8006F590 +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* D_8006F590 +0x00C: finalize -- release the buffer's counted object array,
 * then the parent D_8006F240's. */
void TodSet__Finalize(DataSrc33808 *self) {
    CountedBuf33808 *buf = self->buffer;

    ReleaseBasicClassArray((BasicClass **)buf->entries, buf->count);
    ((DataSrc33808Methods *)GetTodMethods())->finalize(self);
}
```

## Notes

- No shared header was edited. `Class6D430.h`, `Class6B5CC.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TodSet__Finalize**, tier A. Slot +0x00C: releases the buffer's counted object array, then the parent Tod's finalize.
