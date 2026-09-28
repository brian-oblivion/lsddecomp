# MoviePlayer__MarkPlaying -- MATCHED (3/3 words)

> Renamed from `func_800458AC` on 2026-09-25 (tools/rename.py). Address 0x800458ac.

Round 82, runner echo (GraphicsResources session), 2026-09-25. Unit `GraphicsResources`.
Byte-exact on the FIRST build; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 3/3, 0 insertions / 0
deletions, no out-of-range drift. Fresh ground (carved revision 18, no prior report).

## What it does

Setter: `li v0,1; jr ra; sw v0,0x50(a0)` = `self->unk50 = 1;` through a minimal unit-local view.

Table slot (`tools/classtable.py`): none (in no method table; called directly).

## Source

```c
/* A class with an s32 at +0x50, set to 1 / -1 by the two setters below;
 * the class is not yet identified (neither setter sits in a method table). */
typedef struct Obj33808_50 {
    u8 pad0[0x50];
    s32 unk50;
} Obj33808_50;

void MoviePlayer__MarkPlaying(Obj33808_50 *self) {
    self->unk50 = 1;
}
```

## Notes

- Only `common.h` is included; no shared header was edited. Local declarations
  (the `extern s32 D_...[]` table symbol or the unit-local struct view) sit
  directly above the function in `src/graphics/GraphicsResources.c`.

## Naming

- **MoviePlayer__MarkPlaying**, tier A. Sets the tri-state play flag (+0x50 in the movie-player's own struct) to 1; called from Play.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/graphics/GraphicsResources.c` are gone; Obj33808_50 is gone; its "class not yet identified" was MoviePlayer (Play is the one caller). `unk50` kept (1 here, -1 in MarkStopped; Advance reads it). Byte-identical; `typeviews.py --warnings` 0 new.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `unk50` | `pendingStart` | B | see MoviePlayer__MoviePlayer |
