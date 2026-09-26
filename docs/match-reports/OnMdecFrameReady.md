# OnMdecFrameReady -- MATCHED (14/14 words)

> Renamed from `func_80045DE0` on 2026-09-25 (tools/rename.py). Address 0x80045de0.

Round 82, runner echo (code_33808 session, echo #6), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 14/14 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`if (gActiveMoviePlayer != NULL) gActiveMoviePlayer->methods->slot60(gActiveMoviePlayer);` -- gp-relative global (resolved `gp_rel`, --gp-symbols), slot +0x060 cast to an unprototyped pointer (the unified macro declares it `void (*)(void)`).

Table slot (`tools/classtable.py`): none (no method table lists it).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `CountedBuf33808` sit at the top of `src/code_33808.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
extern DataSrc33808 *gActiveMoviePlayer;

/* Slot +0x060 of the object in gActiveMoviePlayer, when there is one. */
void OnMdecFrameReady(void) {
    if (gActiveMoviePlayer != NULL) {
        ((void (*)())gActiveMoviePlayer->methods->slot60)(gActiveMoviePlayer);
    }
}
```

## Notes

- No shared header was edited. `FileResource.h`, `Class6B5CC.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **OnMdecFrameReady**, tier A. The DecDCToutCallback target: forwards to the active movie's own slot60 when there is one.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/code_33808.c` are gone; `gActiveMoviePlayer` is a `MoviePlayer *`, so the call is `gActiveMoviePlayer->methods->drawStrip(gActiveMoviePlayer)` with no function-pointer cast. Byte-identical; `typeviews.py --warnings` 0 new.
