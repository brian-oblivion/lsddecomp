# TimBlockSrc__SetEntryShift -- MATCHED (9/9 words)

> Renamed from `func_80043538` on 2026-09-25 (tools/rename.py). Address 0x80043538.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 9/9 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`sll 4; addiu 0x40; addu` = `&self->entries[index]` (16-byte entries at +0x40); `sh a2` then `lhu` of the SAME halfword back = store through a u16 field then read it again (`e->mask = 1 << e->shift`), GCC 2.6.3 reloads rather than zero-extending the argument.

Table slot (`tools/classtable.py`): D_8006F0B8 +0x078.

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* D_8006F0B8 +0x078: set entry `index`'s shift, and its mask from it. */
typedef struct Ent6F0B8 {
    /* +0x00 */ u16 shift;
    /* +0x02 */ u16 mask;
    /* +0x04 */ u8 pad4[0xC];
} Ent6F0B8;

typedef struct Obj6F0B8 {
    /* +0x000 */ u8 pad0[0x40];
    /* +0x040 */ Ent6F0B8 entries[1];
} Obj6F0B8;

void TimBlockSrc__SetEntryShift(Obj6F0B8 *self, s32 index, s32 shift) {
    Ent6F0B8 *e = &self->entries[index];

    e->shift = shift;
    e->mask = 1 << e->shift;
}
```

## Notes

- No shared header was edited. `Class6D430.h`, `Class6B5CC.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TimBlockSrc__SetEntryShift**, tier B. Occupies slot +0x078: sets one CLUT-fade entry's shift and derives its mask.
