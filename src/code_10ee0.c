/*
 * code_10ee0 -- GAME code carved from the head of psyq_10ee0 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x10EE0..0x11474 (vram
 * 0x800206E0..0x80020C74). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the 19 methods of D_8006C070, the game's
 * screen/graphics singleton, matched as DrawSystem this round. `main.c`
 * builds the one instance (`New_DrawSystem`) and hands it into the game's
 * startup chain, which lands it in `code_2b78c.c`'s `Class6E4F0__InitSystems`
 * as its `source` argument -- that unit dispatches `source`'s own +0x044
 * slot, the address this unit's table lists as `initGraph`
 * (`DrawSystem__InitGraph`, GsInitGraph setup), confirming the two units see
 * the same object. Three OTHER units independently called
 * `GetDrawSystem()`'s return "the draw singleton" in their own comments
 * before this rename, and two of them (`code_2bb9c.c`, `code_179d8_q.c`)
 * independently chose the names `loadImage`/`moveImage` for the exact same
 * slots this unit matched as LoadImage/MoveImage -- three-way convergent
 * naming evidence, not a guess. libgpu/sys starts right after, at
 * ResetGraph (now psyq_11474).
 *
 * Round 81 (bravo) matched the ten small methods/accessors; round 81
 * (alpha) matched ten more (the allocator, ctor, init and the RECT/VRAM
 * helpers). Round 82 (alpha) matched the last four (DrawSystem__InitGraph,
 * DrawSystem__StoreImage, DrawSystem__RunLoop, DrawSystem__ClearImage); the
 * unit is complete. Round 82 (bravo): naming pass -- class named DrawSystem,
 * every function and both gp-variable accessors renamed via
 * tools/rename.py, method-table slots named for the methods they hold. See
 * each function's report `## Naming` for tier and evidence.
 */
#include "common.h"
#include "BasicClass.h"

/*
 * Local view of the D_8006C070 object (a BasicClass subclass): the game's
 * screen/graphics singleton (see the header comment above for the
 * cross-unit evidence). Only the fields this unit's matched methods touch
 * are named; the singleton other units reach through GetDrawSystem() is an
 * object of this class.
 */
typedef struct DrawSystem DrawSystem;
typedef struct DrawSystemMethods DrawSystemMethods;

/* LIBGPU.H's RECT, declared locally (never #include a psyq header). */
typedef struct {
    short x, y;
    short w, h;
} RECT;

/* A 4-short RECT source: x/y/w at +0/+2/+4, h at +8 (ConvertRect). */
typedef struct {
    /* +0x0 */ s16 x;
    /* +0x2 */ s16 y;
    /* +0x4 */ s16 w;
    /* +0x6 */ s16 unk6;
    /* +0x8 */ s16 h;
} DrawSystemRect;

/* What DrawSystem__GetDims fills in: two zero shorts then the +0x14 pair. */
typedef struct {
    /* +0x0 */ s16 x;
    /* +0x2 */ s16 y;
    /* +0x4 */ s32 w;
    /* +0x8 */ s32 h;
} DrawSystemDims;

typedef struct {
    /* +0x0 */ s32 w;
    /* +0x4 */ s32 h;
} DrawSystemSize;

struct DrawSystemMethods {
    BASICCLASS_SLOTS(DrawSystem, (DrawSystem *self));
    /* +0x040 */ void (*init)(DrawSystem *self);                  /* DrawSystem__Init */
    /* +0x044 */ void *initGraph;                                 /* DrawSystem__InitGraph */
    /* +0x048 */ void *start;                                     /* DrawSystem__Start */
    /* +0x04C */ void *stop;                                      /* DrawSystem__Stop */
    /* +0x050 */ void *swapBuffers;                               /* DrawSystem__SwapBuffers */
    /* +0x054 */ void *getActiveBuffer;                           /* DrawSystem__GetActiveBuffer */
    /* +0x058 */ void *loadImage;                                 /* DrawSystem__LoadImage */
    /* +0x05C */ void *storeImage;                                /* DrawSystem__StoreImage */
    /* +0x060 */ void *slot60;                                    /* DrawSystem__func_80020A1C, always returns 0 */
    /* +0x064 */ void *moveImage;                                 /* DrawSystem__MoveImage */
    /* +0x068 */ void (*runLoop)(DrawSystem *self);               /* DrawSystem__RunLoop */
    /* +0x06C */ void *countFrames;                               /* DrawSystem__CountFrames */
    /* +0x070 */ void (*setVSyncCount)(DrawSystem *self, s32 value); /* DrawSystem__SetVSyncCount */
    /* +0x074 */ void *getVSyncCount;                             /* DrawSystem__GetVSyncCount */
    /* +0x078 */ void (*clearImage)(DrawSystem *self, u8 *color, DrawSystemRect *src); /* DrawSystem__ClearImage */
    /* +0x07C */ DrawSystemSize *(*getDims)(DrawSystem *self, DrawSystemDims *out); /* DrawSystem__GetDims */
    /* +0x080 */ void (*setSyncMode)(DrawSystem *self, s32 value); /* DrawSystem__SetSyncMode */
};

struct DrawSystem {
    BASICCLASS_FIELDS(DrawSystemMethods);
    /* +0x00C */ s32 unkC;            /* DrawSystem__CountFrames sets to 1 once unk24 reaches unk20 */
    /* +0x010 */ s32 running;         /* cleared by Stop; SetVSyncCount stores unk20 only while 0; RunLoop's while condition */
    /* +0x014 */ DrawSystemSize size; /* GetDims returns its address */
    /* +0x01C */ s32 vramMode;        /* InitGraph: GsInitGraph vram mode */
    /* +0x020 */ s32 unk20;           /* SetVSyncCount sets, GetVSyncCount gets; also RunLoop's VSync() argument and CountFrames's threshold -- one field, two uses, neither fully pinned down */
    /* +0x024 */ s32 unk24;           /* CountFrames counts up to unk20 */
    /* +0x028 */ u8 pad28[0x2C - 0x28];
    /* +0x02C */ s32 syncMode;        /* SetSyncMode sets; gates the post-transfer DrawSync(0) in LoadImage/StoreImage and the running-bypass there and in ClearImage's dispatch */
    /* +0x030 */ void (*callback)(void); /* SetCallback sets, RunLoop calls each VSync */
};

extern DrawSystemMethods D_8006C070;  /* the class's method table */
extern DrawSystem *gDrawSystem;        /* sdata: the singleton GetDrawSystem returns */

extern void GsSwapDispBuff(void);     /* LIBGS.H */
extern void GsInitGraph(unsigned short x_res, unsigned short y_res,
                        unsigned short intmode, unsigned short dith,
                        unsigned short varmmode);   /* LIBGS.H */
extern void GsDefDispBuff(unsigned short x0, unsigned short y0,
                          unsigned short x1, unsigned short y1); /* LIBGS.H */
extern int GsGetActiveBuff(void);     /* LIBGS.H */
extern int LoadImage(RECT *rect, u_long *p);        /* LIBGPU.H */
extern int MoveImage(RECT *rect, int x, int y);     /* LIBGPU.H */
extern int DrawSync(int mode);                      /* LIBGPU.H */
extern int ClearImage(RECT *rect, u_char r, u_char g, u_char b); /* LIBGPU.H */
extern int VSync(int mode);                         /* LIBETC.H */
extern int StoreImage(RECT *rect, u_long *p);       /* LIBGPU.H */
extern void *BMemPMgrAlloc(s32 size);

DrawSystemMethods *Get_vtable_DrawSystem(void);
DrawSystem *GetDrawSystem(void);
void ConvertRect(RECT *dst, DrawSystemRect *src);

DrawSystem *New_DrawSystem(void) {
    DrawSystem *p = BMemPMgrAlloc(0x34);

    if (p != NULL) {
        Get_vtable_DrawSystem()->ctor(p);
        return p;
    }
    return NULL;
}
void DrawSystem__DrawSystem(DrawSystem *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_DrawSystem();
    self->methods->init(self);
}
void DrawSystem__Init(DrawSystem *self) {
    self->running = 0;
    self->methods->setVSyncCount(self, 3);
    self->methods->setSyncMode(self, 1);
    self->callback = NULL;
}
void DrawSystem__InitGraph(DrawSystem *self, DrawSystemSize *size, s32 vramMode) {
    GsInitGraph(size->w, size->h, 0, 1, vramMode);
    GsDefDispBuff(0, 0, 0, size->h);
    self->size = *size;
    self->vramMode = vramMode;
}
void DrawSystem__Start(DrawSystem *self) {
    if (self->running == 0) {
        self->running = 1;
        self->methods->runLoop(self);
    }
}
void DrawSystem__Stop(DrawSystem *self) {
    if (self->running != 0) {
        self->running = 0;
    }
}
void DrawSystem__SwapBuffers(DrawSystem *self) {
    GsSwapDispBuff();
}
s32 DrawSystem__GetActiveBuffer(DrawSystem *self) {
    return GsGetActiveBuff();
}
void DrawSystem__LoadImage(DrawSystem *self, DrawSystemRect *src, u_long *pixels) {
    RECT rect;

    if (self->running == 0 || self->syncMode != 0) {
        ConvertRect(&rect, src);
        LoadImage(&rect, pixels);
        if (self->syncMode != 0) {
            DrawSync(0);
        }
    }
}
void ConvertRect(RECT *dst, DrawSystemRect *src) {
    dst->x = src->x;
    dst->y = src->y;
    dst->w = src->w;
    dst->h = src->h;
}
void DrawSystem__StoreImage(DrawSystem *self, u_long *pixels, DrawSystemRect *src) {
    RECT rect;

    if (self->running == 0 || self->syncMode != 0) {
        ConvertRect(&rect, src);
        StoreImage(&rect, pixels);
        if (self->syncMode != 0) {
            DrawSync(0);
        }
    }
}
s32 DrawSystem__func_80020A1C(DrawSystem *self) {
    return 0;
}
void DrawSystem__MoveImage(DrawSystem *self, DrawSystemRect *src, s16 x, s16 y) {
    RECT rect;

    ConvertRect(&rect, src);
    MoveImage(&rect, x, y);
}
void DrawSystem__RunLoop(DrawSystem *self) {
    while (self->running != 0) {
        VSync(self->unk20);
        if (self->callback != NULL) {
            self->callback();
        }
        self->methods->notifyParents(self, 2);
    }
}
void DrawSystem__CountFrames(DrawSystem *self) {
    DrawSystem *obj = GetDrawSystem();

    obj->unk24++;
    if (obj->unk24 >= obj->unk20 && obj->unkC == 0) {
        obj->unkC = 1;
        obj->unk24 = 0;
    }
}
void DrawSystem__SetVSyncCount(DrawSystem *self, s32 value) {
    if (self->running == 0) {
        self->unk20 = value;
    }
}
s32 DrawSystem__GetVSyncCount(DrawSystem *self) {
    return self->unk20;
}
void DrawSystem__ClearImage(DrawSystem *self, u8 *color, DrawSystemRect *src) {
    DrawSystemDims dims;
    RECT rect;

    if (src == NULL) {
        self->methods->getDims(self, &dims);
        self->methods->clearImage(self, color, (DrawSystemRect *)&dims);
    } else {
        ConvertRect(&rect, src);
        ClearImage(&rect, color[0], color[1], color[2]);
    }
}
DrawSystemSize *DrawSystem__GetDims(DrawSystem *self, DrawSystemDims *out) {
    if (out != NULL) {
        out->x = 0;
        out->y = 0;
        out->w = self->size.w;
        out->h = self->size.h * 2;
    }
    return &self->size;
}
void DrawSystem__SetSyncMode(DrawSystem *self, s32 value) {
    self->syncMode = value;
}
void DrawSystem__SetCallback(DrawSystem *self, void (*callback)(void)) {
    self->callback = callback;
}
DrawSystemMethods *Get_vtable_DrawSystem(void) {
    return &D_8006C070;
}
DrawSystem *GetDrawSystem(void) {
    return gDrawSystem;
}
void SetDrawSystem(DrawSystem *obj) {
    gDrawSystem = obj;
}
