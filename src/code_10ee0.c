/*
 * code_10ee0 -- GAME code carved from the head of psyq_10ee0 on 2026-09-25
 * (FINISHING-PLAN revision 18). 0x10EE0..0x11474 (vram
 * 0x800206E0..0x80020C74). It was counted as Psy-Q SDK by segment name;
 * tools/gameinsdk.py measured it as game (a call into game code, a method-
 * table entry beside game methods, or contiguity with those, and no Sony
 * fingerprint). What it holds: the 19 methods of D_8006C070 (new_class_6c078
 * allocates it) and a getter/setter pair for the game gp variable D_8008A83C.
 * libgpu/sys starts right after, at ResetGraph (now psyq_11474).
 *
 * Round 81 (bravo) matched the ten small methods/accessors; round 81
 * (alpha) matched ten more (the allocator, ctor, init and the RECT/VRAM
 * helpers). func_800207DC, func_800209A0, func_80020A74 and func_80020B74
 * are still fresh track-1 ground.
 */
#include "common.h"
#include "BasicClass.h"

/*
 * Local view of the D_8006C070 object (a BasicClass subclass). Only the
 * fields this unit's matched methods touch are named; the draw singleton
 * other units reach through func_80020C5C() is (per its callers) an object
 * of this class.
 */
typedef struct Class6C070 Class6C070;
typedef struct Class6C070Methods Class6C070Methods;

/* LIBGPU.H's RECT, declared locally (never #include a psyq header). */
typedef struct {
    short x, y;
    short w, h;
} RECT;

/* A 4-short RECT source: x/y/w at +0/+2/+4, h at +8 (func_80020970). */
typedef struct {
    /* +0x0 */ s16 x;
    /* +0x2 */ s16 y;
    /* +0x4 */ s16 w;
    /* +0x6 */ s16 unk6;
    /* +0x8 */ s16 h;
} Class6C070Rect;

/* What func_80020C08 fills in: two zero shorts then the +0x14 pair. */
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
    /* +0x040 */ void (*init)(Class6C070 *self);                 /* func_80020784 */
    /* +0x044 */ void *slot44;
    /* +0x048 */ void *slot48;
    /* +0x04C */ void *slot4C;
    /* +0x050 */ void *slot50;
    /* +0x054 */ void *slot54;
    /* +0x058 */ void *slot58;
    /* +0x05C */ void *slot5C;
    /* +0x060 */ void *slot60;
    /* +0x064 */ void *slot64;
    /* +0x068 */ void (*slot68)(Class6C070 *self);               /* func_80020A74 */
    /* +0x06C */ void *slot6C;
    /* +0x070 */ void (*slot70)(Class6C070 *self, s32 value);    /* func_80020B4C */
    /* +0x074 */ void *slot74;
    /* +0x078 */ void *slot78;
    /* +0x07C */ void *slot7C;
    /* +0x080 */ void (*slot80)(Class6C070 *self, s32 value);    /* func_80020C3C */
};

struct Class6C070 {
    BASICCLASS_FIELDS(Class6C070Methods);
    /* +0x00C */ s32 unkC;            /* 80020AF4 sets to 1 once unk24 reaches unk20 */
    /* +0x010 */ s32 unk10;           /* cleared by 8002089C; 80020B4C stores unk20 only while 0 */
    /* +0x014 */ Class6C070Size size; /* 80020C08 returns its address */
    /* +0x01C */ u8 pad1C[0x20 - 0x1C];
    /* +0x020 */ s32 unk20;           /* 80020B4C sets, 80020B68 gets */
    /* +0x024 */ s32 unk24;           /* 80020AF4 counts up to unk20 */
    /* +0x028 */ u8 pad28[0x2C - 0x28];
    /* +0x02C */ s32 unk2C;           /* 80020C3C sets */
    /* +0x030 */ s32 unk30;           /* 80020C44 sets */
};

extern Class6C070Methods D_8006C070;  /* the class's method table */
extern Class6C070 *D_8008A83C;        /* sdata: the singleton func_80020C5C returns */

extern void GsSwapDispBuff(void);     /* LIBGS.H */
extern int GsGetActiveBuff(void);     /* LIBGS.H */
extern int LoadImage(RECT *rect, u_long *p);        /* LIBGPU.H */
extern int MoveImage(RECT *rect, int x, int y);     /* LIBGPU.H */
extern int DrawSync(int mode);                      /* LIBGPU.H */
extern void *BMemPMgrAlloc(s32 size);

Class6C070Methods *func_80020C4C(void);
Class6C070 *func_80020C5C(void);
void func_80020970(RECT *dst, Class6C070Rect *src);

Class6C070 *new_class_6c078(void) {
    Class6C070 *p = BMemPMgrAlloc(0x34);

    if (p != NULL) {
        func_80020C4C()->ctor(p);
        return p;
    }
    return NULL;
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020730);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020784);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800207DC);
void func_8002085C(Class6C070 *self) {
    if (self->unk10 == 0) {
        self->unk10 = 1;
        self->methods->slot68(self);
    }
}
void func_8002089C(Class6C070 *self) {
    if (self->unk10 != 0) {
        self->unk10 = 0;
    }
}
void func_800208B8(Class6C070 *self) {
    GsSwapDispBuff();
}
s32 func_800208D8(Class6C070 *self) {
    return GsGetActiveBuff();
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800208F8);
void func_80020970(RECT *dst, Class6C070Rect *src) {
    dst->x = src->x;
    dst->y = src->y;
    dst->w = src->w;
    dst->h = src->h;
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_800209A0);
s32 func_80020A1C(Class6C070 *self) {
    return 0;
}
void func_80020A24(Class6C070 *self, Class6C070Rect *src, s16 x, s16 y) {
    RECT rect;

    func_80020970(&rect, src);
    MoveImage(&rect, x, y);
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020A74);
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020AF4);
void func_80020B4C(Class6C070 *self, s32 value) {
    if (self->unk10 == 0) {
        self->unk20 = value;
    }
}
s32 func_80020B68(Class6C070 *self) {
    return self->unk20;
}
INCLUDE_ASM("asm/nonmatchings/code_10ee0", func_80020B74);
Class6C070Size *func_80020C08(Class6C070 *self, Class6C070Dims *out) {
    if (out != NULL) {
        out->x = 0;
        out->y = 0;
        out->w = self->size.w;
        out->h = self->size.h * 2;
    }
    return &self->size;
}
void func_80020C3C(Class6C070 *self, s32 value) {
    self->unk2C = value;
}
void func_80020C44(Class6C070 *self, s32 value) {
    self->unk30 = value;
}
Class6C070Methods *func_80020C4C(void) {
    return &D_8006C070;
}
Class6C070 *func_80020C5C(void) {
    return D_8008A83C;
}
void func_80020C68(Class6C070 *obj) {
    D_8008A83C = obj;
}
