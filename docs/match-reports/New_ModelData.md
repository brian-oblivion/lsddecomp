# New_ModelData -- MATCHED (28/28 words)

> Renamed from `func_8004468C` on 2026-09-25 (tools/rename.py). Address 0x8004468c.

Round 82, runner echo (GraphicsResources session, echo #7), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 28/28 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. If the constructor returns zero the object is freed and NULL returned: `if (obj != NULL) { if (ctor(obj, a)) return obj; BMemPMgrFree(obj); } return NULL;`. Here the constructor gets (obj, arg0, 1).

Table slot (`tools/classtable.py`): none (allocator for gModelDataMethods, object size 0x38).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/GraphicsResources.c`.

```c
#include "ModelData.h"

/* Allocate and construct a gModelDataMethods object (second constructor argument 1); freed and NULL when the constructor fails. */
ModelData *New_ModelData(ResourceSource *src) {
    void *obj = BMemPMgrAlloc(0x38);

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetModelDataMethods())->ctor(obj, src, 1)) {
            return obj;
        }
        BMemPMgrFree(obj);
    }
    return NULL;
}
```

## Notes

- Byte-exact on the first build.
- Not referenced by any data word (`grep` of asm/data finds no pointer to it); called from code elsewhere or unused.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **New_ModelData**, tier A. include/TodActor.h's Unk5CObj already names this object's own +0x2C/+0x30 fields "tmd"/"tods" (populated by New_LinkResource/New_TodSet), and src/world/TodActor.c's own header comment calls this allocator's result "modelData".

## Track 4

2026-09-25, round 84 (delta): ModelData (gModelDataMethods) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. The allocator now reads `ModelData *New_ModelData(ResourceSource *src)` (was `void *` from `s32 arg0`): its argument is the ctor's descriptor, passed through unchanged in $a1, so the bytes are the same. Its callers cast: TriggerWorld__BuildResources, InitDreamAux (dream_aux) and TodActor__AcquireModelData (TodActor), whose local externs of it are deleted. The ctor is still reached through the unprototyped UnprototypedCtorTable view, because FILERESOURCE_SLOTS declares +0x008 returning void. Image byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(ModelData)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
