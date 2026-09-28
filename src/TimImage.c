/*
 * TimImage: the game's TIM-image loader, a FileResource data source (class id
 * 0x103, table gTimImageMethods, include/TimImage.h), plus one free VRAM
 * helper.
 *
 * Every caller builds a ".TIM" path and hands it to New_TimImage (the ctor
 * requests the file into `buffer`), then calls the handle's upload slot
 * (TimImage__Upload, +0x078) and usually FileResource__FreeBuffer (+0x05C).
 * TimImage__GetTimInfo describes the file with Sony's GsGetTimInfo; Upload
 * sends its pixel block and, when the TIM carries one, its CLUT to the draw
 * singleton (include/DrawSystem.h) through the loadImage slot. The slots at
 * +0x07C..+0x094 are empty, and +0x098 only sets flag48, which no code reads.
 *
 * RotateVramRectRight is not a TimImage method (no method table lists it): it
 * circularly scrolls a VRAM rectangle right, one column at a time, through the
 * draw singleton's moveImage slot, for class_3bb8c_n.c's StyleScrollVramStrips.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "FileResource.h"
#include "DrawSystem.h"
#include "TimImage.h"

/* An s16 point. */
typedef struct DrawPoint {
    /* +0x00 */ s16 x;
    /* +0x02 */ s16 y;
} DrawPoint;

extern void *BMemPMgrAlloc(s32 size);
extern FileResourceMethods *GetActiveDataSourceMethods(void);

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
    self->flag48 = 0;
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

/* TimImage +0x098: sets flag48 (the ctor clears it); no code reads it. */
void TimImage__SetFlag48(TimImage *self) {
    self->flag48 = 1;
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
