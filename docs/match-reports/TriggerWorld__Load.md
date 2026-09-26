# TriggerWorld__Load -- MATCHED (12/12 words)

> Renamed from `func_80044B58` on 2026-09-25 (tools/rename.py). Address 0x80044b58.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 12/12 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`self->methods->slot78(self)` through the unified `void *slot78` cast to an unprototyped function pointer. No s0 is involved, so passing self explicitly costs nothing (a0 still holds it).

Table slot (`tools/classtable.py`): gTriggerWorldMethods +0x064.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTriggerWorldMethods +0x064: slot +0x078. */
void TriggerWorld__Load(DataSrc33808 *self) {
    ((s32 (*)())self->methods->slot78)(self);
}
```

## Notes

- No shared header was edited. `FileResource.h`, `Class6B5CC.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TriggerWorld__Load**, tier A. Slot +0x064: slot78 (BuildParts).

## Track 4 (2026-09-26, round 88, bravo)

Now `void TriggerWorld__Load(TriggerWorld *self)` (include/TriggerWorld.h); the +0x078 call keeps its `s32 (*)()` cast (FileResource's slot78 is `void *`; the occupant is TriggerWorld__BuildResources). Bytes unchanged.
