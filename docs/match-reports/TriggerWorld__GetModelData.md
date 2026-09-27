# TriggerWorld__GetModelData -- MATCHED (13/13 words)

> Renamed from `TriggerWorld__GetOffset` on 2026-09-26 (tools/rename.py). Address 0x80044c90.

> Renamed from `func_80044C90` on 2026-09-25 (tools/rename.py). Address 0x80044c90.

Round 82, runner echo (GraphicsResources session, echo #6), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 13/13 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`buf = self->buffer; if (index < buf->count) return buf->entries[index]; return 0;` with an unsigned index (`sltu`). The `j` + `addu v0,zero,zero` delay slot is GCC's layout for the out-of-range return; matched on the first build as written.

Table slot (`tools/classtable.py`): gTriggerWorldMethods +0x088.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/GraphicsResources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTriggerWorldMethods +0x088: entry `index` of the buffer's counted word array, 0 when
 * out of range. */
s32 TriggerWorld__GetModelData(DataSrc33808 *self, u32 index) {
    SubBlockTable *buf = self->buffer;

    if (index < buf->count) {
        return buf->entries[index];
    }
    return 0;
}
```

## Notes

- No shared header was edited. `FileResource.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TriggerWorld__GetModelData**, tier A. Slot +0x088: entry `index` of the buffer's counted word array, 0 out of range.

## Track 4 (2026-09-26, round 88, bravo)

Renamed from `TriggerWorld__GetOffset`, and its return type is now
`ModelData *` (was `s32`; the word is returned unchanged in $v0, image
byte-identical). The words it reads are not offsets once the object is
built: TriggerWorld__BuildResources overwrites each `entries[i]` with the
`New_ModelData` it made over buffer + entries[i], and the ctor runs that
build (via +0x064 -> +0x078) whenever the descriptor has a buffer.

The caller confirms it. `ProcessDreamAuxTriggerRecord` (code_4cd08) gets its
object from `New_TriggerWorld` (FireDreamAuxTriggerEntries), calls this slot
(+0x088) with the record's parity, and stores the result at `scratch[3]`
(+0x00C), which `SpawnDreamAuxTriggerEntity` passes as `New_Entity`'s
descriptor; Entity__Entity hands it to TodActor__TodActor, whose
TodActor__AcquireModelData borrows the ModelData at the descriptor's
+0x00C (TodActorDesc.modelData). Class unified in `include/TriggerWorld.h`
(slot +0x088 `getModelData`).
