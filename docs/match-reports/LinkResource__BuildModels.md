# LinkResource__BuildModels -- MATCHED (75/75 words)

> Renamed from `func_800439EC` on 2026-09-25 (tools/rename.py). Address 0x800439ec.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 75/75 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Allocate a (count + 1)-word array (count at buffer +0x08) into +0x2C (1 when that fails), map the TMD through its own +0x078 (LinkResource__MapModel), then build one New_TmdModel object per 0x1C-byte record from buffer +0x0C into the array. On a NULL, walk back releasing (slot +0x004) every one already built, free the array and return 1. Otherwise NULL-terminate the array (the terminator LinkResource__Finalize's finalize walks to), call the active driver's setFlag and return 0.

Table slot (`tools/classtable.py`): D_8006F13C +0x064 (setFlag override).

## Source

The unit-local views `DataSrc33808` (Class6D430 subclass via the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* D_8006F13C +0x064: build a NULL-ended array at +0x2C of one
 * New_TmdModel object per 0x1C-byte record of the buffer (from +0x0C,
 * +0x08 of them), after mapping the TMD (own +0x078); 1 when an allocation
 * fails (everything built so far released and the array freed), otherwise
 * the active driver's setFlag and 0. */
typedef struct Buf439EC {
    /* +0x00 */ u8 pad0[8];
    /* +0x08 */ u32 count;
    /* +0x0C */ u8 recs[1][0x1C];
} Buf439EC;

extern void *New_TmdModel(void *arg);

s32 LinkResource__BuildModels(DataSrc33808 *self) {
    DataSrc33808 **objs;
    u32 i;

    objs = BMemPMgrAlloc((((Buf439EC *)self->buffer)->count + 1) * 4);
    if (objs == NULL) {
        return 1;
    }
    self->unk2C = (s32)objs;
    ((void (*)())self->methods->slot78)(self);
    for (i = 0; i < ((Buf439EC *)self->buffer)->count; i++) {
        *objs = New_TmdModel(((Buf439EC *)self->buffer)->recs[i]);
        if (*objs == NULL) {
            while (i != 0) {
                i--;
                objs--;
                (*objs)->methods->release(*objs);
            }
            BMemPMgrFree(objs);
            return 1;
        }
        objs++;
    }
    *objs = NULL;
    GetActiveDataSourceMethods()->setFlag((Class6D430 *)self);
    return 0;
}
```

## Notes

Second build (the first did not compile: `Rec6F13C` is defined later in the unit, so the local buffer view uses `u8 recs[1][0x1C]`). Byte-exact at once with the unwinding loop in the shape TodSet__BuildTods established this session -- `while (i != 0) { i--; objs--; release(*objs); }` -- which also yields retail's post-loop `addiu s1,s1,4` fix-up so the array base reaches BMemPMgrFree. The early `if (objs == NULL) return 1;` was fine here. New_TmdModel (code_fa50.c) is prototyped locally with void * return; slot78 is cast at the call site.

## Naming

- **LinkResource__BuildModels**, tier A. Slot +0x064: builds the NULL-ended array of New_TmdModel objects LinkResource holds, one per 0x1C-byte record.
