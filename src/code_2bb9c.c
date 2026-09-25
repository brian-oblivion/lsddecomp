/*
 * code_2bb9c -- GAME code carved from psyq_2bb9c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2BB9C..0x2BF70 (vram 0x8003B39C..0x8003B770). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint).
 *
 * What it holds: TimImage, a Class6D430 (data-source) subclass -- 12 of its
 * own methods (table `gTimImageMethods`, id 0x103), plus the class's own
 * alloc-then-ctor helper (`func_8003B39C`, "new TimImage(name)") and table
 * getter (`GetTimImageMethods`). Every call site project-wide that reaches
 * TimImage does so by building "CARD\\<name>.TIM" or another `.TIM` path and
 * handing it to `func_8003B39C`, then calling the returned handle's slot78
 * (`TimImage__Upload`) and usually slot5C (`Class6D430__FreeBuffer`) --
 * TimImage is the game's TIM-image loader: `buffer` (inherited from
 * Class6D430) holds the raw file, `TimImage__GetTimInfo` describes it with
 * Sony's `GsGetTimInfo`, and `TimImage__Upload` uploads the pixel block and,
 * when present, the CLUT to the draw singleton (`Class6C070`, `code_10ee0.c`)
 * through its `loadImage`/`MoveImage`-shaped method pair.
 *
 * Also holds `func_8003B624`, not a TimImage method (`classtable.py
 * D_8006E558` lists it nowhere): a free function that circularly scrolls a
 * VRAM rectangle right by one column at a time through the draw singleton's
 * `moveImage` slot, called from `class_3bb8c_n.c`'s `DrawStyleTables`.
 *
 * Fully matched in round 81 (runner echo). Naming pass round 81 (runner
 * bravo): every function and the class table named; see each function's
 * report `## Naming` for tier and evidence. `func_8003B39C` and
 * `func_8003B624` themselves kept their `func_` names -- both have an
 * explicit line in the symbols file under a stale track-2 `// unidentified:`
 * comment, which trips a known `tools/rename.py` bug (it resolves the
 * address from the name and never finds the existing line, so it appends a
 * duplicate and `make extract` fails with "Duplicate symbol"). Proposed
 * names `New_TimImage` and `ScrollImageRight` are recorded in their reports.
 */
#include "common.h"
#include "Class6D430.h"

/* LIBGS.H GsIMAGE, laid out as the SDK declares it: GsGetTimInfo fills it
 * and TimImage__Upload reads it field by field. */
typedef struct GsIMAGE {
    /* +0x00 */ u32 pmode;
    /* +0x04 */ s16 px;
    /* +0x06 */ s16 py;
    /* +0x08 */ u16 pw;
    /* +0x0A */ u16 ph;
    /* +0x0C */ u32 *pixel;
    /* +0x10 */ s16 cx;
    /* +0x12 */ s16 cy;
    /* +0x14 */ u16 cw;
    /* +0x16 */ u16 ch;
    /* +0x18 */ u32 *clut;
} GsIMAGE;

typedef struct TimImage TimImage;

/* TimImage's own table (gTimImageMethods): Class6D430's slots with this
 * class's two-argument ctor, then its own +0x07C..+0x09C
 * (tools/classtable.py D_8006E558, the table's pre-rename address name). */
typedef struct TimImageMethods {
    CLASS6D430_SLOTS(TimImage, (TimImage *self, char *name));
    /* +0x07C */ void (*slot7C)(void);
    /* +0x080 */ void (*slot80)(void);
    /* +0x084 */ void (*slot84)(void);
    /* +0x088 */ void (*slot88)(void);
    /* +0x08C */ void (*slot8C)(void);
    /* +0x090 */ void (*slot90)(void);
    /* +0x094 */ void (*slot94)(void);
    /* +0x098 */ void (*slot98)(TimImage *self);
    /* +0x09C */ void (*getTimInfo)(TimImage *self, GsIMAGE *tim);
} TimImageMethods;

/* Unit-local view of the TimImage class: a Class6D430 data source whose
 * buffer holds a TIM image (TimImage__GetTimInfo hands buffer+4, past the TIM id
 * word, to GsGetTimInfo). TimImage__Upload has the TIM described into +0x02C
 * and uploads its pixel and CLUT blocks from there. +0x048 is cleared by the
 * ctor and set to 1 by TimImage__func_8003B5E4; the ctor also clears +0x04C. The
 * object is 0x50 bytes (func_8003B39C's allocation). */
struct TimImage {
    CLASS6D430_FIELDS(TimImageMethods);
    /* +0x02C */ GsIMAGE tim;
    /* +0x048 */ s32 unk48;
    /* +0x04C */ s32 unk4C;
};

/* LIBGS.H: void GsGetTimInfo(unsigned long *im, GsIMAGE *tim); */
void GsGetTimInfo(u32 *im, GsIMAGE *tim);

/* A rectangle as the draw singleton's methods take it: 16-bit origin,
 * 32-bit extent. */
typedef struct DrawRect {
    /* +0x00 */ s16 x;
    /* +0x02 */ s16 y;
    /* +0x04 */ s32 w;
    /* +0x08 */ s32 h;
} DrawRect;

/* An s16 point. */
typedef struct DrawPoint {
    /* +0x00 */ s16 x;
    /* +0x02 */ s16 y;
} DrawPoint;

typedef struct Class6C070 Class6C070;
/* Only the two slots this unit reaches. */
typedef struct Class6C070Methods {
    /* +0x000 */ u8 pad0[0x58];
    /* +0x058 */ void (*loadImage)(Class6C070 *self, DrawRect *rect, u32 *data);
    /* +0x05C */ u8 pad5C[0x8];
    /* +0x064 */ void (*moveImage)(Class6C070 *self, DrawRect *rect, s32 x, s32 y);
} Class6C070Methods;
struct Class6C070 {
    /* +0x000 */ Class6C070Methods *methods;
};

extern Class6C070 *func_80020C5C(void); /* returns the draw singleton */
extern void *BMemPMgrAlloc(s32 size);
extern Class6D430Methods *GetActiveDataSourceMethods(void);
TimImageMethods *GetTimImageMethods(void);

extern TimImageMethods gTimImageMethods;

/* new TimImage(name). Proposed name New_TimImage -- see "## Naming" in
 * this function's report for why the rename itself is blocked. */
TimImage *func_8003B39C(char *name) {
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
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = GetTimImageMethods();
    self->unk48 = 0;
    self->unk4C = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}
/* TimImage +0x00C: finalize, straight to the active driver's. */
void TimImage__Finalize(TimImage *self) {
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
/* TimImage +0x078: describe the TIM, then upload its pixel block and, when
 * pmode bit 3 says it has one, its CLUT. */
void TimImage__Upload(TimImage *self) {
    Class6C070 *draw;
    DrawRect rect;
    GsIMAGE *tim;

    draw = func_80020C5C();
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
void TimImage__func_8003B5AC(void) {
}
/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5B4(void) {
}
/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5BC(void) {
}
/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5C4(void) {
}
/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5CC(void) {
}
/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5D4(void) {
}
/* TimImage slot (tools/classtable.py); empty body. */
void TimImage__func_8003B5DC(void) {
}
/* TimImage +0x098: sets unk48 to 1; unk48's purpose beyond that flag is
 * unestablished (no caller reads it outside the ctor/this setter). */
void TimImage__func_8003B5E4(TimImage *self) {
    self->unk48 = 1;
}
/* TimImage +0x09C: describe the TIM held in the buffer. */
void TimImage__GetTimInfo(TimImage *self, GsIMAGE *tim) {
    GsGetTimInfo((u32 *)self->buffer + 1, tim);
}
/* The class's table getter (called by func_8003B39C and TimImage__TimImage). */
TimImageMethods *GetTimImageMethods(void) {
    return &gTimImageMethods;
}
/* Not in gTimImageMethods's table. Three calls to the draw singleton's slot
 * +0x064 per iteration, built from r's edges and p; nothing but i changes
 * between iterations. */
void func_8003B624(DrawRect *r, s32 count, DrawPoint *p) {
    Class6C070 *draw;
    void (*fn)(Class6C070 *, DrawRect *, s32, s32);
    DrawRect rect;
    s32 i;

    draw = func_80020C5C();
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
