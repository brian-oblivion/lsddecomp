/**
 * @file tim_image.h
 * @brief TimImage, the FileResource over one TIM image that uploads it to
 *        VRAM, its method table, and one free VRAM helper.
 */
#ifndef TIM_IMAGE_H
#define TIM_IMAGE_H

#include "file_resource.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

typedef struct TimImage TimImage;
typedef struct TimImageMethods TimImageMethods;

/** TimImage's class id (gTimImageMethods word +0x000): FileResource's 0x3,
 * one level down. */
#define TIMIMAGE_CLASS_ID 0x103

/**
 * @brief TimImage's method table, gTimImageMethods: FileResource's slots,
 *        then nine of its own.
 *
 * It overrides +0x008 ctor (TimImage__TimImage) and +0x00C finalize
 * (TimImage__Finalize). The inherited +0x078 processBuffer holds
 * TimImage__Upload, called through TimImageUploadFn.
 */
struct TimImageMethods {
    FILERESOURCE_SLOTS(TimImage, (TimImage * self, char *name));
    /* +0x07C */ void (*slot7C)(void);                             /**< @see TimImage__NoOpSlot7C */
    /* +0x080 */ void (*slot80)(void);                             /**< @see TimImage__NoOpSlot80 */
    /* +0x084 */ void (*slot84)(void);                             /**< @see TimImage__NoOpSlot84 */
    /* +0x088 */ void (*slot88)(void);                             /**< @see TimImage__NoOpSlot88 */
    /* +0x08C */ void (*slot8C)(void);                             /**< @see TimImage__NoOpSlot8C */
    /* +0x090 */ void (*slot90)(void);                             /**< @see TimImage__NoOpSlot90 */
    /* +0x094 */ void (*slot94)(void);                             /**< @see TimImage__NoOpSlot94 */
    /* +0x098 */ void (*setFlag)(TimImage *self);                  /**< @see TimImage__SetFlag */
    /* +0x09C */ void (*getTimInfo)(TimImage *self, GsIMAGE *tim); /**< @see TimImage__GetTimInfo */
};

/**
 * @brief One TIM image (class id 0x103): a FileResource whose buffer holds a
 *        TIM file, described into a GsIMAGE and uploaded to VRAM.
 *
 * Parent FileResource, through the active data-source driver; no subclasses.
 * Methods in src/graphics/tim_image.c. The object is 0x50 bytes
 * (New_TimImage).
 *
 * Most callers build a ".TIM" path and call New_TimImage(path), which requests
 * the file, then processBuffer (upload), then usually freeBuffer or release
 * once the sprites made from it hold what they need; a Sprite's `texture` is
 * a TimImage, and its reset keeps &texture->tim. TimArraySrc instead makes
 * them with New_TimImage(NULL), points `buffer` into its own block and sets
 * `clutBase`.
 */
struct TimImage {
    FILERESOURCE_FIELDS(TimImageMethods); /**< buffer: the TIM file */
    /* +0x02C */ GsIMAGE tim; /**< the TIM as TimImage__Upload describes it; a Sprite's reset keeps its address */
    /* +0x048 */ s32 flag;          /**< 0 from the ctor, 1 from setFlag; nothing reads it */
    /* +0x04C */ intptr_t clutBase; /**< 0 from the ctor; TimArraySrc sets it to the address of the fade ramp the image's CLUT row falls in */
};

/** @brief TimImage__Upload as its callers reach it through the void-typed
 *         processBuffer slot. */
typedef void (*TimImageUploadFn)(TimImage *self);

/** GsIMAGE.pmode is the TIM file's flag word: the low bits the pixel mode,
 * this bit (CF) set when the file carries a CLUT, which TimImage__Upload then
 * uploads too. */
#define TIM_PMODE_CLUT_BIT 3

/** The pixel mode's low two bits, the colour depth as GetTPage's `tp` takes
 * it (0 4-bit CLUT, 1 8-bit CLUT, 2 15-bit direct); InitGsSprite reads it. */
#define TIM_PMODE_DEPTH_MASK 0x3

/** TimImage's method table. */
extern TimImageMethods gTimImageMethods;

/**
 * @brief Returns TimImage's method table.
 * @return &gTimImageMethods.
 */
extern TimImageMethods *GetTimImageMethods(void);

/**
 * @brief Allocates a TimImage from the pool and constructs it.
 * @param name A ".TIM" file to request, or NULL for a buffer the caller sets.
 * @return The new object, or NULL when the pool is exhausted.
 */
TimImage *New_TimImage(char *name);

/**
 * @brief Constructor (slot +0x008): the active driver's, flag and clutBase
 *        cleared, then requests `name` when there is one.
 * @param self The object to construct.
 * @param name A file to request, or NULL.
 */
void TimImage__TimImage(TimImage *self, char *name);

/**
 * @brief Finalizer (slot +0x00C): the active driver's, nothing of its own.
 * @param self The object being destroyed.
 */
void TimImage__Finalize(TimImage *self);

/**
 * @brief Slot +0x078 (processBuffer): describes the TIM into `tim`, then
 *        uploads its pixel block and, when it carries one, its CLUT through
 *        the draw system's loadImage. Does nothing without a buffer.
 * @param self The image.
 */
void TimImage__Upload(TimImage *self);

/** @brief Slot +0x07C: does nothing. */
void TimImage__NoOpSlot7C(void);

/** @brief Slot +0x080: does nothing. */
void TimImage__NoOpSlot80(void);

/** @brief Slot +0x084: does nothing. */
void TimImage__NoOpSlot84(void);

/** @brief Slot +0x088: does nothing. */
void TimImage__NoOpSlot88(void);

/** @brief Slot +0x08C: does nothing. */
void TimImage__NoOpSlot8C(void);

/** @brief Slot +0x090: does nothing. */
void TimImage__NoOpSlot90(void);

/** @brief Slot +0x094: does nothing. */
void TimImage__NoOpSlot94(void);

/**
 * @brief Slot +0x098: sets `flag`, which nothing reads.
 * @param self The image.
 */
void TimImage__SetFlag(TimImage *self);

/**
 * @brief Slot +0x09C: describes the TIM in the buffer (past its id word) with
 *        libgs's GsGetTimInfo.
 * @param self The image, its buffer holding a TIM file.
 * @param tim  Receives the image's pixel and CLUT positions, sizes and
 *             addresses.
 */
void TimImage__GetTimInfo(TimImage *self, GsIMAGE *tim);

/** @brief A VRAM point in s16 coordinates. */
typedef struct DrawPoint {
    /* +0x00 */ s16 x; /**< VRAM x */
    /* +0x02 */ s16 y; /**< VRAM y */
} DrawPoint;

struct DrawRect; /* include/draw_system.h */

/**
 * @brief Rotates a VRAM rectangle one column to the right, `count` times.
 *
 * Each step moves the last column to `scratch`, the rest right by one, and
 * the scratch column back as column 0, through the draw system's moveImage.
 * Not a TimImage method; style_layer.c's StyleScrollVramStrips calls it.
 * @param area    The rectangle to rotate.
 * @param count   How many columns to rotate it by; 0 does nothing.
 * @param scratch The top of a free one-column VRAM area as tall as `area`.
 */
void RotateVramRectRight(struct DrawRect *area, s32 count, DrawPoint *scratch);

#endif
