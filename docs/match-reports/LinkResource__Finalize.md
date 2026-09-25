# LinkResource__Finalize -- MATCHED (38/38 words)

> Renamed from `func_80043954` on 2026-09-25 (tools/rename.py). Address 0x80043954.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 38/38 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Finalize: walks the NULL-terminated object-pointer array at +0x2C releasing each (own slot +0x004), frees the array (BMemPMgrFree), then the active driver's finalize.

Table slot (`tools/classtable.py`): D_8006F13C +0x00C.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* D_8006F13C +0x00C: finalize -- release every object in the NULL-ended
 * array at +0x2C, free the array, then the active driver's. */
void LinkResource__Finalize(DataSrc33808 *self) {
    DataSrc33808 **objs = (DataSrc33808 **)self->unk2C;

    while (*objs != NULL) {
        (*objs)->methods->release(*objs);
        objs++;
    }
    BMemPMgrFree((void *)self->unk2C);
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
```

## Notes

First build. A plain `while (*objs != NULL)` is rotated by GCC into the top-test + bottom-test shape retail has.
