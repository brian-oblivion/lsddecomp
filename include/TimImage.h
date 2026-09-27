#ifndef TIMIMAGE_H
#define TIMIMAGE_H

#include "FileResource.h"

/*
 * TimImage -- a FileResource data source (class id 0x103, method table
 * gTimImageMethods) whose buffer holds one TIM image. Methods in
 * src/code_2bb9c.c. No classes derive from it (`typeviews.py --tree`), so
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
 * src/GraphicsResources.c) makes them with New_TimImage(NULL), points `buffer`
 * into its own block and sets `clutBase`.
 *
 * +0x078 is FileResource's `void *slot78` (NULL there); this table's occupant
 * is TimImage__Upload, called through TimImageUploadFn (no code; FINISHING-
 * PLAN track 4 step 6).
 *
 * `tim` is <libgs.h>'s GsIMAGE, so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */

typedef struct TimImage TimImage;
typedef struct TimImageMethods TimImageMethods;

struct TimImageMethods {
    FILERESOURCE_SLOTS(TimImage, (TimImage * self, char *name));
    /* +0x078 is FileResource's slot78; this table's occupant is
     * TimImage__Upload (TimImageUploadFn). */
    /* +0x07C..+0x094: empty bodies (TimImage__NoOpSlot7C..5DC); no C
     * caller names them. */
    /* +0x07C */ void (*slot7C)(void);
    /* +0x080 */ void (*slot80)(void);
    /* +0x084 */ void (*slot84)(void);
    /* +0x088 */ void (*slot88)(void);
    /* +0x08C */ void (*slot8C)(void);
    /* +0x090 */ void (*slot90)(void);
    /* +0x094 */ void (*slot94)(void);
    /* +0x098 */ void (*slot98)(TimImage *self); /* TimImage__func_8003B5E4: unk48 = 1 */
    /* +0x09C */ void (*getTimInfo)(TimImage *self, GsIMAGE *tim); /* TimImage__GetTimInfo */
}; /* 39 slots, 0xA0 bytes */

struct TimImage {
    FILERESOURCE_FIELDS(TimImageMethods); /* buffer: the TIM file */
    /* +0x02C */ GsIMAGE tim; /* TimImage__Upload describes the TIM here; a Sprite's reset keeps its address */
    /* +0x048 */ s32 unk48; /* 0 from the ctor, 1 from slot98; no reader found */
    /* +0x04C */ s32 clutBase; /* 0 from the ctor; TimArraySrc__BuildImages: from the CLUT row GsGetTimInfo reports */
}; /* 0x50 bytes: New_TimImage */

typedef void (*TimImageUploadFn)(TimImage *self);

extern TimImageMethods gTimImageMethods;
extern TimImageMethods *GetTimImageMethods(void);

TimImage *New_TimImage(char *name);
void TimImage__TimImage(TimImage *self, char *name);
void TimImage__Finalize(TimImage *self);
void TimImage__Upload(TimImage *self);
void TimImage__NoOpSlot7C(void);
void TimImage__NoOpSlot80(void);
void TimImage__NoOpSlot84(void);
void TimImage__func_8003B5C4(void);
void TimImage__func_8003B5CC(void);
void TimImage__func_8003B5D4(void);
void TimImage__func_8003B5DC(void);
void TimImage__func_8003B5E4(TimImage *self);
void TimImage__GetTimInfo(TimImage *self, GsIMAGE *tim);

#endif
