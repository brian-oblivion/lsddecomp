# New_TodSet -- MATCHED (28/28 words)

> Renamed from `func_800451B8` on 2026-09-25 (tools/rename.py). Address 0x800451b8.

Round 82, runner echo (graphics_resources session, echo #7), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 28/28 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. If the constructor returns zero the object is freed and NULL returned: `if (obj != NULL) { if (ctor(obj, a)) return obj; BMemPMgrFree(obj); } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for gTodSetMethods, object size 0x2C).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.

```c
/* Allocate and construct a gTodSetMethods object; freed and NULL when the constructor fails. */
void *New_TodSet(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x2C);

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetTodSetMethods())->ctor(obj, arg0)) {
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

- **New_TodSet**, tier A. src/world/TodActor.c's Unk30Obj is already "the TOD set, see Unk30Obj", built by this allocator over an array of Tod objects.

## Track 4 (2026-09-26, round 88, delta)

Now `TodSet *New_TodSet(struct ResourceSource *src)` (include/TodSet.h): the argument is the construction descriptor TodSet__TodSet hands straight to Tod__Tod, and the object is a TodSet (0x2C bytes, no own fields). The ctor is still reached through the unprototyped UnprototypedCtorTable view, because TOD_SLOTS types +0x008 returning void while this ctor returns self or NULL. ModelData__BuildResources, the one caller, casts `(ResourceSource *)&req` in and `(FileResource *)` out (ModelData.todSet is still `FileResource *`). Bytes unchanged.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(TodSet)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
