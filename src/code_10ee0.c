/*
 * code_10ee0 -- GAME code carved from the head of psyq_10ee0 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x10EE0..0x11474 (vram
 * 0x800206E0..0x80020C74). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the 19 methods of D_8006C070 (New_DrawSystem
 * allocates it) and a getter/setter pair for the game gp variable D_8008A83C.
 * libgpu/sys starts right after, at ResetGraph (now psyq_11474).
 *
 * Round 81 (bravo) matched the ten small methods/accessors; round 81
 * (alpha) matched ten more (the allocator, ctor, init and the RECT/VRAM
 * helpers). Round 82 (alpha) matched the last four (DrawSystem__InitGraph,
 * DrawSystem__StoreImage, DrawSystem__RunLoop, DrawSystem__ClearImage); the unit is complete.
 */
#include "common.h"
#include "BasicClass.h"

/*
 * Local view of the D_8006C070 object (a BasicClass subclass). Only the
 * fields this unit's matched methods touch are named; the draw singleton
 * other units reach through GetDrawSystem() is (per its callers) an object
 * of this class.
 */
typedef struct Class6C070 Class6C070;
typedef struct Class6C070Methods Class6C070Methods;

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
} Class6C070Rect;

/* What DrawSystem__GetDims fills in: two zero shorts then the +0x14 pair. */
typedef struct {
    /* +0x0 */ s16 x;
    /* +0x2 */ s16 y;
    /* +0x4 */ s32 w;
    /* +0x8 */ s32 h;
} Class6C070Dims;

typedef struct {
    /* +0x0 */ s32 w;
    /* +0x4 */ s32 h;
} Class6C070Size;

struct Class6C070Methods {
    BASICCLASS_SLOTS(Class6C070, (Class6C070 *self));
    /* +0x040 */ void (*init)(Class6C070 *self);                 /* DrawSystem__Init */
    /* +0x044 */ void *initGraph;                                /* DrawSystem__InitGraph */
    /* +0x048 */ void *start;                                    /* DrawSystem__Start */
    /* +0x04C */ void *stop;                                     /* DrawSystem__Stop */
    /* +0x050 */ void *swapBuffers;                               /* DrawSystem__SwapBuffers */
    /* +0x054 */ void *getActiveBuffer;                          /* DrawSystem__GetActiveBuffer */
    /* +0x058 */ void *loadImage;                                /* DrawSystem__LoadImage */
    /* +0x05C */ void *storeImage;                                /* DrawSystem__StoreImage */
    /* +0x060 */ void *slot60;                                   /* DrawSystem__func_80020A1C, always returns 0 */
    /* +0x064 */ void *moveImage;                                /* DrawSystem__MoveImage */
    /* +0x068 */ void (*runLoop)(Class6C070 *self);              /* DrawSystem__RunLoop */
    /* +0x06C */ void *countFrames;                              /* DrawSystem__CountFrames */
    /* +0x070 */ void (*setVSyncCount)(Class6C070 *self, s32 value); /* DrawSystem__SetVSyncCount */
    /* +0x074 */ void *getVSyncCount;                            /* DrawSystem__GetVSyncCount */
    /* +0x078 */ void (*clearImage)(Class6C070 *self, u8 *color, Class6C070Rect *src); /* DrawSystem__ClearImage */
    /* +0x07C */ Class6C070Size *(*getDims)(Class6C070 *self, Class6C070Dims *out); /* DrawSystem__GetDims */
    /* +0x080 */ void (*setSyncMode)(Class6C070 *self, s32 value); /* DrawSystem__SetSyncMode */
};

struct Class6C070 {
    BASICCLASS_FIELDS(Class6C070Methods);
    /* +0x00C */ s32 unkC;            /* 80020AF4 sets to 1 once unk24 reaches unk20 */
    /* +0x010 */ s32 running;         /* cleared by 8002089C; 80020B4C stores unk20 only while 0; 80020A74's while condition */
    /* +0x014 */ Class6C070Size size; /* 80020C08 returns its address */
    /* +0x01C */ s32 vramMode;        /* 800207DC: GsInitGraph vram mode */
    /* +0x020 */ s32 unk20;           /* 80020B4C sets, 80020B68 gets; also 80020A74's VSync() argument and 80020AF4's threshold -- one field, two uses, neither fully pinned down */
    /* +0x024 */ s32 unk24;           /* 80020AF4 counts up to unk20 */
    /* +0x028 */ u8 pad28[0x2C - 0x28];
    /* +0x02C */ s32 syncMode;        /* 80020C3C sets; gates the post-transfer DrawSync(0) in LoadImage/StoreImage and the running-bypass there and in ClearImage's dispatch */
    /* +0x030 */ void (*callback)(void); /* 80020C44 sets, 80020A74 calls each VSync */
};

extern Class6C070Methods D_8006C070;  /* the class's method table */
extern Class6C070 *D_8008A83C;        /* sdata: the singleton GetDrawSystem returns */

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

Class6C070Methods *Get_vtable_DrawSystem(void);
Class6C070 *GetDrawSystem(void);
void ConvertRect(RECT *dst, Class6C070Rect *src);

Class6C070 *New_DrawSystem(void) {
    Class6C070 *p = BMemPMgrAlloc(0x34);

    if (p != NULL) {
        Get_vtable_DrawSystem()->ctor(p);
        return p;
    }
    return NULL;
}
void DrawSystem__DrawSystem(Class6C070 *self) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_DrawSystem();
    self->methods->init(self);
}
void DrawSystem__Init(Class6C070 *self) {
    self->running = 0;
    self->methods->setVSyncCount(self, 3);
    self->methods->setSyncMode(self, 1);
    self->callback = NULL;
}
void DrawSystem__InitGraph(Class6C070 *self, Class6C070Size *size, s32 vramMode) {
    GsInitGraph(size->w, size->h, 0, 1, vramMode);
    GsDefDispBuff(0, 0, 0, size->h);
    self->size = *size;
    self->vramMode = vramMode;
}
void DrawSystem__Start(Class6C070 *self) {
    if (self->running == 0) {
        self->running = 1;
        self->methods->runLoop(self);
    }
}
void DrawSystem__Stop(Class6C070 *self) {
    if (self->running != 0) {
        self->running = 0;
    }
}
void DrawSystem__SwapBuffers(Class6C070 *self) {
    GsSwapDispBuff();
}
s32 DrawSystem__GetActiveBuffer(Class6C070 *self) {
    return GsGetActiveBuff();
}
void DrawSystem__LoadImage(Class6C070 *self, Class6C070Rect *src, u_long *pixels) {
    RECT rect;

    if (self->running == 0 || self->syncMode != 0) {
        ConvertRect(&rect, src);
        LoadImage(&rect, pixels);
        if (self->syncMode != 0) {
            DrawSync(0);
        }
    }
}
void ConvertRect(RECT *dst, Class6C070Rect *src) {
    dst->x = src->x;
    dst->y = src->y;
    dst->w = src->w;
    dst->h = src->h;
}
void DrawSystem__StoreImage(Class6C070 *self, u_long *pixels, Class6C070Rect *src) {
    RECT rect;

    if (self->running == 0 || self->syncMode != 0) {
        ConvertRect(&rect, src);
        StoreImage(&rect, pixels);
        if (self->syncMode != 0) {
            DrawSync(0);
        }
    }
}
s32 DrawSystem__func_80020A1C(Class6C070 *self) {
    return 0;
}
void DrawSystem__MoveImage(Class6C070 *self, Class6C070Rect *src, s16 x, s16 y) {
    RECT rect;

    ConvertRect(&rect, src);
    MoveImage(&rect, x, y);
}
void DrawSystem__RunLoop(Class6C070 *self) {
    while (self->running != 0) {
        VSync(self->unk20);
        if (self->callback != NULL) {
            self->callback();
        }
        self->methods->notifyParents(self, 2);
    }
}
void DrawSystem__CountFrames(Class6C070 *self) {
    Class6C070 *obj = GetDrawSystem();

    obj->unk24++;
    if (obj->unk24 >= obj->unk20 && obj->unkC == 0) {
        obj->unkC = 1;
        obj->unk24 = 0;
    }
}
void DrawSystem__SetVSyncCount(Class6C070 *self, s32 value) {
    if (self->running == 0) {
        self->unk20 = value;
    }
}
s32 DrawSystem__GetVSyncCount(Class6C070 *self) {
    return self->unk20;
}
void DrawSystem__ClearImage(Class6C070 *self, u8 *color, Class6C070Rect *src) {
    Class6C070Dims dims;
    RECT rect;

    if (src == NULL) {
        self->methods->getDims(self, &dims);
        self->methods->clearImage(self, color, (Class6C070Rect *)&dims);
    } else {
        ConvertRect(&rect, src);
        ClearImage(&rect, color[0], color[1], color[2]);
    }
}
Class6C070Size *DrawSystem__GetDims(Class6C070 *self, Class6C070Dims *out) {
    if (out != NULL) {
        out->x = 0;
        out->y = 0;
        out->w = self->size.w;
        out->h = self->size.h * 2;
    }
    return &self->size;
}
void DrawSystem__SetSyncMode(Class6C070 *self, s32 value) {
    self->syncMode = value;
}
void DrawSystem__SetCallback(Class6C070 *self, void (*callback)(void)) {
    self->callback = callback;
}
Class6C070Methods *Get_vtable_DrawSystem(void) {
    return &D_8006C070;
}
Class6C070 *GetDrawSystem(void) {
    return D_8008A83C;
}
void SetDrawSystem(Class6C070 *obj) {
    D_8008A83C = obj;
}
