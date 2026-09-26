# MoviePlayer__MarkStopped -- MATCHED (3/3 words)

> Renamed from `func_8004593C` on 2026-09-25 (tools/rename.py). Address 0x8004593c.

Round 82, runner echo (code_33808 session), 2026-09-25. Unit `code_33808`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 3/3, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Setter: `li v0,-1; jr ra; sw v0,0x50(a0)` = `self->unk50 = -1;`, same view as MoviePlayer__MarkPlaying.

Table slot (`tools/classtable.py`): none (in no method table; called directly).

## Source

```c
void MoviePlayer__MarkStopped(Obj33808_50 *self) {
    self->unk50 = -1;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/code_33808.c`.

## Naming

- **MoviePlayer__MarkStopped**, tier A. Sets the tri-state play flag to -1; called from Stop.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/code_33808.c` are gone; Obj33808_50 is gone (see MarkPlaying). Stop passes this function and the player to CdStream's slot7C. Byte-identical; `typeviews.py --warnings` 0 new.
