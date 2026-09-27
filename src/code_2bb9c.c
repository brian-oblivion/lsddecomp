/*
 * code_2bb9c -- GAME code carved from psyq_2bb9c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2BB9C..0x2BF70 (vram 0x8003B39C..0x8003B770). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint).
 *
 * What it holds: TimImage, a FileResource (data-source) subclass -- 12 of its
 * own methods (table `gTimImageMethods`, id 0x103), plus the class's own
 * alloc-then-ctor helper (`New_TimImage`, "new TimImage(name)") and table
 * getter (`GetTimImageMethods`). Every call site project-wide that reaches
 * TimImage does so by building "CARD\\<name>.TIM" or another `.TIM` path and
 * handing it to `New_TimImage`, then calling the returned handle's slot78
 * (`TimImage__Upload`) and usually slot5C (`FileResource__FreeBuffer`) --
 * TimImage is the game's TIM-image loader: `buffer` (inherited from
 * FileResource) holds the raw file, `TimImage__GetTimInfo` describes it with
 * Sony's `GsGetTimInfo`, and `TimImage__Upload` uploads the pixel block and,
 * when present, the CLUT to the draw singleton (DrawSystem, `include/DrawSystem.h`)
 * through its loadImage slot.
 *
 * Also holds `RotateVramRectRight`, not a TimImage method (`classtable.py
 * D_8006E558` lists it nowhere): a free function that circularly scrolls a
 * VRAM rectangle right by one column at a time through the draw singleton's
 * `moveImage` slot, called from `class_3bb8c_n.c`'s `StyleScrollVramStrips`.
 *
 * Fully matched in round 81 (runner echo). Naming pass round 81 (runner
 * bravo): every function and the class table named; see each function's
 * report `## Naming` for tier and evidence. `RotateVramRectRight` kept its
 * `func_` name (proposed `ScrollImageRight`, recorded in its report);
 * `New_TimImage` was renamed in track 4 (round 88).
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

    self = BMemPMgrAlloc(0x50);
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
        if ((self->tim.pmode >> 3) & 1) {
            rect.x = self->tim.cx;
            rect.y = self->tim.cy;
            rect.w = self->tim.cw;
            rect.h = self->tim.ch;
            draw->methods->loadImage(draw, &rect, (u32 *)self->tim.clut);
        }
    }
}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__NoOpSlot7C(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__NoOpSlot80(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__NoOpSlot84(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__NoOpSlot88(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__NoOpSlot8C(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__NoOpSlot90(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__NoOpSlot94(void) {}

/* TimImage +0x098: sets unk48 to 1; unk48's purpose beyond that flag is
 * unestablished (no caller reads it outside the ctor/this setter). */
void TimImage__SetFlag48(TimImage *self) {
    self->flag48 = 1;
}

/* TimImage +0x09C: describe the TIM held in the buffer. */
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
