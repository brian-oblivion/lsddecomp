# TimArraySrc__UploadImages -- MATCHED (30/30 words)

> Renamed from `TimArraySrc__NotifyImages` on 2026-09-26 (tools/rename.py). Address 0x80043dfc.

> Renamed from `func_80043DFC` on 2026-09-25 (tools/rename.py). Address 0x80043dfc.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 30/30 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Walks the object-pointer array at +0x30 (+0x2C entries, count re-read every iteration) and calls each object's own slot +0x078 with that object.

Table slot (`tools/classtable.py`): gTimArraySrcMethods +0x078.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `CountedBuf33808` sit at the top of
`src/code_33808.c`.

```c
/* gTimArraySrcMethods +0x078: slot +0x078 of every object in the array at +0x30
 * (+0x2C entries). */
void TimArraySrc__UploadImages(DataSrc33808 *self) {
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

## Naming

- **TimArraySrc__UploadImages**, tier A. Slot +0x078: forwards slot78 to every image in the array.

## Track 4 (2026-09-26, round 88)

The array at +0x30 holds TimImages (TimArraySrc__BuildImages fills it with
New_TimImage(NULL)), so `objs` is `TimImage **` and the +0x078 call goes
through `TimImageUploadFn` (include/TimImage.h: FileResource's `void *slot78`,
whose occupant here is TimImage__Upload) instead of an unprototyped cast of
DataSrc33808's slot. Image byte-identical.

## Track 4 (2026-09-26, round 88, runner alpha)

Renamed from `TimArraySrc__NotifyImages`. The slot it forwards is +0x078 of
every object in `images` (+0x030), and every such object is a TimImage:
TimArraySrc__BuildImages fills the array with `New_TimImage(NULL)` and
nothing else writes it. TimImage's +0x078 occupant is TimImage__Upload
(include/TimImage.h), so the body uploads every image of the block; its one
reach is TimBlockSrc__AdvanceLoadState, which calls this slot right after
setFlag (BuildImages) on each new TimArraySrc. The call goes through
TimImageUploadFn (no code). Class header: include/TimArraySrc.h.


## Track 4 (2026-09-26, round 88, runner alpha)
Class unified: self is TimArraySrc (include/TimArraySrc.h), images read as TimImage ** and each call goes through TimImageUploadFn. Byte-identical.
