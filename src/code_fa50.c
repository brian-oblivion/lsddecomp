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

/* One TMD primitive: a 4-byte header, then u16 words (vertex indices among
 * them, at mode-dependent positions). */
typedef struct TmdPrim_fa50 {
    u8 olen;                /* +0x000 */
    u8 ilen;                /* +0x001 */
    u8 flag;                /* +0x002 */
    u8 mode;                /* +0x003 */
    u16 h[20];              /* +0x004 */
} TmdPrim_fa50;

/* One TMD object-table entry (28 bytes). */
typedef struct Rec28_fa50 {
    SVec_fa50 *verts;       /* +0x000 */
    s32 nverts;             /* +0x004 */
    void *normals;          /* +0x008 */
    s32 nnormals;           /* +0x00C */
    TmdPrim_fa50 *prims;    /* +0x010 */
    u32 nprims;             /* +0x014 */
    s32 scale;              /* +0x018 */
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
TmdPrim_fa50 *func_80020050(Class6BEA0 *self, TmdPrim_fa50 *p, s32 *n, Vec3_fa50 *out, u32 *count) {
    s32 idx[4];
    Rec28_fa50 *rec = self->unk10;
    SVec_fa50 *verts;
    s32 size;
    s32 i;

    if (rec->nprims == 0 || *count >= rec->nprims) {
        return NULL;
    }
    if (*count == 0) {
        p = rec->prims;
    }
    *n = 4;
    switch (p->mode) {
    case 0x20:
    case 0x22:
        if (p->flag & 4) {
            idx[0] = p->h[7];
            idx[1] = p->h[8];
            idx[2] = p->h[9];
            size = 0x18;
            goto tri;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[4];
            idx[2] = p->h[5];
            size = 0x10;
            goto tri;
        }
        break;
    case 0x21:
    case 0x23:
        idx[0] = p->h[2];
        idx[1] = p->h[3];
        idx[2] = p->h[4];
        size = 0x10;
        goto tri;
    case 0x24:
    case 0x26:
        idx[0] = p->h[7];
        idx[1] = p->h[8];
        idx[2] = p->h[9];
        size = 0x18;
        goto tri;
    case 0x25:
    case 0x27:
        idx[0] = p->h[8];
        idx[1] = p->h[9];
        idx[2] = p->h[10];
        size = 0x1C;
        goto tri;
    case 0x28:
    case 0x2A:
        if (p->flag & 4) {
            idx[0] = p->h[9];
            idx[1] = p->h[10];
            idx[2] = p->h[11];
            idx[3] = p->h[12];
            size = 0x20;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[4];
            idx[2] = p->h[5];
            idx[3] = p->h[6];
            size = 0x14;
        }
        break;
    case 0x29:
    case 0x2B:
        idx[0] = p->h[2];
        idx[1] = p->h[3];
        idx[2] = p->h[4];
        idx[3] = p->h[5];
        size = 0x10;
        break;
    case 0x2C:
    case 0x2E:
        idx[0] = p->h[9];
        idx[1] = p->h[10];
        idx[2] = p->h[11];
        idx[3] = p->h[12];
        size = 0x20;
        break;
    case 0x2D:
    case 0x2F:
        idx[0] = p->h[10];
        idx[1] = p->h[11];
        idx[2] = p->h[12];
        idx[3] = p->h[13];
        size = 0x20;
        break;
    case 0x30:
    case 0x32:
        if (p->flag & 4) {
            idx[0] = p->h[7];
            idx[1] = p->h[9];
            idx[2] = p->h[11];
            size = 0x1C;
            goto tri;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[5];
            idx[2] = p->h[7];
            size = 0x14;
            goto tri;
        }
        break;
    case 0x31:
    case 0x33:
        idx[0] = p->h[6];
        idx[1] = p->h[7];
        idx[2] = p->h[8];
        size = 0x18;
        goto tri;
    case 0x34:
    case 0x36:
        idx[0] = p->h[7];
        idx[1] = p->h[9];
        idx[2] = p->h[11];
        size = 0x1C;
        goto tri;
    case 0x35:
    case 0x37:
        idx[0] = p->h[12];
        idx[1] = p->h[13];
        idx[2] = p->h[14];
        size = 0x24;
    tri:
        *n = 3;
        break;
    case 0x38:
    case 0x3A:
        if (p->flag & 4) {
            idx[0] = p->h[9];
            idx[1] = p->h[11];
            idx[2] = p->h[13];
            idx[3] = p->h[15];
            size = 0x24;
        } else {
            idx[0] = p->h[3];
            idx[1] = p->h[5];
            idx[2] = p->h[7];
            idx[3] = p->h[9];
            size = 0x18;
        }
        break;
    case 0x39:
    case 0x3B:
        idx[0] = p->h[8];
        idx[1] = p->h[9];
        idx[2] = p->h[10];
        idx[3] = p->h[11];
        size = 0x1C;
        break;
    case 0x3C:
    case 0x3E:
        idx[0] = p->h[9];
        idx[1] = p->h[11];
        idx[2] = p->h[13];
        idx[3] = p->h[15];
        size = 0x24;
        break;
    case 0x3D:
    case 0x3F:
        idx[0] = p->h[16];
        idx[1] = p->h[17];
        idx[2] = p->h[18];
        idx[3] = p->h[19];
        size = 0x2C;
        break;
    default:
        *n = 0;
        break;
    }
    verts = self->unk10->verts;
    for (i = 0; i < *n; i++) {
        out[i] = *(Vec3_fa50 *)((u8 *)verts + (idx[i] << 3));
    }
    (*count)++;
    return (TmdPrim_fa50 *)((u8 *)p + size);
}
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
