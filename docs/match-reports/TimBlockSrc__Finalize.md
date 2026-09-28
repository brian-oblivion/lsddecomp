# TimBlockSrc__Finalize -- MATCHED (22/22 words)

> Renamed from `func_800431A8` on 2026-09-25 (tools/rename.py). Address 0x800431a8.

Round 82, runner echo (GraphicsResources session, echo #7), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 22/22 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Releases the BasicClass array at +0x30 (+0x2C entries), frees that allocation, then calls the active data-source driver's finalize.

Table slot (`tools/classtable.py`): gTimBlockSrcMethods +0x00C (finalize).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/GraphicsResources.c`.

```c
/* gTimBlockSrcMethods +0x00C: finalize -- release the object array at +0x30 (+0x2C
 * entries), free it, then the active driver's. */
void TimBlockSrc__Finalize(DataSrc33808 *self) {
    ReleaseBasicClassArray((BasicClass **)self->unk30, self->unk2C);
    BMemPMgrFree(self->unk30);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}
```

## Notes

- Byte-exact on the first build.
- +0x30 is typed `DataSrc33808 *unk30` in the unit view (its meaning for gModelDataMethods/gTriggerWorldMethods); for this class it is the array, so the call casts.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TimBlockSrc__Finalize**, tier B. FileResource finalize override for TimBlockSrc: releases the block array and the active driver's finalize.

## Track 4 (2026-09-25, round 83, bravo)

Occupant of +0x00C: releases `blocks` (`blockCount` entries), frees it, then the active driver's finalize. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/graphics/GraphicsResources.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
