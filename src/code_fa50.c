/*
 * code_fa50 -- GAME code carved from psyq_fa50 on 2026-09-25 (FINISHING-PLAN
 * revision 18). 0xFA50..0x10D48 (vram 0x8001F250..0x80020548). It was counted
 * as Psy-Q SDK by segment name; tools/gameinsdk.py measured it as game (a call
 * into game code, a method-table entry beside game methods, or contiguity with
 * those, and no Sony fingerprint). What it holds: the class of method table
 * D_8006BEA0 (new_class_6bea0 allocates it with BMemPMgrAlloc and chains to
 * Get_vtable_BasicClass) and helpers called only from Class6B5CC/BaseObjO
 * code. Owns jtbl_80010354 (attached rodata sub-slot 0xB54).
 *
 * Nothing here is matched yet: every function is fresh track-1 ground.
 */
#include "common.h"
#include "BasicClass.h"

/* The four words the class copies in through its slot +0x040. */
typedef struct Quad_fa50 {
    s32 w[4];
} Quad_fa50;

/* One 28-byte record of the model data slot +0x048 indexes. */
typedef struct Rec28_fa50 {
    u8 pad[0x1C];
} Rec28_fa50;

typedef struct ModelData_fa50 {
    u32 head[3];            /* +0x000; GsMapModelingData gets &head[1] */
    Rec28_fa50 recs[1];     /* +0x00C */
} ModelData_fa50;

typedef struct Class6BEA0 Class6BEA0;
typedef struct Class6BEA0Methods Class6BEA0Methods;

struct Class6BEA0Methods {
    BASICCLASS_SLOTS(Class6BEA0, (Class6BEA0 *self, void *arg));
};

struct Class6BEA0 {
    BASICCLASS_FIELDS(Class6BEA0Methods);
    ModelData_fa50 *data;   /* +0x00C */
    void *unk10;            /* +0x010 */
    Quad_fa50 quad;         /* +0x014 */
};

typedef struct Target_fa50 {
    u8 pad0[0x6];
    s16 unk6;               /* +0x006 */
} Target_fa50;

typedef struct Inner_fa50 {
    u8 pad0[0x10];
    Target_fa50 *unk10;     /* +0x010 */
} Inner_fa50;

typedef struct Outer_fa50 {
    u8 pad0[0x10];
    Inner_fa50 *unk10;      /* +0x010 */
} Outer_fa50;

extern void GsMapModelingData(unsigned long *p);
extern s32 func_8001F3B0(void *self, void *buf);
extern s32 D_8008AC4C;
extern s32 D_8008B21C[];
extern s32 D_8006BEA0[];
extern void *BMemPMgrAlloc(s32 size);
void func_8001F394(Class6BEA0 *self);
Class6BEA0Methods *func_8001F384(void);

Class6BEA0 *new_class_6bea0(void *arg) {
    Class6BEA0 *p = BMemPMgrAlloc(0x24);

    if (p != NULL) {
        func_8001F384()->ctor(p, arg);
        return p;
    }
    return NULL;
}
void func_8001F2B0(Class6BEA0 *self, void *arg) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = func_8001F384();
    self->unk10 = arg;
    self->data = (ModelData_fa50 *)((u8 *)arg - 0xC);
    func_8001F394(self);
}
void func_8001F314(Class6BEA0 *self, Quad_fa50 *src) {
    self->quad = *src;
}
void func_8001F33C(Class6BEA0 *self) {
    GsMapModelingData((unsigned long *)&self->data->head[1]);
}
Rec28_fa50 *func_8001F360(Class6BEA0 *self, s32 i) {
    return &self->data->recs[i];
}
void func_8001F37C(void) {
}
Class6BEA0Methods *func_8001F384(void) {
    return (Class6BEA0Methods *)D_8006BEA0;
}
void func_8001F394(Class6BEA0 *self) {
    D_8008AC4C = 1;
}
s32 func_8001F3A4(void *self) {
    return D_8008AC4C;
}
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F3B0);
void func_8001F4E4(void *self) {
    func_8001F3B0(self, D_8008B21C);
}
void *func_8001F50C(void *self, s32 i) {
    return D_8008B21C;
}
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F51C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F66C);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F8B8);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_80020050);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_800204D0);
void func_80020510(Outer_fa50 *self, s16 *xy) {
    Target_fa50 *t = self->unk10->unk10;
    s32 v;

    v = xy[0] / 16;
    t->unk6 = v;
    t->unk6 = v + xy[1] * 64;
}
