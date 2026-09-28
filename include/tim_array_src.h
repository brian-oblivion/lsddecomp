/**
 * @file tim_array_src.h
 * @brief TimArraySrc, the FileResource that turns one block of TIM images
 *        into TimImage objects, and its method table.
 */
#ifndef TIM_ARRAY_SRC_H
#define TIM_ARRAY_SRC_H

#include "file_resource.h"

typedef struct TimArraySrc TimArraySrc;
typedef struct TimArraySrcMethods TimArraySrcMethods;
struct TimImage;

/**
 * @brief TimArraySrc's method table, gTimArraySrcMethods: FileResource's
 *        slots, with no new ones.
 *
 * It overrides +0x008 ctor (TimArraySrc__TimArraySrc), +0x00C finalize
 * (TimArraySrc__Finalize) and +0x064 onRequestDone (TimArraySrc__BuildImages);
 * +0x058 loadFile is NULL. The inherited +0x078 processBuffer holds
 * TimArraySrc__UploadImages, called through TimArraySrcUploadFn.
 */
struct TimArraySrcMethods {
    FILERESOURCE_SLOTS(TimArraySrc, (TimArraySrc * self, char *name));
};

/**
 * @brief One block of TIM images (class id 0xC03): a buffer holding an image
 *        count and each image's byte offset from the block's start, turned
 *        into one TimImage per image.
 *
 * Parent FileResource, through the active data-source driver; no subclasses.
 * Methods in src/graphics/graphics_resources.c. The object is 0x3C bytes
 * (New_TimArraySrc).
 *
 * Its one builder is TimBlockSrc__AdvanceLoadState: per block it makes one
 * with New_TimArraySrc(NULL), points `buffer` at the block it read (size 0,
 * so the TimArraySrc never frees it) and `clutBase` at its own fade ramps,
 * then runs onRequestDone (BuildImages) and processBuffer (UploadImages). The
 * TimBlockSrc keeps it in `blocks` and releases it.
 */
struct TimArraySrc {
    FILERESOURCE_FIELDS(TimArraySrcMethods); /**< buffer: the block (count, then offsets) */
    /* +0x02C */ s32 count;                  /**< images built: the block's first word */
    /* +0x030 */ struct TimImage **images; /**< one TimImage per image, each over its TIM in the block */
    /* +0x034 */ s32 clutBase; /**< the address of the TimBlockSrc's fade ramps (TimBlockSrcEntry[4]); each image's clutBase is the ramp its CLUT row falls in */
    /* +0x038 */ s32 ready;    /**< 0 from the ctor, 1 once BuildImages built the array */
};

/** @brief TimArraySrc__UploadImages as TimBlockSrc__AdvanceLoadState calls it
 *         through the void-typed processBuffer slot. */
typedef void (*TimArraySrcUploadFn)(TimArraySrc *self);

/** TimArraySrc's method table. */
extern TimArraySrcMethods gTimArraySrcMethods;

/**
 * @brief Returns TimArraySrc's method table.
 * @return &gTimArraySrcMethods.
 */
extern TimArraySrcMethods *GetTimArraySrcMethods(void);

/**
 * @brief Allocates a TimArraySrc from the pool and constructs it.
 * @param name A file to request, or NULL for a buffer the caller sets.
 * @return The new object, or NULL when the pool is exhausted.
 */
TimArraySrc *New_TimArraySrc(char *name);

/**
 * @brief Constructor (slot +0x008): the active driver's, with no images yet,
 *        then requests `name` when there is one.
 * @param self The object to construct.
 * @param name A file to request, or NULL.
 */
void TimArraySrc__TimArraySrc(TimArraySrc *self, char *name);

/**
 * @brief Finalizer (slot +0x00C): releases the images and frees the array,
 *        then the active driver's finalizer.
 * @param self The object being destroyed.
 */
void TimArraySrc__Finalize(TimArraySrc *self);

/**
 * @brief Slot +0x064 (onRequestDone): once the buffer is in, builds one
 *        TimImage over each of its images, in place.
 *
 * Each image's clutBase is set to the fade ramp its CLUT row falls in. On
 * success it sets `ready` and runs the driver's onRequestDone; when the array
 * cannot be allocated nothing is built.
 * @param self The object.
 */
void TimArraySrc__BuildImages(TimArraySrc *self);

/**
 * @brief Slot +0x078 (processBuffer): uploads every image to VRAM
 *        (TimImage__Upload on each).
 * @param self The object, its images built.
 */
void TimArraySrc__UploadImages(TimArraySrc *self);

#endif
