/*
 * TimImage: the game's TIM-image loader, a FileResource data source (class id
 * 0x103, table gTimImageMethods, include/tim_image.h), plus one free VRAM
 * helper.
 *
 * Every caller builds a ".TIM" path and hands it to New_TimImage (the ctor
 * requests the file into `buffer`), then calls the handle's upload slot
 * (TimImage__Upload, +0x078) and usually FileResource__FreeBuffer (+0x05C).
 * TimImage__GetTimInfo describes the file with Sony's GsGetTimInfo; Upload
 * sends its pixel block and, when the TIM carries one, its CLUT to the draw
 * singleton (include/draw_system.h) through the loadImage slot. The slots at
 * +0x07C..+0x094 are empty, and +0x098 only sets flag, which no code reads.
 *
 * RotateVramRectRight is not a TimImage method (no method table lists it): it
 * circularly scrolls a VRAM rectangle right, one column at a time, through the
 * draw singleton's moveImage slot, for style_layer.c's StyleScrollVramStrips.
 * The method table closes the file.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "file_resource.h"
#include "draw_system.h"
#include "tim_image.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/* new TimImage(name). */
TimImage *New_TimImage(char *name) {
    TimImage *self;

    self = BMemPMgrAlloc(sizeof(TimImage));
    if (self != NULL) {
        GetTimImageMethods()->ctor(self, name);
        return self;
    }
    return NULL;
}

/* TimImage +0x008: the ctor. */
void TimImage__TimImage(TimImage *self, char *name) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTimImageMethods();
    self->flag = 0;
    self->clutBase = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}

/* TimImage +0x00C: finalize, straight to the active driver's. */
void TimImage__Finalize(TimImage *self) {
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* TimImage +0x078: describe the TIM, then upload its pixel block and, when
 * pmode bit 3 says it has one, its CLUT. */
void TimImage__Upload(TimImage *self) {
    DrawSystem *draw;
    DrawRect rect;
    GsIMAGE *tim;

    draw = GetDrawSystem();
    tim = &self->tim;
    if (self->buffer != NULL) {
        self->methods->getTimInfo(self, tim);
        rect.x = self->tim.px;
        rect.y = self->tim.py;
        rect.w = self->tim.pw;
        rect.h = self->tim.ph;
        draw->methods->loadImage(draw, &rect, (u32 *)self->tim.pixel);
        if ((self->tim.pmode >> TIM_PMODE_CLUT_BIT) & 1) {
            rect.x = self->tim.cx;
            rect.y = self->tim.cy;
            rect.w = self->tim.cw;
            rect.h = self->tim.ch;
            draw->methods->loadImage(draw, &rect, (u32 *)self->tim.clut);
        }
    }
}

/* TimImage +0x07C: empty. */
void TimImage__NoOpSlot7C(void) {}

/* TimImage +0x080: empty. */
void TimImage__NoOpSlot80(void) {}

/* TimImage +0x084: empty. */
void TimImage__NoOpSlot84(void) {}

/* TimImage +0x088: empty. */
void TimImage__NoOpSlot88(void) {}

/* TimImage +0x08C: empty. */
void TimImage__NoOpSlot8C(void) {}

/* TimImage +0x090: empty. */
void TimImage__NoOpSlot90(void) {}

/* TimImage +0x094: empty. */
void TimImage__NoOpSlot94(void) {}

/* TimImage +0x098: sets flag (the ctor clears it); no code reads it. */
void TimImage__SetFlag(TimImage *self) {
    self->flag = 1;
}

/* TimImage +0x09C: describe the TIM held in the buffer (past its id word). */
void TimImage__GetTimInfo(TimImage *self, GsIMAGE *tim) {
    GsGetTimInfo((unsigned long *)self->buffer + 1, tim);
}

/* The class's table getter (called by New_TimImage and TimImage__TimImage). */
TimImageMethods *GetTimImageMethods(void) {
    return &gTimImageMethods;
}

/* Rotates the VRAM rectangle `area` one column to the right, count times, through
 * the one-column scratch area at `scratch`: the last column goes there, the rest moves
 * right by one, and the scratch column comes back as column 0 (three moveImage calls each). */
void RotateVramRectRight(DrawRect *area, s32 count, DrawPoint *scratch) {
    DrawSystem *draw;
    void (*moveImage)(DrawSystem *, DrawRect *, s32, s32);
    DrawRect rect;
    s32 i;

    draw = GetDrawSystem();
    moveImage = draw->methods->moveImage;
    if (count != 0) {
        for (i = 0; i < count; i++) {
            rect.x = area->x + area->w - 1;
            rect.y = area->y;
            rect.w = 1;
            rect.h = area->h;
            moveImage(draw, &rect, scratch->x, scratch->y);
            rect.x = area->x;
            rect.y = area->y;
            rect.w = area->w - 1;
            rect.h = area->h;
            moveImage(draw, &rect, area->x + 1, area->y);
            rect.x = scratch->x;
            rect.y = scratch->y;
            rect.w = 1;
            rect.h = area->h;
            moveImage(draw, &rect, area->x, area->y);
        }
    }
}

/* TimImage's method table (include/tim_image.h): FileResource's slots with
 * TimImage's ctor and finalize, Upload in processBuffer, then its own
 * nine. A (void *) entry is a method whose declared parameters differ from
 * the slot's, usually a base method on BasicClass * or FileResource *. */
TimImageMethods gTimImageMethods = {
    /* +0x000 header */ TIMIMAGE_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ TimImage__TimImage,
    /* +0x00C finalize */ TimImage__Finalize,
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
    /* +0x064 onRequestDone */ (void *)FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TimImage__Upload,
    /* +0x07C slot7C */ TimImage__NoOpSlot7C,
    /* +0x080 slot80 */ TimImage__NoOpSlot80,
    /* +0x084 slot84 */ TimImage__NoOpSlot84,
    /* +0x088 slot88 */ TimImage__NoOpSlot88,
    /* +0x08C slot8C */ TimImage__NoOpSlot8C,
    /* +0x090 slot90 */ TimImage__NoOpSlot90,
    /* +0x094 slot94 */ TimImage__NoOpSlot94,
    /* +0x098 setFlag */ TimImage__SetFlag,
    /* +0x09C getTimInfo */ TimImage__GetTimInfo,
};
