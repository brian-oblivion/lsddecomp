# TimBlockSrc__SetEntryShift -- MATCHED (9/9 words)

> Renamed from `func_80043538` on 2026-09-25 (tools/rename.py). Address 0x80043538.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 9/9 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`sll 4; addiu 0x40; addu` = `&self->entries[index]` (16-byte entries at +0x40); `sh a2` then `lhu` of the SAME halfword back = store through a u16 field then read it again (`e->mask = 1 << e->shift`), GCC 2.6.3 reloads rather than zero-extending the argument.

Table slot (`tools/classtable.py`): gTimBlockSrcMethods +0x078.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* gTimBlockSrcMethods +0x078: set entry `index`'s shift, and its mask from it. */
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

- No shared header was edited. `FileResource.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TimBlockSrc__SetEntryShift**, tier B. Occupies slot +0x078: sets one CLUT-fade entry's shift and derives its mask.

## Track 4 (2026-09-25, round 83, bravo)

Occupant of +0x078, which is FileResource's untyped `slot78`; not given a TimBlockSrc slot of its own because the slot belongs to the parent's layout. Writes `entries[index].shift`/`.mask`. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/TimBlockSrc.h`. Any source block above is the pre-unification spelling; the live body in `src/code_33808.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
