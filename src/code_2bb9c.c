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
 * Also holds `func_8003B624`, not a TimImage method (`classtable.py
 * D_8006E558` lists it nowhere): a free function that circularly scrolls a
 * VRAM rectangle right by one column at a time through the draw singleton's
 * `moveImage` slot, called from `class_3bb8c_n.c`'s `DrawStyleTables`.
 *
 * Fully matched in round 81 (runner echo). Naming pass round 81 (runner
 * bravo): every function and the class table named; see each function's
 * report `## Naming` for tier and evidence. `func_8003B624` kept its
 * `func_` name (proposed `ScrollImageRight`, recorded in its report);
 * `New_TimImage` was renamed in track 4 (round 88).
 */
#include "common.h"
#include "FileResource.h"
#include "DrawSystem.h"
#include "TimImage.h"

/* LIBGS.H: void GsGetTimInfo(unsigned long *im, GsIMAGE *tim); */
void GsGetTimInfo(u32 *im, GsIMAGE *tim);

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
    self->unk48 = 0;
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
        draw->methods->loadImage(draw, &rect, self->tim.pixel);
        if ((self->tim.pmode >> 3) & 1) {
            rect.x = self->tim.cx;
            rect.y = self->tim.cy;
            rect.w = self->tim.cw;
            rect.h = self->tim.ch;
            draw->methods->loadImage(draw, &rect, self->tim.clut);
        }
    }
}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5AC(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5B4(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5BC(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5C4(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5CC(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5D4(void) {}

/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5DC(void) {}

/* TimImage +0x098: sets unk48 to 1; unk48's purpose beyond that flag is
 * unestablished (no caller reads it outside the ctor/this setter). */
void TimImage__func_8003B5E4(TimImage *self) {
    self->unk48 = 1;
}

/* TimImage +0x09C: describe the TIM held in the buffer. */
void TimImage__GetTimInfo(TimImage *self, GsIMAGE *tim) {
    GsGetTimInfo((u32 *)self->buffer + 1, tim);
}

/* The class's table getter (called by New_TimImage and TimImage__TimImage). */
TimImageMethods *GetTimImageMethods(void) {
    return &gTimImageMethods;
}

/* Not in gTimImageMethods's table. Three calls to the draw singleton's slot
 * +0x064 per iteration, built from r's edges and p; nothing but i changes
 * between iterations. */
void func_8003B624(DrawRect *r, s32 count, DrawPoint *p) {
    DrawSystem *draw;
    void (*fn)(DrawSystem *, DrawRect *, s32, s32);
    DrawRect rect;
    s32 i;

    draw = GetDrawSystem();
    fn = draw->methods->moveImage;
    if (count != 0) {
        for (i = 0; i < count; i++) {
            rect.x = r->x + r->w - 1;
            rect.y = r->y;
            rect.w = 1;
            rect.h = r->h;
            fn(draw, &rect, p->x, p->y);
            rect.x = r->x;
            rect.y = r->y;
            rect.w = r->w - 1;
            rect.h = r->h;
            fn(draw, &rect, r->x + 1, r->y);
            rect.x = p->x;
            rect.y = p->y;
            rect.w = 1;
            rect.h = r->h;
            fn(draw, &rect, r->x, r->y);
        }
    }
}
