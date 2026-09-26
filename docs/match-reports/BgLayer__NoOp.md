# BgLayer__NoOp -- MATCHED (2/2 words)

> Renamed from `func_80044674` on 2026-09-25 (tools/rename.py). Address 0x80044674.

Round 82, runner echo (code_33808 session), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 2/2, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Empty method (`jr ra; nop`).

Table slot (`tools/classtable.py`): `gBgLayerMethods` +0x0BC.

## Source

```c
void BgLayer__NoOp(void) {
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/code_33808.c`.

## Naming

- **BgLayer__NoOp**, tier C. Empty body filling BgLayer's own extra slot +0x0BC; no call site found for it in this unit, mechanics-only name.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/BgLayer.h`: its own slot +0x0BC, named `slotBC` there (empty, no known caller), so the function keeps its name. Byte-identical.
