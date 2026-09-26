/*
 * code_fa50 -- the TmdModel class (include/TmdModel.h; method table
 * gTmdModelMethods, class tag 9): one object of a TMD file (the "model"
 * SceneNode__LinkModel, src/code_d294_c.c, links into a GsDOBJ2). Its
 * methods map the TMD to the GS (TmdModel__MapModelingData), walk its
 * primitives (TmdModel__NextPrimitive, owns jtbl_80010354), compute an
 * axis-aligned bounding box or its eight corners (TmdModel__ComputeBounds,
 * TmdModel__GetHull, and the shared buffer of gTmdModelBoundsCount boxes,
 * TmdModel__UpdateBoundsBuffer / TmdModel__GetBoundsBuffer /
 * TmdModel__GetBoundsCount), and ray-cast a segment against every face
 * (TmdModel__RaycastFaces) for SceneNode's own collision helpers in
 * code_d294_b.c/code_d294_c.c.
 *
 * RotateAndOffsetHullList takes a hull list, not a TmdModel, and is a free
 * function; the tail of the file (AccumulateTargetOffset, SetTargetOffset)
 * is NOT TmdModel either: a separate Outer_fa50/Inner_fa50/Target_fa50
 * pointer chain with no confirmed owning class, and SetTargetOffset's one
 * caller is class_3bb8c_o.c.
 *
 * Tiers and match evidence for every function are in each function's own
 * docs/match-reports/ file.
 */
#include "common.h"
#include <libgte.h>
#include "TmdModel.h"

/* A box with a leading word: TmdModel__GetHull's local. */
typedef struct TypedBox_fa50 {
    s32 type;   /* +0x000 */
    TmdBox box; /* +0x004 */
} TypedBox_fa50;

/* The eight corners of a box: v[0..3] one face, v[4..7] the other. */
typedef struct Corners_fa50 {
    TmdVec3 f[2][4];
} Corners_fa50;

/* A counted list of boxes' corners (TmdHull is the one-box case). */
typedef struct HullList_fa50 {
    s32 n;             /* +0x000 */
    Corners_fa50 c[1]; /* +0x004 */
} HullList_fa50;

/* A segment: start and direction (end - start). */
typedef struct Ray_fa50 {
    TmdVec3 org; /* +0x000 */
    TmdVec3 dir; /* +0x006 */
} Ray_fa50;

#define ABS_fa50(x) ((x) < 0 ? ~(x) + 1 : (x))

/* The scratch VECTOR that also holds the candidate triangle's box. */
typedef union VecBox_fa50 {
    VECTOR v;
    TmdBox b;
} VecBox_fa50;

typedef struct Target_fa50 {
    u8 pad0[0x6];
    s16 offset; /* +0x006: AccumulateTargetOffset/SetTargetOffset's field */
} Target_fa50;

typedef struct Inner_fa50 {
    u8 pad0[0x10];
    Target_fa50 *target; /* +0x010 */
} Inner_fa50;

typedef struct Outer_fa50 {
    u8 pad0[0x10];
    Inner_fa50 *inner; /* +0x010 */
} Outer_fa50;

extern void GsMapModelingData(unsigned long *p);
extern TmdBox gTmdModelBoundsBuf[];
extern void *BMemPMgrAlloc(s32 size);

TmdModel *New_TmdModel(TmdObject *object) {
    TmdModel *p = BMemPMgrAlloc(sizeof(TmdModel));

    if (p != NULL) {
        Get_vtable_TmdModel()->ctor(p, object);
        return p;
    }
    return NULL;
}

void TmdModel__TmdModel(TmdModel *self, TmdObject *object) {
    Get_vtable_BasicClass()->ctor((BasicClass *)self);
    self->methods = Get_vtable_TmdModel();
    self->object = object;
    self->data = (TmdFile *)((u8 *)object - offsetof(TmdFile, objects));
    TmdModel__InitBoundsCount(self);
}

void TmdModel__SetQuad(TmdModel *self, TmdModelQuad *src) {
    self->quad = *src;
}

void TmdModel__MapModelingData(TmdModel *self) {
    GsMapModelingData((unsigned long *)&self->data->flags);
}

TmdObject *TmdModel__GetObject(TmdModel *self, s32 i) {
    return &self->data->objects[i];
}

void TmdModel__func_8001F37C(void) {}

TmdModelMethods *Get_vtable_TmdModel(void) {
    return &gTmdModelMethods;
}

void TmdModel__InitBoundsCount(TmdModel *self) {
    gTmdModelBoundsCount = 1;
}

s32 TmdModel__GetBoundsCount(TmdModel *self) {
    return gTmdModelBoundsCount;
}

void TmdModel__ComputeBounds(TmdModel *self, TmdBox *box) {
    s32 i;
    s32 n;
    TmdVertex *v;
    s16 *miny = &box->min.y;
    s16 *minz = &box->min.z;
    s16 *maxx = &box->max.x;
    s16 *maxy = &box->max.y;
    s16 *maxz = &box->max.z;

    v = self->object->verts;
    n = self->object->nverts - 1;
    box->min.x = v->x;
    box->min.y = v->y;
    box->min.z = v->z;
    box->max = box->min;
    for (i = 0; i < n; i++) {
        v++;
        if (v->x < box->min.x)
            box->min.x = v->x;
        if (v->y < *miny)
            *miny = v->y;
        if (v->z < *minz)
            *minz = v->z;
        if (*maxx < v->x)
            *maxx = v->x;
        if (*maxy < v->y)
            *maxy = v->y;
        if (*maxz < v->z)
            *maxz = v->z;
    }
}

void TmdModel__UpdateBoundsBuffer(TmdModel *self) {
    TmdModel__ComputeBounds(self, gTmdModelBoundsBuf);
}

TmdBox *TmdModel__GetBoundsBuffer(TmdModel *self, s32 i) {
    return gTmdModelBoundsBuf;
}

void TmdModel__GetHull(TmdModel *self, TmdHull *out) {
    TypedBox_fa50 b;

    TmdModel__ComputeBounds(self, &b.box);
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
    out->count = 1;
}

void RotateAndOffsetHullList(HullList_fa50 *h, s32 turn, s32 back, s32 d) {
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

/* Casts the segment origin..end against every triangle/quad of the model and
 * keeps the nearest hit: *best = its distance, *hitOut = the point, *height =
 * the point's y above the face's box. Returns whether anything was hit. The
 * VECTOR locals are scratch named by their frame slot; v60 is never used but
 * holds retail's slot, and dist is an 8-byte array because retail keeps it in
 * memory at the slot after uF0. */
s32 TmdModel__RaycastFaces(TmdModel *self, s32 *best, TmdVec3 *hitOut, s32 *height, TmdVec3 *origin,
                           TmdVec3 *end) {
    TmdVec3 tri[4];
    VECTOR plane;
    Ray_fa50 ray;
    TmdVec3 hit;
    s32 nverts;
    u32 count;
    TmdPrim *p;
    s32 found;

    count = 0;
    *best = 0x7FFFFFFF;
    ray.org.x = origin->x;
    ray.org.y = origin->y;
    ray.org.z = origin->z;
    ray.dir.x = end->x - origin->x;
    ray.dir.y = end->y - origin->y;
    ray.dir.z = end->z - origin->z;
    found = 0;
    while ((p = TmdModel__NextPrimitive(self, p, &nverts, tri, &count)) != NULL) {
        VECTOR v60;
        TmdVertex e1;
        TmdVertex e2;
        VECTOR v80;
        VECTOR v90;
        VECTOR vA0;
        VECTOR vB0;
        VECTOR vC0;
        VECTOR vD0;
        VECTOR vE0;
        VecBox_fa50 uF0;
        s32 dist[2];
        s32 frac;
        s32 q;
        s32 hi;
        s32 t;
        s16 *minx;
        s16 *miny;
        s16 *minz;
        s16 *maxx;
        s16 *maxy;
        s16 *maxz;
        TmdVec3 *v;
        s32 i;

        e1.x = tri[1].x - tri[0].x;
        e1.y = tri[1].y - tri[0].y;
        e1.z = tri[1].z - tri[0].z;
        e2.x = tri[2].x - tri[0].x;
        e2.y = tri[2].y - tri[0].y;
        e2.z = tri[2].z - tri[0].z;
        v90.vx = e1.x;
        v90.vy = e1.y;
        v90.vz = e1.z;
        vA0.vx = e2.x;
        vA0.vy = e2.y;
        vA0.vz = e2.z;
        OuterProduct0(&v90, &vA0, &v80);
        plane.vx = v80.vx;
        plane.vy = v80.vy;
        plane.vz = v80.vz;
        plane.vx /= ONE;
        plane.vy /= ONE;
        plane.vz /= ONE;
        plane.pad = -(tri[0].x * plane.vx + tri[0].y * plane.vy + tri[0].z * plane.vz);
        vC0.vx = ray.dir.x * plane.vx + ray.dir.y * plane.vy + ray.dir.z * plane.vz;
        if (ABS_fa50(vC0.vx) <= 0) {
            vC0.vy = 2;
        } else {
            vC0.vz = ray.org.x * plane.vx + ray.org.y * plane.vy + ray.org.z * plane.vz;
            vC0.vz += plane.pad;
            v80.vx = vC0.vx;
            v80.vy = 1;
            vB0.vx = -vC0.vz;
            vB0.vy = 1;
            vB0.vz = vB0.vx * v80.vy;
            vB0.pad = vB0.vy * v80.vx;
            if (ABS_fa50(vB0.pad) >= ONE) {
                vB0.vz /= ONE;
                vB0.pad /= ONE;
            }
            frac = ABS_fa50(vB0.vz % vB0.pad) << 16;
            q = vB0.vz / vB0.pad;
            if (q != 0) {
                hi = q << 16;
            } else {
                hi = (vB0.vz * vB0.pad) & 0x80000000;
            }
            t = hi | (frac / ABS_fa50(vB0.pad));
            if (t < 0) {
                vC0.vy = 0;
            } else {
                vD0.vx = ray.dir.x;
                vD0.vy = ray.dir.y;
                vD0.vz = ray.dir.z;
                vA0.vx = (ray.dir.x * t) >> 16;
                v90.vx = ray.org.x + vA0.vx;
                vA0.vx = v90.vx - ray.org.x;
                vA0.vy = (ray.dir.y * t) >> 16;
                v90.vy = ray.org.y + vA0.vy;
                vA0.vy = v90.vy - ray.org.y;
                vA0.vz = (ray.dir.z * t) >> 16;
                v90.vz = ray.org.z + vA0.vz;
                vA0.vz = v90.vz - ray.org.z;
                Square0(&vD0, &vE0);
                vC0.vx = vE0.vx + vE0.vy + vE0.vz;
                vC0.vx = SquareRoot0(vC0.vx);
                Square0(&vA0, &uF0.v);
                vC0.vz = uF0.v.vx + uF0.v.vy + uF0.v.vz;
                vC0.vz = SquareRoot0(vC0.vz);
                if (vC0.vx >= vC0.vz) {
                    dist[0] = vC0.vz;
                    vC0.vy = 1;
                    hit.x = v90.vx;
                    hit.y = v90.vy;
                    hit.z = v90.vz;
                } else {
                    vC0.vy = 0;
                }
            }
        }
        if (vC0.vy != 1) {
            continue;
        }
        minx = &uF0.b.min.x;
        miny = &uF0.b.min.y;
        minz = &uF0.b.min.z;
        maxx = &uF0.b.max.x;
        maxy = &uF0.b.max.y;
        maxz = &uF0.b.max.z;
        v = tri;
        uF0.b.min = *v;
        uF0.b.max = uF0.b.min;
        for (i = 0; i < nverts - 1; i++) {
            v++;
            if (v->x < *minx)
                *minx = v->x;
            if (v->y < *miny)
                *miny = v->y;
            if (v->z < *minz)
                *minz = v->z;
            if (*maxx < v->x)
                *maxx = v->x;
            if (*maxy < v->y)
                *maxy = v->y;
            if (*maxz < v->z)
                *maxz = v->z;
        }
        if (hit.x < uF0.b.min.x - 24 || hit.y < uF0.b.min.y - 24 || hit.z < uF0.b.min.z - 24 ||
            uF0.b.max.x + 24 < hit.x || uF0.b.max.y + 24 < hit.y || uF0.b.max.z + 24 < hit.z) {
            continue;
        }
        found = 1;
        if (dist[0] < *best) {
            *best = dist[0];
            *hitOut = hit;
            if (height != NULL) {
                *height = hit.y - uF0.b.min.y;
            }
        }
    }
    return found;
}

TmdPrim *TmdModel__NextPrimitive(TmdModel *self, TmdPrim *p, s32 *n, TmdVec3 *out, u32 *count) {
    s32 idx[4];
    TmdObject *rec = self->object;
    TmdVertex *verts;
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
    verts = self->object->verts;
    for (i = 0; i < *n; i++) {
        /* MATCHING: verts[idx[i]] and every equivalent pointer-arithmetic
         * form swap two registers and cost a word (docs/match-reports/
         * TmdModel__NextPrimitive.md, build 7). */
        out[i] = *(TmdVec3 *)((u8 *)verts + (idx[i] << 3));
    }
    (*count)++;
    return (TmdPrim *)((u8 *)p + size);
}

void AccumulateTargetOffset(Outer_fa50 *self, s32 *xy) {
    Target_fa50 *t = self->inner->target;

    t->offset += xy[0] / 16;
    t->offset += xy[1] * 64;
}

void SetTargetOffset(Outer_fa50 *self, s16 *xy) {
    Target_fa50 *t = self->inner->target;
    s32 v;

    v = xy[0] / 16;
    t->offset = v;
    t->offset = v + xy[1] * 64;
}
