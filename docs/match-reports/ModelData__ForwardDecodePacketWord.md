# ModelData__ForwardDecodePacketWord -- MATCHED (17/17 words)

> Renamed from `func_800449B8` on 2026-09-25 (tools/rename.py). Address 0x800449b8.

Round 82, runner echo (GraphicsResources session, echo #6), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 17/17 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Forwarder with six arguments: `return self->unk30->methods->slot80(self->unk30, arg1..arg5);` -- the incoming stack args 5/6 are copied to the outgoing frame's +0x10/+0x14 and +0x30 is loaded twice (once for the table, once for a0).

Table slot (`tools/classtable.py`): gModelDataMethods +0x084 and gTriggerWorldMethods +0x084 (`decodeTodPacket` in src/TodActor.c).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/GraphicsResources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
#include "ModelData.h"

/* gModelDataMethods/gTriggerWorldMethods +0x084: forwarded to slot +0x080 of the object at +0x30. */
void *ModelData__ForwardDecodePacketWord(ModelData *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5) {
    return ((DataSrc33808 *)self->todSet)->methods->slot80(self->todSet, arg1, arg2, arg3, arg4, arg5);
}
```

## Notes

- No shared header was edited. `FileResource.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **ModelData__ForwardDecodePacketWord**, tier A. Slot +0x084, shared with TriggerWorld: forwards to slot80 of the tods object at +0x30.

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. Slot +0x084 is `decodePacketWord`. The TodSet's slot +0x080 lies past FileResource's table, and the TodSet class (gTodSetMethods) is not unified, so the call reaches it through `((DataSrc33808 *)self->todSet)->methods->slot80`: a pointer cast, no code. Image byte-identical.

## Track 4 (2026-09-26, round 88, delta)

The forwarded call now reaches the TodSet at +0x030 through its own table, `((TodSet *)self->todSet)->methods->decodePacketWord(...)`, instead of casting it to the unit-local DataSrc33808 for `slot80`; the s32 arguments (ModelData.h's slot type, unchanged) are cast to the slot's pointer types, which emits no code. Holding the pointer in a local first did NOT match (the whole-image SHA1 went red); the double cast inline does. Bytes unchanged.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `arg1`..`arg5` | `packet`, `objId`, `type`, `flag`, `len` | A | forwarded to DecodeTodPacketWord; still `s32` because include/ModelData.h declares them so |
