# TimArraySrc__NotifyImages -- MATCHED (30/30 words)

> Renamed from `func_80043DFC` on 2026-09-25 (tools/rename.py). Address 0x80043dfc.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 30/30 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Walks the object-pointer array at +0x30 (+0x2C entries, count re-read every iteration) and calls each object's own slot +0x078 with that object.

Table slot (`tools/classtable.py`): D_8006F1C4 +0x078.

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* D_8006F1C4 +0x078: slot +0x078 of every object in the array at +0x30
 * (+0x2C entries). */
void TimArraySrc__NotifyImages(DataSrc33808 *self) {
    DataSrc33808 **objs = (DataSrc33808 **)self->unk30;
    s32 i;

    for (i = 0; i < self->unk2C; i++) {
        ((void (*)())(*objs)->methods->slot78)(*objs);
        objs++;
    }
}
```

## Notes

First build. Pointer walk with the increment after the call (the addiu lands in the jalr delay slot). slot78 is `void *` in the unified macro; cast at the call site.
