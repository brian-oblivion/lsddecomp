# TriggerWorld__ReleaseResources -- MATCHED (14/14 words)

> Renamed from `TriggerWorld__ReleaseParts` on 2026-09-26 (tools/rename.py). Address 0x80044c58.

> Renamed from `func_80044C58` on 2026-09-25 (tools/rename.py). Address 0x80044c58.

Round 82, runner echo (GraphicsResources session, echo #6), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 14/14 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`ReleaseBasicClassArray((BasicClass **)((u8 *)self->buffer + 8), self->unk38); self->unk38 = 0;`

Table slot (`tools/classtable.py`): gTriggerWorldMethods +0x07C.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/GraphicsResources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTriggerWorldMethods +0x07C: release the object array in the buffer (past its first
 * two words), +0x38 entries long, and zero the count. */
void TriggerWorld__ReleaseResources(DataSrc33808 *self) {
    ReleaseBasicClassArray((BasicClass **)((u8 *)self->buffer + 8), self->unk38);
    self->unk38 = 0;
}
```

## Notes

- No shared header was edited. `FileResource.h`, `SceneNode.h`, `basic_class.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TriggerWorld__ReleaseResources**, tier A. Slot +0x07C: releases the ModelData array, +0x38 entries, and zeroes the count.

## Track 4 (2026-09-26, round 88, bravo)

Renamed from `TriggerWorld__ReleaseParts`: it occupies +0x07C, ModelData's
`releaseResources` slot (MODELDATA_SLOTS in `include/ModelData.h`), and the
body does exactly what the slot says for this class (release the ModelData
array TriggerWorld__BuildResources built, zero the count at +0x038). An
override is named for its slot (FINISHING-PLAN track 4 step 6). Class
unified in `include/TriggerWorld.h`.

Retyped in the same round: `self` is `TriggerWorld *`, +0x038 is `modelDataCount` (was DataSrc33808.unk38). Bytes unchanged.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `(u8 *)buffer + 8` | `SubBlockTable.entries` | A | the ModelData BuildResources wrote over the counted offsets |
