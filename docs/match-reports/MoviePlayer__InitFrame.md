# MoviePlayer__InitFrame -- MATCHED (68/68 words)

> Renamed from `func_8004564C` on 2026-09-25 (tools/rename.py). Address 0x8004564c.

Round 82, runner echo (code_33808 session, echo #9), 2026-09-25. Unit `code_33808`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 68/68 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Set up the MDEC player's buffers and frame: store `external` at +0x0C; unless external, zero +0x1C/+0x18/+0x14/+0x10, then allocate the two decode buffers (w * h * 2 + 0x1000 each) at +0x14/+0x18, the 0x12000 stream ring at +0x10 and the h * 32 strip buffer at +0x1C, any failure going to MoviePlayer__FreeFrameBuffers (free them) and return 1. Then copy the 12-byte frame descriptor {s16 x, y; s32 w, h} to +0x2C and from there to +0x20, set the strip width (+0x30) to 16 and +0x38 = (h << 4) >> 1 (the strip's size in words, the DecDCTout size MoviePlayer__DrawStrip uses). 0.

Table slot (`tools/classtable.py`): not in any method table (called by the gMoviePlayerMethods ctor MoviePlayer__MoviePlayer).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `Ctor33808`, `CountedBuf33808` and `Req44858` sit at the
top of / earlier in `src/code_33808.c`.

```c
/* Set up the MDEC player's frame: keep `external` at +0x0C and, unless the
 * caller provides the buffers, allocate the two decode buffers (w * h * 2 +
 * 0x1000 each), the 0x12000 ring and the h * 32 strip buffer -- on a failure
 * free what was allocated (MoviePlayer__FreeFrameBuffers) and return 1. Then the frame
 * descriptor goes to +0x2C and +0x20, the strip at +0x2C is 16 wide and
 * +0x38 is its size in words. 0. */
typedef struct Frame4564C {
    /* +0x00 */ s16 x;
    /* +0x02 */ s16 y;
    /* +0x04 */ s32 w;
    /* +0x08 */ s32 h;
} Frame4564C;

typedef struct Obj4564C {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 external;
    /* +0x010 */ void *ring;
    /* +0x014 */ void *frames[2];
    /* +0x01C */ void *strip;
    /* +0x020 */ Frame4564C frame;
    /* +0x02C */ Frame4564C cur;
    /* +0x038 */ s32 stripSize;
} Obj4564C;

s32 MoviePlayer__InitFrame(Obj4564C *self, Frame4564C *desc, s32 external) {
    s32 size;
    s32 unused[2];

    self->external = external;
    if (external == 0) {
        self->strip = NULL;
        self->frames[1] = NULL;
        self->frames[0] = NULL;
        self->ring = NULL;
        size = desc->w * desc->h * 2 + 0x1000;
        if ((self->frames[0] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->frames[1] = BMemPMgrAlloc(size)) == NULL) {
            goto fail;
        }
        if ((self->ring = BMemPMgrAlloc(0x12000)) == NULL) {
            goto fail;
        }
        if ((self->strip = BMemPMgrAlloc(desc->h << 5)) == NULL) {
            goto fail;
        }
    }
    self->cur = *desc;
    self->frame = self->cur;
    self->cur.w = 16;
    self->stripSize = (self->cur.h << 4) >> 1;
    return 0;
fail:
    MoviePlayer__FreeFrameBuffers(self);
    return 1;
}
```

## Notes

Second build. The first build was byte-identical except the frame (0x20 against retail's 0x28; 58/68 because every save offset moved). Retail touches no stack slot besides the four saves, so the extra 8 bytes are an unused local: `s32 unused[2];` matches (the -Wall baseline gains one unused-variable warning). The `>> 1` is literal: retail has `sll 4; sra 1` with no rounding fix-up, which `/ 2` would add. `goto fail` with the label after `return 0` gives retail's layout directly. Local views `Frame4564C`/`Obj4564C` just above.

## Naming

- **MoviePlayer__InitFrame**, tier A. Sets up the frame descriptor and, unless the caller supplies its own buffers, allocates the two decode buffers, the ring buffer and the strip buffer.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/code_33808.c` are gone; Obj4564C and Frame4564C are gone: Frame4564C was DrawSystem.h's DrawRect, `cur` is `stripRect` and `frame` stays `frame`. The `--merge` CONFLICT at +0x02C (DrawRect here, all-s16 Rect45BC8 in DrawStrip) is settled by this function's `sw` of w = 16 and the 12-byte whole-struct copy: DrawRect. Byte-identical; `typeviews.py --warnings` 0 new.
