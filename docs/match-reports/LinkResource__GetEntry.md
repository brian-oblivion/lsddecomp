# LinkResource__GetEntry -- MATCHED (6/6 words)

> Renamed from `func_80043B58` on 2026-09-25 (tools/rename.py). Address 0x80043b58.

Round 82, runner echo (code_33808 session, echo #5), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 6/6 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Indexed getter: `lw v0,0x2C(a0); sll a1,2; addu; lw v0,0(a1)` = `return self->entries[index];`
with `entries` an `s32 *` at +0x2C. Class6D430 (UNIFIED, `include/Class6D430.h`) is 0x2C
bytes, so +0x2C is the D_8006F13C subclass's own first field; a unit-local minimal
view `Obj6F13C` (pad to +0x2C, then the pointer) carries it rather than touching the
shared header. Return type `s32` is a reading of one `lw`, not a proven type.

Table slot (`tools/classtable.py D_8006F13C`): `D_8006F13C` +0x080.

## Source

```c
/* D_8006F13C +0x080: returns entry `index` of the word array at +0x2C
 * (the first field past the 0x2C-byte Class6D430 base). */
typedef struct Obj6F13C {
    /* +0x000 */ u8 pad0[0x2C];
    /* +0x02C */ s32 *entries;
} Obj6F13C;

s32 LinkResource__GetEntry(Obj6F13C *self, s32 index) {
    return self->entries[index];
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/code_33808.c`.
