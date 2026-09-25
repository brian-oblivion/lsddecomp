# TimArraySrc__TimArraySrc -- MATCHED (30/30 words)

> Renamed from `func_80043BE8` on 2026-09-25 (tools/rename.py). Address 0x80043be8.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 30/30 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: runs the active data-source driver's ctor (GetActiveDataSourceMethods()->ctor), installs D_8006F1C4 (GetTimArraySrcMethods), zeroes +0x2C/+0x30/+0x38, and when `name` is non-NULL calls its own requestLoadFile (+0x06C) with it. Returns nothing (v0 is left as the last jalr's).

Table slot (`tools/classtable.py`): D_8006F1C4 +0x008.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* D_8006F1C4 +0x008: constructor -- the active driver's, then this table,
 * clear +0x2C/+0x30/+0x38, and request `name` when there is one. */
void TimArraySrc__TimArraySrc(DataSrc33808 *self, char *name) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTimArraySrcMethods();
    self->unk2C = 0;
    self->unk30 = NULL;
    self->unk38 = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}
```

## Notes

First build. No lever needed.
