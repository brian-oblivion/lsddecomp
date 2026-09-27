# MoviePlayer__Stop -- MATCHED (33/33 words)

> Renamed from `func_800458B8` on 2026-09-25 (tools/rename.py). Address 0x800458b8.

Round 82, runner echo (code_33808 session, echo #8), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 33/33 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Only when self is the object held in the global gActiveMoviePlayer: zero +0x3C/+0x40/+0x44/+0x48, set +0x4C = 1, call the +0x60 object's slot +0x07C with (obj, MoviePlayer__MarkStopped, self) -- MoviePlayer__MarkStopped sets +0x50 = -1, so it is a completion callback --, clear +0x64, then call the +0x60 object's slot +0x058 with it alone. This is the slot +0x044 that MoviePlayer__PollActive calls every ~100 polls.

Table slot (`tools/classtable.py`): gMoviePlayerMethods +0x044.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/code_33808.c`.

```c
typedef struct Methods458B8 {
    /* +0x000 */ u8 pad0[0x48];
    /* +0x048 */ void (*slot48)();
    /* +0x04C */ u8 pad4C[0xC];
    /* +0x058 */ void (*slot58)();
    /* +0x05C */ u8 pad5C[0x20];
    /* +0x07C */ void (*slot7C)();
} Methods458B8;

typedef struct Sub458B8 {
    /* +0x000 */ Methods458B8 *methods;
} Sub458B8;

typedef struct Obj458B8 {
    /* +0x000 */ u8 pad0[0x3C];
    /* +0x03C */ s32 unk3C;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ s32 unk44;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
    /* +0x050 */ s32 unk50;
    /* +0x054 */ s32 unk54;
    /* +0x058 */ u8 pad58[8];
    /* +0x060 */ Sub458B8 *unk60;
    /* +0x064 */ s32 unk64;
} Obj458B8;

void MoviePlayer__Stop(Obj458B8 *self) {
    Obj458B8 *cur = (Obj458B8 *)gActiveMoviePlayer;

    if (cur == self) {
        cur->unk40 = 0;
        cur->unk3C = 0;
        cur->unk4C = 1;
        cur->unk48 = 0;
        cur->unk44 = 0;
        cur->unk60->methods->slot7C(cur->unk60, MoviePlayer__MarkStopped, cur);
        cur->unk64 = 0;
        cur->unk60->methods->slot58(cur->unk60);
    }
}
```

## Notes

First build. Written through a local copy of the global (`cur`) -- retail keeps the global in s0 and reads every field through it, not through self. Stores written in retail's order (0x40, 0x3C, 0x4C, 0x48, 0x44). The +0x60 object's class is not identified; a per-function methods view with unprototyped slots. (The views gained slot48/unk54 additively for MoviePlayer__Abort; this report's inlined copy is the current one.)

## Naming

- **MoviePlayer__Stop**, tier A. Slot +0x044: when this is the active movie, resets its state words and marks it stopped (callback cleared, MarkStopped).

## Track 4 (2026-09-26, round 87)

The +0x060 object is a CdStream (include/CdStream.h, unified this round). `Sub458B8`/`Methods458B8` are deleted; `unk60` is `CdStream *`; slot58 is `restart`, slot7C stays `slot7C` (empty occupant, typed from this call's arguments). MoviePlayer's own view and field names are unchanged. Zero bytes changed.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/code_33808.c` are gone; Obj458B8 is gone; `cur` is a `MoviePlayer *` straight from `gActiveMoviePlayer` (now declared `MoviePlayer *`, was the unit's DataSrc33808 view). `unk64` -> `started`. Byte-identical; `typeviews.py --warnings` 0 new.
