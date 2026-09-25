# New_ModelData -- MATCHED (28/28 words)

> Renamed from `func_8004468C` on 2026-09-25 (tools/rename.py). Address 0x8004468c.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 28/28 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `Ctor33808` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. If the constructor returns zero the object is freed and NULL returned: `if (obj != NULL) { if (ctor(obj, a)) return obj; BMemPMgrFree(obj); } return NULL;`. Here the constructor gets (obj, arg0, 1).

Table slot (`tools/classtable.py`): none (allocator for D_8006F384, object size 0x38).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
#include "ModelData.h"

/* Allocate and construct a D_8006F384 object (second constructor argument 1); freed and NULL when the constructor fails. */
ModelData *New_ModelData(Src6F240 *src) {
    void *obj = BMemPMgrAlloc(0x38);

    if (obj != NULL) {
        if (((Ctor33808 *)GetModelDataMethods())->ctor(obj, src, 1)) {
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

- **New_ModelData**, tier A. include/code_55dd4.h's Unk5CObj already names this object's own +0x2C/+0x30 fields "tmd"/"tods" (populated by New_LinkResource/New_TodSet), and src/code_55dd4.c's own header comment calls this allocator's result "modelData".

## Track 4

2026-09-25, round 84 (delta): ModelData (D_8006F384) is unified in `include/ModelData.h`; the unit-shared `DataSrc33808` view no longer types it. The allocator now reads `ModelData *New_ModelData(Src6F240 *src)` (was `void *` from `s32 arg0`): its argument is the ctor's descriptor, passed through unchanged in $a1, so the bytes are the same. Its callers cast: TriggerWorld__BuildParts, InitDreamAux (code_4cd08) and Class65650__AcquireModelData (code_55dd4), whose local externs of it are deleted. The ctor is still reached through the unprototyped Ctor33808 view, because CLASS6D430_SLOTS declares +0x008 returning void. Image byte-identical.
