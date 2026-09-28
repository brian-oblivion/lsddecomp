#ifndef TIMIMAGE_H
#define TIMIMAGE_H

#include "FileResource.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/*
 * TimImage -- a FileResource data source (class id 0x103, method table
 * gTimImageMethods) whose buffer holds one TIM image. Methods in
 * src/graphics/TimImage.c. No classes derive from it (`typeviews.py --tree`), so
 * there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TimImage__TimImage's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize forwards to the active
 * driver's, as TimBlockSrc and LbdFile do.
 *
 * What its own methods do: getTimInfo (+0x09C, TimImage__GetTimInfo)
 * describes the TIM in `buffer` (past its id word) into a GsIMAGE with
 * Sony's GsGetTimInfo; TimImage__Upload, at +0x078, describes it into `tim`
 * and uploads the pixel block and, when pmode bit 3 says there is one, the
 * CLUT through the draw singleton's loadImage (include/DrawSystem.h).
 *
 * How it is used, at every New_TimImage call site: New_TimImage(path) with
 * a ".TIM" path (the ctor requests the file), then +0x078 (upload), then
 * usually freeBuffer (+0x05C) or release (+0x004) once the sprites made
 * from it hold what they need. A Sprite's `texture` is a TimImage: its
 * reset keeps &texture->tim (include/Sprite.h). TimArraySrc (gTimArraySrcMethods,
 * src/graphics/GraphicsResources.c) makes them with New_TimImage(NULL), points `buffer`
 * into its own block and sets `clutBase`.
 *
 * +0x078 is FileResource's `processBuffer` (NULL there); this table's occupant
 * is TimImage__Upload, called through TimImageUploadFn (no code).
 *
 * `tim` is <libgs.h>'s GsIMAGE.
 */

typedef struct TimImage TimImage;
typedef struct TimImageMethods TimImageMethods;

struct TimImageMethods {
    FILERESOURCE_SLOTS(TimImage, (TimImage * self, char *name));
    /* +0x078 is FileResource's processBuffer; this table's occupant is
     * TimImage__Upload (TimImageUploadFn). */
    /* +0x07C..+0x094: empty bodies (TimImage__NoOpSlot7C..TimImage__NoOpSlot94); no C
     * caller names them. */
    /* +0x07C */ void (*slot7C)(void);
    /* +0x080 */ void (*slot80)(void);
    /* +0x084 */ void (*slot84)(void);
    /* +0x088 */ void (*slot88)(void);
    /* +0x08C */ void (*slot8C)(void);
    /* +0x090 */ void (*slot90)(void);
    /* +0x094 */ void (*slot94)(void);
    /* +0x098 */ void (*setFlag)(TimImage *self);                  /* TimImage__SetFlag: flag = 1 */
    /* +0x09C */ void (*getTimInfo)(TimImage *self, GsIMAGE *tim); /* TimImage__GetTimInfo */
}; /* 39 slots, 0xA0 bytes */

struct TimImage {
    FILERESOURCE_FIELDS(TimImageMethods); /* buffer: the TIM file */
    /* +0x02C */ GsIMAGE tim; /* TimImage__Upload describes the TIM here; a Sprite's reset keeps its address */
    /* +0x048 */ s32 flag; /* 0 from the ctor, 1 from setFlag; nothing reads it */
    /* +0x04C */ s32 clutBase; /* 0 from the ctor; TimArraySrc__BuildImages: from the CLUT row GsGetTimInfo reports */
}; /* 0x50 bytes: New_TimImage */

typedef void (*TimImageUploadFn)(TimImage *self);

/* GsIMAGE.pmode is the TIM file's flag word: the low bits the pixel mode, bit 3
 * (CF) set when the file carries a CLUT (TimImage__Upload uploads it then). */
#define TIM_PMODE_CLUT_BIT 3

extern TimImageMethods gTimImageMethods;
extern TimImageMethods *GetTimImageMethods(void);

TimImage *New_TimImage(char *name);
void TimImage__TimImage(TimImage *self, char *name);
void TimImage__Finalize(TimImage *self);
void TimImage__Upload(TimImage *self);
void TimImage__NoOpSlot7C(void);
void TimImage__NoOpSlot80(void);
void TimImage__NoOpSlot84(void);
void TimImage__NoOpSlot88(void);
void TimImage__NoOpSlot8C(void);
void TimImage__NoOpSlot90(void);
void TimImage__NoOpSlot94(void);
void TimImage__SetFlag(TimImage *self);
void TimImage__GetTimInfo(TimImage *self, GsIMAGE *tim);

/* An s16 VRAM point. */
typedef struct DrawPoint {
    /* +0x00 */ s16 x;
    /* +0x02 */ s16 y;
} DrawPoint;

/* Rotates the VRAM rectangle `area` one column to the right, count times,
 * through the one-column scratch area at `scratch`. Not a TimImage method. */
struct DrawRect; /* include/DrawSystem.h */
void RotateVramRectRight(struct DrawRect *area, s32 count, DrawPoint *scratch);

#endif
