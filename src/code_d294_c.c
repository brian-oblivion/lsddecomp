/*
 * code_d294_c -- SceneNode (include/SceneNode.h), part 3 of 3: the class's
 * methods that sit in no slot, and the vector helpers the game calls by
 * symbol.
 *
 * Methods: RotateLocalVector and LocalOffsetToWorldPos (an offset in the
 * node's own frame, rotated into its parent's, or made a world position),
 * RaycastVertical (a vertical ray against the node's model), GetRotationDegrees
 * and FaceTarget (the rotation read, and aimed at another node, as Ratio16
 * degrees), LinkModel and UnlinkModel (the TmdModel whose TMD GsLinkObject4
 * links to the node's embedded GsDOBJ2).
 *
 * Helpers: SubVec3S16, RatioToFixed12 (a Ratio16 as 20.12 fixed point),
 * CalcBoxOutcode (ClipSegmentToBox's outcodes), GetSetBitField (behind the
 * attribute setters), ApplyMatrixToSVArray and ApplyMatrixToLVArray (a matrix
 * over an array of vectors), IsVec3WithinRange, and GetSetHitHeightGate (the
 * switch ClassifyAgainstPlanes reads).
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_d294.h"
#include "TmdModel.h"

/* How far RaycastVertical's ray reaches from its origin, first along -y and
 * then along +y, in the model's own units. */
#define RAYCAST_PROBE_LENGTH 1024

/* CalcBoxOutcode's bits: per axis, MAX when the point is past the box's
 * maximum and MIN when it is before its minimum. */
#define OUTCODE_Y_MIN 0x01
#define OUTCODE_Y_MAX 0x02
#define OUTCODE_X_MIN 0x04
#define OUTCODE_X_MAX 0x08
#define OUTCODE_Z_MIN 0x10
#define OUTCODE_Z_MAX 0x20

/* dst = `src`, three s16s in the node's own frame, rotated by the node's
 * rotation and widened to s32: the same offset in the parent's frame.
 * getRotMatrix's 0 asks for the un-negated angles (local to parent). */
void SceneNode__RotateLocalVector(SceneNode *self, LongVec3 *dst, s16 *src) {
    MATRIX rot;

    self->methods->getRotMatrix(self, &rot, 0);
    dst->x = src[0];
    dst->y = src[1];
    dst->z = src[2];
    ApplyMatrixToLVArray(dst, dst, 1, &rot);
}

/* dst = `src` rotated by the node's rotation (as RotateLocalVector) plus the
 * node's world position, coord2->workm.t. A node with no parent reads the
 * position through NULL.
 * MATCHING: the parent test is repeated per axis; hoisting it changes the code. */
void SceneNode__LocalOffsetToWorldPos(SceneNode *self, s32 *dst, s32 *src, s32 unused) {
    MATRIX rot;
    long *worldPos;

    self->methods->getRotMatrix(self, &rot, 0);
    ApplyMatrixToLVArray(dst, src, 1, &rot);

    worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[0] = dst[0] + worldPos[0];

    worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[1] = dst[1] + worldPos[1];

    worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[2] = dst[2] + worldPos[2];
}

/* out[i] = GsCOORD2PARAM.rotate's angle i (ONE to the turn) in degrees, over
 * 1: `* 45 >> 9` is `* 360 / ONE` reduced by 8, rounding down. The Ratio16[3]
 * shape FaceTarget builds and updateRotation takes.
 * MATCHING: num is written before den; retail's den-first order comes from the delay slot. */
void SceneNode__GetRotationDegrees(SceneNode *self, Ratio16 *out) {
    GsCOORD2PARAM *src;

    src = self->coord2->param;
    out[0].num = src->rotate.vx * 45 >> 9;
    out[0].den = 1;
    out[1].num = src->rotate.vy * 45 >> 9;
    out[1].den = 1;
    out[2].num = src->rotate.vz * 45 >> 9;
    out[2].den = 1;
}

/* Keeps `model`, a TmdModel, as the node's model, puts its TMD object in
 * GsDOBJ2.tmd, and has GsLinkObject4 link object 0 of the TMD to the node's
 * GsDOBJ2 (which starts at `attribute`).
 * MATCHING: the call re-reads self->model instead of using `model`. */
void SceneNode__LinkModel(SceneNode *self, void *model) {
    self->model = model;
    self->tmd = (s32)((TmdModel *)model)->object;
    /* Casts: Sony types tmd_base as an address (`unsigned long`), and
     * SceneNode.h spells the embedded GsDOBJ2 as four separate fields. */
    GsLinkObject4((u_long)((TmdModel *)self->model)->data->objects, (GsDOBJ2 *)&self->attribute, 0);
}

/* Clears what LinkModel set, GsDOBJ2.tmd and the model; no GS call. */
void SceneNode__UnlinkModel(SceneNode *self) {
    self->tmd = 0;
    self->model = 0;
}

extern void SubVec3S16(s32 *dest, s16 *from, s16 *to);

/* Casts a vertical ray through `target`, a world position, against the
 * node's model. On a hit, `offset` gets the hit point less `target`, both in
 * the node's own frame, and it returns 1; with no model or no hit, 0.
 *
 * A node with GsDOFF set and a parent first recomputes its world position,
 * workm.t, as its own coord.t plus every ancestor's coord.t (translation
 * only). `target` less that position is rotated into the node's frame by
 * composeAndApplyRotation, and the ray runs from there RAYCAST_PROBE_LENGTH
 * along -y, then, on a miss, along +y.
 *
 * MATCHING, each measured (the report has the alternatives):
 *  - coord.t is copied as one LongVec3 assignment, not three;
 *  - the parent ternary is written twice per axis and reached by field;
 *  - the hit branch is written once per probe, inside the model test;
 *  - `node->workm.t != NULL` tests the array's address, as retail does. */
s32 SceneNode__RaycastVertical(SceneNode *self, s32 *offset, s32 *target) {
    long *worldPos;
    SVECTOR origin;
    SVECTOR end;
    SVECTOR hit;
    s32 best;
    GsCOORDINATE2 *node;
    SceneNode *cur;

    if (self->model != NULL) {
        if ((self->attribute & GsDOFF) && self->parent != NULL) {
            node = self->coord2;
            if (node->workm.t != NULL) {
                *(LongVec3 *)node->workm.t = *(LongVec3 *)node->coord.t;

                cur = self->parent;
                if (cur != NULL) {
                    do {
                        ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->x =
                            ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->x +
                            cur->coord2->coord.t[0];
                        ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->y =
                            ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->y +
                            cur->coord2->coord.t[1];
                        ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->z =
                            ((LongVec3 *)(self->parent != 0 ? self->coord2->workm.t : (long *)0))->z +
                            cur->coord2->coord.t[2];

                        cur = cur->parent;
                    } while (cur != NULL);
                }
            }
        }

        worldPos = self->parent != 0 ? self->coord2->workm.t : 0;
        end.vx = (u16)target[0] - (u16)worldPos[0];
        end.vy = (u16)target[1] - (u16)worldPos[1];
        end.vz = (u16)target[2] - (u16)worldPos[2];

        self->methods->composeAndApplyRotation(self, 0, &origin, &end, 1);

        end.vx = origin.vx;
        end.vy = (u16)origin.vy - RAYCAST_PROBE_LENGTH;
        end.vz = origin.vz;
        if (TmdModel__RaycastFaces(self->model, &best, (TmdVec3 *)&hit, NULL, (TmdVec3 *)&origin,
                                   (TmdVec3 *)&end)) {
            SubVec3S16(offset, &origin.vx, &hit.vx);
            return 1;
        }
        end.vy = (u16)origin.vy + RAYCAST_PROBE_LENGTH;
        if (TmdModel__RaycastFaces(self->model, &best, (TmdVec3 *)&hit, NULL, (TmdVec3 *)&origin,
                                   (TmdVec3 *)&end)) {
            SubVec3S16(offset, &origin.vx, &hit.vx);
            return 1;
        }
    }
    return 0;
}

/* dest = to - from, per component, widening s16 to s32. */
void SubVec3S16(s32 *dest, s16 *from, s16 *to) {
    dest[0] = to[0] - from[0];
    dest[1] = to[1] - from[1];
    dest[2] = to[2] - from[2];
}

/* Turns the node towards `target`: from the node's coord.t to the target's
 * workm.t, yaw is ratan2(dx, dz) and pitch ratan2(dz, dy) plus a quarter
 * turn, a zero first argument passed as 1; both go to degrees and are set
 * with updateRotation as {pitch, yaw, 0}. `zeroPitch` clears the pitch,
 * `noHalfTurn == 0` adds a half turn to the yaw, and a non-NULL
 * `extraRotation` (Ratio16[3]) is then added. A target with no parent reads
 * its position through NULL. */
void SceneNode__FaceTarget(SceneNode *self, SceneNode *target, s32 zeroPitch, s32 noHalfTurn,
                           void *extraRotation) {
    long *pos;
    long *targetPos;
    s32 dx;
    s32 dy;
    s32 dz;
    Ratio16 out[3];

    pos = self->coord2->coord.t;
    targetPos = target->parent != 0 ? target->coord2->workm.t : 0;

    if (targetPos[0] != pos[0]) {
        dx = targetPos[0] - pos[0];
        dz = targetPos[2] - pos[2];
        out[1].num = ratan2(dx, dz);
    } else {
        dz = targetPos[2] - pos[2];
        out[1].num = ratan2(1, dz);
    }

    if (targetPos[2] != pos[2]) {
        dz = targetPos[2] - pos[2];
        dy = targetPos[1] - pos[1];
        out[0].num = ratan2(dz, dy);
    } else {
        dy = targetPos[1] - pos[1];
        out[0].num = ratan2(1, dy);
    }

    out[0].num = (out[0].num + ONE / 4) * 360 / ONE;
    out[1].num = out[1].num * 360 / ONE;

    out[2].num = 0;
    out[2].den = 1;
    out[1].den = 1;
    out[0].den = 1;
    if (zeroPitch != 0) {
        out[0].num = 0;
    }
    if (noHalfTurn == 0) {
        out[1].num = out[1].num + 180;
    }

    self->methods->updateRotation(self, 1, out);
    if (extraRotation != 0) {
        self->methods->updateRotation(self, 0, extraRotation);
    }
}

/* `pair` (a Ratio16) as 20.12 fixed point, from the quotient and the
 * remainder so that num * ONE cannot overflow; the / and % share one div. */
s32 RatioToFixed12(void *pair) {
    Ratio16 *p;
    s32 whole, rem, frac;

    p = (Ratio16 *)pair;
    whole = p->num / p->den;
    rem = p->num % p->den;
    frac = rem * ONE / p->den;
    return whole * ONE + frac;
}

/* The OUTCODE_* bits of `point` against `box`, 0 when it is inside. The
 * callers mask the result to a byte. */
s32 CalcBoxOutcode(TmdBox *box, TmdVec3 *point) {
    s32 flags;

    flags = 0;
    if (box->max.x < point->x) {
        flags = OUTCODE_X_MAX;
    } else if (point->x < box->min.x) {
        flags = OUTCODE_X_MIN;
    }
    if (box->max.y < point->y) {
        flags |= OUTCODE_Y_MAX;
    } else if (point->y < box->min.y) {
        flags |= OUTCODE_Y_MIN;
    }
    if (box->max.z < point->z) {
        flags |= OUTCODE_Z_MAX;
    } else if (point->z < box->min.z) {
        flags |= OUTCODE_Z_MIN;
    }
    return flags;
}

/* Replaces the `width` bits at bit `shift` of *word with `value` and returns
 * the field's old contents. The attribute setters of SceneNode, Sprite and
 * BoxFill wrap it.
 * MATCHING: the mask is built by a loop, not as (1 << width) - 1. */
u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value) {
    u32 mask;
    s32 i;
    u32 old;

    mask = 1;
    for (i = 0; i < width; i++) {
        mask <<= 1;
    }
    mask -= 1;
    mask <<= shift;
    old = (*word & mask) >> shift;
    *word = *word & ~mask;
    *word = *word | (value << shift);
    return old;
}

/* Sony's, from libgte; this SDK's <libgte.h> does not declare it. */
extern SVECTOR *ApplyMatrixSV(MATRIX *m, SVECTOR *v0, SVECTOR *v1);

/* dst[i] = m * src[i] over `count` TmdVec3s, through ApplyMatrixSV (which
 * reads and writes an SVECTOR's first three). dst == src works in place.
 * MATCHING: each element is copied through an all-s16 struct (lwl/lwr, swl/swr). */
void ApplyMatrixToSVArray(TmdVec3 *dst, TmdVec3 *src, s32 count, MATRIX *m) {
    TmdVec3 *end;

    end = dst + count;
    while (dst < end) {
        TmdVec3 buf;

        buf = *src;
        ApplyMatrixSV(m, (SVECTOR *)&buf, (SVECTOR *)dst);
        src++;
        dst++;
    }
}

/* dst[i] = m * src[i] over `count` LongVec3s, through ApplyMatrixLV.
 * dst == src works in place.
 * MATCHING: the dead six-argument call sizes retail's outgoing-argument area. */
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m) {
    LongVec3 *end;

    end = (LongVec3 *)dst + count;
    while ((LongVec3 *)dst < end) {
        ApplyMatrixLV(m, src, dst);
        dst = (LongVec3 *)dst + 1;
        src = (LongVec3 *)src + 1;
    }
    if (0) {
        /* Cast to an unprototyped function type: libgte.h's prototype
         * rejects six arguments, and only the count matters here. */
        ((void (*)())ApplyMatrixLV)(m, src, dst, 0, 0, 0);
    }
}

/* 1 when each component of `b` is within `range` of `a`'s, else 0.
 * MATCHING: the pointer bumps sit in the for's increment clause. */
s32 IsVec3WithinRange(s32 *a, s32 range, s32 *b) {
    s32 i;

    for (i = 0; i < 3; i++, a++, b++) {
        if (*b < *a - range) {
            return 0;
        }
        if (*a + range < *b) {
            return 0;
        }
    }
    return 1;
}

extern s32 gHitHeightGate;

/* Sets gHitHeightGate and returns its old value. While it is non-zero,
 * SceneNode__RaycastHullAgainstFaces's segment test accepts only a hit whose
 * height (TmdModel__RaycastFaces: above the face box's minimum y) is at least
 * 513, which its edge tests always require. ObjM__InitStyleAndWorld sets it
 * for stage 0 and stages 3, 5 and 6. */
s32 GetSetHitHeightGate(s32 value) {
    s32 old;

    old = gHitHeightGate;
    gHitHeightGate = value;
    return old;
}
