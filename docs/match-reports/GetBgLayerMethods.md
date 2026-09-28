# GetBgLayerMethods -- MATCHED (4/4 words)

> Renamed from `func_8004467C` on 2026-09-25 (tools/rename.py). Address 0x8004467c.

Round 82, runner echo (GraphicsResources session, echo #5), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gBgLayerMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gBgLayerMethods[];`.

Table slot (`tools/classtable.py`): none: no data word references it (reached some other way).

## Source

```c
extern s32 gBgLayerMethods[];

void *GetBgLayerMethods(void) {
    return gBgLayerMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/graphics/GraphicsResources.c`.

## Naming

- **GetBgLayerMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 88, alpha)

Class unified in `include/BgLayer.h`: returns `BgLayerMethods *` (`&gBgLayerMethods`, declared `extern BgLayerMethods gBgLayerMethods;` there). The local `extern s32 gBgLayerMethods[];` and `void *` prototype in GraphicsResources.c are gone. Byte-identical.
