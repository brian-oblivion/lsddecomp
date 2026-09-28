# MoviePlayer__Finalize -- MATCHED (30/30 words)

> Renamed from `func_800455D4` on 2026-09-25 (tools/rename.py). Address 0x800455d4.

Round 82, runner echo (GraphicsResources session, echo #8), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 30/30 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

Finalize: releases the object at +0x60 through its own release slot (+0x004) and stores the result (NULL) back, DecDCToutCallback(NULL), DecDCTReset(0) (libpress, Sony), MoviePlayer__FreeFrameBuffers(self) (frees four buffers), then GetBasicClassMethods()->finalize(self).

Table slot (`tools/classtable.py`): gMoviePlayerMethods +0x00C.

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable` and `SubBlockTable` sit at the top of
`src/graphics/GraphicsResources.c`.

```c
typedef struct Obj455D4 {
    /* +0x000 */ u8 pad0[0x60];
    /* +0x060 */ BasicClass *unk60;
} Obj455D4;

void MoviePlayer__Finalize(Obj455D4 *self) {
    self->unk60 = self->unk60->methods->release(self->unk60);
    DecDCToutCallback(NULL);
    DecDCTReset(0);
    MoviePlayer__FreeFrameBuffers(self);
    GetBasicClassMethods()->finalize((BasicClass *)self);
}
```

## Notes

First build. DecDCT* declared locally with LIBPRESS.H's prototypes; MoviePlayer__FreeFrameBuffers forward-declared unprototyped (its definition later in the file takes the unit-local Obj4575C *).

## Naming

- **MoviePlayer__Finalize**, tier A. Releases the CD stream object, detaches and resets the MDEC decoder, frees the frame buffers, then BasicClass's finalize.

## Track 4 (2026-09-26, round 87)

The +0x060 object is a CdStream (include/CdStream.h, unified this round). `Obj455D4::unk60` was `BasicClass *`; it is the CdStream, now `CdStream *` (release is the inherited slot). MoviePlayer's own view and field names are unchanged. Zero bytes changed.

## Track 4 (2026-09-26, round 89)

Class unified in `include/MoviePlayer.h` (id 0x70, table `gMoviePlayerMethods`, was `D_8006F614`; a direct BasicClass subclass, 0x6C bytes). The unit-local views in `src/graphics/GraphicsResources.c` are gone; Obj455D4 is gone; `unk60` -> `stream`. Byte-identical; `typeviews.py --warnings` 0 new.
