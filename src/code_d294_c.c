/* code_d294_c -- the third and last carve of the Class6B5CC segment.
 *
 * Class6B5CC (method table gClass6B5CCMethods, class tag 4) is this game's
 * POSITIONED 3D OBJECT base class. Every instance embeds a Psy-Q `GsDOBJ2`
 * at +0x10 (attribute / coord2 / tmd) and owns the `GsCOORDINATE2` that
 * GsDOBJ2 points at -- MEASURED, see the "PSY-Q IDENTIFICATION" note in
 * include/code_d294.h, which pins Class6B5CCSub14 == GsCOORDINATE2 and
 * Class6B5CCSub44 == GsCOORD2PARAM field by field. DreamSys, Entity and the
 * class_3bb8c object family all carry that same layout, which is why this
 * unit's helpers are called from a dozen other units.
 *
 * NONE of this unit's functions is a vtable slot (`tools/classtable.py
 * gClass6B5CCMethods` stops at Class6B5CC__NotifyTaggedParents). It is the class's FREE-FUNCTION tail:
 * five instance helpers that dispatch through the table (`Class6B5CC__*`)
 * and eight standalone leaves -- vector, matrix, fixed-point, bounding-box
 * and bitfield primitives -- that the rest of the game calls by symbol.
 *
 * Every function in this unit is now decompiled; func_8001E7BC, the last
 * INCLUDE_ASM, matched in round 57 (docs/match-reports/func_8001E7BC.md).
 */

#include "common.h"
#include "code_d294.h"

/* Rotates a 3-element s16 vector, given in the object's own local frame,
 * by the object's own orientation, widening it into `dst`. `slot84`
 * (Class6B5CC__GetRotMatrix, code_d294_b) builds that rotation with RotMatrix from
 * GsCOORD2PARAM.rotate; its `0` argument selects the un-negated angles,
 * i.e. local -> parent, not the inverse. `dst` is a bare 3-word vector:
 * class_3bb8c_o's own call site (BaseObjO__ApplyRotatedVec14) passes a local `Vec3O`. */
void Class6B5CC__RotateLocalVector(Class6B5CCObj *self, Vec3_d294 *dst, s16 *src) {
    u8 buf[0x20];

    self->methods->getRotMatrix(self, buf, 0);
    dst->x = src[0];
    dst->y = src[1];
    dst->z = src[2];
    ApplyMatrixToLVArray(dst, dst, 1, buf);
}

/* Turns an offset given in the object's own local frame into a world
 * position: rotate `src` by the object's orientation (slot84, un-negated
 * angles, same as Class6B5CC__RotateLocalVector above) and add the object's
 * accumulated world translation. That translation is `unk14->unk38`, which
 * is GsCOORDINATE2.workm.t -- the composed world matrix's own translation
 * (+0x24 workm, +0x14 into MATRIX = +0x38). func_8001E7BC is the function
 * that maintains it, by summing coord.t down the owner chain.
 *
 * The `self->unkC != 0 ? ... : 0` ternary is recomputed before EACH of the
 * three additions rather than hoisted: retail genuinely redoes the NULL
 * test and the address computation three times. When `unkC` is NULL the
 * resulting NULL is still dereferenced, exactly as retail does. */
void Class6B5CC__LocalOffsetToWorldPos(Class6B5CCObj *self, s32 *dst, s32 *src) {
    u8 buf[0x20];
    s32 *table;

    self->methods->getRotMatrix(self, buf, 0);
    ApplyMatrixToLVArray(dst, src, 1, buf);

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[0] = dst[0] + table[0];

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[1] = dst[1] + table[1];

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    dst[2] = dst[2] + table[2];
}

/* Reads the object's three Euler angles -- GsCOORD2PARAM.rotate, i.e.
 * `unk14->unk44->vec`, in PSX-native 4096-per-turn units -- and writes them
 * as a 3-entry ratio table in DEGREES: `whole = angle * 45 >> 9`, which is
 * exactly `angle * 360 / 4096`, with `frac` (the denominator RatioToFixed12
 * divides by, below) a constant 1. The same {degrees, 1} shape
 * Class6B5CC__FaceTarget builds and updateRotation consumes.
 *
 * Statement order is load-bearing and counter-intuitive: `.whole` is
 * written BEFORE `.frac` even though retail EMITS the `frac` store first
 * (the compiler sinks the constant store into the delay slot itself).
 * See docs/match-reports/Class6B5CC__GetRotationDegrees.md. */
void Class6B5CC__GetRotationDegrees(Class6B5CCObj *self, WholeFrac_d294 *out) {
    Class6B5CCSub44 *src;

    src = self->unk14->unk44;
    out[0].whole = src->vec.x * 45 >> 9;
    out[0].frac = 1;
    out[1].whole = src->vec.y * 45 >> 9;
    out[1].frac = 1;
    out[2].whole = src->vec.z * 45 >> 9;
    out[2].frac = 1;
}

/* Attaches model data to the object and hands it to the GS. `&self->unk10`
 * is the GsDOBJ2 embedded in every Class6B5CC instance (attribute at +0x10,
 * coord2 at +0x14, tmd at +0x18), and Sony's GsLinkObject4(tmd_base, objp,
 * n) links object `n` of a TMD to it -- the `+ 0xC` skips the TMD file
 * header. `self->unk20` keeps the object the data came from.
 *
 * Retail RE-READS `self->unk20` for the call's first argument instead of
 * reusing the `other` register it stored one statement earlier; writing it
 * through the field is what matches. */
void Class6B5CC__LinkModel(Class6B5CCObj *self, GenericObj_d294 *other) {
    self->unk20 = other;
    self->unk18 = other->unk10;
    GsLinkObject4((u8 *)((GenericObj_d294 *)self->unk20)->unkC + 0xC, &self->unk10, 0);
}

/* Clears exactly the two fields Class6B5CC__LinkModel sets: the GsDOBJ2's
 * tmd pointer (+0x18) and the source object (+0x20). No GS call -- the
 * pairing with LinkModel is by construction, not by a Sony API. */
void Class6B5CC__UnlinkModel(Class6B5CCObj *self) {
    self->unk18 = 0;
    self->unk20 = 0;
}

extern s32 func_8001F8B8(void *arg0, void *arg1, void *arg2, s32 arg3, void *arg4, s16 *arg5);
extern void SubVec3S16(s32 *dest, s16 *from, s16 *to);

/* MATCHED round 57 (revisit) -- see docs/match-reports/func_8001E7BC.md.
 *
 * Kept as `func_8001E7BC` on purpose (track 3, tier C): the whole second
 * half hangs off `func_8001F8B8`, which is still undecompiled Psy-Q
 * (`psyq_fa50`), so any verb for the function as a whole would be a guess.
 * What it DOES is settled. First it maintains the object's world
 * translation: `unk14->unk38` is GsCOORDINATE2.workm.t, and the guarded
 * block rewrites it as this object's own coord.t plus every owner's
 * coord.t, walking the `self->unkC` owner list. Then it takes the target
 * point `arg2` relative to that world translation, rotates the delta into
 * the object's own frame through slotA4 (Class6B5CC__ComposeAndApplyRotation, the
 * inverse-chain matrix), and probes `func_8001F8B8` twice -- Y minus 0x400
 * and, on failure, Y plus 0x400, i.e. -90 and +90 degrees in BAM. On
 * success `arg1` receives `buf28 - buf18` and the function returns 1.
 *
 * Four source shapes here are load-bearing; each was measured against the
 * oracle and the report records what the alternatives scored.
 *
 *  - `(s32)self->unk10 < 0`: `unk10` is a `u32` bitfield word, so without
 *    the cast the comparison is constant-false and GCC deletes the whole
 *    guarded block silently (round 19).
 *  - The backup copy is ONE `Vec3_d294` struct assignment, not three
 *    scalar ones. Both spell lw/lw/lw + sw/sw/sw, but only the struct copy
 *    puts `node` in retail's $a3; the scalar form gives it $a1. That was
 *    the last residue, open since round 44.
 *  - The `self->unkC != 0 ? ... : 0` ternary is written out TWICE per axis
 *    -- once for the store, once for the load -- because retail evaluates
 *    it twice (the diamond blocks it from CSE). Caching it in a variable
 *    costs two instructions per axis. It is also cast to `Vec3_d294 *` and
 *    reached by FIELD, not indexed as `[i]`: `(cond ? p : NULL)[i]`
 *    distributes the index into both arms, which turns the NULL arm into
 *    the literal `i*4` and folds `0x38 + i*4` into one addiu, where retail
 *    keeps a constant 0x38 base and puts the axis in the load/store
 *    displacement.
 *  - The success body is written out twice, once per probe, with the bare
 *    `return 0` last and the null-`unk20` guard as an enclosing
 *    `if (self->unk20 != NULL)` rather than an early return. jump.c
 *    cross-jumps the two copies into the single `jal SubVec3S16` retail
 *    has (the giveaway is `move a0,s5` appearing in BOTH probe delay
 *    slots), and that placement is what puts the shared `v0 = 0` block
 *    after the success tail instead of before it.
 *
 * `(u8 *)node + 0x38 != NULL` is retail's own check, not a typo for
 * `node != NULL`: the disassembly forms the sum first and tests THAT. */
s32 func_8001E7BC(Class6B5CCObj *self, s32 *arg1, s32 *arg2) {
    s32 *table;
    s16 buf18[4];
    s16 delta[4];
    s16 buf28[4];
    s16 buf30[4];
    Class6B5CCSub14 *node;
    UnkOwner_d294 *cur;

    if (self->unk20 != NULL) {
        if ((s32)self->unk10 < 0 && self->unkC != NULL) {
            node = self->unk14;
            if ((u8 *)node + 0x38 != NULL) {
                *(Vec3_d294 *)node->unk38 = *(Vec3_d294 *)&node->unk18;

                cur = self->unkC;
                if (cur != NULL) {
                    do {
                        ((Vec3_d294 *)(self->unkC != 0 ? self->unk14->unk38 : (s32 *)0))->x =
                            ((Vec3_d294 *)(self->unkC != 0 ? self->unk14->unk38 : (s32 *)0))->x + cur->unk14->unk18;
                        ((Vec3_d294 *)(self->unkC != 0 ? self->unk14->unk38 : (s32 *)0))->y =
                            ((Vec3_d294 *)(self->unkC != 0 ? self->unk14->unk38 : (s32 *)0))->y + cur->unk14->unk1C;
                        ((Vec3_d294 *)(self->unkC != 0 ? self->unk14->unk38 : (s32 *)0))->z =
                            ((Vec3_d294 *)(self->unkC != 0 ? self->unk14->unk38 : (s32 *)0))->z + cur->unk14->unk20;

                        cur = cur->next;
                    } while (cur != NULL);
                }
            }
        }

        table = self->unkC != 0 ? self->unk14->unk38 : 0;
        delta[0] = (u16)arg2[0] - (u16)table[0];
        delta[1] = (u16)arg2[1] - (u16)table[1];
        delta[2] = (u16)arg2[2] - (u16)table[2];

        self->methods->composeAndApplyRotation(self, 0, buf18, delta, 1);

        delta[0] = buf18[0];
        delta[1] = (u16)buf18[1] - 0x400;
        delta[2] = buf18[2];
        if (func_8001F8B8(self->unk20, buf30, buf28, 0, buf18, delta)) {
            SubVec3S16(arg1, buf18, buf28);
            return 1;
        }
        delta[1] = (u16)buf18[1] + 0x400;
        if (func_8001F8B8(self->unk20, buf30, buf28, 0, buf18, delta)) {
            SubVec3S16(arg1, buf18, buf28);
            return 1;
        }
    }
    return 0;
}

/* dest = to - from, over three components, widening s16 inputs to s32.
 * Parameter ORDER is the subtrahend first: the value subtracted is the 2nd
 * argument, the value subtracted FROM is the 3rd. What the two vectors
 * represent is not established -- func_8001E7BC is the only known caller,
 * and it passes `buf18` (slotA4's rotated delta) as `from` and `buf28`
 * (func_8001F8B8's own output) as `to`. */
void SubVec3S16(s32 *dest, s16 *from, s16 *to) {
    dest[0] = to[0] - from[0];
    dest[1] = to[1] - from[1];
    dest[2] = to[2] - from[2];
}

/* Points the object at `target`: two ratan2 calls over `target`'s world
 * position minus `self`'s own coord translation give yaw and pitch, both
 * converted to degrees, packed into a {pitch, yaw, 0} ratio triple and
 * dispatched to updateRotation (Class6B5CC__UpdateRotation, the rotation setter that writes
 * GsCOORD2PARAM.rotate). `arg2 != 0` zeroes the pitch entry; `arg3 == 0`
 * adds 180 degrees to yaw; a non-NULL `arg4` fires a second updateRotation with the
 * caller's own table forwarded verbatim.
 *
 * `self` and `target` are used SYMMETRICALLY -- the subtraction is always
 * target minus self -- and the `arg3 == 0` half-turn is exactly the
 * correction for a caller that already swapped the two at the call site.
 * That is the mechanism behind the argument-swap correlation Entity.h
 * records; see docs/match-reports/Class6B5CC__FaceTarget.md. */
void Class6B5CC__FaceTarget(Class6B5CCObj *self, Class6B5CCObj *target, s32 arg2, s32 arg3, void *arg4) {
    s32 *pos;
    s32 *table;
    s32 dx;
    s32 dz;
    WholeFrac_d294 out[3];

    pos = &self->unk14->unk18;
    table = target->unkC != 0 ? target->unk14->unk38 : 0;

    if (table[0] != pos[0]) {
        dx = table[0] - pos[0];
        dz = table[2] - pos[2];
        out[1].whole = ratan2(dx, dz);
    } else {
        dz = table[2] - pos[2];
        out[1].whole = ratan2(1, dz);
    }

    if (table[2] != pos[2]) {
        dz = table[2] - pos[2];
        dx = table[1] - pos[1];
        out[0].whole = ratan2(dz, dx);
    } else {
        dx = table[1] - pos[1];
        out[0].whole = ratan2(1, dx);
    }

    out[0].whole = (out[0].whole + 0x400) * 360 / 4096;
    out[1].whole = out[1].whole * 360 / 4096;

    out[2].whole = 0;
    out[2].frac = 1;
    out[1].frac = 1;
    out[0].frac = 1;
    if (arg2 != 0) {
        out[0].whole = 0;
    }
    if (arg3 == 0) {
        out[1].whole = out[1].whole + 0xB4;
    }

    self->methods->updateRotation(self, 1, out);
    if (arg4 != 0) {
        self->methods->updateRotation(self, 0, arg4);
    }
}

/* `(pair->whole << 12) / pair->frac` in 20.12 fixed point, computed as a
 * split division so the shift cannot overflow: quotient and remainder from
 * one divide, then the shifted remainder divided again. GCC 2.6.3 fuses the
 * `/` and `%` over the same operands into a single `div`.
 *
 * So the pair is a RATIO, numerator over denominator -- the field names
 * `whole`/`frac` (inherited, include/code_d294.h) describe a mixed number
 * and are the weaker reading. Every producer in this unit sets the second
 * field to 1. */
s32 RatioToFixed12(void *pair) {
    WholeFrac_d294 *p;
    s32 q1, r1, q2;

    p = (WholeFrac_d294 *)pair;
    q1 = p->whole / p->frac;
    r1 = p->whole % p->frac;
    q2 = (r1 << 12) / p->frac;
    return (q1 << 12) + q2;
}

/* Cohen-Sutherland style outcode: one bit pair per axis, high bit set when
 * the point is past the box maximum and low bit when it is before the
 * minimum (x -> 8/4, y -> 2/1, z -> 0x20/0x10). Returned unmasked; callers
 * do their own `andi ..., 0xFF`. The first axis assigns rather than ORs
 * only because `flags` is provably 0 there. */
s32 CalcBoxOutcode(BoundsBox_d294 *box, Vec3S16_d294 *point) {
    s32 flags;

    flags = 0;
    if (box->hi.x < point->x) {
        flags = 8;
    } else if (point->x < box->lo.x) {
        flags = 4;
    }
    if (box->hi.y < point->y) {
        flags |= 2;
    } else if (point->y < box->lo.y) {
        flags |= 1;
    }
    if (box->hi.z < point->z) {
        flags |= 0x20;
    } else if (point->z < box->lo.z) {
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
 * reproducer in round 19). `Vec3S16_d294` is exactly that shape, and is
 * the right READING too: ApplyMatrixSV consumes SVECTORs, so the 6 bytes
 * are three s16 components, not the "32-bit value + trailing s16" the
 * former local `Rec6_d294` typedef guessed. Byte-identical either way. */
void ApplyMatrixToSVArray(void *dst, void *src, s32 count, void *m) {
    u8 *end;

    end = (u8 *)dst + count * 6;
    while ((u8 *)dst < end) {
        Vec3S16_d294 buf;

        buf = *(Vec3S16_d294 *)src;
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
 * this is also why `ApplyMatrixLV` is declared unprototyped in
 * include/code_d294.h. */
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m) {
    u8 *end;

    end = (u8 *)dst + count * 0xC;
    while ((u8 *)dst < end) {
        ApplyMatrixLV(m, src, dst);
        dst = (u8 *)dst + 0xC;
        src = (u8 *)src + 0xC;
    }
    if (0) {
        ApplyMatrixLV(m, src, dst, 0, 0, 0);
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

extern s32 D_8008A838;

/* TIER C -- deliberately still `func_`. Mechanically this is a get-and-set
 * of the global `D_8008A838` (read old, store new, return old), the shape
 * the project spells `GetSet...` elsewhere. What the global MEANS is not
 * established, so there is no noun to put in the name: its only known
 * reader is Class6B5CC__ClassifyAgainstPlanes (code_d294_b), where `D_8008A838 == 0 || outWord
 * >= 0x201` gates accepting a hit, and its only known writer is
 * class_3bb8c_l.c's func_80052F10, which passes a flag derived from a
 * stage/mode value of 3, 5 or 6. Two call sites, neither naming the thing.
 * D_8008A838 keeps its placeholder name for the same reason. */
s32 func_8001EF60(s32 value) {
    s32 old;

    old = D_8008A838;
    D_8008A838 = value;
    return old;
}
