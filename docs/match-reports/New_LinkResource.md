# New_LinkResource -- MATCHED (28/28 words)

> Renamed from `func_80043840` on 2026-09-25 (tools/rename.py). Address 0x80043840.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 28/28 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `UnprototypedCtorTable` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. If the constructor returns zero the object is freed and NULL returned: `if (obj != NULL) { if (ctor(obj, a)) return obj; BMemPMgrFree(obj); } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for LinkResource (gLinkResourceMethods), object size 0x30).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/code_33808.c`.

```c
/* Allocate and construct a LinkResource (gLinkResourceMethods) object; freed and NULL when the constructor fails. */
void *New_LinkResource(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x30);

    if (obj != NULL) {
        if (((UnprototypedCtorTable *)GetLinkResourceMethods())->ctor(obj, arg0)) {
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

- **New_LinkResource**, tier B (head review, round 83: was A). src/class_3bb8c.c and include/code_55dd4.h already declare this allocator's return type as `LinkResource *` at their own call sites.
  Head review, round 83: the class name rests on one caller's local view type, named by an earlier runner (class_3bb8c.c round 20 for LinkResource; code_4cd08.c round 43 for TriggerWorld), not on this body. The body shows mechanics only, so tier B; track 4 may sharpen it.

## Track 4

2026-09-26, round 89 (delta): LinkResource (table `gLinkResourceMethods`,
renamed from D_8006F13C) is unified in `include/LinkResource.h`. The
unit-local views this body used (`DataSrc33808`, `Obj6F13C`, `Buf439EC`,
`Rec6F13C`/`Buf6F13C`, the `extern s32 D_8006F13C[]` array) are gone:
`self` is `LinkResource *`, its +0x02C is `TmdModel **models`, the buffer is
read as `TmdFile *` (include/TmdModel.h), the allocator's descriptor is
`ResourceSource *`, and the getter returns `&gLinkResourceMethods`.
Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| the size literal | `sizeof(LinkResource)` | A | each equals the object size the class header records (and the allocation retail makes); the image is byte-identical |
