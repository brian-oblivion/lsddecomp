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
typedef struct SVec_fa50 {
    s16 x, y, z, pad;
} SVec_fa50;

typedef struct Vec3_fa50 {
    s16 x, y, z;
} Vec3_fa50;

/* An axis-aligned bounding box over a vertex list. */
typedef struct Box_fa50 {
    Vec3_fa50 min;          /* +0x000 */
    Vec3_fa50 max;          /* +0x006 */
} Box_fa50;

/* A box with a leading word: func_8001F51C's local. */
typedef struct TypedBox_fa50 {
    s32 type;               /* +0x000 */
    Box_fa50 box;           /* +0x004 */
} TypedBox_fa50;

/* The eight corners of a box: v[0..3] one face, v[4..7] the other. */
typedef struct Corners_fa50 {
    Vec3_fa50 f[2][4];
} Corners_fa50;

/* A counted list of boxes' corners; func_8001F51C writes a list of one. */
typedef struct Hull_fa50 {
    s32 type;               /* +0x000, the count */
    Vec3_fa50 v[8];         /* +0x004 */
} Hull_fa50;

typedef struct HullList_fa50 {
    s32 n;                  /* +0x000 */
    Corners_fa50 c[1];      /* +0x004 */
} HullList_fa50;

typedef struct Rec28_fa50 {
    SVec_fa50 *verts;       /* +0x000 */
    s32 nverts;             /* +0x004 */
    u8 pad8[0x14];
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
    Rec28_fa50 *unk10;      /* +0x010 */
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
void func_8001F3B0(Class6BEA0 *self, Box_fa50 *box) {
    s32 i;
    s32 n;
    SVec_fa50 *v;
    s16 *miny = &box->min.y;
    s16 *minz = &box->min.z;
    s16 *maxx = &box->max.x;
    s16 *maxy = &box->max.y;
    s16 *maxz = &box->max.z;

    v = self->unk10->verts;
    n = self->unk10->nverts - 1;
    box->min.x = v->x;
    box->min.y = v->y;
    box->min.z = v->z;
    box->max = box->min;
    for (i = 0; i < n; i++) {
        v++;
        if (v->x < box->min.x) box->min.x = v->x;
        if (v->y < *miny) *miny = v->y;
        if (v->z < *minz) *minz = v->z;
        if (*maxx < v->x) *maxx = v->x;
        if (*maxy < v->y) *maxy = v->y;
        if (*maxz < v->z) *maxz = v->z;
    }
}
void func_8001F4E4(Class6BEA0 *self) {
    func_8001F3B0(self, (Box_fa50 *)D_8008B21C);
}
void *func_8001F50C(void *self, s32 i) {
    return D_8008B21C;
}
void func_8001F51C(Class6BEA0 *self, Hull_fa50 *out) {
    TypedBox_fa50 b;

    func_8001F3B0(self, &b.box);
    b.type = 1;
    out->v[0].x = b.box.min.x;
    out->v[0].y = b.box.min.y;
    out->v[0].z = b.box.min.z;
    out->v[1].x = b.box.min.x;
    out->v[1].y = b.box.max.y;
    out->v[1].z = b.box.min.z;
    out->v[2].x = b.box.max.x;
    out->v[2].y = b.box.max.y;
    out->v[2].z = b.box.min.z;
    out->v[3].x = b.box.max.x;
    out->v[3].y = b.box.min.y;
    out->v[3].z = b.box.min.z;
    out->v[4].x = b.box.min.x;
    out->v[4].y = b.box.min.y;
    out->v[4].z = b.box.max.z;
    out->v[5].x = b.box.min.x;
    out->v[5].y = b.box.max.y;
    out->v[5].z = b.box.max.z;
    out->v[6].x = b.box.max.x;
    out->v[6].y = b.box.max.y;
    out->v[6].z = b.box.max.z;
    out->v[7].x = b.box.max.x;
    out->v[7].y = b.box.min.y;
    out->v[7].z = b.box.max.z;
    out->type = 1;
}
void func_8001F66C(HullList_fa50 *h, s32 turn, s32 back, s32 d) {
    Corners_fa50 tmp;
    Corners_fa50 *c;
    s32 i;

    for (i = 0; i < h->n; i++) {
        c = &h->c[i];
        if (turn != 0) {
            tmp = *c;
            c->f[0][3] = tmp.f[0][0];
            c->f[0][2] = tmp.f[0][1];
            c->f[1][2] = tmp.f[0][2];
            c->f[1][3] = tmp.f[0][3];
            c->f[0][0] = tmp.f[1][0];
            c->f[0][1] = tmp.f[1][1];
            c->f[1][1] = tmp.f[1][2];
            c->f[1][0] = tmp.f[1][3];
            if (back == 0) {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[0][k].x += d;
                }
            } else {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[1][k].x += d;
                }
            }
        } else {
            if (back == 0) {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[0][k].z += d;
                }
            } else {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->f[1][k].z += d;
                }
            }
        }
    }
}
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_8001F8B8);
INCLUDE_ASM("asm/nonmatchings/code_fa50", func_80020050);
void func_800204D0(Outer_fa50 *self, s32 *xy) {
    Target_fa50 *t = self->unk10->unk10;

    t->unk6 += xy[0] / 16;
    t->unk6 += xy[1] * 64;
}
void func_80020510(Outer_fa50 *self, s16 *xy) {
    Target_fa50 *t = self->unk10->unk10;
    s32 v;

    v = xy[0] / 16;
    t->unk6 = v;
    t->unk6 = v + xy[1] * 64;
}
