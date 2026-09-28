# GetMoviePlayerMethods -- MATCHED (4/4 words)

> Renamed from `func_80045E44` on 2026-09-25 (tools/rename.py). Address 0x80045e44.

Round 82, runner echo (GraphicsResources session, echo #5), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 4/4 words, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Table getter: returns the method table `gMoviePlayerMethods` (`lui/addiu; jr; nop`), declared locally as `extern s32 gMoviePlayerMethods[];`.

Table slot (`tools/classtable.py`): none: no data word references it (reached some other way).

## Source

```c
extern s32 gMoviePlayerMethods[];

void *GetMoviePlayerMethods(void) {
    return gMoviePlayerMethods;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  sit directly above the function in `src/graphics/GraphicsResources.c`.

## Naming

- **GetMoviePlayerMethods**, tier A. Table getter.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/graphics/GraphicsResources.c` are gone; it returns `MoviePlayerMethods *` (`&gMoviePlayerMethods`; the local `extern s32 gMoviePlayerMethods[]` is gone). Byte-identical; `typeviews.py --warnings` 0 new.
