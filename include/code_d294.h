#ifndef CODE_D294_H
#define CODE_D294_H

#include "common.h"
#include "Class6B5CC.h"

/* code_d294, code_d294_b, code_d294_c: the methods of Class6B5CC, whose one
 * definition (object, method table, getter, method prototypes) is
 * include/Class6B5CC.h since track 4 (round 81). This header keeps what only
 * these three units use: the bounds-box helpers' types and the prototypes of
 * the Psy-Q and utility functions the methods call.
 *
 * GetClass6B5CCMethods is a plain no-argument getter (`lui/addiu
 * %hi/%lo(gClass6B5CCMethods); jr $ra`). Several units used to call it with
 * one or two arguments through an unprototyped declaration; round 59 measured
 * every one of those arguments as zero-cost (the `jal`'s delay slot holds a
 * callee-save spill), and track 4 dropped them. */

/* ================= PSY-Q IDENTIFICATION (round 50, charlie) =================
 *
 * Class6B5CC is a POSITIONED 3D OBJECT class built directly on libgs's own
 * scene-graph types, and three of the structs below are Sony's, reached
 * under this project's own placeholder names. This is offset arithmetic
 * against include/psyq/LIBGS.H, not a resemblance argument -- every field
 * already recorded below lands where Sony's does, and the two sizes the
 * ctor allocates (0x50 and 0x28) are the two Sony struct sizes exactly.
 *
 *   Class6B5CCSub14  ==  GsCOORDINATE2   (0x50 bytes)
 *     +0x00 unk0   == flg     the "matrix needs recomputing" flag; already
 *                             documented below as a pending-update flag
 *     +0x04        == coord   MATRIX, 0x20 bytes (s16 m[3][3], pad, s32 t[3])
 *     +0x18 unk18/unk1C/unk20 == coord.t[0..2]   (+0x04 + 0x14 = +0x18)
 *     +0x24 unk24  == workm   MATRIX, the COMPOSED world matrix; already
 *                             recorded below as an address-only span
 *     +0x38 unk38[3] == workm.t[0..2]            (+0x24 + 0x14 = +0x38)
 *     +0x44 unk44  == param   GsCOORD2PARAM *
 *     +0x48 unk48  == super   the PARENT coordinate; Class6B5CC__AttachToParent's
 *                             "attach" sets it to the owner's own unk14,
 *                             which is exactly what super means
 *     +0x4C        == sub     (not yet touched by any carved function)
 *
 *   Class6B5CCSub44  ==  GsCOORD2PARAM   (0x28 bytes)
 *     +0x00 unk0/unk4/unk8 == scale.vx/vy/vz (VECTOR, +0x0C is its pad)
 *     +0x10 vec            == rotate        (SVECTOR; x/y/z/w == vx/vy/vz/pad)
 *     +0x18 pad18          == trans         (VECTOR)
 *   so S16Quad_d294 is an SVECTOR, and the 4096-per-turn angle reading
 *   Class6B5CC__UpdateRotation's full-turn wrap already established is Sony's own.
 *
 *   Class6B5CC +0x10 .. +0x1C  ==  an embedded GsDOBJ2
 *     +0x10 unk10 == attribute  (the packed flags word the GetSetBitField
 *                                family sets fields in)
 *     +0x14 unk14 == coord2     (the GsCOORDINATE2 above)
 *     +0x18 unk18 == tmd        (the model data pointer)
 *   Class6B5CC__LinkModel passes `&self->unk10` to Sony's GsLinkObject4 as
 *   its GsDOBJ2 argument, which only type-checks at this layout, and the
 *   ctor calls GsInitCoordinate2 on the 0x50-byte block.
 *
 * TRACK 4 (round 81, include/Class6B5CC.h) named the GsDOBJ2 words on the
 * object (attribute, coord2, tmd, id) and GsCOORDINATE2's super/sub, but kept
 * the project's own Class6B5CCSub14/Class6B5CCSub44 types and their field
 * names (tx/ty/tz, unk24, unk38, param, rotate): retyping them to LIBGS.H's
 * GsCOORDINATE2/GsCOORD2PARAM re-paths every accessor (`coord.t[0]` for `tx`,
 * `workm.t` for `unk38`) in the class's units and in the subclass views that
 * reach these blocks. A later pass can do it with the offsets above.
 * ========================================================================= */

/* Round 13 (BisectSegmentToBox): an axis-aligned bounding box, low corner then
 * high corner -- MEASURED from BisectSegmentToBox's own field offsets
 * (+0x0/+0x2/+0x4 = lo.x/y/z, +0x6/+0x8/+0xA = hi.x/y/z). */
typedef struct BoundsBox_d294 {
    Vec3S16_d294 lo;
    Vec3S16_d294 hi;
} BoundsBox_d294;

/* Round 13 (Class6B5CC__CheckBoundsOverlap): a 12-byte, all-s16, 6-field record -- MEASURED,
 * same all-s16-struct-copy idiom as Vec3S16_d294 (whole-value assignment
 * compiles to unaligned lwl/lwr). Used as TmdModel__GetBoundsBuffer's own return-array
 * element type and as this function's own second running-tracker. Round 73
 * (the match): the pairing is NOT scrambled -- TmdModel__GetBoundsBuffer's records are
 * BoundsBox_d294 boxes (f0..f2 = lo, f3..f5 = hi), the matched body reads
 * them through that type, and its tail is an ordinary per-axis AABB overlap
 * (docs/match-reports/Class6B5CC__CheckBoundsOverlap.md). This view is kept
 * only for any remaining accessor; track 4 folds it into BoundsBox_d294. */
typedef struct Sixteen6_d294 {
    s16 f0;
    s16 f1;
    s16 f2;
    s16 f3;
    s16 f4;
    s16 f5;
} Sixteen6_d294;

/* Round 13 (Class6B5CC__CheckBoundsOverlap): `arg1`'s own struct -- a count followed by the
 * FIRST corner (`hdr`), with `count*8 - 1` more Vec3S16_d294 corners
 * immediately after (stride 6, walked by raw pointer arithmetic since a
 * C89 flexible array member isn't available). MEASURED: `count*48` is the
 * byte span from `&hdr` to the array's end, i.e. 8 corners per `count`. */
typedef struct CornerList_d294 CornerList_d294;
struct CornerList_d294 {
    s32 count;         /* +0x000 */
    Vec3S16_d294 hdr;  /* +0x004, corner[0]; corner[1..] follow at +0x00A */
};

extern void *BMemPMgrAlloc(s32 size);
extern void BMemPMgrFree(void *arg);

/* RatioToFixed12 (round 54 correction: this banner was STALE -- it is
 * now carved and MATCHED in src/code_d294_c.c, not code_d294_b, and the
 * nop_mflo_mfhi toolchain flag it was once blocked on is RESOLVED per
 * CLAUDE.md's "Open toolchain blockers" table; see
 * docs/match-reports/RatioToFixed12.md for the current history): reads
 * a `WholeFrac_d294` at the
 * given pointer and returns a 20.12 fixed-point value (`whole << 12 |
 * frac`'s own division-derived low bits) -- read off its own
 * disassembly (a `div` by the pair's own two fields, not decompiled
 * here). `Class6B5CC__UpdateScale`/`Class6B5CC__UpdateRotation` (both matched, this unit) apply
 * it three times in a row, at offsets +0x0/+0x4/+0x8 of their own 3rd
 * argument. Declared here with a `void *` argument since this unit's
 * chosen functions only ever pass the pointer through, never dereference
 * the pair themselves. */
extern s32 RatioToFixed12(void *pair);

/* ROTATION_ZERO/SCALE_ONE (rodata): two 3-entry, 0xC-byte tables in the shape
 * RatioToFixed12 reads (see above) -- Class6B5CC__Reset's own literal `data`
 * arguments to slot +0x044/+0x048. Declared as opaque byte blobs since
 * nothing this unit's chosen functions read out of them directly (only
 * their address is taken and forwarded). */
extern u8 ROTATION_ZERO[0xC];
extern u8 SCALE_ONE[0xC];

/* GsInitCoordinate2 (Psy-Q libgs/matrix, linked from Sony's object, not game
 * code): Class6B5CC__Reset's call; declared with the shape it needs. */
extern void GsInitCoordinate2(s32 arg0, void *dest);


/* The model Class6B5CC keeps at +0x020 is a TmdModel (code_fa50). Its
 * methods -- TmdModel__GetHull, TmdModel__GetBoundsCount,
 * TmdModel__UpdateBoundsBuffer/GetBoundsBuffer, TmdModel__RaycastFaces --
 * are declared once, in include/TmdModel.h, which code_d294_b.c and
 * code_d294_c.c include themselves (track 4, round 87). */

/* GsLinkObject4 (psyq_GsLinkObject4.s, Psy-Q library, not game code; symbol
 * address per config/symbols.slps01556.lsdde.txt, 0x8001EF70 -- the first
 * function of that segment, immediately after code_d294_c's own tail).
 * Class6B5CC__LinkModel (code_d294_c) calls it as
 * `GsLinkObject4((u8 *)other->unkC + 0xC, &self->unk10, 0)`; declared only
 * with the opaque `void *`/`s32` shape that call site needs. */
extern void GsLinkObject4(void *arg0, void *arg1, s32 arg2);

/* ApplyMatrixToSVArray (src/code_d294_c.c; MATCHED round 19, echo -- see
 * docs/match-reports/ApplyMatrixToSVArray.md): `dst[i] = m * src[i]` for
 * `count` elements of 6 bytes each. Each iteration copies one element out
 * of `src` into an all-s16 stack local (alignment 2, which is what makes
 * retail's unaligned lwl/lwr + swl/swr copy come out) and forwards it to
 * Sony's `ApplyMatrixSV(m, &buf, dst)` -- so the 1st parameter is the
 * WRITE destination and the 2nd the read source, confirmed against the
 * byte-exact disassembly. `Class6B5CC__TransformAndNotifyParents` (code_d294_b) calls it with both
 * equal to the SAME address, which is why the asymmetry was invisible
 * until this function was actually matched; round 50 renamed the
 * parameters (names only) to say which is which. Declared with the opaque
 * shape its callers need. */
extern void ApplyMatrixToSVArray(void *dst, void *src, s32 count, void *m);

/* ApplyMatrixLV (still uncarved, a different/earlier segment): called once
 * per iteration by ApplyMatrixToLVArray below as `(fixed, b, a)`; not decompiled
 * here, declared only with the opaque shape that call site needs.
 *
 * DELIBERATELY UNPROTOTYPED (round 19, echo -- see
 * docs/match-reports/ApplyMatrixToLVArray.md's "MATCHED" section for the full
 * derivation): ApplyMatrixToLVArray's own outgoing-argument stack reservation is
 * 24 bytes (six words), not the 16-byte/three-word minimum its one LIVE
 * call site needs. An isolated reproducer under the pinned toolchain
 * confirmed retail's exact bytes -- frame size, live call site, AND the
 * unreachable-code source shape -- only when `ApplyMatrixLV` is called
 * BOTH with 3 live arguments (this unit's real call, inside the loop) AND
 * with 6 arguments inside a `if (0) { ... }` dead branch elsewhere in the
 * SAME function (GCC 2.6.3 sizes the outgoing-arg area from every call
 * expression's arg count during RTL expansion, before the dead branch is
 * eliminated -- so the frame remembers an arg count the emitted code
 * never uses). A K&R/unprototyped declaration is required for this: an
 * ANSI prototype would make the mismatched-arity calls a compile error. */
extern void ApplyMatrixLV(); /* arity-ok: deliberately unprototyped (round 19, see above); the real callee takes 3 (include/class_3bb8c.h), the 6-argument `if (0)` call here only sizes the outgoing-arg area */

/* ApplyMatrixToLVArray (this unit, round 14; MATCHED round 19, echo -- see
 * docs/match-reports/ApplyMatrixToLVArray.md): a paired-array iteration sibling
 * to ApplyMatrixToSVArray above -- `count` iterations, 0xC bytes/element (no
 * unaligned-load complication this time, both `a`/`b` are read directly),
 * calling `ApplyMatrixLV(fixed, b, a)` once per element and advancing both
 * `a`/`b` by 0xC each time while `fixed` stays constant across every call.
 * Class6B5CC__RotateLocalVector (this unit, round 14) calls it as `ApplyMatrixToLVArray(dst, dst,
 * 1, &buf)` where `dst` is Class6B5CC__RotateLocalVector's own 2nd argument and `buf` is a
 * 0xC-byte stack vector `self->methods`'s own `slot84` just filled in --
 * i.e. `a`/`b` here are the SAME pointer at that one call site, so it does
 * not by itself distinguish their roles. */
/* CHANGED round 50 (charlie), parameter NAMES only -- no type, arity or
 * order change: (a, b, count, fixed) -> (dst, src, count, m). `a` is the
 * WRITE destination and `b` the read source, per the byte-exact call
 * `ApplyMatrixLV(m, src, dst)` inside the loop. The same correction
 * applies to ApplyMatrixToSVArray's declaration above. */
void ApplyMatrixToLVArray(void *dst, void *src, s32 count, void *m);

/* GetSetBitField (round 54 correction: this banner was STALE -- it is
 * now carved and MATCHED in src/code_d294_c.c, not code_d294_b): a
 * generic packed-bitfield accessor. Given a word pointer, a bit SHIFT, a
 * bit WIDTH and a VALUE, it clears WIDTH bits at bit-offset SHIFT in *word,
 * ORs in (value << shift), and returns the PREVIOUS contents of that
 * bitfield (shifted back down to bit 0). MEASURED from its own disassembly
 * (asm/code_d294_b.s @ GetSetBitField): a `while` loop builds `(1 << width)
 * - 1` one bit at a time (i.e. computes a WIDTH-bit mask, not a
 * `(1<<width)-1` closed form -- retail's own source apparently spelled it
 * as the loop), then shifts that mask into position, clears/sets, and
 * shifts the old value back down. Five of this unit's own functions
 * (Class6B5CC__SetDisplay/D374/D3A0/D3CC/D3F8) are thin wrappers around this,
 * always over `&self->unk10`, at five non-overlapping bit positions
 * (shift 3 width 3, shift 6 width 1, shift 28 width 2, shift 30 width 1,
 * shift 31 width 1) -- i.e. self->unk10 is a packed flags/small-fields
 * register and these five functions are its per-field setters. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

/* RotMatrix (Psy-Q libgte/fgo_01, linked from Sony's object, not game
 * code): takes an
 * s16-quad-shaped pointer (this call site's own S16Quad_d294) and a 2nd
 * argument this unit's own caller passes straight through, unexamined.
 * $v0 is never read after this call site's own `jal`, so declared void
 * here -- other units may see a different arity/return, same per-call-site
 * precedent as GetClass6B5CCMethods above. */
extern void RotMatrix(S16Quad_d294 *vec, s32 a1);

/* MulMatrix2 (Psy-Q libgte/mtx, linked from Sony's object, not game
 * code): loads its
 * own arg0 into GTE control regs 0-4 via `ctc2` (5 words, the packed
 * MATRIX rotation part) then combines it with arg1 -- a matrix-compose
 * primitive (PsyQ's `CompMatrix` family). Class6B5CC__ComposeAndApplyRotation (round 13, this
 * unit) calls it as `MulMatrix2(buf2, buf1)`, both 0x20-byte opaque
 * local buffers; declared only with that shape. */
extern void MulMatrix2(void *arg0, void *arg1);

void BisectSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *near, Vec3S16_d294 *far);

/* CalcBoxOutcode (round 54 correction: this banner was STALE -- it is
 * now carved and MATCHED in src/code_d294_c.c):
 * computes the SAME 6-bit box-vs-point outcode BisectSegmentToBox's own `flags`
 * computation does (bit-for-bit identical comparison chain against the
 * same 6 field offsets) -- MEASURED, not guessed; this is the shared
 * primitive both functions build on. Returns the accumulated flags in
 * `$v0` unmasked (the mask is the CALLER's job, per ClipSegmentToBox's own
 * repeated `andi ...,0xFF` every time it re-reads a stored result). */
extern s32 CalcBoxOutcode(BoundsBox_d294 *box, Vec3S16_d294 *point);

s32 ClipSegmentToBox(Vec3S16_d294 *out, BoundsBox_d294 *box, Vec3S16_d294 *p1, Vec3S16_d294 *p2);

/* ratan2 (Psy-Q library, not game code; symbol address per
 * config/symbols.slps01556.lsdde.txt, 0x8001F0C8): arctangent of
 * (dy, dx) in PSX-native 4096-per-circle BAM units, matching every other
 * angle representation this unit's own functions already use (see
 * `WholeFrac_d294` above and `Class6B5CC__GetRotationDegrees`'s degree conversion). Real
 * argument order confirmed from Class6B5CC__FaceTarget's own two call sites,
 * below. */
extern s32 ratan2(s32 dy, s32 dx);

/* Class6B5CC__FaceTarget (prototype in include/Class6B5CC.h; round 14, this unit -- the SAME external symbol
 * Entity_b/c/d/e.c call via their own separate `Entity.h` declaration,
 * `Class6B5CC__FaceTarget(Entity *this, void *arg1, s32 arg2, s32 arg3, s32
 * arg4)`; same "per-call-site signature, not a callee property"
 * precedent as `GetClass6B5CCMethods`/`Class6B5CC__func_1d33c` above -- this unit's own
 * view differs in the first two parameters' types only, see below).
 *
 * A "face target" orientation setter: computes yaw/pitch from `self` to
 * `target` via two `ratan2` calls, converts both to degrees (see
 * `Class6B5CC__GetRotationDegrees`'s same `x*360>>12` idiom -- pitch gets an EXTRA `+
 * 0x400` [90 degrees] added before conversion, yaw does not), builds a
 * `WholeFrac_d294[3]` {pitch, yaw, 0} table (each `.frac = 1`), and
 * dispatches it to `updateRotation`. `arg2 != 0` forces the pitch entry to 0
 * (a "yaw only" mode); `arg3 == 0` adds 180 degrees to yaw (see below);
 * a non-NULL `arg4` fires a SECOND `updateRotation(self, 0, arg4)` call with the
 * caller's own table forwarded as-is.
 *
 * THE ARGUMENT-SWAP FINDING, CONFIRMED FROM THIS FUNCTION'S OWN BODY:
 * `self` and `target` (this unit's own params 1/2) are used completely
 * SYMMETRICALLY -- both need only a `Class6B5CC`-SHAPED object
 * (`->unkC` null-checked, `->unk14->unk38` read as a 3-word table), and
 * the position subtraction is always `target - self`. `arg3` is what
 * makes this safe to call with the roles swapped: `Entity.h`'s own
 * documented finding (all `a3==0` call sites pass `(this, this->unk94)`,
 * all `a3==1` sites pass `(this->unk94, this)`) now has a mechanism, not
 * just a correlation -- swapping which object is `self` vs `target`
 * negates the computed direction, and the function's own `+180 degrees
 * on arg3==0` step is EXACTLY the correction needed to compensate. A
 * caller that already swapped the two objects at the call site (`a3==1`)
 * skips the correction because it does not need it; a caller passing
 * them in the "natural" order (`a3==0`) gets the correction applied
 * internally. This resolves the mechanism (not just the correlation)
 * without asserting a name for whatever base type `Entity`/`Unk94Obj`/
 * `Class6B5CC` share -- that question stays open, per the caller-side
 * finding in Entity.h and DECOMPILATION_LEARNINGS.
 *
 * This unit's own two parameters are typed `Class6B5CC *` rather than
 * a shared/generic type: `self->methods` is dispatched directly (needs
 * the real vtable type), and `target`'s `->unkC`/`->unk14->unk38` shape
 * matches `Class6B5CC` exactly, with no evidence in this call site
 * alone for anything narrower or wider. */

#endif
