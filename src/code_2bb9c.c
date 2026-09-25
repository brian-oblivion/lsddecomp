/*
 * code_2bb9c -- GAME code carved from psyq_2bb9c on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0x2BB9C..0x2BF70 (vram 0x8003B39C..0x8003B770). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: 12 methods of D_8006E558, a
 * Class6D430 (data-source) subclass calling GetActiveDataSourceMethods.
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "Class6D430.h"

/* LIBGS.H GsIMAGE, laid out as the SDK declares it: GsGetTimInfo fills it
 * and func_8003B4A8 reads it field by field. */
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

typedef struct D_8006E558Obj D_8006E558Obj;

/* D_8006E558's own table: Class6D430's slots with this class's two-argument
 * ctor, then its own +0x07C..+0x09C (tools/classtable.py D_8006E558). */
typedef struct D_8006E558Methods {
    CLASS6D430_SLOTS(D_8006E558Obj, (D_8006E558Obj *self, char *name));
    /* +0x07C */ void (*slot7C)(void);
    /* +0x080 */ void (*slot80)(void);
    /* +0x084 */ void (*slot84)(void);
    /* +0x088 */ void (*slot88)(void);
    /* +0x08C */ void (*slot8C)(void);
    /* +0x090 */ void (*slot90)(void);
    /* +0x094 */ void (*slot94)(void);
    /* +0x098 */ void (*slot98)(D_8006E558Obj *self);
    /* +0x09C */ void (*getTimInfo)(D_8006E558Obj *self, GsIMAGE *tim);
} D_8006E558Methods;

/* Unit-local view of the D_8006E558 class: a Class6D430 data source whose
 * buffer holds a TIM image (func_8003B5F0 hands buffer+4, past the TIM id
 * word, to GsGetTimInfo). func_8003B4A8 has the TIM described into +0x02C
 * and uploads its pixel and CLUT blocks from there. +0x048 is cleared by the
 * ctor and set to 1 by func_8003B5E4; the ctor also clears +0x04C. The
 * object is 0x50 bytes (func_8003B39C's allocation). */
struct D_8006E558Obj {
    CLASS6D430_FIELDS(D_8006E558Methods);
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

typedef struct DrawObj DrawObj;
/* Only the two slots this unit reaches. */
typedef struct DrawObjMethods {
    /* +0x000 */ u8 pad0[0x58];
    /* +0x058 */ void (*loadImage)(DrawObj *self, DrawRect *rect, u32 *data);
    /* +0x05C */ u8 pad5C[0x8];
    /* +0x064 */ void (*slot64)(DrawObj *self, DrawRect *rect, s32 x, s32 y);
} DrawObjMethods;
struct DrawObj {
    /* +0x000 */ DrawObjMethods *methods;
};

extern DrawObj *func_80020C5C(void); /* returns the draw singleton */
extern void *BMemPMgrAlloc(s32 size);
extern Class6D430Methods *GetActiveDataSourceMethods(void);
D_8006E558Methods *func_8003B614(void);

extern D_8006E558Methods D_8006E558;

/* new D_8006E558(name). */
D_8006E558Obj *func_8003B39C(char *name) {
    D_8006E558Obj *self;

    self = BMemPMgrAlloc(0x50);
    if (self != NULL) {
        func_8003B614()->ctor(self, name);
        return self;
    }
    return NULL;
}
/* D_8006E558 +0x008: the ctor. */
void func_8003B3FC(D_8006E558Obj *self, char *name) {
    GetActiveDataSourceMethods()->ctor((Class6D430 *)self);
    self->methods = func_8003B614();
    self->unk48 = 0;
    self->unk4C = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}
/* D_8006E558 +0x00C: finalize, straight to the active driver's. */
void func_8003B470(D_8006E558Obj *self) {
    GetActiveDataSourceMethods()->finalize((Class6D430 *)self);
}
INCLUDE_ASM("asm/nonmatchings/code_2bb9c", func_8003B4A8);
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5AC(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5B4(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5BC(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5C4(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5CC(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5D4(void) {
}
/* D_8006E558 slot (tools/classtable.py); empty body. */
void func_8003B5DC(void) {
}
/* D_8006E558 +0x098. */
void func_8003B5E4(D_8006E558Obj *self) {
    self->unk48 = 1;
}
/* D_8006E558 +0x09C: describe the TIM held in the buffer. */
void func_8003B5F0(D_8006E558Obj *self, GsIMAGE *tim) {
    GsGetTimInfo((u32 *)self->buffer + 1, tim);
}
/* The class's table getter (called by func_8003B39C and func_8003B3FC). */
D_8006E558Methods *func_8003B614(void) {
    return &D_8006E558;
}
INCLUDE_ASM("asm/nonmatchings/code_2bb9c", func_8003B624);
