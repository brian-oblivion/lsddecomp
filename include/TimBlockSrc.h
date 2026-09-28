#ifndef TIMBLOCKSRC_H
#define TIMBLOCKSRC_H

#include "file_resource.h"
#include "draw_system.h"

/*
 * TimBlockSrc -- a FileResource data source (class id 0xF03, method table
 * gTimBlockSrcMethods) that loads a file of TIM blocks one sector-buffer at a time
 * and fades up to four 256-colour CLUT rows. Methods in src/graphics/graphics_resources.c.
 *
 * Loading (onRequestDone, +0x064, is TimBlockSrc__AdvanceLoadState: the driver
 * runs it when a read completes). The ctor reads the file's first sector;
 * its first 0x24 bytes are a header -- a block count, the blocks' file
 * offsets from +0x04, their sizes from +0x14 (FindMaxTimBlockSize) -- copied
 * into `buffer`. Each block is then read into `sector` and handed to a new
 * TimArraySrc (gTimArraySrcMethods, include/TimArraySrc.h), whose clutBase is
 * `entries`, into `blocks`.
 *
 * Fading. `entries[i]` is CLUT row i's fade ramp: `mask` (1 << shift) rows
 * from VRAM y 0x1E0 + i * mask, the first the CLUT itself and the rest
 * FadeClutRow's steps toward `color`.
 *
 * NO FIELDS/SLOTS MACROS, although `typeviews.py --tree` puts four classes
 * below 0xF03 (Tod 0x4F03, TodSet 0x14F03, ModelData 0x5F03, TriggerWorld
 * 0x15F03). Their objects are 0x2C, 0x2C, 0x38 and 0x3C bytes (New_Tod,
 * New_TodSet, New_ModelData, New_TriggerWorld) against this class's 0x84
 * (New_TimBlockSrc), their constructors chain to the active driver's, not
 * to TimBlockSrc__TimBlockSrc, and their +0x07C/+0x080 occupants have other
 * signatures (ScanTodPackets returns a u8 from four arguments where
 * TimBlockSrc__FadeAllEntries takes two and returns nothing). None of them
 * carries a byte of this class's layout, so they expand FILERESOURCE's macros
 * directly.
 */

/* TimBlockSrc__AdvanceLoadState's steps, in FileResource's loadState. */
enum TimBlockLoadState {
    TIMBLOCK_LOAD_IDLE = 0,   /* done, or not started */
    TIMBLOCK_LOAD_HEADER = 9, /* the ctor's header-sector read is pending */
    TIMBLOCK_LOAD_BLOCK = 10  /* a block read into `sector` is pending */
};

typedef struct TimBlockSrc TimBlockSrc;
typedef struct TimBlockSrcMethods TimBlockSrcMethods;

/* One CLUT row's fade ramp. 0x10 bytes: TimArraySrc__BuildImages steps a
 * TimImage's clutBase through them 16 bytes a CLUT row. */
typedef struct TimBlockSrcEntry {
    /* +0x00 */ u16 shift; /* TimBlockSrc__SetEntryShift */
    /* +0x02 */ u16 mask;  /* 1 << shift: the ramp's rows */
    /* +0x04 */ u16 clutX; /* +0x04..+0x0A a RECT: the ctor lays out 0, 0x1E0 + i * mask, 0x100, 1 */
    /* +0x06 */ u16 clutY;
    /* +0x08 */ u16 clutW;
    /* +0x0A */ u16 clutH;      /* set to mask by FadeClutRow */
    /* +0x0C */ ColorRgb color; /* TimBlockSrc__FadeEntry */
    /* +0x0F */ u8 padF;
} TimBlockSrcEntry;

struct TimBlockSrcMethods {
    FILERESOURCE_SLOTS(TimBlockSrc, (TimBlockSrc * self, char *name));
    /* +0x078 is FileResource's processBuffer; this table's occupant is
     * TimBlockSrc__SetEntryShift(self, index, shift). */
    /* +0x07C */ void (*fadeAllEntries)(TimBlockSrc *self, ColorRgb *color); /* TimBlockSrc__FadeAllEntries */
    /* +0x080 */ void (*fadeEntry)(TimBlockSrc *self, s32 index, ColorRgb *color); /* TimBlockSrc__FadeEntry */
};

struct TimBlockSrc {
    FILERESOURCE_FIELDS(TimBlockSrcMethods); /* loadState: TIMBLOCK_LOAD_* */
    /* +0x02C */ s32 blockCount;             /* TimArraySrcs built so far */
    /* +0x030 */ struct TimArraySrc **blocks; /* one per block; ReleaseBasicClassArray'd by Finalize */
    /* +0x034 */ void *sector; /* the read buffer: 0x800 for the header, then the largest block size */
    /* +0x038 */ s32 sectorSize;
    /* +0x03C */ s32 loaded; /* set after the last block */
    /* +0x040 */ TimBlockSrcEntry entries[4];
    /* +0x080 */ s32 failed; /* an allocation failed */
}; /* 0x84 bytes: New_TimBlockSrc */

extern TimBlockSrcMethods gTimBlockSrcMethods;
extern TimBlockSrcMethods *GetTimBlockSrcMethods(void);

TimBlockSrc *New_TimBlockSrc(char *name); /* name: the path the ctor opens */
void TimBlockSrc__TimBlockSrc(TimBlockSrc *self, char *name);
void TimBlockSrc__Finalize(TimBlockSrc *self);
void TimBlockSrc__AdvanceLoadState(TimBlockSrc *self);
void TimBlockSrc__SetEntryShift(TimBlockSrc *self, s32 index, s32 shift);
void TimBlockSrc__FadeAllEntries(TimBlockSrc *self, ColorRgb *color);
void TimBlockSrc__FadeEntry(TimBlockSrc *self, s32 index, ColorRgb *color);
void FadeClutRow(TimBlockSrcEntry *entry, s32 index);

#endif
