# TimBlockSrc__FadeAllEntries -- MATCHED (29/29 words)

> Renamed from `func_8004355C` on 2026-09-25 (tools/rename.py). Address 0x8004355c.

Round 82, runner echo (code_33808 session, echo #7), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 29/29 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Under Lock/UnlockActiveDataSource, calls slot +0x080 as (self, i, arg) for i = 0..3.

Table slot (`tools/classtable.py`): D_8006F0B8 +0x07C.

## Source

The unit-local view `DataSrc33808` (a Class6D430 subclass built with the unified
`CLASS6D430_SLOTS`/`CLASS6D430_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.

```c
/* D_8006F0B8 +0x07C: slot +0x080 for entries 0..3, under the data-source
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
