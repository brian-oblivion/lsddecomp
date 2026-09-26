# New_TimArraySrc -- MATCHED (24/24 words)

> Renamed from `func_80043B88` on 2026-09-25 (tools/rename.py). Address 0x80043b88.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 24/24 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `Ctor33808` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. Shape: `if (obj != NULL) { ctor; return obj; } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for D_8006F1C4, object size 0x3C).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* Allocate and construct a D_8006F1C4 object. */
void *New_TimArraySrc(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x3C);

    if (obj != NULL) {
        ((Ctor33808 *)GetTimArraySrcMethods())->ctor(obj, arg0);
        return obj;
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

- **New_TimArraySrc**, tier A. Allocator for D_8006F1C4, which builds arrays of TimImage objects (New_TimImage, confirmed elsewhere as "new TimImage(name)", src/code_2bb9c.c).


## Track 4 (2026-09-26, round 88, runner alpha)
Class unified as TimArraySrc (include/TimArraySrc.h). Now declared `TimArraySrc *New_TimArraySrc(char *name)` and calls the prototyped `GetTimArraySrcMethods()->ctor(obj, name)` instead of casting the table to the unit-local Ctor33808: the ctor's second parameter is `name` (TimArraySrc__TimArraySrc passes it to requestLoadFile) and the one caller, TimBlockSrc__AdvanceLoadState, passes 0 (now NULL). Byte-identical.
