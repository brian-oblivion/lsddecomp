/*
 * code_d294_c -- SceneNode (include/SceneNode.h), part 3 of 3: no slots.
 * The class's non-virtual methods (RotateLocalVector, LocalOffsetToWorldPos,
 * RaycastVertical, GetRotationDegrees, FaceTarget, LinkModel, UnlinkModel)
 * and the free helpers the rest of the game calls by symbol: vector and
 * matrix array transforms, RatioToFixed12, CalcBoxOutcode and
 * GetSetBitField, the packed-field accessor behind the attribute setters.
 */

#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "code_d294.h"
#include "TmdModel.h"

/* Rotates a 3-element s16 vector, given in the object's own local frame,
 * by the object's own orientation, widening it into `dst`. `slot84`
 * (SceneNode__GetRotMatrix, code_d294_b) builds that rotation with RotMatrix from
 * GsCOORD2PARAM.rotate; its `0` argument selects the un-negated angles,
 * i.e. local -> parent, not the inverse. `dst` is a bare 3-word vector:
 * class_3bb8c_o's own call site (Actor__AddLocalTranslation) passes a local `Vec3O`. */
void SceneNode__RotateLocalVector(SceneNode *self, LongVec3 *dst, s16 *src) {
    u8 buf[0x20];

    self->methods->getRotMatrix(self, buf, 0);
    dst->x = src[0];
    dst->y = src[1];
    dst->z = src[2];
    ApplyMatrixToLVArray(dst, dst, 1, buf);
}

/* Turns an offset given in the object's own local frame into a world
 * position: rotate `src` by the object's orientation (slot84, un-negated
 * angles, same as SceneNode__RotateLocalVector above) and add the object's
 * accumulated world translation. That translation is `coord2->workm.t`, which
 * is GsCOORDINATE2.workm.t -- the composed world matrix's own translation
 * (+0x24 workm, +0x14 into MATRIX = +0x38). SceneNode__RaycastVertical is the function
 * that maintains it, by summing coord.t down the owner chain.
 *
 * The `self->unkC != 0 ? ... : 0` ternary is recomputed before EACH of the
 * three additions rather than hoisted: retail genuinely redoes the NULL
 * test and the address computation three times. When `unkC` is NULL the
 * resulting NULL is still dereferenced, exactly as retail does. */
void SceneNode__LocalOffsetToWorldPos(SceneNode *self, s32 *dst, s32 *src, s32 unused) {
    u8 buf[0x20];
    long *table;

    self->methods->getRotMatrix(self, buf, 0);
    ApplyMatrixToLVArray(dst, src, 1, buf);

    table = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[0] = dst[0] + table[0];

    table = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[1] = dst[1] + table[1];

    table = self->parent != 0 ? self->coord2->workm.t : 0;
    dst[2] = dst[2] + table[2];
}

/* Reads the object's three Euler angles -- GsCOORD2PARAM.rotate, i.e.
 * `unk14->unk44->vec`, in PSX-native 4096-per-turn units -- and writes them
 * as a 3-entry ratio table in DEGREES: `whole = angle * 45 >> 9`, which is
 * exactly `angle * 360 / 4096`, with `frac` (the denominator RatioToFixed12
 * divides by, below) a constant 1. The same {degrees, 1} shape
 * SceneNode__FaceTarget builds and updateRotation consumes.
 *
 * Statement order is load-bearing and counter-intuitive: `.num` is
 * written BEFORE `.den` even though retail EMITS the `den` store first
 * (the compiler sinks the constant store into the delay slot itself).
 * See docs/match-reports/SceneNode__GetRotationDegrees.md. */
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

/* Attaches model data to the object and hands it to the GS. `&self->attribute`
 * is the GsDOBJ2 embedded in every SceneNode instance (attribute at +0x10,
 * coord2 at +0x14, tmd at +0x18), and Sony's GsLinkObject4(tmd_base, objp,
 * n) links object `n` of a TMD to it. `model` is a TmdModel
 * (include/TmdModel.h): its `data->objects` is the object table past the
 * 0xC-byte TMD header, and its `object` pointer is what GsDOBJ2.tmd gets.
 * `self->model` keeps the TmdModel.
 *
 * Retail RE-READS `self->model` for the call's first argument instead of
 * reusing the `model` register it stored one statement earlier; writing it
 * through the field is what matches. */
void SceneNode__LinkModel(SceneNode *self, void *model) {
    self->model = model;
    self->tmd = (s32)((TmdModel *)model)->object;
    /* Casts: Sony types tmd_base as an address (`unsigned long`), and
     * SceneNode.h spells the embedded GsDOBJ2 as four separate fields. */
    GsLinkObject4((u_long)((TmdModel *)self->model)->data->objects, (GsDOBJ2 *)&self->attribute, 0);
}

/* Clears exactly the two fields SceneNode__LinkModel sets: the GsDOBJ2's
 * tmd pointer (+0x18) and the source object (+0x20). No GS call -- the
 * pairing with LinkModel is by construction, not by a Sony API. */
void SceneNode__UnlinkModel(SceneNode *self) {
    self->tmd = 0;
    self->model = 0;
}

extern void SubVec3S16(s32 *dest, s16 *from, s16 *to);

/* First it maintains the object's world translation, `coord2->workm.t`:
 * the guarded block rewrites it as this object's own coord.t plus every
 * owner's coord.t, walking the `parent` chain. Then it takes the target
 * point `arg2` relative to that world translation, rotates the delta into
 * the object's own frame through slotA4 (SceneNode__ComposeAndApplyRotation, the
 * inverse-chain matrix), and probes `TmdModel__RaycastFaces` twice -- Y minus 0x400
 * and, on failure, Y plus 0x400, i.e. -90 and +90 degrees in BAM. On
 * success `arg1` receives `buf28 - buf18` and the function returns 1.
 *
 * Four source shapes here are load-bearing; each was measured against the
 * oracle and the report records what the alternatives scored.
 *
 *  - `(s32)self->unk10 < 0`: `unk10` is a `u32` bitfield word, so without
 *    the cast the comparison is constant-false and GCC deletes the whole
 *    guarded block silently (round 19).
 *  - The backup copy is ONE `LongVec3` struct assignment, not three
 *    scalar ones. Both spell lw/lw/lw + sw/sw/sw, but only the struct copy
 *    puts `node` in retail's $a3; the scalar form gives it $a1. That was
 *    the last residue, open since round 44.
 *  - The `self->unkC != 0 ? ... : 0` ternary is written out TWICE per axis
 *    -- once for the store, once for the load -- because retail evaluates
 *    it twice (the diamond blocks it from CSE). Caching it in a variable
 *    costs two instructions per axis. It is also cast to `LongVec3 *` and
 *    reached by FIELD, not indexed as `[i]`: `(cond ? p : NULL)[i]`
 *    distributes the index into both arms, which turns the NULL arm into
 *    the literal `i*4` and folds `0x38 + i*4` into one addiu, where retail
 *    keeps a constant 0x38 base and puts the axis in the load/store
 *    displacement.
 *  - The success body is written out twice, once per probe, with the bare
 *    `return 0` last and the null-`unk20` guard as an enclosing
 *    `if (self->model != NULL)` rather than an early return. jump.c
 *    cross-jumps the two copies into the single `jal SubVec3S16` retail
 *    has (the giveaway is `move a0,s5` appearing in BOTH probe delay
 *    slots), and that placement is what puts the shared `v0 = 0` block
 *    after the success tail instead of before it.
 *
 * `node->workm.t != NULL` is retail's own check, not a typo for
 * `node != NULL`: the disassembly forms &workm.t (node + 0x38) first and
 * tests THAT. */
s32 SceneNode__RaycastVertical(SceneNode *self, s32 *arg1, s32 *arg2) {
    long *table;
    s16 buf18[4];
    s16 delta[4];
    s16 buf28[4];
    s16 buf30[4];
    GsCOORDINATE2 *node;
    SceneNode *cur;

    if (self->model != NULL) {
        if ((s32)self->attribute < 0 && self->parent != NULL) {
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

        table = self->parent != 0 ? self->coord2->workm.t : 0;
        delta[0] = (u16)arg2[0] - (u16)table[0];
        delta[1] = (u16)arg2[1] - (u16)table[1];
        delta[2] = (u16)arg2[2] - (u16)table[2];

        self->methods->composeAndApplyRotation(self, 0, buf18, delta, 1);

        delta[0] = buf18[0];
        delta[1] = (u16)buf18[1] - 0x400;
        delta[2] = buf18[2];
        if (TmdModel__RaycastFaces(self->model, (s32 *)buf30, (TmdVec3 *)buf28, NULL,
                                   (TmdVec3 *)buf18, (TmdVec3 *)delta)) {
            SubVec3S16(arg1, buf18, buf28);
            return 1;
        }
        delta[1] = (u16)buf18[1] + 0x400;
        if (TmdModel__RaycastFaces(self->model, (s32 *)buf30, (TmdVec3 *)buf28, NULL,
                                   (TmdVec3 *)buf18, (TmdVec3 *)delta)) {
            SubVec3S16(arg1, buf18, buf28);
            return 1;
        }
    }
    return 0;
}

/* dest = to - from, over three components, widening s16 inputs to s32.
 * Parameter ORDER is the subtrahend first: the value subtracted is the 2nd
 * argument, the value subtracted FROM is the 3rd. What the two vectors
 * represent is not established -- SceneNode__RaycastVertical is the only known caller,
 * and it passes `buf18` (slotA4's rotated delta) as `from` and `buf28`
 * (TmdModel__RaycastFaces's own output) as `to`. */
void SubVec3S16(s32 *dest, s16 *from, s16 *to) {
    dest[0] = to[0] - from[0];
    dest[1] = to[1] - from[1];
    dest[2] = to[2] - from[2];
}

/* Points the object at `target`: two ratan2 calls over `target`'s world
 * position minus `self`'s own coord translation give yaw and pitch, both
 * converted to degrees, packed into a {pitch, yaw, 0} ratio triple and
 * dispatched to updateRotation (SceneNode__UpdateRotation, the rotation setter that writes
 * GsCOORD2PARAM.rotate). `arg2 != 0` zeroes the pitch entry; `arg3 == 0`
 * adds 180 degrees to yaw; a non-NULL `arg4` fires a second updateRotation with the
 * caller's own table forwarded verbatim.
 *
 * `self` and `target` are used SYMMETRICALLY -- the subtraction is always
 * target minus self -- and the `arg3 == 0` half-turn is exactly the
 * correction for a caller that already swapped the two at the call site.
 * That is the mechanism behind the argument-swap correlation Entity.h
 * records; see docs/match-reports/SceneNode__FaceTarget.md. */
void SceneNode__FaceTarget(SceneNode *self, SceneNode *target, s32 arg2, s32 arg3, void *arg4) {
    long *pos;
    long *table;
    s32 dx;
    s32 dz;
    Ratio16 out[3];

    pos = self->coord2->coord.t;
    table = target->parent != 0 ? target->coord2->workm.t : 0;

    if (table[0] != pos[0]) {
        dx = table[0] - pos[0];
        dz = table[2] - pos[2];
        out[1].num = ratan2(dx, dz);
    } else {
        dz = table[2] - pos[2];
        out[1].num = ratan2(1, dz);
    }

    if (table[2] != pos[2]) {
        dz = table[2] - pos[2];
        dx = table[1] - pos[1];
        out[0].num = ratan2(dz, dx);
    } else {
        dx = table[1] - pos[1];
        out[0].num = ratan2(1, dx);
    }

    out[0].num = (out[0].num + 0x400) * 360 / 4096;
    out[1].num = out[1].num * 360 / 4096;

    out[2].num = 0;
    out[2].den = 1;
    out[1].den = 1;
    out[0].den = 1;
    if (arg2 != 0) {
        out[0].num = 0;
    }
    if (arg3 == 0) {
        out[1].num = out[1].num + 0xB4;
    }

    self->methods->updateRotation(self, 1, out);
    if (arg4 != 0) {
        self->methods->updateRotation(self, 0, arg4);
    }
}

/* `(pair->num << 12) / pair->den` in 20.12 fixed point, computed as a
 * split division so the shift cannot overflow: quotient and remainder from
 * one divide, then the shifted remainder divided again. GCC 2.6.3 fuses the
 * `/` and `%` over the same operands into a single `div`.
 *
 * The pair is a ratio, numerator over denominator; every producer in this
 * unit sets `den` to 1. */
s32 RatioToFixed12(void *pair) {
    Ratio16 *p;
    s32 q1, r1, q2;

    p = (Ratio16 *)pair;
    q1 = p->num / p->den;
    r1 = p->num % p->den;
    q2 = (r1 << 12) / p->den;
    return (q1 << 12) + q2;
}

/* Cohen-Sutherland style outcode: one bit pair per axis, high bit set when
 * the point is past the box maximum and low bit when it is before the
 * minimum (x -> 8/4, y -> 2/1, z -> 0x20/0x10). Returned unmasked; callers
 * do their own `andi ..., 0xFF`. The first axis assigns rather than ORs
 * only because `flags` is provably 0 there. */
s32 CalcBoxOutcode(TmdBox *box, TmdVec3 *point) {
    s32 flags;

    flags = 0;
    if (box->max.x < point->x) {
        flags = 8;
    } else if (point->x < box->min.x) {
        flags = 4;
    }
    if (box->max.y < point->y) {
        flags |= 2;
    } else if (point->y < box->min.y) {
        flags |= 1;
    }
    if (box->max.z < point->z) {
        flags |= 0x20;
    } else if (point->z < box->min.z) {
        flags |= 0x10;
    }
    return flags;
}

/* Packed-bitfield accessor: replaces the `width` bits at bit `shift` of
 * `*word` with `value` and RETURNS the previous contents of that field,
 * shifted down to bit 0. The mask is built one bit at a time by a loop
 * rather than as `(1 << width) - 1`; that loop is retail's own shape, not
 * an artefact. Thirteen thin per-field setters across code_d294.c,
 * code_d294_b.c and code_2cc8c_f.c are wrappers around this. */
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

extern void ApplyMatrixSV(void *m, void *v0, void *v1);

/* `dst[i] = m * src[i]` for `count` elements of 6 bytes each, via Sony's
 * ApplyMatrixSV (`v1 = m * v0`, SVECTOR in and out). NOTE the parameter
 * order: the WRITE destination is the 1st argument and the read source the
 * 2nd, which is the opposite of the names this body carried before -- read
 * off the byte-exact call, `ApplyMatrixSV(m, &buf, dst)` with `buf` copied
 * out of `src`. Both of this function's call sites (code_d294_b) pass the
 * same address for both, so the asymmetry is invisible from them.
 *
 * The per-element stack copy must be a struct whose members are ALL s16:
 * that gives it alignment 2, which is what makes the whole-struct
 * assignment compile to unaligned lwl/lwr + swl/swr (the idiom in
 * DECOMPILATION_LEARNINGS, confirmed here by an isolated toolchain
 * reproducer in round 19). `TmdVec3` is exactly that shape, and is
 * the right READING too: ApplyMatrixSV consumes SVECTORs, so the 6 bytes
 * are three s16 components, not the "32-bit value + trailing s16" the
 * former local `Rec6_d294` typedef guessed. Byte-identical either way. */
void ApplyMatrixToSVArray(void *dst, void *src, s32 count, void *m) {
    u8 *end;

    end = (u8 *)dst + count * 6;
    while ((u8 *)dst < end) {
        TmdVec3 buf;

        buf = *(TmdVec3 *)src;
        ApplyMatrixSV(m, &buf, dst);
        src = (u8 *)src + 6;
        dst = (u8 *)dst + 6;
    }
}

/* `dst[i] = m * src[i]` for `count` elements of 0xC bytes each, via Sony's
 * ApplyMatrixLV (`v1 = m * v0`, VECTOR in and out) -- the long-vector
 * sibling of ApplyMatrixToSVArray above, and no unaligned-copy dance
 * needed since both arrays are read and written in place.
 *
 * The `if (0)` call is NOT dead code to delete: retail reserves 24 bytes of
 * outgoing-argument space, six words, where this function's one live call
 * needs four. GCC 2.6.3 sizes that area from every call expression's
 * argument count during RTL expansion, before dead-branch elimination, so
 * the 6-argument call in the unreachable branch is what produces retail's
 * frame. Full derivation in docs/match-reports/ApplyMatrixToLVArray.md;
 * this is also why that call goes through an unprototyped function type. */
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m) {
    u8 *end;

    end = (u8 *)dst + count * 0xC;
    while ((u8 *)dst < end) {
        ApplyMatrixLV(m, src, dst);
        dst = (u8 *)dst + 0xC;
        src = (u8 *)src + 0xC;
    }
    if (0) {
        /* Cast to an unprototyped function type: libgte.h's prototype
         * rejects six arguments, and only the count matters here. */
        ((void (*)())ApplyMatrixLV)(m, src, dst, 0, 0, 0);
    }
}

/* 1 when every component of `b` lies within +/- `range` of the matching
 * component of `a`, 0 as soon as one does not. The three pointer walks live
 * in the `for`'s own increment clause on purpose: indexing lets GCC pick
 * its own strength-reduction order and puts the wrong bump in the branch
 * delay slot. */
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

/* TIER C -- deliberately still `func_`. Mechanically this is a get-and-set
 * of the global `gHitHeightGate` (read old, store new, return old), the shape
 * the project spells `GetSet...` elsewhere. What the global MEANS is not
 * established, so there is no noun to put in the name: its only known
 * reader is SceneNode__ClassifyAgainstPlanes (code_d294_b), where `gHitHeightGate == 0 || outWord
 * >= 0x201` gates accepting a hit, and its only known writer is
 * class_3bb8c_l.c's ObjM__InitStyleAndWorld, which passes a flag derived from a
 * stage/mode value of 3, 5 or 6. Two call sites, neither naming the thing.
 * gHitHeightGate keeps its placeholder name for the same reason. */
s32 GetSetHitHeightGate(s32 value) {
    s32 old;

    old = gHitHeightGate;
    gHitHeightGate = value;
    return old;
}
