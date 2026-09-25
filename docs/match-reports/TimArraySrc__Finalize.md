# TimArraySrc__Finalize -- MATCHED (22/22 words)

> Renamed from `func_80043C60` on 2026-09-25 (tools/rename.py). Address 0x80043c60.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 22/22 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Identical body to TimBlockSrc__Finalize: release the +0x30 array of +0x2C entries, free it, active driver's finalize.

Table slot (`tools/classtable.py`): D_8006F1C4 +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* D_8006F1C4 +0x00C: finalize -- same shape as D_8006F0B8's. */
void TimArraySrc__Finalize(DataSrc33808 *self) {
    ReleaseBasicClassArray((BasicClass **)self->unk30, self->unk2C);
    BMemPMgrFree(self->unk30);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
```

## Notes

- Byte-exact on the first build.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.
