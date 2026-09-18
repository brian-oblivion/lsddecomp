#ifndef CODE_D294_H
#define CODE_D294_H

#include "common.h"

/* code_d294: a FRESH CARVE (round 10), the first 20-function slice of a
 * 55-function segment never carved before. `tools/classtable.py D_8006B5CC`
 * shows this unit's own method table starts at D_8006B5CC and its slots,
 * in order, ARE this unit's functions: +0x008 func_8001CAF4 (ctor),
 * +0x00C func_8001CBA4 (dtor), +0x010 func_8001CC48, +0x014 func_8001CCB4,
 * +0x018 func_8001CD20, [+0x01C..+0x038 seven slots inherited verbatim from
 * BasicClass, D_8006B58C], +0x038 func_8001CD60 (override), +0x03C null,
 * +0x040 func_8001CE30, +0x044 func_8001CEB4, +0x048 func_8001D008,
 * +0x04C func_8001D0EC, +0x050 func_8001D1A4, +0x054 func_8001D204,
 * +0x058 func_8001D280, +0x05C func_8001D33C (already-matched no-op stub),
 * +0x060 func_8001D344, +0x064 func_8001D374, +0x068 func_8001D3A0,
 * +0x06C func_8001D3CC, +0x070 func_8001D3F8, and continuing past this
 * unit's slice into the next carve (code_d294_b) up to +0x0B4
 * func_8001E4A4 (45 slots total). This is proof, not a guess -- `--vs
 * D_8006B58C` confirms the class overrides 5 BasicClass slots (+0x008,
 * +0x00C, +0x010, +0x014, +0x018), inherits 7 verbatim (+0x01C..+0x038),
 * then overrides +0x038 and adds everything from +0x040 on as new virtuals.
 *
 * The class is unnamed -- no hypothesis about its purpose has surfaced yet
 * (no suggestive strings/symbol names reachable from this slice). Named
 * `Class6B5CC` after its table address, following this project's
 * established convention for classes discovered by table address rather
 * than by a plausible role (see Class6D3C8, Class86AA0, Class65650).
 *
 * func_8001CA94 is NOT a table slot -- it is the `New_Class6B5CC` allocator
 * wrapper (allocates 0x44 bytes, calls the ctor via `func_8001E57C()->ctor`,
 * frees and returns NULL on ctor failure). func_8001E57C is a plain
 * no-argument getter, `lui/addiu %hi/%lo(D_8006B5CC); jr $ra` -- MEASURED,
 * see include/class_3bb8c.h's own note on this exact symbol for the
 * "arity/signature is per-call-site, not a callee property" precedent this
 * follows: other units call the same func_8001E57C symbol with a different
 * argument count and are equally byte-exact. Declared here with 0 arguments
 * and a Class6B5CCMethods* return type, matching only THIS unit's call site
 * (func_8001CA94).
 */

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
 *     +0x48 unk48  == super   the PARENT coordinate; func_8001D0EC's
 *                             "attach" sets it to the owner's own unk14,
 *                             which is exactly what super means
 *     +0x4C        == sub     (not yet touched by any carved function)
 *
 *   Class6B5CCSub44  ==  GsCOORD2PARAM   (0x28 bytes)
 *     +0x00 unk0/unk4/unk8 == scale.vx/vy/vz (VECTOR, +0x0C is its pad)
 *     +0x10 vec            == rotate        (SVECTOR; x/y/z/w == vx/vy/vz/pad)
 *     +0x18 pad18          == trans         (VECTOR)
 *   so S16Quad_d294 is an SVECTOR, and the 4096-per-turn angle reading
 *   func_8001CEB4's full-turn wrap already established is Sony's own.
 *
 *   Class6B5CCObj +0x10 .. +0x1C  ==  an embedded GsDOBJ2
 *     +0x10 unk10 == attribute  (the packed flags word the GetSetBitField
 *                                family sets fields in)
 *     +0x14 unk14 == coord2     (the GsCOORDINATE2 above)
 *     +0x18 unk18 == tmd        (the model data pointer)
 *   Class6B5CC__LinkModel passes `&self->unk10` to Sony's GsLinkObject4 as
 *   its GsDOBJ2 argument, which only type-checks at this layout, and the
 *   ctor calls GsInitCoordinate2 on the 0x50-byte block.
 *
 * NOT ACTED ON HERE, DELIBERATELY. Retyping these to Sony's names and
 * renaming their fields is cross-unit work: code_d294.c, code_d294_b.c,
 * class_3bb8c_o.c and class_3bb8c_p.c all read these fields, and a naming
 * runner owns ONE unit (docs/PARALLEL-RUNS.md section 2). It is a
 * FINISHING-PLAN track 4 job, and a large one -- flagged, with the
 * derivation above, so that round does not have to rediscover it.
 * ========================================================================= */

typedef struct Class6B5CCObj Class6B5CCObj;
typedef struct Class6B5CCMethods Class6B5CCMethods;
typedef struct Class6B5CCSub14 Class6B5CCSub14;
typedef struct Class6B5CCSub44 Class6B5CCSub44;

/* Round 13 (code_d294_b, func_8001D4DC): a plain s16 quad, all-s16 members
 * (alignment 2) -- MEASURED, this is what makes retail's whole-struct copy
 * of it compile to unaligned lwl/lwr instead of a plain lw/sw (see
 * DECOMPILATION_LEARNINGS' "struct whose members are all s8/s16" idiom).
 * Only x/y/z are ever written by func_8001D4DC's own two paths; w is left
 * as whatever was already in the destination on the negate path, exactly
 * as retail's own disassembly does (no store to it there). Likely an
 * SVECTOR-shaped angle triple (PsyQ's own SVECTOR isn't typedef'd anywhere
 * reachable from this unit's headers, so this is a fresh local type, not a
 * borrowed one). */
typedef struct S16Quad_d294 S16Quad_d294;
struct S16Quad_d294 {
    s16 x;  /* +0x000 */
    s16 y;  /* +0x002 */
    s16 z;  /* +0x004 */
    s16 w;  /* +0x006, never written by func_8001D4DC's negate path */
};

/* Round 13: self->unk14->unk44's own struct (the 0x28-byte second heap
 * block) -- only the S16Quad_d294 at +0x10 is known, from func_8001D4DC.
 * Everything else in the block is still opaque, so this stays a
 * pad-then-known-field shape like Class6B5CCSub14 itself. */
typedef struct Class6B5CCBlock44 Class6B5CCBlock44;
struct Class6B5CCBlock44 {
    u8 pad0[0x010];
    S16Quad_d294 vec;  /* +0x010 */
};

/* self->unk14's target: a 0x50-byte block allocated by the ctor
 * (func_8001CAF4). Round 2 (func_8001D0EC, func_8001CE30, func_8001D1A4)
 * filled in most of the rest of this layout:
 *   +0x000  a flag/state word: 1 after func_8001CE30 (the ctor's own init
 *           hook) runs, 0 again after func_8001D0EC's "attach" and after
 *           func_8001CEB4/func_8001D008 (still queued, but their own tails
 *           both end `self->unk14->unk0 = 0` per their disassembly) do
 *           their work -- reads like a "pending update" flag.
 *   +0x018..+0x020  a Vec3 (x,y,z), written wholesale by func_8001D0EC from
 *           its optional 3rd argument (or zeroed if that argument is NULL).
 *   +0x044  a second heap block (0x28 bytes, alloc'd by the ctor).
 *   +0x048  a flag/back-reference word: zeroed by the ctor and by
 *           func_8001D1A4 (slot +0x050), set to `obj->unk14` by
 *           func_8001D0EC's "attach".
 * +0x004..+0x018 and +0x024..+0x044 are still unknown -- not padded out
 * field-by-field, since nothing this unit's chosen functions touch reads
 * them. */
/* A plain 3-word vector, used as func_8001D0EC's optional 3rd argument
 * (copied wholesale into Class6B5CCSub14::unk18/unk1C/unk20) and as
 * Class6B5CCSub14's own +0x038 field below. Declared here, ahead of
 * Class6B5CCSub14, so the struct below can use it directly. */
typedef struct Vec3_d294 {
    s32 x;
    s32 y;
    s32 z;
} Vec3_d294;

struct Class6B5CCSub14 {
    s32 unk0;                    /* +0x000, state/pending flag -- see above */
    u8 pad04[0x018 - 0x004];
    s32 unk18;                   /* +0x018, Vec3.x */
    s32 unk1C;                   /* +0x01C, Vec3.y */
    s32 unk20;                   /* +0x020, Vec3.z */
    /* +0x024, round 12 (code_d294_b, Class6B5CC__TransformAndNotifyParents): only its ADDRESS is
     * taken (`&self->unk14->unk24`, forwarded to ApplyMatrixToSVArray as a
     * write-destination base) -- nothing dereferences through it in this
     * unit's chosen functions, so it stays an opaque byte span rather than
     * a typed field. Renamed from `pad24` now that something references it
     * by name. */
    u8 unk24[0x038 - 0x024];
    /* +0x038, a 3-word (0xC-byte) per-axis `s32` quantity. TWO runners
     * measured this field in the same round from different units and agreed
     * on the LAYOUT while disagreeing on the name and the reading:
     *   - code_d294_b's func_8001D714 (STALLED) subtracts
     *     `other->unk14`'s +0x038 from `self->unk14`'s to get a relative
     *     offset and range-checks each axis -- read as a position.
     *   - code_d294_c's Class6B5CC__LocalOffsetToWorldPos and Class6B5CC__FaceTarget (both MATCHED) take
     *     its ADDRESS and walk it as `[i]` for `i` in 0..2, adding each word
     *     into a caller-supplied vector as a per-axis delta.
     * The head kept the INDEXABLE array form, because matched code requires
     * indexing and a `Vec3_d294` cannot be indexed, whereas nothing matched
     * used the named-vector form. The two readings are compatible: a
     * per-axis delta and a position differ in interpretation, not in shape.
     * Real per-word meaning still unknown. */
    s32 unk38[3];
    /* +0x044, RETYPED (round 13) from an opaque `void *` to
     * `Class6B5CCSub44 *`. Two runners retyped this field in the same round
     * from different call sites and gave the type two different NAMES
     * (`Class6B5CCSub44` from func_8001D008/func_8001CEB4, and
     * `Class6B5CCBlock44` from func_8001D4DC); the head unified them on
     * `Class6B5CCSub44`, which matches the sibling `Class6B5CCSub14`'s
     * naming and is the one already referenced from src/code_d294.c. Their
     * two views of the CONTENTS agreed and are unioned below. Still a
     * second heap block (0x28 bytes, alloc'd by the ctor); the retype only
     * narrows what is KNOWN about its contents, it does not change the
     * allocation. */
    Class6B5CCSub44 *unk44;
    s32 unk48;    /* +0x048, zeroed by the ctor and by func_8001D1A4 (slot +0x050);
                   * set to `obj->unk14` by func_8001D0EC's "attach" */
};

/* Class6B5CCSub14::unk44's target (round 13). Two independent derivations,
 * unioned by the head: func_8001D008/func_8001CEB4 established the three
 * s32 words at the base, and func_8001D4DC established that +0x010 holds an
 * S16Quad_d294 -- which subsumes the first derivation's separate
 * `unk10`/`unk12`/`unk14` s16 fields as `vec.x`/`vec.y`/`vec.z` and adds
 * `vec.w` at +0x016, a byte range the first derivation had recorded as
 * still opaque. Exactly one name per field, deliberately: two names for one
 * field inside a single header is a trap for the next reader.
 *
 * func_8001D008 and func_8001CEB4 both apply `RatioToFixed12` three times to
 * a 3-entry `{s16,s16}` table (their own 3rd argument, `data`) and write the
 * three s32 results into this block -- func_8001D008 into
 * +0x000/+0x004/+0x008 (either overwriting or accumulating, per its own
 * `flag` argument); func_8001CEB4 into `vec.x`/`vec.y`/`vec.z` as u16
 * (either overwriting after dividing each result by 360, or accumulating a
 * /360'd delta into the existing value and wrapping the sum modulo 4096 -- a
 * full-turn wrap, consistent with these being PSX-native 4096-per-circle
 * angle units). +0x00C..+0x010 is still unknown (not read or written by any
 * of the three functions); the block is 0x28 bytes total per the ctor's own
 * allocation size, so the tail past +0x018 is still opaque padding. */
struct Class6B5CCSub44 {
    s32 unk0;   /* +0x000 */
    s32 unk4;   /* +0x004 */
    s32 unk8;   /* +0x008 */
    u8 padC[0x010 - 0x00C];
    S16Quad_d294 vec;  /* +0x010, x/y/z/w at +0x010/+0x012/+0x014/+0x016 */
    u8 pad18[0x028 - 0x018];
};

/* Round 13 (BisectSegmentToBox): a 6-byte, all-s16 Vec3 -- MEASURED, all-s16
 * members give it alignment 2, which is what makes retail's own
 * struct-copy of it (in BisectSegmentToBox's loop tail) compile to unaligned
 * lwl/lwr + swl/swr, same idiom as S16Quad_d294 above and the already-
 * confirmed `func_8004B38C`/`FlashbackRotation` case in
 * DECOMPILATION_LEARNINGS. */
typedef struct Vec3S16_d294 {
    s16 x;
    s16 y;
    s16 z;
} Vec3S16_d294;

/* Round 13 (BisectSegmentToBox): an axis-aligned bounding box, low corner then
 * high corner -- MEASURED from BisectSegmentToBox's own field offsets
 * (+0x0/+0x2/+0x4 = lo.x/y/z, +0x6/+0x8/+0xA = hi.x/y/z). */
typedef struct BoundsBox_d294 {
    Vec3S16_d294 lo;
    Vec3S16_d294 hi;
} BoundsBox_d294;

/* Round 13 (Class6B5CC__CheckBoundsOverlap): a 12-byte, all-s16, 6-field record -- MEASURED,
 * same all-s16-struct-copy idiom as Vec3S16_d294 (whole-value assignment
 * compiles to unaligned lwl/lwr). Used as func_8001F50C's own return-array
 * element type and as this function's own second running-tracker. Field
 * names are placeholders; the tail comparison pairs them with
 * BoundsBox_d294 fields in a scrambled order (f5<->lo.z, f2<->hi.z,
 * f3<->lo.x, f4<->lo.y, f1<->hi.y, f0<->hi.x) consistent with `f0..f2`
 * being some OTHER box's hi corner and `f3..f5` its lo corner, but this is
 * not confirmed beyond the offsets themselves. */
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

/* This unit's own minimal, local view of the shared BasicClass ancestor
 * table (D_8006B58C, returned by Get_vtable_BasicClass, which lives in the
 * still-uncarved code_8220 segment) -- same shape and same "ctor at +0x008,
 * dtor at +0x00C universally" convention already established independently
 * in include/class_16334.h and include/code_171e0.h. Declared again here,
 * under a unit-local name, per this project's policy of NOT unifying
 * independent local views of the same table into one shared header. */
typedef struct BasicClassMethodsD294 BasicClassMethodsD294;
struct BasicClassMethodsD294 {
    s32 header;               /* +0x000 */
    void *unk04;               /* +0x004 */
    void *(*ctor)(void *self); /* +0x008 */
    void *(*dtor)(void *self); /* +0x00C */
    /* +0x010/+0x014, both round-2 finds (func_8001CC48/func_8001CCB4):
     * a `(self, other)` pair this class's own +0x010/+0x014 overrides
     * (func_8001CC48/func_8001CCB4) forward to unconditionally, after/before
     * their own extra work. Real BasicClass-level meaning unknown from
     * this unit alone. */
    void (*slot10)(void *self, void *other); /* +0x010 */
    void (*slot14)(void *self, void *other); /* +0x014 */
    void (*slot18)(void *self);              /* +0x018, func_8001CD20's forward target */
    u8 pad01C[0x038 - 0x01C];
    /* +0x038, func_8001CD60's (this unit) own forward target -- called
     * unconditionally as its very first action, `(self, other, arg2)`,
     * same three-argument shape as func_8001CD60 itself. Real BasicClass-
     * level meaning unknown from this unit alone, same caveat as
     * slot10/slot14 above. */
    void (*slot38)(void *self, void *other, s32 arg2);
};

extern BasicClassMethodsD294 *Get_vtable_BasicClass(void);
extern void *func_80017B34(s32 size);
extern void func_80017CFC(void *arg);

/* A second small class, only ever seen through self->unkC (see
 * Class6B5CCObj below) and through func_8001D0EC's 2nd argument -- an
 * "owner" this class can register/unregister with. Two slots seen so far,
 * `slot10`/`slot14` (func_8001D0EC/func_8001D1A4's call sites), both
 * `(owner, Class6B5CCObj *self)` -- read as an attach/detach pair. `unk14`
 * is a plain data field (func_8001D0EC reads it into
 * `self->unk14->unk48`); real meaning unknown. Real class identity
 * unknown -- named for the field it lives behind, per this unit's own
 * convention (see Class6B5CCSub14, also field-named).
 *
 * Round 13 (Class6B5CC__ComposeAndApplyRotation): self->unkC is also walked as an intrusive
 * singly-linked list -- MEASURED, `lw $s0, 0xC($s0)` repeated with a
 * `bnez` back-edge -- so this class has its own self-referential `next` at
 * +0xC, and its vtable has a slot at +0x84 with the same `(self, void
 * *out, s32 flag)` shape Class6B5CCMethods' own +0x84 has (see below);
 * likely a shared ancestor slot both classes inherit, not a coincidence,
 * but the ancestor itself is out of this unit's reach. */
typedef struct UnkOwner_d294 UnkOwner_d294;
typedef struct UnkOwnerMethods_d294 UnkOwnerMethods_d294;
struct UnkOwnerMethods_d294 {
    u8 pad0[0x010];
    void (*slot10)(UnkOwner_d294 *owner, Class6B5CCObj *self); /* +0x010, "attach" */
    void (*slot14)(UnkOwner_d294 *owner, Class6B5CCObj *self); /* +0x014, "detach" */
    /* +0x084, Class6B5CC__ComposeAndApplyRotation's own call target on each list node (round 13).
     * `out` is a 0x20-byte buffer (MATRIX-shaped -- MulMatrix2, this
     * call site's own consumer, loads it into GTE control regs 0-4 via
     * `ctc2`, PsyQ, not decompiled). */
    u8 pad018[0x084 - 0x018];
    void (*slot84)(UnkOwner_d294 *self, void *out, s32 flag);
};
struct UnkOwner_d294 {
    UnkOwnerMethods_d294 *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    UnkOwner_d294 *next;            /* +0x00C, round 13 (Class6B5CC__ComposeAndApplyRotation) -- MEASURED, see above */
    u8 pad10[0x014 - 0x010];
    /* RETYPED round 19 (echo, func_8001E7BC): from a plain `s32` to
     * `Class6B5CCSub14 *` -- func_8001E7BC dereferences it
     * (`node->unk14->unk18/unk1C/unk20`, the same position shape as
     * `Class6B5CCObj::unk14`'s own field). Safe: `func_8001D0EC` (already
     * matched, same unit) only ever COPIES this field's raw value
     * (`sub->unk48 = obj->unk14;`, a plain 32-bit word copy either way,
     * never dereferenced there) -- reverified after this retype,
     * func_8001D0EC's own match is unaffected (whole-image SHA1 stays
     * green with it still compiled as real C). */
    Class6B5CCSub14 *unk14;        /* +0x014 */
};

/* A generic "just enough to dispatch" view of some OTHER class, used where
 * this unit's queued functions read another object's own vtable header tag
 * or call one specific slot without knowing (or needing) the rest of that
 * class's layout. Same shape/convention as class_3bb8c.h's
 * `GenericTagInst_3bb8c_c`. */
typedef struct GenericMethods_d294 GenericMethods_d294;
typedef struct GenericObj_d294 GenericObj_d294;
/* Forward-declared here (full definition below, with the other small
 * "count + data" shapes) so GenericObj_d294's own +0x030 field (round 13,
 * func_8001D714) can be typed to it. */
typedef struct GenericCountList_d294 GenericCountList_d294;
struct GenericMethods_d294 {
    s32 header;                  /* +0x000, low nibble is a class-tag; func_8001CC48/CCB4 compare it against 9 */
    u8 pad004[0x010 - 0x004];
    /* +0x010, round 13 (func_8001E4A4): dispatched as `(entry, arg)` where
     * `entry` is the receiver itself and `arg` is func_8001E4A4's own
     * `self` parameter (a Class6B5CCObj*) -- only reached once the FULL
     * byte at +0x000 (not just its low nibble) equals 0x34, i.e. a more
     * specific check than the header-tag-4 test guarding entry into this
     * whole block. */
    void (*slot10)(GenericObj_d294 *self, Class6B5CCObj *arg); /* +0x010 */
    u8 pad014[0x038 - 0x014];
    /* +0x038, round 13 (func_8001D714's own call site): dispatched as
     * `(other, self, 4)` where `self` is func_8001D714's own Class6B5CCObj*
     * parameter and `4` a literal -- MEASURED from that call site alone,
     * real meaning of the literal unknown. */
    void (*slot38)(GenericObj_d294 *self, Class6B5CCObj *arg1, s32 arg2); /* +0x038 */
    u8 pad03C[0x050 - 0x03C];
    void (*slot50)(GenericObj_d294 *self); /* +0x050, func_8001D204's call target */
};
struct GenericObj_d294 {
    GenericMethods_d294 *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    void *unkC;                    /* +0x00C, func_8001D280 (still queued) compares this to a Class6B5CCObj* */
    s32 unk10;                     /* +0x010, round 13 (code_d294_c, Class6B5CC__LinkModel): read into self->unk18 */
    /* +0x014, round 13 (func_8001D714): same role as Class6B5CCObj's own
     * `unk14` -- `other->unk14`'s +0x038 is subtracted from
     * `self->unk14`'s. MEASURED, not a name unification guess: both call
     * sites reach the identical +0x038 field through this one, at the same
     * offset, in the same function. */
    Class6B5CCSub14 *unk14;
    u8 pad18[0x02C - 0x018];
    /* +0x02C, round 13 (func_8001D714): only its ADDRESS is taken
     * (forwarded as a vtable call's own opaque argument) -- real shape
     * unknown, so this stays an opaque byte span like Class6B5CCSub14's
     * own `unk24`. */
    u8 unk2C[0x030 - 0x02C];
    /* +0x030, round 13 (func_8001D714): same shape/usage as
     * Class6B5CC__TransformAndNotifyParents's own 2nd argument -- `unk0` read once and multiplied
     * by 8, `&unk4` forwarded as an opaque data pointer. */
    GenericCountList_d294 *unk30;
};

/* A third small "count + data" shape, seen only through Class6B5CC__TransformAndNotifyParents's own
 * 2nd argument (round 12): `unk0` is read once and multiplied by 8 to form
 * ApplyMatrixToSVArray's own iteration count, and `&unk4` (the address only, never
 * dereferenced here) is ApplyMatrixToSVArray's own source/dest base pointer. Real
 * class identity unknown -- opaque past `unk4`'s own address, so `unk4` is
 * left a single byte rather than a guessed element type. */
struct GenericCountList_d294 {
    s32 unk0;  /* +0x000, multiplied by 8 to form ApplyMatrixToSVArray's count arg */
    u8 unk4;   /* +0x004, address-only */
};

/* Class6B5CC's own table (D_8006B5CC). Only the slots this unit's chosen
 * functions actually dispatch through are typed; see the file banner above
 * for the full slot census from classtable.py. */
struct Class6B5CCMethods {
    s32 header;                              /* +0x000 */
    void *unk04;                              /* +0x004, BasicClass__func_17eb0, inherited, unused here */
    void *(*ctor)(void *self);                /* +0x008, func_8001CAF4 (this unit) */
    void  (*dtor)(void *self);                /* +0x00C, func_8001CBA4 (this unit) */
    u8 pad010[0x030 - 0x010];
    /* +0x030, BasicClass__NotifyParents, inherited verbatim (per the file
     * banner's `--vs D_8006B58C` census) -- NOT decompiled here, BasicClass
     * is a different unit's own ancestor code. Class6B5CC__TransformAndNotifyParents (round 12,
     * this unit) dispatches through it as `(self, s32 arg1)`. */
    void (*slot30)(Class6B5CCObj *self, s32 arg1);
    u8 pad034[0x040 - 0x034];
    void (*slot40)(Class6B5CCObj *self);      /* +0x040, func_8001CE30 (this unit) */
    /* +0x044/+0x048, a `(self, s32 flag, void *data)` pair -- func_8001CE30
     * calls both with flag=1 and `data` pointing at a 3-entry table of
     * {s16,s16} pairs (D_8006B684/D_8006B690, 0xC bytes each). func_8001D008
     * (slot +0x048's own occupant, still queued) confirms the `data` shape:
     * it reads three such pairs via RatioToFixed12. */
    void (*slot44)(Class6B5CCObj *self, s32 flag, void *data); /* +0x044, func_8001CEB4 -- still queued */
    void (*slot48)(Class6B5CCObj *self, s32 flag, void *data); /* +0x048, func_8001D008 -- still queued */
    u8 pad04C[0x050 - 0x04C];
    void (*slot50)(Class6B5CCObj *self);      /* +0x050, func_8001D1A4 (this unit) */
    void (*slot54)(Class6B5CCObj *self);      /* +0x054, func_8001D204 (this unit) */
    /* +0x058, func_8001D280 (still queued). func_8001D204's own call site
     * establishes its signature: writes an output entry pointer and an
     * output "more remain" flag through its 2nd/3rd arguments. */
    void (*slot58)(Class6B5CCObj *self, GenericObj_d294 **outEntry, s32 *outCont); /* +0x058 */
    /* +0x05C, func_8001D33C -- already matched as a no-op `void(void)`
     * body, but THIS call site (func_8001CBA4's dtor) passes it 2 args
     * (self, 0). Both are right about their own codegen: the callee body
     * ignores every argument, so the caller's arity is unconstrained. Same
     * "per-call-site signature" precedent as func_8001E57C above. */
    void (*slot5C)(Class6B5CCObj *self, s32 arg1);
    /* +0x060..+0x084: func_8001D344/D374/D3A0/D3CC/D3F8/D424/D450/D480/D4AC
     * (all this unit, all already matched) -- not typed here as struct
     * fields because nothing dispatches through the table at these offsets;
     * every call site invokes them directly by symbol name. */
    u8 pad060[0x084 - 0x060];
    /* +0x084, occupant `func_8001D4DC` per `tools/classtable.py
     * D_8006B5CC` (it lives in code_d294_b). Class6B5CC__RotateLocalVector (code_d294_c)
     * dispatches to it as `slot84(self, out, 0)`, `out` pointing at a
     * 0x20-byte stack buffer -- MEASURED from the caller's own frame size:
     * Class6B5CC__RotateLocalVector's stack layout only closes if `out` is a full 0x20
     * bytes, not the 0xC that would fit just the leading vector its own
     * caller reads back. Only those leading 0xC bytes are read back, by
     * Class6B5CC__RotateLocalVector's `ApplyMatrixToLVArray` call a few lines later; the rest is
     * opaque callback output that call site never touches. Class6B5CC__ComposeAndApplyRotation
     * (code_d294_b) calls the same slot on `self` itself, and
     * UnkOwnerMethods_d294's own +0x084 has the same number and shape --
     * likely a shared ancestor method. */
    void (*slot84)(Class6B5CCObj *self, void *out, s32 arg2);
    u8 pad088[0x08C - 0x088];
    /* +0x08C/+0x090, round 13 (Class6B5CC__NotifyIfUnk20Active's own call site): dispatched
     * as `(self, dest)` and `(self, a1, a2)` respectively, matching
     * Class6B5CC__ReadUnk20Data/Class6B5CC__TransformAndNotifyParents's own direct-call prototypes below
     * exactly (both already matched, code_d294_b) -- `tools/classtable.py
     * D_8006B5CC` confirms they occupy these two slots. */
    void (*slot8C)(Class6B5CCObj *self, void *dest); /* +0x08C, Class6B5CC__ReadUnk20Data */
    void (*slot90)(Class6B5CCObj *self, GenericCountList_d294 *a1, s32 a2); /* +0x090, Class6B5CC__TransformAndNotifyParents */
    /* +0x094/+0x098/+0x09C, a `(self, GenericObj_d294 *other, s32 arg2)`
     * triple -- func_8001CD60 (code_d294) dispatches to exactly one of
     * these three depending on `other->methods->header & 0xF` (2 -> +0x094,
     * 5 -> +0x098, 4 -> +0x09C; any other tag value fires none of them).
     * Real per-slot meaning unknown from that call site alone. */
    void (*slot94)(Class6B5CCObj *self, GenericObj_d294 *other, s32 arg2);
    void (*slot98)(Class6B5CCObj *self, GenericObj_d294 *other, s32 arg2);
    void (*slot9C)(Class6B5CCObj *self, GenericObj_d294 *other, s32 arg2);
    /* +0x0A0, func_8001D6B4's own call target (round 12): dispatched with
     * only `self`, per that function's own disassembly (`jalr $v0` with
     * `$a0` untouched since function entry). func_8001D714 (still queued)
     * is this slot's occupant per `tools/classtable.py D_8006B5CC`. */
    void (*slotA0)(Class6B5CCObj *self);
    /* +0x0A4, round 13 (func_8001D714's own call site): this slot's
     * occupant is Class6B5CC__ComposeAndApplyRotation itself (`tools/classtable.py D_8006B5CC`),
     * already matched this round -- dispatched indirectly (through the
     * vtable, not a direct `jal`) since the slot is polymorphic even
     * though this class's own table happens to point at it. Signature
     * matches Class6B5CC__ComposeAndApplyRotation's own exactly. */
    void (*slotA4)(Class6B5CCObj *self, void *arg1, void *arg2, void *arg3, s32 count);
    /* +0x0A8, occupant Class6B5CC__CheckBoundsOverlap (still queued this round). Return
     * value IS tested by this call site (truthy -> continue, falsy ->
     * early return), so non-void. */
    s32 (*slotA8)(Class6B5CCObj *self, void *arg1, Vec3S16_d294 *arg2);
    /* +0x0AC, occupant func_8001DDF4 -- the documented gp_rel blocker
     * (docs/match-reports/func_8001DDF4.md), NOT decompiled by this
     * runner. Typed here only for this call site's own dispatch shape;
     * return value is also tested truthy/falsy like slotA8's. */
    s32 (*slotAC)(Class6B5CCObj *self, void *arg1, Vec3S16_d294 *arg2, void *arg3);
};

/* MEASURED: func_8001E57C's whole body is `lui/addiu %hi/%lo(D_8006B5CC);
 * jr $ra` (asm/code_d294_b.s) -- a plain no-argument getter for this unit's
 * own vtable. See the file banner for the cross-unit precedent on why this
 * local 0-argument declaration doesn't need to agree with other units'. */
extern Class6B5CCMethods *func_8001E57C(void);

/* This unit's own vtable data, D_8006B5CC (see `tools/classtable.py
 * D_8006B5CC` in the file banner above) -- func_8001E57C (round 12,
 * code_d294_b) just returns `&D_8006B5CC`. Still rodata (not carved as C
 * here), so only the address is declared, typed to the return of the one
 * getter this unit implements. */
extern Class6B5CCMethods D_8006B5CC;

struct Class6B5CCObj {
    Class6B5CCMethods *methods; /* +0x000 */
    /* +0x004..+0x00C: BasicClass instance fields, owned by whatever unit
     * decompiles BasicClass itself -- EXCEPT +0x004, read directly by
     * func_8001D280 (this unit) as a list-head pointer (seeded into its
     * search cursor whenever the caller hasn't found an entry yet). Split
     * out of the opaque blob for that one reason; still don't know
     * BasicClass's own name for it. */
    void *unk4;
    u8 unk8[0x00C - 0x008];
    /* +0x00C, an owner back-reference. RETYPED round 2 from an untyped
     * `void *` to `UnkOwner_d294 *` once func_8001D0EC (this unit's own
     * "attach" -- sets `self->unkC = obj` and calls `obj->methods->slot10`)
     * and func_8001D1A4 (the "detach", `self->unkC->methods->slot14`)
     * were both matched: both dispatch through the exact same 2-slot
     * shape, `UnkOwnerMethods_d294`, at the exact same slot offsets. Not
     * just a plausible guess -- the two call sites agree byte-for-byte on
     * what lives at +0x010/+0x014 of whatever this points to. */
    UnkOwner_d294 *unkC;
    u32 unk10;                  /* +0x010, a packed bit-flags word -- see GetSetBitField below */
    Class6B5CCSub14 *unk14;     /* +0x014, the ctor's 0x50-byte allocation */
    s32 unk18;                  /* +0x018, zeroed by the ctor */
    u8 unk1C[0x020 - 0x01C];    /* unknown; not touched by this unit's chosen functions */
    /* +0x020, zeroed by the ctor. func_8001CC48's still-queued forward
     * target Class6B5CC__LinkModel (code_d294_b.s) stores its own 2nd argument into
     * this offset. RETYPED round 12 (code_d294_b, Class6B5CC__ReadUnk20Data): that
     * function passes `self->unk20` straight through as func_8001F51C's own
     * `void *` arg0 (psyq_fa50.s; func_8001F51C forwards it
     * unmodified to func_8001F3B0, which dereferences it at +0x10) --
     * genuinely a pointer, not an always-zero s32. Still opaque: nothing
     * this unit's chosen functions dereference through it directly. */
    void *unk20;
    s32 unk24;                  /* +0x024, zeroed by func_8001CE30 (this unit) */
    /* +0x028, round 12 (code_d294_b): func_8001D6B4 sets this to its own
     * `a1` (a plain `s32`, per m2c's own inference) when called with
     * a2==4. RETYPED round 13 (func_8001D714): `self->unk28 = other;`
     * where `other` is a `GenericObj_d294 *` -- same offset, genuinely a
     * pointer here. func_8001D6B4's own `self->unk28 = a1;` (a1 still
     * `s32`) keeps compiling under this type (int-to-pointer, a warning
     * not an error, and identical codegen either way -- MEASURED: rebuilt
     * whole-image after this retype and func_8001D6B4 is still
     * byte-exact). */
    GenericObj_d294 *unk28;
    s32 unk2C;  /* +0x02C, round 12 (Class6B5CC__TransformAndNotifyParents): zeroed, alongside unk28 */
    /* +0x030, round 12 (Class6B5CC__TransformAndNotifyParents): set to that call's own 2nd argument
     * for the duration of a single `self->methods->slot30(self, a2)`
     * dispatch, then zeroed again right after -- reads like a "currently
     * processing" scratch slot rather than a durable field. */
    GenericCountList_d294 *unk30;
};

/* The `{s16 whole; s16 frac;}` pair `RatioToFixed12` (below) reads and
 * `Class6B5CC__GetRotationDegrees` (round 14, code_d294_c) writes, three-in-a-row, at
 * +0x0/+0x4/+0x8 of a 3-entry table (an angle-like x/y/z triple,
 * degrees-and-fraction each -- `D_8006B684`/`D_8006B690` are exactly this
 * shape). `Class6B5CC__GetRotationDegrees` writes `frac` as a constant `1` in every entry
 * it produces; real per-field meaning of `frac` beyond that one producer
 * is still only inferred from `RatioToFixed12`'s own `div`-by-`frac` body,
 * not independently confirmed. */
typedef struct WholeFrac_d294 WholeFrac_d294;
struct WholeFrac_d294 {
    s16 whole;
    s16 frac;
};

/* RatioToFixed12 (round 54 correction: this banner was STALE -- it is
 * now carved and MATCHED in src/code_d294_c.c, not code_d294_b, and the
 * nop_mflo_mfhi toolchain flag it was once blocked on is RESOLVED per
 * CLAUDE.md's "Open toolchain blockers" table; see
 * docs/match-reports/RatioToFixed12.md for the current history): reads
 * a `WholeFrac_d294` at the
 * given pointer and returns a 20.12 fixed-point value (`whole << 12 |
 * frac`'s own division-derived low bits) -- read off its own
 * disassembly (a `div` by the pair's own two fields, not decompiled
 * here). `func_8001D008`/`func_8001CEB4` (both matched, this unit) apply
 * it three times in a row, at offsets +0x0/+0x4/+0x8 of their own 3rd
 * argument. Declared here with a `void *` argument since this unit's
 * chosen functions only ever pass the pointer through, never dereference
 * the pair themselves. */
extern s32 RatioToFixed12(void *pair);

/* D_8006B684/D_8006B690 (rodata): two 3-entry, 0xC-byte tables in the shape
 * RatioToFixed12 reads (see above) -- func_8001CE30's own literal `data`
 * arguments to slot +0x044/+0x048. Declared as opaque byte blobs since
 * nothing this unit's chosen functions read out of them directly (only
 * their address is taken and forwarded). */
extern u8 D_8006B684[0xC];
extern u8 D_8006B690[0xC];

/* Class6B5CC__LinkModel/Class6B5CC__UnlinkModel -- now carved (round 14, src/code_d294_c.c),
 * and GsInitCoordinate2 (Psy-Q libgs/matrix, linked from Sony's object,
 * not game code): three
 * helpers this unit's own +0x010/+0x014/+0x018/+0x040 overrides forward
 * into. Class6B5CC__UnlinkModel's whole body is `self->unk18 = 0; self->unk20 = 0;`
 * (MEASURED, two `sw $zero` stores, no branches) -- confirms unk18/unk20
 * above independently of the ctor. GsInitCoordinate2 is NOT decompiled
 * here (Psy-Q, linked as an object); declared only with the argument shape
 * its call site needs. */
extern void Class6B5CC__UnlinkModel(Class6B5CCObj *self);
extern void Class6B5CC__LinkModel(Class6B5CCObj *self, GenericObj_d294 *other);
extern void GsInitCoordinate2(s32 arg0, void *dest);

/* GetNextBasicClass (round 54 correction: this banner was STALE -- it is
 * now carved and MATCHED, in src/code_8220_b.c, a DIFFERENT unit; that
 * unit still has one unrelated stall of its own, func_80018464, per its
 * own file banner): a
 * generic intrusive-list "pop next" step. Given `out` and `cursor`
 * (both `T **`), if `*cursor` is non-NULL: `*out = (*cursor)->unk4`
 * (the node's own "next" field) and `*cursor = (*cursor)->unk0` (some
 * other per-node link -- NOT necessarily the same "next" field, going by
 * its own disassembly). If `*cursor` is NULL, `*out = NULL`. Declared
 * here typed to func_8001D280's own call site (the only caller reachable
 * from this unit) rather than generically. */
extern void GetNextBasicClass(GenericObj_d294 **out, GenericObj_d294 **cursor);

/* func_8001F51C (asm/psyq_fa50.s, Psy-Q library, not game code):
 * fills a caller-supplied struct (its own arg1) from a small on-stack
 * buffer via func_8001F3B0 (its own arg0 forwarded straight through). Its
 * own last write to $v0 is leftover from an unrelated `lhu` a few
 * instructions earlier, not a deliberate return value -- read as `void`.
 * Class6B5CC__ReadUnk20Data (this unit, round 12) calls it as `func_8001F51C(self->unk20,
 * dest)`; declared here only with the opaque `void *` shape that call site
 * needs. */
extern void func_8001F51C(void *arg0, void *dest);

/* func_8001F3A4 (asm/psyq_fa50.s, Psy-Q library, not game code): a
 * predicate over the same opaque `self->unk20` pointer Class6B5CC__ReadUnk20Data and
 * func_8001F51C above already treat as `void *` -- Class6B5CC__NotifyIfUnk20Active (round 13,
 * this unit) tests its `$v0` result for non-zero, so declared `s32`
 * (boolean-ish) here. Not decompiled in this project. */
extern s32 func_8001F3A4(void *arg0);

/* func_8001F4E4/func_8001F50C (asm/psyq_fa50.s, PsyQ library, not
 * game code): func_8001F4E4 fills a PsyQ-internal global
 * (D_8008B21C, via func_8001F3B0) from its own argument; func_8001F50C
 * IGNORES both its arguments and just returns `&D_8008B21C` -- MEASURED,
 * its whole body is `lui/addiu %hi/%lo(D_8008B21C); jr $ra`. Class6B5CC__CheckBoundsOverlap
 * (round 13, code_d294_b) calls the pair as `func_8001F4E4(self->unk20);
 * arr = func_8001F50C(self->unk20, 0);` -- declared here typed to that
 * call site's own use of the result (an array of Sixteen6_d294). */
extern void func_8001F4E4(void *arg0);
extern Sixteen6_d294 *func_8001F50C(void *arg0, s32 arg1);

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
extern void ApplyMatrixLV();

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

extern Class6B5CCObj *func_8001CA94(void);
void *func_8001CAF4(Class6B5CCObj *self);
void func_8001CBA4(Class6B5CCObj *self);
void func_8001CC48(Class6B5CCObj *self, GenericObj_d294 *other);
void func_8001CCB4(Class6B5CCObj *self, GenericObj_d294 *other);
void func_8001CD20(Class6B5CCObj *self);
void func_8001CE30(Class6B5CCObj *self);
Class6B5CCObj *func_8001D0EC(Class6B5CCObj *self, UnkOwner_d294 *obj, Vec3_d294 *vec);
Class6B5CCObj *func_8001D1A4(Class6B5CCObj *self);
void func_8001D204(Class6B5CCObj *self);
void func_8001D280(Class6B5CCObj *self, GenericObj_d294 **entry, GenericObj_d294 **cursor);

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
 * (func_8001D344/D374/D3A0/D3CC/D3F8) are thin wrappers around this,
 * always over `&self->unk10`, at five non-overlapping bit positions
 * (shift 3 width 3, shift 6 width 1, shift 28 width 2, shift 30 width 1,
 * shift 31 width 1) -- i.e. self->unk10 is a packed flags/small-fields
 * register and these five functions are its per-field setters. */
extern u32 GetSetBitField(u32 *word, s32 shift, s32 width, u32 value);

s32 func_8001D344(Class6B5CCObj *self, s32 a1);
u32 func_8001D374(Class6B5CCObj *self, s32 a1);
u32 func_8001D3A0(Class6B5CCObj *self, u32 a1);
u32 func_8001D3CC(Class6B5CCObj *self, s32 a1);
u32 func_8001D3F8(Class6B5CCObj *self, u32 a1);

/* Round 12 (code_d294_b): four more self->unk10 bitfield siblings, same
 * family as the five above. See src/code_d294_b.c for the per-function
 * shift/width/return-type notes. */
u32 func_8001D424(Class6B5CCObj *self, u32 a1);
s32 Class6B5CC__GetSetUnk10Flag7(Class6B5CCObj *self, s32 a1);
u32 Class6B5CC__GetSetUnk10Field9(Class6B5CCObj *self, u32 a1);
s32 func_8001D4AC(Class6B5CCObj *self, s32 a1);

/* RotMatrix (Psy-Q libgte/fgo_01, linked from Sony's object, not game
 * code): takes an
 * s16-quad-shaped pointer (this call site's own S16Quad_d294) and a 2nd
 * argument this unit's own caller passes straight through, unexamined.
 * $v0 is never read after this call site's own `jal`, so declared void
 * here -- other units may see a different arity/return, same per-call-site
 * precedent as func_8001E57C above. */
extern void RotMatrix(S16Quad_d294 *vec, s32 a1);

/* MulMatrix2 (Psy-Q libgte/mtx, linked from Sony's object, not game
 * code): loads its
 * own arg0 into GTE control regs 0-4 via `ctc2` (5 words, the packed
 * MATRIX rotation part) then combines it with arg1 -- a matrix-compose
 * primitive (PsyQ's `CompMatrix` family). Class6B5CC__ComposeAndApplyRotation (round 13, this
 * unit) calls it as `MulMatrix2(buf2, buf1)`, both 0x20-byte opaque
 * local buffers; declared only with that shape. */
extern void MulMatrix2(void *arg0, void *arg1);

/* BasicClass__func_1816c (src/code_8220.c, code_8220 unit, already matched
 * there as `void BasicClass__func_1816c(BasicClass *self, BasicClass
 * **outParent, BasicClassListNode **cursor)` -- "getNextParentRef": on the
 * first call for a given walk (`*outParent == NULL`), seeds `*cursor` from
 * `self->parentRefs`; every call pops one entry via `GetNextBasicClass`.
 * Redeclared here with this unit's own opaque/local types rather than
 * `#include "code_8220.h"`, per this project's per-unit-local-view
 * convention (same precedent as BasicClassMethodsD294 above) -- pointer
 * shapes are ABI-identical across translation units, so this is safe. */
extern void BasicClass__func_1816c(void *self, GenericObj_d294 **outParent, void **cursor);

void func_8001D4DC(Class6B5CCObj *self, s32 a1, s32 a2);
void Class6B5CC__NotifyIfUnk20Active(Class6B5CCObj *self, s32 a1);

void Class6B5CC__ReadUnk20Data(Class6B5CCObj *self, void *dest);
void Class6B5CC__TransformAndNotifyParents(Class6B5CCObj *self, GenericCountList_d294 *a1, s32 a2);
void func_8001D6B4(Class6B5CCObj *self, s32 a1, s32 a2);
void Class6B5CC__ComposeAndApplyRotation(Class6B5CCObj *self, void *arg1, void *arg2, void *arg3, s32 count);
void func_8001E4A4(Class6B5CCObj *self, void *node);
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
s32 Class6B5CC__CheckBoundsOverlap(Class6B5CCObj *self, void *arg1, Vec3S16_d294 *arg2);

/* ratan2 (Psy-Q library, not game code; symbol address per
 * config/symbols.slps01556.lsdde.txt, 0x8001F0C8): arctangent of
 * (dy, dx) in PSX-native 4096-per-circle BAM units, matching every other
 * angle representation this unit's own functions already use (see
 * `WholeFrac_d294` above and `Class6B5CC__GetRotationDegrees`'s degree conversion). Real
 * argument order confirmed from Class6B5CC__FaceTarget's own two call sites,
 * below. */
extern s32 ratan2(s32 dy, s32 dx);

/* Class6B5CC__FaceTarget (round 14, this unit -- the SAME external symbol
 * Entity_b/c/d/e.c call via their own separate `Entity.h` declaration,
 * `Class6B5CC__FaceTarget(Entity *this, void *arg1, s32 arg2, s32 arg3, s32
 * arg4)`; same "per-call-site signature, not a callee property"
 * precedent as `func_8001E57C`/`func_8001D33C` above -- this unit's own
 * view differs in the first two parameters' types only, see below).
 *
 * A "face target" orientation setter: computes yaw/pitch from `self` to
 * `target` via two `ratan2` calls, converts both to degrees (see
 * `Class6B5CC__GetRotationDegrees`'s same `x*360>>12` idiom -- pitch gets an EXTRA `+
 * 0x400` [90 degrees] added before conversion, yaw does not), builds a
 * `WholeFrac_d294[3]` {pitch, yaw, 0} table (each `.frac = 1`), and
 * dispatches it to `slot44`. `arg2 != 0` forces the pitch entry to 0
 * (a "yaw only" mode); `arg3 == 0` adds 180 degrees to yaw (see below);
 * a non-NULL `arg4` fires a SECOND `slot44(self, 0, arg4)` call with the
 * caller's own table forwarded as-is.
 *
 * THE ARGUMENT-SWAP FINDING, CONFIRMED FROM THIS FUNCTION'S OWN BODY:
 * `self` and `target` (this unit's own params 1/2) are used completely
 * SYMMETRICALLY -- both need only a `Class6B5CCObj`-SHAPED object
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
 * `Class6B5CCObj` share -- that question stays open, per the caller-side
 * finding in Entity.h and DECOMPILATION_LEARNINGS.
 *
 * This unit's own two parameters are typed `Class6B5CCObj *` rather than
 * a shared/generic type: `self->methods` is dispatched directly (needs
 * the real vtable type), and `target`'s `->unkC`/`->unk14->unk38` shape
 * matches `Class6B5CCObj` exactly, with no evidence in this call site
 * alone for anything narrower or wider. */
void Class6B5CC__FaceTarget(Class6B5CCObj *self, Class6B5CCObj *target, s32 arg2, s32 arg3, void *arg4);

#endif
