# TimArraySrc__BuildImages -- MATCHED (81/81 words)

> Renamed from `func_80043CB8` on 2026-09-25 (tools/rename.py). Address 0x80043cb8.

Round 82, runner echo (GraphicsResources session, echo #9), 2026-09-25. Unit `GraphicsResources`.
Byte-exact; whole-image SHA1 green (`./build-and-verify.sh`:
`OK: build matches retail SLPS_015.56`), funcdiff 81/81 words, no out-of-range
drift. Fresh ground (carved revision 18, no prior report).

## What it does

When the buffer is present (or flags bit 0x200 is set): the buffer is a count followed by that many offsets. Store the count at +0x2C, allocate a count-word array at +0x30, and for each offset create a TimImage with New_TimImage(NULL) (no file), point its buffer at buffer+offset (size 0), ask it for its GsIMAGE (its +0x09C, TimImage__GetTimInfo) and set its +0x4C to ((cy - 0x1E0) >> sTimClutRowShift) * 16 + self->+0x34 (a CLUT slot address from the image's CLUT row). Then +0x38 = 1 and the active driver's setFlag.

Table slot (`tools/classtable.py`): gTimArraySrcMethods +0x064 (setFlag override).

## Source

The unit-local views `DataSrc33808` (FileResource subclass via the unified
`FILERESOURCE_SLOTS`/`FILERESOURCE_FIELDS` macros plus `slot7C`/`slot80` and own
fields +0x2C..+0x38), `UnprototypedCtorTable`, `SubBlockTable` and `ResourceSourceArgs` sit at the
top of / earlier in `src/graphics/GraphicsResources.c`.

```c
/* gTimArraySrcMethods +0x064: when the buffer is there (or flag 0x200 is set),
 * build one TimImage (New_TimImage(NULL)) per image of the buffer -- a
 * count, then that many offsets -- into an array at +0x30 (+0x2C entries),
 * each adopting its image in place (size 0), and set each one's +0x4C from
 * the CLUT row its GsGetTimInfo reports (from y 0x1E0, >> sTimClutRowShift, 16
 * bytes a step past +0x34); then mark +0x38 and the active driver's
 * setFlag. */
typedef struct Image43CB8 {      /* LIBGS.H GsIMAGE */
    /* +0x00 */ u32 pmode;
    /* +0x04 */ s16 px;
    /* +0x06 */ s16 py;
    /* +0x08 */ u16 pw;
    /* +0x0A */ u16 ph;
    /* +0x0C */ u32 *pixel;
    /* +0x10 */ s16 cx;
    /* +0x12 */ s16 cy;
    /* +0x14 */ u16 cw;
    /* +0x16 */ u16 ch;
    /* +0x18 */ u32 *clut;
} Image43CB8;

typedef struct Tim43CB8 Tim43CB8;

typedef struct TimMethods43CB8 {
    FILERESOURCE_SLOTS(Tim43CB8, (Tim43CB8 *self, char *name));
    /* +0x07C */ u8 pad7C[0x20];
    /* +0x09C */ void (*getTimInfo)(Tim43CB8 *self, Image43CB8 *info);
} TimMethods43CB8;

struct Tim43CB8 {                /* TimImage (code_2bb9c.c) */
    FILERESOURCE_FIELDS(TimMethods43CB8);
    /* +0x02C */ u8 pad2C[0x20];
    /* +0x04C */ s32 clutBase;
};

typedef struct Obj43CB8 {
    FILERESOURCE_FIELDS(DataSrc33808Methods);
    /* +0x02C */ s32 count;
    /* +0x030 */ Tim43CB8 **images;
    /* +0x034 */ s32 base;
    /* +0x038 */ s32 ready;
} Obj43CB8;

extern Tim43CB8 *New_TimImage(char *name);
extern s16 sTimClutRowShift;

void TimArraySrc__BuildImages(Obj43CB8 *self) {
    Image43CB8 info;
    Tim43CB8 **objs;
    s32 i;
    s32 *offs;

    if ((self->flags & 0x200) || self->buffer != NULL) {
        self->count = *(s32 *)self->buffer;
        self->images = BMemPMgrAlloc(*(s32 *)self->buffer * 4);
        if (self->images != NULL) {
            objs = self->images;
            offs = (s32 *)self->buffer + 1;
            for (i = 0; i < self->count; i++) {
                *objs = New_TimImage(NULL);
                (*objs)->buffer = (u8 *)self->buffer + *offs;
                (*objs)->bufferSize = 0;
                (*objs)->methods->getTimInfo(*objs, &info);
                (*objs)->clutBase = ((info.cy - 0x1E0) >> sTimClutRowShift) * 16 + self->base;
                offs++;
                objs++;
            }
            self->ready = 1;
            GetActiveDataSourceMethods()->setFlag((FileResource *)self);
        }
    }
}
```

## Notes

First build. The GsIMAGE local (0x1C bytes) is what gives retail's 0x50 frame. Local views: `Image43CB8` (LIBGS.H GsIMAGE, same layout as code_2bb9c.c's local GsIMAGE), `Tim43CB8`/`TimMethods43CB8` (TimImage as this function sees it: FileResource plus +0x09C getTimInfo and +0x04C), `Obj43CB8` (gTimArraySrcMethods's +0x2C..+0x38). New_TimImage is prototyped locally returning the local view, as TitleMenuTaskObjF.c does.

## Naming

- **TimArraySrc__BuildImages**, tier A. Slot +0x064: builds one TimImage (New_TimImage(NULL)) per image record in the buffer, each adopting its sub-buffer and computing its CLUT base from GsGetTimInfo-style fields.

## Track 4 (2026-09-26, round 88)

TimImage is unified (`include/TimImage.h`). This function's local views of
it -- `Tim43CB8`, `TimMethods43CB8` and `Image43CB8` (GsIMAGE) -- and its
local `extern` of New_TimImage are deleted; `objs` is `TimImage **`,
`info` a `GsIMAGE`, and `Obj43CB8::images` is `TimImage **`. `clutBase`
(+0x04C) is the name TimImage.h took from this view. Image byte-identical.


## Track 4 (2026-09-26, round 88, runner alpha)
Class unified as TimArraySrc (include/TimArraySrc.h); the unit-local Obj43CB8 view is deleted and its fields kept their names except base -> clutBase (+0x034): its one writer, TimBlockSrc__AdvanceLoadState, stores the address of its own four CLUT fade ramps (`entries`) there, and this body adds 16 bytes per CLUT row to it for each TimImage's clutBase. Kept s32, as TimImage's clutBase is. Byte-identical.

## Round 93 polish (charlie, track 7)

### Naming

| old | new | tier | evidence |
| --- | --- | --- | --- |
| `0x200` | `CD_FLAG_LOAD_FILE_DONE` | A | src/cd/cd_driver.c's name for the bit the driver sets when a loadFile request completes (VabStreamObj__AdvanceLoadState tests the same bit) |
| `*(s32 *)buffer`, `(s32 *)buffer + 1` | `TimArrayBuf` `count`, `offsets` | A | a count, then that many byte offsets of images in the buffer |
| `0x1E0` | `CLUT_FADE_Y` | A | as in TimBlockSrc__TimBlockSrc |
| `16` | `sizeof(TimBlockSrcEntry)` | A | clutBase points at a TimBlockSrc's `entries` (AdvanceLoadState sets it), 16 bytes a ramp |
| `* 4` | `* sizeof(*self->images)` | A | the TimImage pointer array |
