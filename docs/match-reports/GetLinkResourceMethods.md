# GetLinkResourceMethods -- MATCHED (4/4 words)

> Renamed from `func_80043B78` on 2026-09-25 (tools/rename.py). Address 0x80043b78.

Round 82, runner echo (code_33808 session), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gLinkResourceMethods`, declared locally as `extern s32 gLinkResourceMethods[];`.

Table slot (`tools/classtable.py`): `D_8006D430` +0x094 (the Class6D430 `Get...Methods` getter list).

## Source

```c
extern s32 gLinkResourceMethods[];

void *GetLinkResourceMethods(void) {
    return gLinkResourceMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/code_33808.c`.

## Naming

- **GetLinkResourceMethods**, tier A. Table getter.
