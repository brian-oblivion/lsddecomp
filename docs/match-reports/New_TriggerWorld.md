# New_TriggerWorld -- MATCHED (28/28 words)

> Renamed from `func_80044A0C` on 2026-09-25 (tools/rename.py). Address 0x80044a0c.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 28/28 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocator shape: `BMemPMgrAlloc(size)`, then the constructor (+0x008) through the class's table getter, called through the unit-local `Ctor33808` view (an unprototyped `s32 (*ctor)()` at +0x008, declared at the top of the unit with the getter prototypes), since each class's constructor takes different arguments. If the constructor returns zero the object is freed and NULL returned: `if (obj != NULL) { if (ctor(obj, a)) return obj; BMemPMgrFree(obj); } return NULL;`.

Table slot (`tools/classtable.py`): none (allocator for D_8006F40C, object size 0x3C).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* Allocate and construct a D_8006F40C object; freed and NULL when the constructor fails. */
void *New_TriggerWorld(s32 arg0) {
    void *obj = BMemPMgrAlloc(0x3C);

    if (obj != NULL) {
        if (((Ctor33808 *)GetTriggerWorldMethods())->ctor(obj, arg0)) {
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

- **New_TriggerWorld**, tier A. code_4cd08.c already declares `extern TriggerWorld *func_80044A0C(s32 *ctx)`, and docs/match-reports/FireDreamAuxTriggerEntries.md (a caller in that same unit) already ties this object into the dream-aux trigger system.
