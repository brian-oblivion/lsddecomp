# GetTileMapMethods -- MATCHED (4/4 words)

> Renamed from `GetGridIndexSrcMethods` on 2026-09-25 (tools/rename.py). Address 0x80044f20.

> Renamed from `func_80044F20` on 2026-09-25 (tools/rename.py). Address 0x80044f20.

Round 82, runner echo (code_33808 session, echo #5), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gTileMapMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gTileMapMethods[];`.

Table slot (`tools/classtable.py`): `D_8006D430` +0x088.

## Source

```c
extern s32 gTileMapMethods[];

void *GetTileMapMethods(void) {
    return gTileMapMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/code_33808.c`.

## Naming

- **GetTileMapMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/TileMap.h`. Returns `TileMapMethods *` and `&gTileMapMethods` (was `void *` over `extern s32 gTileMapMethods[]`). Byte-identical.
