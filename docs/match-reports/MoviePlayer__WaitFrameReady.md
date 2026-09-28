# MoviePlayer__WaitFrameReady -- MATCHED (9/9 words)

> Renamed from `func_80045E18` on 2026-09-25 (tools/rename.py). Address 0x80045e18.

Round 82, runner echo (GraphicsResources session, echo #6), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 9/9 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

`lw 0x4C; bnez out; move v0,zero; L: beqz v0,L` = `while (self->unk4C == 0) {}` on a non-volatile field: GCC loads once and, when zero, spins on the constant forever. The owning class is unknown (no table lists it), so a minimal local view `Obj45E18` carries +0x4C.

Table slot (`tools/classtable.py`): none (no data word references it).

## Source

The unit-local view `DataSrc33808` (a FileResource subclass built with the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros, plus `slot7C`/`slot80`, and own
fields +0x2C..+0x38) and `SubBlockTable` sit at the top of `src/graphics/GraphicsResources.c`.
Slot +0x078 is `void *slot78` in the unified macro, so calls cast it.

```c
/* Hang until +0x4C is nonzero (it is read once). MoviePlayer__DecodeFrame's only call
 * passes its sActiveMoviePlayer object, so the parameter is that Obj45CFC view. */
void MoviePlayer__WaitFrameReady(Obj45CFC *self) {
    while (self->unk4C == 0) {
    }
}
```

## Declaration (round 82, alpha)

Was matched with its own view `Obj45E18` and no prototype, so its only
caller MoviePlayer__DecodeFrame (defined earlier in ROM order) declared it implicitly
as `int` and cc1 warned six times. The parameter is now MoviePlayer__DecodeFrame's own
`Obj45CFC` (whose +0x4C is the same `unk4C`), with a prototype above that
caller. Byte-identical.

## Notes

- No shared header was edited. `FileResource.h`, `SceneNode.h`, `BasicClass.h` are
  included; prototypes for other units' functions (GetActiveDataSourceMethods,
  ReleaseBasicClassArray, BMemPMgrFree) are local to the unit.
- Types of arguments and returns are readings of the registers used, not proven.

## Naming

- **MoviePlayer__WaitFrameReady**, tier A. Busy-waits until the frame-ready flag is set; called only from DecodeFrame with the active movie.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/graphics/GraphicsResources.c` are gone; the Obj45CFC parameter is `MoviePlayer *`; `unk4C` -> `frameDone`. Byte-identical; `typeviews.py --warnings` 0 new.

## Round 93 polish (charlie, track 7)

MATCHING line on the loop: `frameDone` is not volatile, so the body reads it once and then spins forever if it was 0 (`move v0, zero; beqz v0, .`), which is retail's code.
