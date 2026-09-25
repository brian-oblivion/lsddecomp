# MoviePlayer__FreeFrameBuffers -- MATCHED (25/25 words)

> Renamed from `func_8004575C` on 2026-09-25 (tools/rename.py). Address 0x8004575c.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 25/25 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Unless the word at +0x0C is set, frees the allocations at +0x14, +0x18, +0x10 and +0x1C, in that order. The owning class is not identified; the view `Obj4575C` is unit-local and minimal.

Table slot (`tools/classtable.py`): none (not in any table).

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* Unless +0x0C is set, free the four allocations at +0x14, +0x18, +0x10,
 * +0x1C. Not referenced by any data word. */
typedef struct Obj4575C {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 unkC;
    /* +0x010 */ void *unk10;
    /* +0x014 */ void *unk14;
    /* +0x018 */ void *unk18;
    /* +0x01C */ void *unk1C;
} Obj4575C;

void MoviePlayer__FreeFrameBuffers(Obj4575C *self) {
    if (self->unkC == 0) {
        BMemPMgrFree(self->unk14);
        BMemPMgrFree(self->unk18);
        BMemPMgrFree(self->unk10);
        BMemPMgrFree(self->unk1C);
    }
}
```

## Notes

- Byte-exact on the first build.
- Not referenced by any data word (`grep` of asm/data finds no pointer to it); called from code elsewhere or unused.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.
