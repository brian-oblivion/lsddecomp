/**
 * @file tim_block_src.h
 * @brief TimBlockSrc, the FileResource that loads a file of TIM blocks and
 *        fades four CLUT rows, and its method table.
 */
#ifndef TIM_BLOCK_SRC_H
#define TIM_BLOCK_SRC_H

#include "file_resource.h"
#include "draw_system.h"

/** @brief TimBlockSrc__AdvanceLoadState's steps, kept in FileResource's
 *         loadState. */
enum TimBlockLoadState {
    TIMBLOCK_LOAD_IDLE = 0,   /**< done, or not started */
    TIMBLOCK_LOAD_HEADER = 9, /**< the ctor's header-sector read is pending */
    TIMBLOCK_LOAD_BLOCK = 10  /**< a block read into `sector` is pending */
};

typedef struct TimBlockSrc TimBlockSrc;
typedef struct TimBlockSrcMethods TimBlockSrcMethods;

/**
 * @brief One CLUT row's fade ramp: the CLUT row itself, then `mask - 1` rows
 *        below it stepping toward `color`.
 *
 * 0x10 bytes: TimArraySrc__BuildImages gives each TimImage the address of the
 * ramp its CLUT row falls in.
 */
typedef struct TimBlockSrcEntry {
    /* +0x00 */ u16 shift;      /**< log2 of the ramp's row count (TimBlockSrc__SetEntryShift) */
    /* +0x02 */ u16 mask;       /**< 1 << shift: the ramp's rows */
    /* +0x04 */ u16 clutX;      /**< the ramp's VRAM RECT, x: 0 */
    /* +0x06 */ u16 clutY;      /**< y: 480 + the rows of the ramps before it */
    /* +0x08 */ u16 clutW;      /**< w: 256 colours */
    /* +0x0A */ u16 clutH;      /**< h: 1 from the ctor, `mask` once FadeClutRow has built it */
    /* +0x0C */ ColorRgb color; /**< the colour the ramp fades toward (TimBlockSrc__FadeEntry) */
    /* +0x0F */ u8 padF;
} TimBlockSrcEntry;

/**
 * @brief TimBlockSrc's method table, gTimBlockSrcMethods: FileResource's
 *        slots, then two of its own.
 *
 * It overrides +0x008 ctor (TimBlockSrc__TimBlockSrc), +0x00C finalize
 * (TimBlockSrc__Finalize) and +0x064 onRequestDone
 * (TimBlockSrc__AdvanceLoadState, run when a read completes). The inherited
 * +0x078 processBuffer holds TimBlockSrc__SetEntryShift.
 */
struct TimBlockSrcMethods {
    FILERESOURCE_SLOTS(TimBlockSrc, (TimBlockSrc * self, char *name));
    /* +0x07C */ void (*fadeAllEntries)(TimBlockSrc *self, ColorRgb *color); /**< @see TimBlockSrc__FadeAllEntries */
    /* +0x080 */ void (*fadeEntry)(TimBlockSrc *self, s32 index, ColorRgb *color); /**< @see TimBlockSrc__FadeEntry */
};

/**
 * @brief A file of TIM blocks (class id 0xF03), read one block at a time into
 *        TimArraySrc objects, plus four 256-colour CLUT fade ramps.
 *
 * The ctor reads the file's first sector, whose first 0x24 bytes are a
 * header: a block count, the blocks' file offsets, their sizes. Each block is
 * then read into `sector` and handed to a new TimArraySrc, whose images take
 * their CLUTs from `entries`. The fade ramps start at VRAM y 480; ramp i's
 * first row is CLUT row i, and FadeClutRow fills the rest toward a colour.
 *
 * Parent FileResource, through the active data-source driver. The classes
 * whose ids sit under 0xF03 (Tod, TodSet, ModelData, TriggerWorld) are not
 * its subclasses: their ctors chain to the driver's, not to this class's,
 * and none carries its layout. Methods in src/graphics/graphics_resources.c.
 * The object is 0x84 bytes (New_TimBlockSrc).
 */
struct TimBlockSrc {
    FILERESOURCE_FIELDS(TimBlockSrcMethods);  /**< loadState: TIMBLOCK_LOAD_* */
    /* +0x02C */ s32 blockCount;              /**< TimArraySrcs built so far */
    /* +0x030 */ struct TimArraySrc **blocks; /**< one per block; released by Finalize */
    /* +0x034 */ void *sector; /**< the read buffer: a sector for the header, then the largest block's size */
    /* +0x038 */ s32 sectorSize;              /**< `sector`'s size once it holds blocks */
    /* +0x03C */ s32 loaded;                  /**< set after the last block */
    /* +0x040 */ TimBlockSrcEntry entries[4]; /**< the fade ramps, one per CLUT row */
    /* +0x080 */ s32 failed;                  /**< an allocation failed while loading */
};

/** TimBlockSrc's method table. */
extern TimBlockSrcMethods gTimBlockSrcMethods;

/**
 * @brief Returns TimBlockSrc's method table.
 * @return &gTimBlockSrcMethods.
 */
extern TimBlockSrcMethods *GetTimBlockSrcMethods(void);

/**
 * @brief Allocates a TimBlockSrc from the pool and constructs it, starting
 *        the load of `name`.
 * @param name The path of the TIM-block file.
 * @return The new object, or NULL when the pool is exhausted.
 */
TimBlockSrc *New_TimBlockSrc(char *name);

/**
 * @brief Constructor (slot +0x008): lays out the four fade ramps
 *        (2^sTimBlockClutShift rows each, one after another from VRAM y 480),
 *        then opens `name` and reads its first sector.
 *
 * Nothing is read when the header or sector buffer cannot be allocated.
 * @param self The object to construct.
 * @param name The path of the TIM-block file.
 */
void TimBlockSrc__TimBlockSrc(TimBlockSrc *self, char *name);

/**
 * @brief Finalizer (slot +0x00C): releases the TimArraySrcs built so far,
 *        then the active driver's finalizer.
 * @param self The object being destroyed.
 */
void TimBlockSrc__Finalize(TimBlockSrc *self);

/**
 * @brief Slot +0x064 (onRequestDone), run when a read completes: steps the
 *        load one read further.
 *
 * Once the header is in, it keeps it and reads the first block into a buffer
 * the size of the largest. Once a block is in, it builds a TimArraySrc over
 * it, uploads its images and reads the next; after the last it frees the
 * buffer, sets `loaded` and runs the driver's onRequestDone. An allocation
 * failure sets `failed`.
 * @param self The object.
 */
void TimBlockSrc__AdvanceLoadState(TimBlockSrc *self);

/**
 * @brief Slot +0x078: sets a fade ramp's row count to 1 << shift.
 * @param self  The object.
 * @param index The ramp, 0..3.
 * @param shift log2 of its new row count.
 */
void TimBlockSrc__SetEntryShift(TimBlockSrc *self, s32 index, s32 shift);

/**
 * @brief Slot +0x07C: fades every ramp toward one colour (fadeEntry on each).
 * @param self  The object.
 * @param color The colour to fade toward.
 */
void TimBlockSrc__FadeAllEntries(TimBlockSrc *self, ColorRgb *color);

/**
 * @brief Slot +0x080: sets one ramp's colour and rebuilds it in VRAM
 *        (FadeClutRow).
 * @param self  The object.
 * @param index The ramp, 0..3.
 * @param color The colour to fade toward.
 */
void TimBlockSrc__FadeEntry(TimBlockSrc *self, s32 index, ColorRgb *color);

/**
 * @brief Rebuilds a fade ramp from its CLUT row.
 *
 * Reads the row back from VRAM, then writes the `mask - 1` rows below it, row
 * i + 1 blending every non-zero colour (i + 1) / mask of the way toward the
 * ramp's colour; the semi-transparency bit is kept.
 * @param entry The ramp.
 * @param index The ramp's index, which places its CLUT row in VRAM.
 */
void FadeClutRow(TimBlockSrcEntry *entry, s32 index);

#endif
