# MoviePlayer__SetAutoPlay -- MATCHED (2/2 words)

> Renamed from `MoviePlayer__SetResult` on 2026-09-26 (tools/rename.py). Address 0x80045e3c.

> Renamed from `func_80045E3C` on 2026-09-25 (tools/rename.py). Address 0x80045e3c.

Round 82, runner echo (code_33808 session), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 2/2, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

One-line setter: `sw a1, 0x68(a0)` in the jr delay slot = `self->unk68 = value;` (s32 field; the class of gMoviePlayerMethods has no header yet, so a minimal unit-local view `Obj6F614`).

Table slot (`tools/classtable.py`): `gMoviePlayerMethods` +0x06C.

## Source

```c
/* gMoviePlayerMethods +0x06C: stores its argument at +0x68. */
typedef struct Obj6F614 {
    u8 pad0[0x68];
    s32 unk68;
} Obj6F614;

void MoviePlayer__SetAutoPlay(Obj6F614 *self, s32 value) {
    self->unk68 = value;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/code_33808.c`.

## Naming

- **MoviePlayer__SetAutoPlay**, tier A. Slot +0x06C: stores its argument into the result field DecodeFrame reads.
