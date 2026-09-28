/*
 * TmdModel.c -- the TmdModel class (include/TmdModel.h; method table
 * gTmdModelMethods, class tag 9): one object of a TMD file (the "model"
 * SceneNode__LinkModel, src/graphics/SceneNode.c, links into a GsDOBJ2). Its
 * methods map the TMD to the GS (TmdModel__MapModelingData), walk its
 * primitives one packet at a time (TmdModel__NextPrimitive, which reads
 * each packet through Sony's own <libgs.h> layouts: GPU_COM_* mode codes,
 * TMD_P_* structs, GsTMDFlagGRD), compute an axis-aligned bounding box or
 * its eight corners (TmdModel__ComputeBounds, TmdModel__GetHull, and the
 * shared buffer of gTmdModelBoundsCount boxes, TmdModel__UpdateBoundsBuffer /
 * TmdModel__GetBoundsBuffer / TmdModel__GetBoundsCount), and ray-cast a
 * segment against every face (TmdModel__RaycastFaces) for SceneNode's own
 * collision helpers in SceneNode.c.
 *
 * RotateAndOffsetHullList is a free function over a TmdHull (a counted list
 * of box corners, the buffer Actor__NotifyMove fills through getModelHull):
 * it turns each box a quarter turn and offsets one face. The last two,
 * TmdModel__AddFirstPrimClut and TmdModel__SetFirstPrimClut, move or set the
 * CLUT id of the model's first primitive (TMD_P_TF3's clut) from a VRAM
 * position; SetStyleEffectSources (ObjMStyleActor.c) calls the second.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "TmdModel.h"

/* A counted box list of one: TmdModel__GetHull's local (its count is set
 * to 1, as the hull's is, and never read). */
typedef struct BoxList {
    s32 count;  /* +0x000 */
    TmdBox box; /* +0x004 */
} BoxList;

/* The eight corners of a box as two faces of four (TmdHull's v[0..3] and
 * v[4..7]: the min-z face, then the max-z face). */
typedef struct BoxCorners {
    TmdVec3 face[2][4];
} BoxCorners;

/* A segment: start and direction (end - start). */
typedef struct Ray {
    TmdVec3 org; /* +0x000 */
    TmdVec3 dir; /* +0x006 */
} Ray;

/* A TMD packet's mode byte is its GPU command code (<libgs.h>'s GPU_COM_*),
 * plus this bit for a semi-transparent face (libgpu's setSemiTrans bit). */
#define TMD_MODE_ABE 0x02

/* MATCHING: ~x + 1, not -x: retail negates with nor/addiu, and -x also
 * changes what CSE keeps live across the branches. */
#define ABS_fa50(x) ((x) < 0 ? ~(x) + 1 : (x))

/* One face's result in TmdModel__RaycastFaces: the segment's line meets the
 * plane but not between origin and end (t < 0, or farther than end), meets
 * it there, or is parallel to it (dir . normal is 0). */
enum RayResult { RAY_MISS = 0, RAY_HIT = 1, RAY_PARALLEL = 2 };

/* How far outside a face's bounding box (each axis, both sides) a plane hit
 * still counts as on the face. */
#define FACE_BOX_MARGIN 24

/* TmdModel__RaycastFaces' per-face scratch: Square0's output VECTOR, then
 * the face's bounding box. */
typedef union VectorOrBox {
    VECTOR v;
    TmdBox b;
} VectorOrBox;

extern TmdBox gTmdModelBoundsBuf[];
extern void *BMemPMgrAlloc(s32 size);

TmdModel *New_TmdModel(TmdObject *object) {
    TmdModel *p = BMemPMgrAlloc(sizeof(TmdModel));

    if (p != NULL) {
        GetTmdModelMethods()->ctor(p, object);
        return p;
    }
    return NULL;
}

void TmdModel__TmdModel(TmdModel *self, TmdObject *object) {
    GetBasicClassMethods()->ctor((BasicClass *)self);
    self->methods = GetTmdModelMethods();
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

TmdModelMethods *GetTmdModelMethods(void) {
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
    /* MATCHING: the five field pointers are hoisted in retail; without them
     * the function changes size. */
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
    BoxList b;

    TmdModel__ComputeBounds(self, &b.box);
    b.count = 1;
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
    out->count = 1; /* MATCHING: the literal, not b.count, which is reloaded */
}

void RotateAndOffsetHullList(TmdHull *h, s32 turn, s32 back, s32 delta) {
    BoxCorners tmp;
    BoxCorners *c;
    s32 i;

    for (i = 0; i < h->count; i++) {
        c = &((BoxCorners *)h->v)[i]; /* v[] as count boxes of eight corners */
        if (turn != 0) {
            tmp = *c;
            c->face[0][3] = tmp.face[0][0];
            c->face[0][2] = tmp.face[0][1];
            c->face[1][2] = tmp.face[0][2];
            c->face[1][3] = tmp.face[0][3];
            c->face[0][0] = tmp.face[1][0];
            c->face[0][1] = tmp.face[1][1];
            c->face[1][1] = tmp.face[1][2];
            c->face[1][0] = tmp.face[1][3];
            /* MATCHING: a k per branch; a single k at the top swaps the
             * counter and pointer registers in all four loops. */
            if (back == 0) {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->face[0][k].x += delta;
                }
            } else {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->face[1][k].x += delta;
                }
            }
        } else {
            if (back == 0) {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->face[0][k].z += delta;
                }
            } else {
                s32 k;
                for (k = 0; k < 4; k++) {
                    c->face[1][k].z += delta;
                }
            }
        }
    }
}

/* Casts the segment origin..end against every triangle/quad of the model and
 * keeps the nearest hit: *best = its distance, *hitOut = the point, *height =
 * the point's y above the face's box. Returns whether anything was hit.
 *
 * Per face: the plane through its first three vertices (normal = edge1 x
 * edge2 / ONE, d in plane.pad), the segment's parameter t on it as a 16.16
 * quotient, the hit point origin + dir * t, kept if it is no farther than
 * end and inside the face's box grown by FACE_BOX_MARGIN. The VECTOR locals
 * are reused: edge1 becomes the hit point, edge2 its offset from the origin,
 * cross the quotient's {denominator, 1}; quot is its {numerator, 1}, then
 * the cross-multiplied numerator (vz) and denominator (pad); work holds the
 * denominator then |dir| in vx, the face's RAY_* result in vy, and the
 * origin's plane distance then |offset| in vz. */
s32 TmdModel__RaycastFaces(TmdModel *self, s32 *best, TmdVec3 *hitOut, s32 *height, TmdVec3 *origin,
                           TmdVec3 *end) {
    TmdVec3 tri[4];
    VECTOR plane;
    Ray ray;
    TmdVec3 hit;
    s32 nverts;
    u32 count;
    TmdPrim *p;
    s32 found;

    count = 0;
    *best = DIST_NONE;
    ray.org.x = origin->x;
    ray.org.y = origin->y;
    ray.org.z = origin->z;
    ray.dir.x = end->x - origin->x;
    ray.dir.y = end->y - origin->y;
    ray.dir.z = end->z - origin->z;
    found = 0;
    while ((p = TmdModel__NextPrimitive(self, p, &nverts, tri, &count)) != NULL) {
        /* MATCHING: these are declared in the loop body, which puts their
         * frame slots after nverts and count. */
        VECTOR unused; /* MATCHING: never read; keeps the frame's layout */
        TmdVertex e1;
        TmdVertex e2;
        VECTOR cross;
        VECTOR edge1;
        VECTOR edge2;
        VECTOR quot;
        VECTOR work;
        VECTOR dirVec;
        VECTOR dirSq;
        VectorOrBox scratch;
        s32 dist[2]; /* MATCHING: an array, so it stays in memory (dist[1] unused) */
        s32 frac;
        s32 q;
        s32 hi;
        s32 t;
        s16 *minx; /* MATCHING: the six box-field pointers set the frame size */
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
        edge1.vx = e1.x;
        edge1.vy = e1.y;
        edge1.vz = e1.z;
        edge2.vx = e2.x;
        edge2.vy = e2.y;
        edge2.vz = e2.z;
        OuterProduct0(&edge1, &edge2, &cross);
        plane.vx = cross.vx;
        plane.vy = cross.vy;
        plane.vz = cross.vz;
        plane.vx /= ONE;
        plane.vy /= ONE;
        plane.vz /= ONE;
        plane.pad = -(tri[0].x * plane.vx + tri[0].y * plane.vy + tri[0].z * plane.vz);
        work.vx = ray.dir.x * plane.vx + ray.dir.y * plane.vy + ray.dir.z * plane.vz;
        if (ABS_fa50(work.vx) <= 0) {
            work.vy = RAY_PARALLEL;
        } else {
            work.vz = ray.org.x * plane.vx + ray.org.y * plane.vy + ray.org.z * plane.vz;
            work.vz += plane.pad;
            cross.vx = work.vx;
            cross.vy = 1;
            quot.vx = -work.vz;
            quot.vy = 1;
            quot.vz = quot.vx * cross.vy;
            quot.pad = quot.vy * cross.vx;
            if (ABS_fa50(quot.pad) >= ONE) {
                quot.vz /= ONE;
                quot.pad /= ONE;
            }
            frac = ABS_fa50(quot.vz % quot.pad) << 16;
            q = quot.vz / quot.pad;
            if (q != 0) {
                hi = q << 16;
            } else {
                hi = (quot.vz * quot.pad) & 0x80000000;
            }
            t = hi | (frac / ABS_fa50(quot.pad));
            if (t < 0) {
                work.vy = RAY_MISS;
            } else {
                dirVec.vx = ray.dir.x;
                dirVec.vy = ray.dir.y;
                dirVec.vz = ray.dir.z;
                edge2.vx = (ray.dir.x * t) >> 16;
                edge1.vx = ray.org.x + edge2.vx;
                edge2.vx = edge1.vx - ray.org.x;
                edge2.vy = (ray.dir.y * t) >> 16;
                edge1.vy = ray.org.y + edge2.vy;
                edge2.vy = edge1.vy - ray.org.y;
                edge2.vz = (ray.dir.z * t) >> 16;
                edge1.vz = ray.org.z + edge2.vz;
                edge2.vz = edge1.vz - ray.org.z;
                Square0(&dirVec, &dirSq);
                work.vx = dirSq.vx + dirSq.vy + dirSq.vz;
                work.vx = SquareRoot0(work.vx);
                Square0(&edge2, &scratch.v);
                work.vz = scratch.v.vx + scratch.v.vy + scratch.v.vz;
                work.vz = SquareRoot0(work.vz);
                if (work.vx >= work.vz) {
                    dist[0] = work.vz;
                    work.vy = RAY_HIT;
                    hit.x = edge1.vx;
                    hit.y = edge1.vy;
                    hit.z = edge1.vz;
                } else {
                    work.vy = RAY_MISS;
                }
            }
        }
        if (work.vy != RAY_HIT) {
            continue;
        }
        minx = &scratch.b.min.x;
        miny = &scratch.b.min.y;
        minz = &scratch.b.min.z;
        maxx = &scratch.b.max.x;
        maxy = &scratch.b.max.y;
        maxz = &scratch.b.max.z;
        v = tri;
        scratch.b.min = *v;
        scratch.b.max = scratch.b.min;
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
        if (hit.x < scratch.b.min.x - FACE_BOX_MARGIN || hit.y < scratch.b.min.y - FACE_BOX_MARGIN ||
            hit.z < scratch.b.min.z - FACE_BOX_MARGIN || scratch.b.max.x + FACE_BOX_MARGIN < hit.x ||
            scratch.b.max.y + FACE_BOX_MARGIN < hit.y || scratch.b.max.z + FACE_BOX_MARGIN < hit.z) {
            continue;
        }
        found = 1;
        if (dist[0] < *best) {
            *best = dist[0];
            *hitOut = hit;
            if (height != NULL) {
                *height = hit.y - scratch.b.min.y;
            }
        }
    }
    return found;
}

/* A primitive's vertex-index fields, read through Sony's layout for its
 * packet type (<libgs.h>'s TMD_P_*). */
#define PRIM(type) ((type *)p)

/* Walks the object's primitive list one packet per call: *count is the
 * cursor (0 starts at the first packet), *n gets the face's vertex count
 * (3 or 4, 0 for a packet type it does not know) and out[] its vertices.
 * Returns the packet after this one, NULL past the end. */
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
        case GPU_COM_F3:
        case GPU_COM_F3 | TMD_MODE_ABE:
            if (p->flag & GsTMDFlagGRD) {
                idx[0] = PRIM(TMD_P_F3G)->v0;
                idx[1] = PRIM(TMD_P_F3G)->v1;
                idx[2] = PRIM(TMD_P_F3G)->v2;
                size = sizeof(TMD_P_F3G);
                goto tri;
            } else {
                idx[0] = PRIM(TMD_P_F3)->v0;
                idx[1] = PRIM(TMD_P_F3)->v1;
                idx[2] = PRIM(TMD_P_F3)->v2;
                size = sizeof(TMD_P_F3);
                goto tri;
            }
            break;
        case GPU_COM_NF3:
        case GPU_COM_NF3 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_NF3)->v0;
            idx[1] = PRIM(TMD_P_NF3)->v1;
            idx[2] = PRIM(TMD_P_NF3)->v2;
            size = sizeof(TMD_P_NF3);
            goto tri;
        case GPU_COM_TF3:
        case GPU_COM_TF3 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TF3)->v0;
            idx[1] = PRIM(TMD_P_TF3)->v1;
            idx[2] = PRIM(TMD_P_TF3)->v2;
            size = sizeof(TMD_P_TF3);
            goto tri;
        case GPU_COM_NTF3:
        case GPU_COM_NTF3 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TNF3)->v0;
            idx[1] = PRIM(TMD_P_TNF3)->v1;
            idx[2] = PRIM(TMD_P_TNF3)->v2;
            size = sizeof(TMD_P_TNF3);
            goto tri;
        case GPU_COM_F4:
        case GPU_COM_F4 | TMD_MODE_ABE:
            if (p->flag & GsTMDFlagGRD) {
                /* libgs.h has no struct for a gradated flat quad: TMD_P_F4
                 * plus three colour words, so v0..v3 are h[9..12]. */
                idx[0] = p->h[9];
                idx[1] = p->h[10];
                idx[2] = p->h[11];
                idx[3] = p->h[12];
                size = 32;
            } else {
                idx[0] = PRIM(TMD_P_F4)->v0;
                idx[1] = PRIM(TMD_P_F4)->v1;
                idx[2] = PRIM(TMD_P_F4)->v2;
                idx[3] = PRIM(TMD_P_F4)->v3;
                size = sizeof(TMD_P_F4);
            }
            break;
        case GPU_COM_NF4:
        case GPU_COM_NF4 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_NF4)->v0;
            idx[1] = PRIM(TMD_P_NF4)->v1;
            idx[2] = PRIM(TMD_P_NF4)->v2;
            idx[3] = PRIM(TMD_P_NF4)->v3;
            size = sizeof(TMD_P_NF4);
            break;
        case GPU_COM_TF4:
        case GPU_COM_TF4 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TF4)->v0;
            idx[1] = PRIM(TMD_P_TF4)->v1;
            idx[2] = PRIM(TMD_P_TF4)->v2;
            idx[3] = PRIM(TMD_P_TF4)->v3;
            size = sizeof(TMD_P_TF4);
            break;
        case GPU_COM_NTF4:
        case GPU_COM_NTF4 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TNF4)->v0;
            idx[1] = PRIM(TMD_P_TNF4)->v1;
            idx[2] = PRIM(TMD_P_TNF4)->v2;
            idx[3] = PRIM(TMD_P_TNF4)->v3;
            size = sizeof(TMD_P_TNF4);
            break;
        case GPU_COM_G3:
        case GPU_COM_G3 | TMD_MODE_ABE:
            if (p->flag & GsTMDFlagGRD) {
                idx[0] = PRIM(TMD_P_G3G)->v0;
                idx[1] = PRIM(TMD_P_G3G)->v1;
                idx[2] = PRIM(TMD_P_G3G)->v2;
                size = sizeof(TMD_P_G3G);
                goto tri;
            } else {
                idx[0] = PRIM(TMD_P_G3)->v0;
                idx[1] = PRIM(TMD_P_G3)->v1;
                idx[2] = PRIM(TMD_P_G3)->v2;
                size = sizeof(TMD_P_G3);
                goto tri;
            }
            break;
        case GPU_COM_NG3:
        case GPU_COM_NG3 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_NG3)->v0;
            idx[1] = PRIM(TMD_P_NG3)->v1;
            idx[2] = PRIM(TMD_P_NG3)->v2;
            size = sizeof(TMD_P_NG3);
            goto tri;
        case GPU_COM_TG3:
        case GPU_COM_TG3 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TG3)->v0;
            idx[1] = PRIM(TMD_P_TG3)->v1;
            idx[2] = PRIM(TMD_P_TG3)->v2;
            size = sizeof(TMD_P_TG3);
            goto tri;
        case GPU_COM_NTG3:
        case GPU_COM_NTG3 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TNG3)->v0;
            idx[1] = PRIM(TMD_P_TNG3)->v1;
            idx[2] = PRIM(TMD_P_TNG3)->v2;
            size = sizeof(TMD_P_TNG3);
        /* MATCHING: every triangle case jumps to this one tail; a copy per
         * case changes the register allocation. */
        tri:
            *n = 3;
            break;
        case GPU_COM_G4:
        case GPU_COM_G4 | TMD_MODE_ABE:
            if (p->flag & GsTMDFlagGRD) {
                /* libgs.h has no struct for a gradated gouraud quad: TMD_P_G4
                 * plus three colour words, so v0..v3 are h[9], h[11], h[13], h[15]. */
                idx[0] = p->h[9];
                idx[1] = p->h[11];
                idx[2] = p->h[13];
                idx[3] = p->h[15];
                size = 36;
            } else {
                idx[0] = PRIM(TMD_P_G4)->v0;
                idx[1] = PRIM(TMD_P_G4)->v1;
                idx[2] = PRIM(TMD_P_G4)->v2;
                idx[3] = PRIM(TMD_P_G4)->v3;
                size = sizeof(TMD_P_G4);
            }
            break;
        case GPU_COM_NG4:
        case GPU_COM_NG4 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_NG4)->v0;
            idx[1] = PRIM(TMD_P_NG4)->v1;
            idx[2] = PRIM(TMD_P_NG4)->v2;
            idx[3] = PRIM(TMD_P_NG4)->v3;
            size = sizeof(TMD_P_NG4);
            break;
        case GPU_COM_TG4:
        case GPU_COM_TG4 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TG4)->v0;
            idx[1] = PRIM(TMD_P_TG4)->v1;
            idx[2] = PRIM(TMD_P_TG4)->v2;
            idx[3] = PRIM(TMD_P_TG4)->v3;
            size = sizeof(TMD_P_TG4);
            break;
        case GPU_COM_NTG4:
        case GPU_COM_NTG4 | TMD_MODE_ABE:
            idx[0] = PRIM(TMD_P_TNG4)->v0;
            idx[1] = PRIM(TMD_P_TNG4)->v1;
            idx[2] = PRIM(TMD_P_TNG4)->v2;
            idx[3] = PRIM(TMD_P_TNG4)->v3;
            size = sizeof(TMD_P_TNG4);
            break;
        default:
            *n = 0;
            break;
    }
    verts = self->object->verts;
    for (i = 0; i < *n; i++) {
        /* MATCHING: verts[idx[i]] and every equivalent pointer-arithmetic
         * form swap two registers and cost a word (its match report). */
        out[i] = *(TmdVec3 *)((u8 *)verts + (idx[i] << 3));
    }
    (*count)++;
    return (TmdPrim *)((u8 *)p + size);
}

/* Adds to the first primitive's CLUT id (every textured TMD packet has it at
 * +0x006, after tu0/tv0) the id of VRAM position (xy[0], xy[1]): x / 16 +
 * y * 64, libgpu's getClut() spelled with a division and an add. */
void TmdModel__AddFirstPrimClut(TmdModel *self, s32 *xy) {
    TMD_P_TF3 *t = (TMD_P_TF3 *)self->object->prims;

    t->clut += xy[0] / 16;
    t->clut += xy[1] * 64;
}

/* Points the first primitive's CLUT id at the CLUT at VRAM (xy[0], xy[1]). */
void TmdModel__SetFirstPrimClut(TmdModel *self, s16 *xy) {
    TMD_P_TF3 *t = (TMD_P_TF3 *)self->object->prims;
    s32 v;

    v = xy[0] / 16;
    t->clut = v;
    t->clut = v + xy[1] * 64;
}
