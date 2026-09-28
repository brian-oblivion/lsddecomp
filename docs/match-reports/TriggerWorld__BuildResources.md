# TriggerWorld__BuildResources -- MATCHED (52/52 words)

> Renamed from `TriggerWorld__BuildParts` on 2026-09-26 (tools/rename.py). Address 0x80044b88.

> Renamed from `func_80044B88` on 2026-09-25 (tools/rename.py). Address 0x80044b88.

Round 82, runner echo (graphics_resources session, echo #9), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 52/52 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Build step: ResourceRequest__Set fills a three-word request {buffer, 0, 1}; for each of the buffer's `count` offsets, point the request at buffer+offset, allocate a gModelDataMethods source through New_ModelData and store it over the offset word itself (entries[i]), counting successes at +0x38. On an allocation failure call its own +0x07C (TriggerWorld__ReleaseResources, which releases the ones built and zeroes the count) and return 1; otherwise 0. The sibling of gModelDataMethods's ModelData__BuildResources.

Table slot (`tools/classtable.py`): gTriggerWorldMethods +0x078.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable`, `SubBlockTable` and `ResourceSourceArgs` sit at the
top of / earlier in `src/graphics/graphics_resources.c`.

```c
/* gTriggerWorldMethods +0x078: build a gModelDataMethods source (not owning) over each
 * sub-block of the buffer's counted offset table, into the table's own
 * words, counting them at +0x38; 0 when all exist, otherwise slot +0x07C
 * (release) and 1. */
s32 TriggerWorld__BuildResources(DataSrc33808 *self) {
    ResourceSourceArgs req;
    SubBlockTable *buf;
    s32 *p;
    s32 i;
    s32 n;

    ResourceRequest__Set(&req, 0, 0, 1);
    buf = self->buffer;
    i = 0;
    n = buf->count;
    p = buf->entries;
    self->unk38 = 0;
    for (; i < n; i++) {
        req.buffer = (u8 *)self->buffer + ((SubBlockTable *)self->buffer)->entries[i];
        *p = (s32)New_ModelData((ResourceSource *)&req);
        if (*p == 0) {
            goto fail;
        }
        self->unk38++;
        p++;
    }
    return 0;
fail:
    self->methods->slot7C(self);
    return 1;
}
```

## Notes

First build. The loop pointer walks `buf->entries` while the offset is re-read through `self->buffer` each pass (retail reloads 0x10(s1) in the loop body); the count is held as s32 (blez/slt). The failure path is `goto fail` with the label after `return 0`.

## Naming

- **TriggerWorld__BuildResources**, tier A. Slot +0x078: builds a ModelData (not owning) over each sub-block of the buffer's counted offset table, counting them at +0x38.
- **SubBlockTable** (type, was `CountedBuf33808`), tier B (round 95, bravo, track 6). TriggerWorld's and TodSet's buffer as their bodies read it: `count` at +0x04, then `entries[count]`, each an offset from the buffer's start to a sub-block, which this body (ModelData) and TodSet__BuildTods (Tod) overwrite in place with the object built over it; Finalize/ReleaseResources release them from the same array, GetModelData indexes it, and TodSet__ScanPackets reads the TOD that follows it. The word at +0x00 is read by no code (padding). Tier B: what the game keeps in these containers is not established here.

## Track 4

2026-09-25, round 84 (delta): Its parent ModelData (gModelDataMethods) is unified in `include/model_data.h` (this class is still its own job). The part allocation reads `*p = (s32)New_ModelData((ResourceSource *)&req)` (was `(s32)&req`), a pointer cast with no code. Image byte-identical.

## Track 4 (2026-09-26, round 88, bravo)

Renamed from `TriggerWorld__BuildParts`. It occupies +0x078, FileResource's
unnamed `slot78` that ModelData's own table fills with
ModelData__BuildResources; this is TriggerWorld's override of that same step
(build its resources, 0 on success, release and 1 on failure), and its
failure path calls +0x07C, ModelData's `releaseResources` slot, whose
TriggerWorld occupant is now TriggerWorld__ReleaseResources. Named after the
parent's occupant so the pair reads as one. Class unified in
`include/trigger_world.h`.

Retyped in the same round: `self` is `TriggerWorld *`, +0x038 is `modelDataCount` (was DataSrc33808.unk38), and the failure path calls `releaseResources(self)` (was the unit-local `slot7C`). Bytes unchanged.

### Track 6 (round 97, alpha)

The request local is now include/file_resource.h's `ResourceRequest`
(`{ ResourceSource src; s32 mode; }`), and ResourceRequest__Set's prototype
comes from that header. The unit's own view of the record and its local
extern are gone. `req.buffer`/`req.name` become `req.src.buffer`/`req.src.name`, and
`(ResourceSource *)&req` becomes `&req.src`. Byte-identical.
