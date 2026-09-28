# New_TriggerWorld -- MATCHED (28/28 words)

> Renamed from `func_80044A0C` on 2026-09-25 (tools/rename.py). Address 0x80044a0c.

Round 82, runner echo (GraphicsResources session, echo #7), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 28/28 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. If the constructor returns zero the object is freed and NULL returned: `if (obj != NULL) { if (ctor(obj, a)) return obj; BMemPMgrFree(obj); } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for gTriggerWorldMethods, object size 0x3C).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/GraphicsResources.c`.

```c
/* Allocate and construct a gTriggerWorldMethods object; freed and NULL when the constructor fails. */
void *New_TriggerWorld(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x3C);

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetTriggerWorldMethods())->ctor(obj, arg0)) {
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

- **New_TriggerWorld**, tier B (head review, round 83: was A). code_4cd08.c already declares `extern TriggerWorld *func_80044A0C(s32 *ctx)`, and docs/match-reports/FireDreamAuxTriggerEntries.md (a caller in that same unit) already ties this object into the dream-aux trigger system.
  Head review, round 83: the class name rests on one caller's local view type, named by an earlier runner (DayTaskStageMap.c round 20 for LinkResource; code_4cd08.c round 43 for TriggerWorld), not on this body. The body shows mechanics only, so tier B; track 4 may sharpen it.

## Track 4 (2026-09-26, round 88, bravo)

Now `TriggerWorld *New_TriggerWorld(struct ResourceSource *src)` (include/TriggerWorld.h): the argument is the construction descriptor the ctor hands to ModelData__ModelData, and the object is a TriggerWorld (0x3C bytes, one own field at +0x038). The ctor is still reached through the unprototyped UnprototypedCtorTable view, because MODELDATA_SLOTS types +0x008 returning void while this ctor returns self or NULL. code_4cd08's FireDreamAuxTriggerEntries, the one caller, casts its stack array in. Bytes unchanged.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(TriggerWorld)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
