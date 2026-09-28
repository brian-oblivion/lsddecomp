# MoviePlayer__MoviePlayer -- MATCHED (68/68 words)

> Renamed from `func_800454C4` on 2026-09-25 (tools/rename.py). Address 0x800454c4.

Round 82, runner echo (GraphicsResources session, echo #9), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 68/68 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Constructor of the MDEC movie player: BasicClass's ctor, install gMoviePlayerMethods, open a CD stream object with New_CdStream(arg2, 15, 0) into +0x60, then MoviePlayer__InitFrame(self, arg1, arg3) sets up the decode buffers. Returns 1 if either fails. Otherwise DecDCTReset(0) the first time any player is built (gMdecInitialized latch), set the DecDCTout callback to OnMdecFrameReady, hand the stream the ring buffer at +0x10 with size 0x12000 (its +0x040), clear +0x50, call its own +0x06C with 1 (MoviePlayer__SetAutoPlay stores it at +0x68) and return 0.

Table slot (`tools/classtable.py`): gMoviePlayerMethods +0x008 (its allocator New_MoviePlayer treats 0 as success).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable`, `SubBlockTable` and `ResourceSourceArgs` sit at the
top of / earlier in `src/graphics/GraphicsResources.c`.

```c
/* gMoviePlayerMethods +0x008: constructor -- BasicClass's, then this table; open a
 * CD stream object (New_CdStream(arg2, 15, 0)) at +0x60 and set up the
 * decode buffers (MoviePlayer__InitFrame); 1 when either fails. Then reset the MDEC
 * the first time any player is built (gMdecInitialized), route its output
 * callback to OnMdecFrameReady, hand the stream the ring buffer at +0x10
 * (0x12000), clear +0x50 and store 1 through its own +0x06C. 0. */
typedef struct StreamMethods454C4 {
    /* +0x000 */ u8 pad0[0x40];
    /* +0x040 */ void (*slot40)();
} StreamMethods454C4;

typedef struct Stream454C4 {
    /* +0x000 */ StreamMethods454C4 *methods;
} Stream454C4;

typedef struct Methods454C4 {
    /* +0x000 */ u8 pad0[0x6C];
    /* +0x06C */ void (*slot6C)();
} Methods454C4;

typedef struct Obj454C4 {
    /* +0x000 */ Methods454C4 *methods;
    /* +0x004 */ u8 pad4[0xC];
    /* +0x010 */ void *ring;
    /* +0x014 */ u8 pad14[0x3C];
    /* +0x050 */ s32 unk50;
    /* +0x054 */ u8 pad54[0xC];
    /* +0x060 */ Stream454C4 *stream;
} Obj454C4;

extern void *New_CdStream(s32 arg1, s32 arg2, s32 arg3);
s32 MoviePlayer__InitFrame();
extern s32 gMdecInitialized;
extern void DecDCTReset(int mode);
extern int DecDCToutCallback(void (*func)());
void OnMdecFrameReady(void);

s32 MoviePlayer__MoviePlayer(Obj454C4 *self, s32 arg1, s32 arg2, s32 arg3) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = GetMoviePlayerMethods();
    self->stream = New_CdStream(arg2, 15, 0);
    if (self->stream != NULL) {
        if (MoviePlayer__InitFrame(self, arg1, arg3) == 0) {
            if (gMdecInitialized == 0) {
                DecDCTReset(0);
            }
            gMdecInitialized = 1;
            DecDCToutCallback(OnMdecFrameReady);
            self->stream->methods->slot40(self->stream, self->ring, 0x12000);
            self->unk50 = 0;
            self->methods->slot6C(self, 1);
            return 0;
        }
    }
    return 1;
}
```

## Notes

First build, written straight away in the nested success-path shape (`if (stream != NULL) { if (MoviePlayer__InitFrame(...) == 0) { ...; return 0; } } return 1;`) that this session's MoviePlayer__Play/MoviePlayer__PullFrame established: retail's two failure exits share one `return 1` block, whose `li v0,1` reorg stole into the second branch's delay slot. Local views `Obj454C4`/`Methods454C4`/`Stream454C4` just above; New_CdStream (CdStream.c) is prototyped locally with void * return. DecDCTReset/DecDCToutCallback are Sony's (LIBPRESS), extern only.

## Naming

- **MoviePlayer__MoviePlayer**, tier A. Constructor: BasicClass's ctor, opens a CD stream object, sets up decode buffers, resets the MDEC decoder the first time any player is built, and routes the MDEC output callback to OnMdecFrameReady.

## Track 4 (2026-09-26, round 87)

The +0x060 object is a CdStream (include/CdStream.h, unified this round). `Stream454C4`/`StreamMethods454C4` and the local New_CdStream extern are deleted; +0x060 `stream` is `CdStream *` and slot40 is `setRing`. MoviePlayer's own view and field names are unchanged. Zero bytes changed.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/graphics/GraphicsResources.c` are gone; it takes `(MoviePlayer *self, DrawRect *frame, s32 speed, s32 external)` (BASICCLASS_SLOTS_R with an `s32` return: 0 success, 1 failure). `unk60` -> `stream`, `ring`, `unk50`; the +0x06C call is `setAutoPlay(self, 1)`. Byte-identical; `typeviews.py --warnings` 0 new.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `speed` | `cdSpeed` | A | as New_MoviePlayer's |
| `15` | `MOVIE_FPS` | A | New_CdStream's fps parameter (include/CdStream.h) |
| `0x12000` | `MOVIE_RING_SIZE` (`36 * CD_SECTOR_SIZE`) | A | setRing hands StSetRing size / 2048 sectors; 0x12000 is 36 of them |
| `unk50` | `pendingStart` (include/MoviePlayer.h) | B | cleared here; MarkPlaying 1, MarkStopped -1; Advance starts the stream read while it is nonzero, counting loops down when negative, then clears it |
| DecDCTReset/DecDCToutCallback prototypes | <libpress.h> | A | local copies deleted |
