# GetBgLayerMethods -- MATCHED (4/4 words)

> Renamed from `func_8004467C` on 2026-09-25 (tools/rename.py). Address 0x8004467c.

Round 82, runner echo (code_33808 session, echo #5), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `D_8006F2C4` (`lui/addiu; jr; nop`), declared locally as `extern s32 D_8006F2C4[];`.

Table slot (`tools/classtable.py`): none: no data word references it (reached some other way).

## Source

```c
extern s32 D_8006F2C4[];

void *GetBgLayerMethods(void) {
    return D_8006F2C4;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/code_33808.c`.

## Naming

- **GetBgLayerMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/BgLayer.h`: returns `BgLayerMethods *` (`&D_8006F2C4`, declared `extern BgLayerMethods D_8006F2C4;` there). The local `extern s32 D_8006F2C4[];` and `void *` prototype in code_33808.c are gone. Byte-identical.
