# ModelData__ReleaseResources -- MATCHED (33/33 words)

> Renamed from `func_800448F8` on 2026-09-25 (tools/rename.py). Address 0x800448f8.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 33/33 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

When +0x34 is nonzero: releases (own slot +0x004) the object at +0x30 if non-NULL, then the object at +0x2C if non-NULL. The release results are discarded (the fields are not cleared).

Table slot (`tools/classtable.py`): D_8006F384 +0x07C.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* D_8006F384 +0x07C: when +0x34 is set, release the objects at +0x30 and
 * +0x2C (each when there is one). */
void ModelData__ReleaseResources(DataSrc33808 *self) {
    if (self->unk34 != 0) {
        if (self->unk30 != NULL) {
            self->unk30->methods->release(self->unk30);
        }
        if ((DataSrc33808 *)self->unk2C != NULL) {
            ((DataSrc33808 *)self->unk2C)->methods->release((DataSrc33808 *)self->unk2C);
        }
    }
}
```

## Notes

First build. +0x2C is `s32` in the unit-local DataSrc33808 view (other classes store a count there), so it is cast at the use rather than retyped.

## Naming

- **ModelData__ReleaseResources**, tier A. Slot +0x07C: releases the tmd/tods sub-objects when owned.
