/*
 * TimArraySrc's methods (include/tim_array_src.h: a FileResource over a
 * file of TIM images, one TimImage built over each and pointed at the fade
 * ramp its CLUT row falls in), in ROM order: the allocator, ctor and
 * finalize, BuildImages (onRequestDone) and UploadImages, ending with its
 * getter GetTimArraySrcMethods; its method table closes the file. A
 * (void *) entry in it is a method whose declared parameters differ from
 * the slot's, usually one inherited from a parent class and declared on
 * the parent's type.
 */
#include "common.h"
#include "tim_block_src.h"
#include "tim_image.h"
#include "tim_array_src.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/* Allocate a TimArraySrc and construct it over the file `name`, or over
 * none. */
TimArraySrc *New_TimArraySrc(char *name) {
    void *obj = BMemPMgrAlloc(sizeof(TimArraySrc));

    if (obj != NULL) {
        GetTimArraySrcMethods()->ctor(obj, name);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): request `name` when there is one. */
void TimArraySrc__TimArraySrc(TimArraySrc *self, char *name) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTimArraySrcMethods();
    self->count = 0;
    self->images = NULL;
    self->ready = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}

/* finalize (+0x00C): release the images. */
void TimArraySrc__Finalize(TimArraySrc *self) {
    ReleaseBasicClassArray((BasicClass **)self->images, self->count);
    BMemPMgrFree(self->images);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

extern s16 sTimClutRowShift;

/** @brief A TimArraySrc's buffer: an image count, then each image's byte
 * offset from the start of the buffer. */
typedef struct TimArrayBuf {
    /* +0x00 */ s32 count;      /**< how many TIM images follow */
    /* +0x04 */ s32 offsets[1]; /**< each image's offset from the buffer's start */
} TimArrayBuf;

/* onRequestDone (+0x064): once the buffer is in, build one TimImage over each of
 * its images, in place, each with the fade ramp (`clutBase`'s entries) its
 * CLUT row falls in. */
void TimArraySrc__BuildImages(TimArraySrc *self) {
    GsIMAGE info;
    TimImage **objs;
    s32 i;
    s32 *offs;

    if ((self->flags & CD_FLAG_LOAD_FILE_DONE) || self->buffer != NULL) {
        self->count = ((TimArrayBuf *)self->buffer)->count;
        self->images = BMemPMgrAlloc(((TimArrayBuf *)self->buffer)->count * sizeof(*self->images));
        if (self->images != NULL) {
            objs = self->images;
            offs = ((TimArrayBuf *)self->buffer)->offsets;
            for (i = 0; i < self->count; i++) {
                *objs = New_TimImage(NULL);
                (*objs)->buffer = (u8 *)self->buffer + *offs;
                (*objs)->bufferSize = 0;
                (*objs)->methods->getTimInfo(*objs, &info);
                (*objs)->clutBase =
                    ((info.cy - CLUT_FADE_Y) >> sTimClutRowShift) * sizeof(TimBlockSrcEntry) +
                    self->clutBase;
                offs++;
                objs++;
            }
            self->ready = 1;
            GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
        }
    }
}

/* +0x078: upload every image (TimImage__Upload). */
void TimArraySrc__UploadImages(TimArraySrc *self) {
    TimImage **objs = self->images;
    s32 i;

    for (i = 0; i < self->count; i++) {
        ((TimImageUploadFn)(*objs)->methods->processBuffer)(*objs);
        objs++;
    }
}

TimArraySrcMethods *GetTimArraySrcMethods(void) {
    return &gTimArraySrcMethods;
}

/* TimArraySrc: build the TimImages, then upload them. */
TimArraySrcMethods gTimArraySrcMethods = {
    /* +0x000 header */ TIMARRAYSRC_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ TimArraySrc__TimArraySrc,
    /* +0x00C finalize */ TimArraySrc__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ TimArraySrc__BuildImages,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TimArraySrc__UploadImages,
};
