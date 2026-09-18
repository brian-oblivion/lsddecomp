/* code_d294_c -- the third and last carve of the Class6B5CC segment.
 *
 * Class6B5CC (method table D_8006B5CC, class tag 4) is this game's
 * POSITIONED 3D OBJECT base class. Every instance embeds a Psy-Q `GsDOBJ2`
 * at +0x10 (attribute / coord2 / tmd) and owns the `GsCOORDINATE2` that
 * GsDOBJ2 points at -- MEASURED, see the "PSY-Q IDENTIFICATION" note in
 * include/code_d294.h, which pins Class6B5CCSub14 == GsCOORDINATE2 and
 * Class6B5CCSub44 == GsCOORD2PARAM field by field. DreamSys, Entity and the
 * class_3bb8c object family all carry that same layout, which is why this
 * unit's helpers are called from a dozen other units.
 *
 * NONE of this unit's functions is a vtable slot (`tools/classtable.py
 * D_8006B5CC` stops at Class6B5CC__NotifyTaggedParents). It is the class's FREE-FUNCTION tail:
 * five instance helpers that dispatch through the table (`Class6B5CC__*`)
 * and eight standalone leaves -- vector, matrix, fixed-point, bounding-box
 * and bitfield primitives -- that the rest of the game calls by symbol.
 *
 * One function is still INCLUDE_ASM: func_8001E7BC, a documented stall
 * (docs/match-reports/func_8001E7BC.md).
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

    self->methods->slot84(self, buf, 0);
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

    self->methods->slot84(self, buf, 0);
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
 * Class6B5CC__FaceTarget builds and slot44 consumes.
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

#if 0
/* STALL -- see docs/match-reports/func_8001E7BC.md. Round 46 (echo): 142/180
 * words, EXACT length (no drift), first real diff at vram 0x8001E810 (a
 * register-identity residue, node in $a1 vs retail's $a3, unchanged).
 * First-ever permuter search this round found ONE real lever: swapping the
 * source order of the `delta[1]`/`delta[2]` assignments right after the
 * `slotA4` call (136 -> 142/180) -- everything else in the search's
 * best-scoring candidate was noise. Round 44's three named residues
 * (node's register identity, the 0x38-vs-0x38+i combined ADDIU, and the
 * tail's 4-register permutation) are otherwise UNCHANGED. Restored to
 * INCLUDE_ASM per project rule. */
s32 func_8001E7BC(Class6B5CCObj *self, s32 *arg1, s32 *arg2) {
    s32 *table;
    s16 buf18[4];
    s16 delta[4];
    s16 buf28[4];
    s16 buf30[4];
    Class6B5CCSub14 *node;
    UnkOwner_d294 *cur;

    if (self->unk20 == NULL) {
        return 0;
    }
    if ((s32)self->unk10 < 0 && self->unkC != NULL) {
        node = self->unk14;
        if ((u8 *)node + 0x38 != NULL) {
            node->unk38[0] = node->unk18;
            node->unk38[1] = node->unk1C;
            node->unk38[2] = node->unk20;

            cur = self->unkC;
            if (cur != NULL) {
                do {
                    (self->unkC != 0 ? &self->unk14->unk38[0] : (s32 *)0)[0] =
                        (self->unkC != 0 ? &self->unk14->unk38[0] : (s32 *)0)[0] + cur->unk14->unk18;
                    (self->unkC != 0 ? &self->unk14->unk38[1] : (s32 *)0)[0] =
                        (self->unkC != 0 ? &self->unk14->unk38[1] : (s32 *)0)[0] + cur->unk14->unk1C;
                    (self->unkC != 0 ? &self->unk14->unk38[2] : (s32 *)0)[0] =
                        (self->unkC != 0 ? &self->unk14->unk38[2] : (s32 *)0)[0] + cur->unk14->unk20;

                    cur = cur->next;
                } while (cur != NULL);
            }
        }
    }

    table = self->unkC != 0 ? self->unk14->unk38 : 0;
    delta[0] = (u16)arg2[0] - (u16)table[0];
    delta[1] = (u16)arg2[1] - (u16)table[1];
    delta[2] = (u16)arg2[2] - (u16)table[2];

    self->methods->slotA4(self, 0, buf18, delta, 1);

    delta[0] = buf18[0];
    delta[1] = (u16)buf18[1] - 0x400;
    delta[2] = buf18[2];
    if (!func_8001F8B8(self->unk20, buf30, buf28, 0, buf18, delta)) {
        delta[1] = (u16)buf18[1] + 0x400;
        if (!func_8001F8B8(self->unk20, buf30, buf28, 0, buf18, delta)) {
            return 0;
        }
    }
    SubVec3S16(arg1, buf18, buf28);
    return 1;
}
#endif

INCLUDE_ASM("asm/nonmatchings/code_d294_c", func_8001E7BC);

/* dest = to - from, over three components, widening s16 inputs to s32.
 * Parameter ORDER is the subtrahend first: the value subtracted is the 2nd
 * argument, the value subtracted FROM is the 3rd. What the two vectors
 * represent is not established -- func_8001E7BC is the only known caller
 * and it is still INCLUDE_ASM. */
void SubVec3S16(s32 *dest, s16 *from, s16 *to) {
    dest[0] = to[0] - from[0];
    dest[1] = to[1] - from[1];
    dest[2] = to[2] - from[2];
}

/* Points the object at `target`: two ratan2 calls over `target`'s world
 * position minus `self`'s own coord translation give yaw and pitch, both
 * converted to degrees, packed into a {pitch, yaw, 0} ratio triple and
 * dispatched to slot44 (func_8001CEB4, the rotation setter that writes
 * GsCOORD2PARAM.rotate). `arg2 != 0` zeroes the pitch entry; `arg3 == 0`
 * adds 180 degrees to yaw; a non-NULL `arg4` fires a second slot44 with the
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

    self->methods->slot44(self, 1, out);
    if (arg4 != 0) {
        self->methods->slot44(self, 0, arg4);
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
