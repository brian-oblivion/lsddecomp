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

- **MoviePlayer__NoOpSlot5C**, tier B. Empty occupant of MoviePlayer's own slot +0x05C, named like its siblings NoOpSlot50/54. (Was `MoviePlayer__NoOpFreeBuffer`, after FileResource's +0x05C freeBuffer; MoviePlayer is a direct BasicClass subclass, so that slot name never applied. Round 89.)

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/code_33808.c` are gone; renamed from `MoviePlayer__NoOpFreeBuffer` (see Naming). Byte-identical; `typeviews.py --warnings` 0 new.
