# MoviePlayer__DecodeFrame -- MATCHED (57/57 words)

> Renamed from `func_80045CFC` on 2026-09-25 (tools/rename.py). Address 0x80045cfc.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 57/57 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Per-frame step of the MDEC movie player, active only when `self` is the object in gActiveMoviePlayer: when a finished frame is pending (+0x44) it tail-returns its own +0x064; otherwise, when a frame is running (+0x40) it waits for the last strip (MoviePlayer__WaitFrameReady spins on +0x4C), clears +0x4C, DrawSyncs when +0x34 is under 0x80, feeds the bitstream at +0x14[+0x3C] to DecDCTin (mode 2) and the first strip buffer (+0x1C, +0x38 words) to DecDCTout; then stores (own +0x058 returned 0) at +0x40 and returns 0. When not the active object it falls off the end (no return value set).

Table slot (`tools/classtable.py`): gMoviePlayerMethods +0x068.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable`, `SubBlockTable` and `ResourceSourceArgs` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* gMoviePlayerMethods +0x068: when this is the object in gActiveMoviePlayer -- with a
 * finished frame pending (+0x44) run its own +0x064 and return that;
 * otherwise, when a frame is going (+0x40), wait for its last strip (+0x4C),
 * clear the flag, DrawSync when +0x34 is under 0x80, and feed the next
 * frame's bitstream (+0x14[+0x3C]) to DecDCTin and the first strip to
 * DecDCTout; then +0x40 = its own +0x058 returned 0, and 0. */
typedef struct Methods45CFC {
    /* +0x000 */ u8 pad0[0x58];
    /* +0x058 */ s32 (*slot58)();
    /* +0x05C */ u8 pad5C[8];
    /* +0x064 */ s32 (*slot64)();
} Methods45CFC;

typedef struct Obj45CFC {
    /* +0x000 */ Methods45CFC *methods;
    /* +0x004 */ u8 pad4[0x10];
    /* +0x014 */ u32 *frames[2];
    /* +0x01C */ u32 *strip;
    /* +0x020 */ u8 pad20[0x14];
    /* +0x034 */ s32 unk34;
    /* +0x038 */ s32 stripSize;
    /* +0x03C */ s32 frameIndex;
    /* +0x040 */ s32 unk40;
    /* +0x044 */ s32 unk44;
    /* +0x048 */ u8 pad48[4];
    /* +0x04C */ s32 unk4C;
} Obj45CFC;

/* LIBPRESS.H */
extern void DecDCTin(u32 *buf, int mode);

s32 MoviePlayer__DecodeFrame(Obj45CFC *self) {
    Obj45CFC *cur = (Obj45CFC *)gActiveMoviePlayer;

    if (cur == self) {
        if (cur->unk44 == 0) {
            if (cur->unk40 != 0) {
                MoviePlayer__WaitFrameReady(cur);
                cur->unk4C = 0;
                if (cur->unk34 < 0x80) {
                    DrawSync(0);
                }
                DecDCTin(cur->frames[cur->frameIndex], 2);
                DecDCTout(cur->strip, cur->stripSize);
            }
            self->unk40 = self->methods->slot58(self) == 0;
            return 0;
        }
        return cur->methods->slot64(cur);
    }
}
```

## Notes

Second build. The first shape, `if (cur->unk44 != 0) return slot64(cur);` ahead of the rest, measured 1/57: the early return put the slot64 call inline, AND cc1 merged `cur` and `self` into one register (the compare's equivalence). Nesting the body under `if (cur->unk44 == 0) { ...; return 0; } return cur->methods->slot64(cur);` fixed both at once -- the global then stays in s0 and self in s1, exactly retail. The function has no return on the `cur != self` path (retail leaves v0 unset). Local views `Methods45CFC`/`Obj45CFC` are unit-local and declared just above it; DecDCTin is Sony's (LIBPRESS), extern only.

## Naming

- **MoviePlayer__DecodeFrame**, tier A. Slot +0x068: when a frame finished pending, runs PollActive's own result; otherwise waits for the last strip, decodes the next frame's bitstream and its first strip.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/code_33808.c` are gone; Obj45CFC/Methods45CFC are gone; +0x058 is `pullFrame`, +0x064 `pollActive`, `unk40`/`unk44`/`unk4C` are `haveFrame`/`finished`/`frameDone`, `unk34` is `stripRect.h`. Byte-identical; `typeviews.py --warnings` 0 new.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `0x80` | `MOVIE_SYNC_HEIGHT` | A | as in DrawStrip |
| DecDCTin prototype | <libpress.h> | A | local copy deleted |

MATCHING line at the closing brace: no return when another player is active, as retail.
