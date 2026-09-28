# TimBlockSrc__FadeAllEntries -- MATCHED (29/29 words)

> Renamed from `func_8004355C` on 2026-09-25 (tools/rename.py). Address 0x8004355c.

Round 82, runner echo (graphics_resources session, echo #7), 2026-09-25. Unit `graphics_resources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 29/29 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Under Lock/UnlockActiveDataSource, calls slot +0x080 as (self, i, arg) for i = 0..3.

Table slot (`tools/classtable.py`): gTimBlockSrcMethods +0x07C.

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/graphics_resources.c`.

```c
/* gTimBlockSrcMethods +0x07C: slot +0x080 for entries 0..3, under the data-source
 * lock. */
extern void LockActiveDataSource(void);
extern void UnlockActiveDataSource(void);

void TimBlockSrc__FadeAllEntries(DataSrc33808 *self, s32 arg) {
    s32 i;

    LockActiveDataSource();
    for (i = 0; i < 4; i++) {
        self->methods->slot80(self, i, arg);
    }
    UnlockActiveDataSource();
}
```

## Notes

- Byte-exact on the first build.
- No shared header was edited; prototypes for other units' functions are local
  to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **TimBlockSrc__FadeAllEntries**, tier B. Slot +0x07C: loops the 4 CLUT-fade entries under the data-source lock, calling slot80 (TimBlockSrc__FadeEntry) on each.

## Track 4 (2026-09-25, round 83, bravo)

Occupant of +0x07C, now the `fadeAllEntries` slot. Its second argument is retyped `TimBlockSrcColor *` (was `s32`): it is passed straight on as `fadeEntry`'s colour; the slot call is now prototyped (`self->methods->fadeEntry(self, i, color)`, was the unprototyped `slot80`), byte-identical. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/tim_block_src.h`. Any source block above is the pre-unification spelling; the live body in `src/graphics/graphics_resources.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `4` | `ARRAY_COUNT(self->entries)` | A | one fadeEntry per ramp |
