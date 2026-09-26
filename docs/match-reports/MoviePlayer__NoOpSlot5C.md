# MoviePlayer__NoOpSlot5C -- MATCHED (2/2 words)

> Renamed from `MoviePlayer__NoOpFreeBuffer` on 2026-09-26 (tools/rename.py). Address 0x80045bc0.

> Renamed from `func_80045BC0` on 2026-09-25 (tools/rename.py). Address 0x80045bc0.

Round 82, runner echo (code_33808 session), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 2/2, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Empty method (`jr ra; nop`).

Table slot (`tools/classtable.py`): `gMoviePlayerMethods` +0x05C.

## Source

```c
void MoviePlayer__NoOpSlot5C(void) {
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/code_33808.c`.

## Naming

- **MoviePlayer__NoOpSlot5C**, tier B. Empty freeBuffer-shaped override (matches the Class6D430 slot MoviePlayer otherwise repurposes); MoviePlayer needs no buffer freed at this slot.
