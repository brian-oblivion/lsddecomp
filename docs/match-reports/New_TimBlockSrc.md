# New_TimBlockSrc -- MATCHED (24/24 words)

> Renamed from `func_80043008` on 2026-09-25 (tools/rename.py). Address 0x80043008.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 24/24 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `Ctor33808` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. Shape: `if (obj != NULL) { ctor; return obj; } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for gTimBlockSrcMethods, object size 0x84).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* Allocate and construct a gTimBlockSrcMethods object. */
void *New_TimBlockSrc(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x84);

    if (obj != NULL) {
        ((Ctor33808 *)GetTimBlockSrcMethods())->ctor(obj, arg0);
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

- **New_TimBlockSrc**, tier B. Allocator for gTimBlockSrcMethods; class named for its own mechanics (see TimBlockSrc__TimBlockSrc).

## Track 4 (2026-09-25, round 83, bravo)

The allocator; 0x84 is the class size the header records. It now reaches the ctor through the typed getter (`GetTimBlockSrcMethods()->ctor(obj, (char *)arg0)`) instead of the unit's `Ctor33808` cast; its own signature is unchanged because `src/class_3bb8c_l.c` declares it `s32 New_TimBlockSrc(s32)` locally. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/code_33808.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
