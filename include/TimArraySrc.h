#ifndef TIMARRAYSRC_H
#define TIMARRAYSRC_H

#include "FileResource.h"

/*
 * TimArraySrc -- a FileResource data source (class id 0xC03, method table
 * D_8006F1C4) whose buffer holds one block of TIM images -- a count, then
 * that many byte offsets from the block's start -- and which turns it into
 * an array of TimImage objects (include/TimImage.h). Methods in
 * src/code_33808.c. No classes derive from it (`typeviews.py --tree`), so
 * there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TimArraySrc__TimArraySrc's first call
 * is GetActiveDataSourceMethods()->ctor, and finalize forwards to the
 * active driver's, as TimBlockSrc, TimImage and TileAtlas do.
 *
 * The name is round 83's, and the evidence is BuildImages: one
 * New_TimImage(NULL) per offset, each adopting its TIM in place.
 *
 * How it is used, at the one New_TimArraySrc call site
 * (TimBlockSrc__AdvanceLoadState, include/TimBlockSrc.h): New_TimArraySrc(0)
 * per block, then the block's sector buffer as `buffer` (size 0, so the
 * TimArraySrc never owns it), `clutBase` = the address of the TimBlockSrc's
 * four CLUT fade ramps, then setFlag (+0x064, BuildImages) and +0x078
 * (UploadImages); the TimBlockSrc keeps it in `blocks` and releases it.
 *
 * SLOTS (`classtable.py D_8006F1C4 --vs gFileResourceMethods`, 30 against 30; the
 * words from +0x07C on are gDataSourceClientGetters, not this table):
 *  - +0x008 ctor, TimArraySrc__TimArraySrc(self, name): the active
 *    driver's ctor, this table, count/images/ready cleared, and
 *    requestLoadFile(name) when name is not NULL (the one caller passes 0);
 *  - +0x00C finalize, TimArraySrc__Finalize: ReleaseBasicClassArray the
 *    images, free the array, then the active driver's finalize;
 *  - +0x058 loadFile is NULL in this table;
 *  - +0x064 setFlag, TimArraySrc__BuildImages (named for what it does; the
 *    driver runs setFlag when a read completes, and TimBlockSrc calls it
 *    directly);
 *  - +0x078 is FileResource's `void *slot78` (NULL there); this table's
 *    occupant is TimArraySrc__UploadImages, called through
 *    TimArraySrcUploadFn (no code). No own slots past it.
 *
 * FIELDS: the object is 0x3C bytes (New_TimArraySrc).
 */

typedef struct TimArraySrc TimArraySrc;
typedef struct TimArraySrcMethods TimArraySrcMethods;
struct TimImage;

struct TimArraySrcMethods {
    FILERESOURCE_SLOTS(TimArraySrc, (TimArraySrc * self, char *name));
    /* +0x078 is FileResource's slot78; this table's occupant is
     * TimArraySrc__UploadImages (TimArraySrcUploadFn). */
}; /* 30 slots, 0x7C bytes */

struct TimArraySrc {
    FILERESOURCE_FIELDS(TimArraySrcMethods); /* buffer: the block (count, then offsets) */
    /* +0x02C */ s32 count;                /* images built: the block's first word */
    /* +0x030 */ struct TimImage **images; /* BMemPMgrAlloc(count * 4), one New_TimImage(NULL) each */
    /* +0x034 */ s32 clutBase; /* address of TimBlockSrc's entries[]; BuildImages adds 16 per CLUT row to it for each TimImage's clutBase */
    /* +0x038 */ s32 ready;    /* 0 from the ctor, 1 once BuildImages built the array */
}; /* 0x3C bytes: New_TimArraySrc */

/* TimArraySrc__UploadImages as TimBlockSrc__AdvanceLoadState calls it
 * through slot78. */
typedef void (*TimArraySrcUploadFn)(TimArraySrc *self);

extern TimArraySrcMethods D_8006F1C4;
extern TimArraySrcMethods *GetTimArraySrcMethods(void); /* returns &D_8006F1C4 */

TimArraySrc *New_TimArraySrc(char *name); /* BMemPMgrAlloc(0x3C), then ctor */
void TimArraySrc__TimArraySrc(TimArraySrc *self, char *name);
void TimArraySrc__Finalize(TimArraySrc *self);
void TimArraySrc__BuildImages(TimArraySrc *self);
void TimArraySrc__UploadImages(TimArraySrc *self);

#endif
