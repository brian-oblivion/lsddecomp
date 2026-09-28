# GetTimBlockSrcMethods -- MATCHED (4/4 words)

> Renamed from `func_80043830` on 2026-09-25 (tools/rename.py). Address 0x80043830.

Round 82, runner echo (graphics_resources session), 2026-09-25. Unit `graphics_resources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gTimBlockSrcMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gTimBlockSrcMethods[];`.

Table slot (`tools/classtable.py`): `gFileResourceMethods` +0x090 (the FileResource `Get...Methods` getter list).

## Source

```c
extern s32 gTimBlockSrcMethods[];

void *GetTimBlockSrcMethods(void) {
    return gTimBlockSrcMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/graphics/graphics_resources.c`.

## Naming

- **GetTimBlockSrcMethods**, tier A. Table getter -- pure `return &table` stub, tier A by the getter convention.

## Track 4 (2026-09-25, round 83, bravo)

Now `TimBlockSrcMethods *GetTimBlockSrcMethods(void) { return &gTimBlockSrcMethods; }` against `extern TimBlockSrcMethods gTimBlockSrcMethods;` in the header (was `void *` over `extern s32 gTimBlockSrcMethods[]`), byte-identical. The class (id 0xF03, table `gTimBlockSrcMethods`) is unified as `TimBlockSrc` in `include/tim_block_src.h`. Any source block above is the pre-unification spelling; the live body in `src/graphics/graphics_resources.c` takes the unified types, byte-identical (whole image green, 0 new `-Wall` warnings, nonmatching green).
