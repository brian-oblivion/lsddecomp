# TimArraySrc__TimArraySrc -- MATCHED (30/30 words)

> Renamed from `func_80043BE8` on 2026-09-25 (tools/rename.py). Address 0x80043be8.

Round 82, runner echo (GraphicsResources session, echo #8), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 30/30 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor: runs the active data-source driver's ctor (GetActiveDataSourceMethods()->ctor), installs gTimArraySrcMethods (GetTimArraySrcMethods), zeroes +0x2C/+0x30/+0x38, and when `name` is non-NULL calls its own requestLoadFile (+0x06C) with it. Returns nothing (v0 is left as the last jalr's).

Table slot (`tools/classtable.py`): gTimArraySrcMethods +0x008.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/GraphicsResources.c`.

```c
/* gTimArraySrcMethods +0x008: constructor -- the active driver's, then this table,
 * clear +0x2C/+0x30/+0x38, and request `name` when there is one. */
void TimArraySrc__TimArraySrc(DataSrc33808 *self, char *name) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
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

## Naming

- **TimArraySrc__TimArraySrc**, tier A. Constructor; requests or adopts a buffer describing the image array.


## Track 4 (2026-09-26, round 88, runner alpha)
Class unified as TimArraySrc (include/TimArraySrc.h); self retyped from the unit-local DataSrc33808. Fields: unk2C -> count, unk30 -> images (TimImage **), unk38 -> ready, named from BuildImages, which is the only writer after this ctor. Byte-identical.

## Round 93 polish (charlie, track 7)

Comment rewritten only; no names or constants changed.
