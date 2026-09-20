#ifndef CLASS_3BB8C_H
#define CLASS_3BB8C_H

#include "common.h"

/*
 * The class whose method table is D_800866E8 (80 slots, resolved with
 * tools/classtable.py 0x800866E8). No FirecatFG name survives, so fields
 * are named by offset until real names are known. This unit (class_3bb8c)
 * is the FIRST to write any of this class's own methods -- a different
 * unit, class_3ac78, already has its own independent local view of the
 * SAME table (`Class866E8`/`Class866E8Methods` in include/class_3ac78.h,
 * established from ITS OWN call sites: ctor +0x008, slot38, slot40
 * (gp_rel-blocked), slot80, slotD0). Per this project's established
 * multiple-independent-local-views convention (see e.g. StreamTaskObj vs.
 * LoaderTaskMethods in code_2c054.h/Class6D3C8.h), this header does NOT
 * edit or extend that one -- it is this unit's own view, named distinctly
 * (`Obj866E8`) to avoid implying a shared type across translation units
 * that never include each other's headers.
 *
 * Only the slots/fields this unit's functions actually touch are given
 * concrete types; the rest stay opaque padding.
 */
typedef struct Obj866E8 Obj866E8;
typedef struct ElemTargetMethods ElemTargetMethods;
typedef struct ElemTarget ElemTarget;
typedef struct EntryChildObj EntryChildObj;
typedef struct Elem Elem;
typedef struct UnkCObj UnkCObj;
typedef struct Unk14Obj Unk14Obj;
typedef struct Unk1BCObj Unk1BCObj;
typedef struct Unk6CObj Unk6CObj;
typedef struct Unk6C14Obj Unk6C14Obj;
typedef struct Unk6C14SubObj Unk6C14SubObj;
typedef struct QueryTemplate866E8 QueryTemplate866E8;
typedef struct LinkTarget866E8 LinkTarget866E8;
typedef struct ResInfo866E8 ResInfo866E8;
typedef struct EntryGpu EntryGpu;

/* self+0x54: an inline (not pointer) 3-word sub-struct, dereferenced by
 * func_8004B44C (arg3) and also matches func_8004C470's `arg1` descriptor
 * (unk0/unk4/unk8, all s32) -- reused for both since the shapes agree and
 * nothing distinguishes them. Field meaning unknown beyond "3 words,
 * read/written as a group". */
typedef struct Unk54Struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk54Struct;

/*
 * self->unk68's pointee. Established from THREE independent functions in
 * this unit, all agreeing on the same 8-byte, 4-byte-aligned layout:
 *  - func_8004C368 reads +0x000 (s16 divisor) as a `div`/`%` operand.
 *  - func_8004B930 reads +0x000 (same divisor) and +0x002 (s16 count,
 *    used as a loop trip count) and +0x004 (s32, gates the whole
 *    function between two totally different code paths).
 *  - func_8004B44C's own `arg2` parameter is fed this exact pointer at
 *    its one call site (func_8004B418) and reads all three fields
 *    (+0x000 s16, +0x002 s16, +0x004 s32) the same way.
 *  - func_8004C470 reads +0x004 alone (a boolean-ish gate).
 */
typedef struct Unk68Struct {
    s16 divisor;   /* +0x000 */
    s16 count;     /* +0x002 */
    s32 unk4;      /* +0x004 */
} Unk68Struct;

/*
 * A 10-byte "descriptor" struct, passed by pointer. Established from TWO
 * independent functions:
 *  - func_8004B44C's `arg4` (5th/stack argument) reads it as four signed
 *    bytes (+0x0..+0x3) followed by three signed halfwords (+0x4, +0x6,
 *    +0x8).
 *  - func_8004B38C block-copies a whole one of these (its own `arg3`)
 *    into `self->unkBC` in ONE retail load-all-then-store-all sequence:
 *    two unaligned lwl/lwr loads (8 bytes) followed by a plain unaligned
 *    swl/swr pair, then a final aligned `sh` for the trailing halfword.
 *    Reproducing the unaligned word copies (rather than per-field copies)
 *    needs this struct's natural alignment to be LESS than 4 -- true here
 *    only because every member is s8/s16 and none is s32, the same
 *    "no s32 member forces alignment 2" idiom already documented for
 *    `FlashbackRotation` in include/DreamSys.h. A struct with a stray s32
 *    member would compile to plain aligned `lw`/`sw` instead and would
 *    not match.
 */
typedef struct Descriptor10 {
    s8 b0;
    s8 b1;
    s8 b2;
    s8 b3;
    s16 h4;
    s16 h6;
    s16 h8;
} Descriptor10;

/*
 * EVIDENCE STATUS, round 10: every declaration in this region was derived
 * from functions that STALLED -- func_8004C1C0 (72/106), func_8004B700
 * (125/140), func_8004BB3C (structurally 104/105). None of it is backed by a
 * byte-exact match. The offsets and access widths are OBSERVED from the
 * disassembly and are reliable; the TYPES and the names are inferred, and the
 * grouping into structs is a hypothesis.
 *
 * (These comments originally said "MATCHED" for func_8004C1C0 and
 * func_8004B700. That was wrong -- the runner matched nothing in this unit
 * this round -- and it mattered, because it presented inferred structure as
 * byte-verified and would have discouraged the next reader from questioning
 * it. Relabelled by the head at merge.)
 *
 * Output struct of func_8004C1C0 (Obj866E8Methods::slot110). A `Descriptor10`
 * embedded at +0x000 (natural alignment 2, so the next member falls at the
 * next 4-byte boundary, +0x00C -- matches exactly) followed by 8 more s32-
 * sized fields the function fills from a resolved Elem/Unk14Obj pair.
 * `base.b0`/`base.b1` are filled by the callee `func_8004C368` (mod/div of
 * the raw rate); `base.b2`/`base.b3`/`h4`/`h6`/`h8` are computed in
 * func_8004C1C0 itself. Field meaning beyond that is unestablished -- named
 * by offset like the rest of this unit's opaque types.
 */
typedef struct Descriptor10Ext {
    Descriptor10 base;   /* +0x000 */
    s32 unkC;            /* +0x00C */
    s32 unk10;           /* +0x010 */
    s32 unk14;           /* +0x014 */
    s32 unk18;           /* +0x018 */
    s32 unk1C;           /* +0x01C */
    s32 unk20;           /* +0x020 */
    Elem *unk24;         /* +0x024, func_8004C1C0: the Elem it resolved via slot11C */
    s32 unk28;           /* +0x028, func_8004C1C0: the raw ElemTarget->unk30 rate, sign-extended */
} Descriptor10Ext;

/*
 * "in" struct of func_8004C1C0 (Obj866E8Methods::slot110's 3rd param).
 * Three fields, each read BOTH as a full s32 (for a coarse cell-index
 * computation) and, separately and later, as just the low `u16` half (for a
 * fine sub-cell offset computation) -- the same memory, two widths, at
 * non-adjacent points in the function, which is why each is a union here
 * rather than a plain s32: writing it as a plain field and casting at the
 * use site would not force retail's observed re-load-at-the-narrower-width
 * behaviour. Provenance: func_8004C158 (this unit) passes
 * `(u8 *)self->unk6C->unk14 + 0x18` as this pointer.
 */
typedef struct QueryPos866E8 {
    union { s32 w; u16 h; } unk0;   /* +0x000 */
    union { s32 w; u16 h; } unk4;   /* +0x004 */
    union { s32 w; u16 h; } unk8;   /* +0x008 */
} QueryPos866E8;

/*
 * func_8004BB3C's 2nd parameter: a 0xC-byte-strided array, one entry per
 * loop iteration. Established from that function alone: `ptr0` is tested
 * for NULL to pick a branch and then, on the non-NULL branch, forwarded
 * VERBATIM (untouched) to `ElemTargetMethods::slot78` -- consistent with a
 * pointer, though its pointee is never dereferenced in this unit; `rate`
 * is read as a plain halfword and copied into the resolved Elem's
 * `unk4->unk30` (already an `s16` there); `id` is read as a full word and
 * passed as `Obj866E8Methods::slot118`'s index argument.
 *
 * NOTE for whoever revisits func_8004BB3C: retail walks `ptr0` and the
 * `rate`/`id` pair via TWO INDEPENDENTLY-INCREMENTING pointers (registers
 * `$s4`/`$s3`, both `+= 0xC` per iteration), not one indexed base. This
 * WAS reproduced, in round 13: the lever is two walkers of DIFFERENTLY
 * BASED types -- a `SetupEntry866E8 *` at `arr1` for `ptr0` and a
 * `SetupSub866E8 *` at `arr1 + 4` for `rate`/`id` (see that typedef
 * below) -- each advanced by a natural `++`. Both `addiu` increments then
 * appear and the function reaches 90/105 words at the CORRECT length.
 * Regroupings that keep ONE base (plain `arr1[i].field`, an `&arr1[i]`
 * element pointer, a nested `arr1[i].sub.field`) all collapse to one
 * induction variable; byte-cast walking regresses further. The remaining
 * residue is a `$s3`/`$s4` identity swap. See the match report.
 */
typedef struct SetupEntry866E8 {
    void *ptr0;     /* +0x0 */
    s16 rate;       /* +0x4 */
    u8 pad6[0x8 - 0x6];
    s32 id;         /* +0x8 */
} SetupEntry866E8;

/* A SECOND view of the SAME 0xC-byte stride, based at `+0x4` instead of
 * `+0x0`. Exists only as the target type of func_8004BB3C's `rate`/`id`
 * walking pointer, so that a natural `++` strides the real array pitch
 * without a byte cast inside the loop; it is deliberately NOT embedded in
 * SetupEntry866E8 (doing so would corrupt that struct's size). Retail
 * seeds this walker at `arr1 + 4` and the `ptr0` walker at `arr1`, which
 * is why two differently-BASED types are needed rather than two pointers
 * of one type. */
typedef struct SetupSub866E8 {
    s16 rate;       /* +0x4 in SetupEntry866E8 terms */
    u8 pad2[0x4 - 0x2];
    s32 id;         /* +0x8 */
    u8 pad8[0xC - 0x8];
} SetupSub866E8;

/* Uncarved helper in this same unit (asm/class_3bb8c.s past this slice),
 * called by func_8004B418 and func_8004B38C (both already matched) and
 * itself attempted-but-stalled this round (58/73, see
 * docs/match-reports/func_8004B44C.md) -- not a byte-exact match, so its
 * body stays raw asm, but the prototype below reflects what the attempt
 * established.
 *
 * Parameter types were tightened this round (all pointer-TYPE-only
 * changes -- same pointer size/ABI, so this does not disturb either
 * existing caller's already-matched bytes): `arg0` is written through as
 * two `s32` words at both known call sites (`s32 *`, was `void *`);
 * `arg2` is `self->unk68` at both call sites (Unk68Struct *, see above);
 * `arg4` is the 5th/stack argument (Descriptor10 *, see above); `outBuf`
 * is fed a 3-word local array by both call sites, so it is typed `s32 *`
 * rather than opaque `void *`. */
extern s32 func_8004B44C(s32 *arg0, s32 *outBuf, Unk68Struct *arg2, Unk54Struct *arg3, Descriptor10 *arg4);

/* Unidentified global, address-only use (func_8004B38C passes `&D_80086904`
 * as an argument, never reads it directly here). Typed `s32` purely as a
 * placeholder since only its address is taken. */
extern s32 D_80086904;

/* Constant `Unk54Struct` (unk0=-1, unk4=0, unk8=0x140014) whole-struct-copied
 * by func_8004CDA4 into self+0x8C+key*0xC. */
extern Unk54Struct D_80086990;

/*
 * func_8004B700's per-outer-loop-iteration key/enable pair, read from its
 * own `arg3` parameter (a 2-byte-strided array, one entry per element of
 * `self->arr`). `key` is copied into the resolved Elem's own `unk2` and
 * doubles as an index into `D_80086838` (`key * 0xC`, i.e. `D_80086838
 * + key`, since that table's own stride is 0xC == sizeof(Unk54Struct));
 * `flag` gates the whole per-element body (skip if 0).
 */
typedef struct TargetSpec866E8 {
    u8 key;   /* +0x0 */
    u8 flag;  /* +0x1 */
} TargetSpec866E8;

/* Data table, 0xC-byte stride, indexed by `TargetSpec866E8::key` in
 * func_8004B700 -- reuses `Unk54Struct`'s shape (three consecutive `s32`
 * words) since that is exactly how func_8004B700 reads it (offsets
 * 0x0/0x4/0x8, all as plain `s32`, no evidence of any other width). Bound
 * unknown from this unit alone (`key` is an arbitrary byte from the
 * caller), so left unsized. */
extern Unk54Struct D_80086838[];

/* Called once per element from func_8004B700's outer loop with seven
 * arguments: `self`, the stack-buffer slot being filled (`&stackBuf[count]`,
 * a `SetupEntry866E8*`), `self->unk68->divisor`, the `val / divisor`
 * quotient's low bit, `val` itself, the earlier
 * `func_8004B930(self, val, flag)` result, and `arg3[i].key`. Matched this
 * round -- see docs/match-reports/func_8004BA40.md. */
extern s32 func_8004BA40(Obj866E8 *self, SetupEntry866E8 *arg1, s32 divisor, s32 flag, s32 val, s32 savedResult, s32 key);
/* NOTE: return type corrected this round from `void` to `s32` (0 or 1,
 * whether the mask test at the top passed) -- retail explicitly sets
 * $v0 to 0 or 1 on every path before returning, which a `void` function
 * would never do. func_8004B700, the only caller, discards it. */

/* `key`-indexed bitmask table, one bit per key (`1 << key`), tested against
 * `savedResult` (func_8004B930's return, forwarded through func_8004B700) by
 * func_8004BA40. Bound is PROVEN, not guessed: the data file places exactly
 * 7 words here (0x8008688C-0800868A8) before D_800868A8 starts, and
 * D_800868A8 below is independently proven to hold exactly 7 `Unk54Struct`
 * entries (0x800868A8-0x800868FC) -- same key domain, consistent. */
extern const s32 D_8008688C[7];

/* `key`-indexed, reuses `Unk54Struct`'s 3-`s32` shape (same evidence as
 * `D_80086838` above: offsets 0x0/0x4/0x8, plain words). Read by
 * func_8004BA40 as: `unk0` gates a `divisor * unk0` multiply (its low 32
 * bits used, `unk4` or `unk8` added depending on a caller-supplied `flag`);
 * when `unk0 == 0` the multiply is skipped entirely and `unk4` alone is
 * used. Sized at 7 (see D_8008688C's comment for why this one is provable
 * where `D_80086838` above is not). */
extern const Unk54Struct D_800868A8[7];

/* `ElemTarget::unk32`-indexed remap table, read by func_8004B5BC as a
 * signed byte (`lb`). 8-entry bound is PROVEN, not guessed: the data file
 * (`asm/data/76DC8.data.s`) places exactly 8 bytes here (values
 * `01 02 03 00 04 05 06 00`) before `D_80086904` starts. The fetched byte
 * (range 0..6) doubles as func_8004B5BC's own return value and, scaled by
 * 4, as the index into `D_80086974` below. */
extern const s8 D_800868FC[8];

/* 7-entry pointer table, first entry NULL, indexed by `D_800868FC`'s
 * fetched byte in func_8004B5BC. Bound PROVEN by the data file: exactly 7
 * words at `D_80086974` (one NULL, six pointers into the 4-word tables
 * `D_80086914`..`D_80086964`) before the next symbol starts. Element type
 * `s32 *` matches `Obj866E8Methods::slotF8`'s own `arg3` (already `s32 *`
 * from `func_8004B38C`'s call site) -- func_8004B5BC forwards a
 * `D_80086974` entry there unchanged. */
extern s32 *D_80086974[7];

/* Only the slots this unit's functions dispatch through (via
 * self->methods->slotNN) are typed; everything else stays opaque so the
 * struct keeps the right size/offsets without requiring every method to be
 * typed up front (same policy as include/class_39e08.h). */
typedef struct Obj866E8Methods {
    u8 pad000[0x30];
    /* = BasicClass::notifyParents (`BasicClass__NotifyParents`, classtable-
     * verified against `D_800866E8`'s own +0x030 entry) -- this class
     * inherits the base BasicClassMethods layout for its low slots (see
     * `include/code_8220.h`). Called by func_8004B5BC as
     * `self->methods->slot30(self, 5)` when the just-copied descriptor's
     * leading raw halfword differs from what was there before. */
    void (*slot30)(Obj866E8 *self, s32 arg1);  /* +0x030 */
    u8 pad034[0x60 - 0x34];
    /* Called by round 45's func_800518F4/func_80051998 as
     * `self->methods->slot60(self, 0)`, tail of the countdown/flush
     * "record + notify" path, when their own trailing flag argument is
     * set. Return unused. */
    void (*slot60)(Obj866E8 *self, s32 arg1);  /* +0x060 */
    u8 pad064[0x88 - 0x64];
    /* Called by func_8004BD14 with a literal 7, one of the object's own
     * Elem array slots, and the loop index. */
    void (*slot88)(Obj866E8 *self, s32 arg1, Elem *entry, s32 arg3); /* +0x088 */
    u8 pad08C[0xA4 - 0x8C];
    /* = the "flush all" finalizer, class_3bb8c_j round 15: called once by
     * func_80051858 right after its countdown-flush loop, with (self,
     * self->unk18, literal 1). Return unused. */
    void (*slotA4)(Obj866E8 *self, s32 arg1, s32 arg2);   /* +0x0A4, func_80051858 */
    /* = the countdown/flush callback, class_3bb8c_j round 15: called by
     * func_80051784 (self, self->unk18, decremented countdown, literal 1),
     * func_80051814 (self, self->unk18, literal 0, literal 1) and
     * func_80051858's own loop (self, loop index, self->unk1C, literal 0).
     * Argument MEANING differs per call site (index vs. self->unk18) but
     * all three pass exactly 3 args beyond self; return unused. */
    void (*slotA8)(Obj866E8 *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0A8, func_80051784/func_80051814/func_80051858 */
    u8 pad0AC[0xC0 - 0xAC];
    /* Called by func_8004B57C right before it zeroes self->unk70. */
    void (*slotC0)(Obj866E8 *self);            /* +0x0C0 */
    u8 pad0C4[0xF8 - 0xC4];
    /* Called by func_8004B38C with func_8004B44C's own return value, the
     * SAME stack buffer that was func_8004B44C's `outBuf` argument, and
     * the address of an unidentified global (`D_80086904`). */
    s32 (*slotF8)(Obj866E8 *self, s32 arg1, s32 *arg2, s32 *arg3);   /* +0x0F8 */
    /* = func_8004BB3C. This IS func_8004BB3C's own identity slot (verified
     * via classtable), not something func_8004BB3C calls -- its actual
     * signature is `(self, arr1, count)`, matching a `SetupEntry866E8`
     * array and a count, per func_8004BB3C's own stalled-but-structurally-
     * derived body (see docs/match-reports/func_8004BB3C.md). Called by
     * func_8004B700 (this round: STALLED at 125/140, register identity only)
     * at the end of its own loop with
     * a 7-slot stack buffer it filled and the number of slots actually
     * used. */
    void (*slotFC)(Obj866E8 *self, SetupEntry866E8 *arr1, s32 count); /* +0x0FC */
    u8 pad100[0x104 - 0x100];
    /* Called by func_8004BD14 with one of the object's own Elem array
     * slots. */
    void (*slot104)(Obj866E8 *self, Elem *entry);  /* +0x104 */
    /* = called by func_8004BB3C (this round) with one of the object's own
     * Elem array slots, in BOTH branches of an `arr1[i].ptr0 != 0` test --
     * guarded by the SAME `entry->unk4->unk2C != 0` condition each time.
     * Return value unused. Distinct from slot104 above, which
     * func_8004BD14 dispatches under a different (mode==1) condition. */
    void (*slot108)(Obj866E8 *self, Elem *entry); /* +0x108 */
    /* Called by func_8004CC74 with its own stack-local query buffer
     * (see `CC74QueryBuf`) and a literal 0; return value unused there.
     * Also called by func_8004C6A8 with the identical (own stack-local
     * CC74QueryBuf, 0) shape; return value unused there either. */
    s32 (*slot10C)(Obj866E8 *self, void *outBuf, s32 arg2); /* +0x10C */
    /* = func_8004C1C0 (this round: STALLED at 72/106). Resolves `in` (may be NULL at
     * other call sites; func_8004C1C0 itself never null-checks it) via
     * slot11C, then fills `out`. Returns 0 on success, 1 if the slot11C
     * lookup misses. Matches class_3ac78's independent view of the same
     * slot -- `self->methods->slot110(self, &buf, gateArg)` in that unit's
     * func_8004AEA4 uses the identical (out-pointer, in-pointer) argument
     * order, which is what fixed these two params as pointers rather than
     * the previous round's placeholder (s32, void*). func_8004C158 (this
     * unit) passes its own `arg1` param through as `out` and a computed
     * pointer as `in` -- see func_8004C158's retyped signature below. */
    s32 (*slot110)(Obj866E8 *self, Descriptor10Ext *out, QueryPos866E8 *in);   /* +0x110 */
    u8 pad114[0x118 - 0x114];
    /* Called by func_8004C470 with a plain array index (0..6); the
     * returned pointer is subsequently read like an Elem slot accessor,
     * so this is almost certainly `return &self->arr[index];` -- not
     * this round's function to match. */
    Elem *(*slot118)(Obj866E8 *self, s32 index);   /* +0x118 */
    /* Called by func_8004C1C0 with its own `in` (QueryPos866E8*) param,
     * forwarded opaquely; return value dereferenced exactly like an Elem
     * (->unk4, ->unkC), so this is almost certainly an Elem-lookup sibling
     * to slot118 (func_8004C434) -- not this round's function to match
     * (func_8004C470, still INCLUDE_ASM in this unit). */
    Elem *(*slot11C)(Obj866E8 *self, QueryPos866E8 *arg1);   /* +0x11C */
    /* Called twice by func_8004CAF0, each time with a small offset off
     * its own arg3; the return value is stored as a freshly-created
     * GridSlot866E8's `elemIdx`. */
    s32 (*slot120)(Obj866E8 *self, s32 arg1);      /* +0x120 */
    /* Called by func_8004CDA4 with its own arg3 (unmodified); the return
     * value is stored into the first word of a freshly-copied 3-word
     * slot at self+0x8C+key*0xC (see func_8004CDA4). */
    s32 (*slot124)(Obj866E8 *self, s32 arg1);      /* +0x124 */
    /* = func_8004C620 (class_3bb8c_b, already matched: `void
     * func_8004C620(Obj866E8 *self)`). Called by func_8004B5BC as
     * `self->methods->slot128(self)`, return unused. Classtable-verified
     * (`D_800866E8`'s own +0x128 entry). */
    void (*slot128)(Obj866E8 *self);               /* +0x128 */
} Obj866E8Methods;

/*
 * Opaque object pointed to by UnkCObj::unk14. `unk1C` established by this
 * round's func_8004C1C0 (plain s32, single-width read). `unk18`/`unk20`
 * were originally typed plain s32 from func_8004C470 (still INCLUDE_ASM,
 * so provisional); func_8004C1C0 (STALLED, 72/106) reads them BOTH as a full s32
 * (coarse) and, separately and later in the function, as just the low
 * `u16` half (fine) -- retail re-loads from memory at the narrower width
 * rather than deriving it from the already-loaded s32, so each is a union
 * here rather than a plain field (same reasoning as `QueryPos866E8`
 * above). func_8004C470's own s32-only reads remain valid against the
 * `.w` member, so this is additive, not a contradiction of what it
 * established.
 */
struct Unk14Obj {
    /* func_8004B700 (round: charlie/4): cleared to 0 right after unk18/
     * unk1C/unk20 are filled -- a genuine RELOAD of the same Unk14Obj*
     * (not the same register kept live), so it is a real memory write, not
     * dead code. */
    s32 unk0;                         /* +0x000 */
    u8 pad4[0x18 - 0x4];
    union { s32 w; u16 h; } unk18;   /* +0x018 */
    s32 unk1C;                        /* +0x01C, func_8004C1C0 */
    union { s32 w; u16 h; } unk20;    /* +0x020 */
};

/*
 * Opaque object returned by Obj866E8Methods::slot118 -- read as +0x00C is
 * a pointer to a Unk14Obj. Deliberately a DIFFERENT top-level type from
 * `Elem` even though slot118's return value is later read exactly like an
 * Elem-array-slot accessor would suggest, because this round's evidence
 * (func_8004C470) only reaches the +0x00C field, never Elem's own +0x000
 * or +0x004 -- unifying them would be guessing past the evidence.
 */
struct UnkCObj {
    u8 pad00[0x14];
    Unk14Obj *unk14;    /* +0x014, func_8004C470 */
};

/* self+0xEC's array-element target object. func_8004C0AC dispatches
 * through its own method table (offset 0) and reads a signed halfword at
 * +0x030; func_8004C3F0 (via self->unk1BC->unk4) and func_8004C1C0 (not
 * this round's function) read the SAME +0x030 field, and func_8004C434
 * (already matched) established +0x032. func_8004BD14 additionally reads
 * +0x02A/+0x02C/+0x02E on this same pointer. */
struct ElemTargetMethods {
    u8 pad000[0x74];
    /* = called by func_8004BB3C (this round) as `e->unk4->methods->slot74(
     * e->unk4)` when `e->unk4->unk2A != 0`, right before zeroing the
     * Elem's own `flag`. Single arg (self), return unused. */
    void (*slot74)(ElemTarget *self);  /* +0x074 */
    /* = called by func_8004BB3C (this round) as `e->unk4->methods->slot78(
     * e->unk4, arr1[i].ptr0)` -- the SAME raw pointer that gated the
     * branch (tested non-NULL, then forwarded verbatim). Return unused. */
    void (*slot78)(ElemTarget *self, void *arg1);  /* +0x078 */
    /* Called by func_8004C0AC as `entry->unk4->methods->slot7C(entry->unk4,
     * entry)` -- dispatch target is the ElemTarget itself, not self. */
    void (*slot7C)(ElemTarget *self, Elem *entry);  /* +0x07C */
};

struct ElemTarget {
    ElemTargetMethods *methods;    /* +0x000, func_8004C0AC */
    u8 pad004[0x010 - 0x004];
    /* func_8004BE54: a small size/offset header block, read at ITS OWN
     * +0x004 and +0x008 (both s32) and combined with this pointer's own
     * address to build byte ranges for a resource-load request. Full body
     * (`ResInfo866E8`) kept in class_3bb8c.c -- nothing else in this unit
     * needs it. */
    ResInfo866E8 *field10;          /* +0x010, func_8004BE54 */
    u8 pad014[0x02A - 0x014];
    u16 unk2A;                     /* +0x02A, func_8004BD14 */
    s16 unk2C;                     /* +0x02C, func_8004BD14/func_8004C5D0 (nonzero test) */
    s16 unk2E;                     /* +0x02E, func_8004BD14 */
    s16 unk30;                     /* +0x030, func_8004C0AC/func_8004C3F0/func_8004C5D0 */
    s16 unk32;                     /* +0x032, func_8004C434/func_8004C588 */
};

/*
 * self+0xEC's array element, one of Obj866E8::arr[7]. Established from
 * func_8004BCE0 (+0x000), func_8004C434 (+0x004), and this round's
 * func_8004C0AC (+0x00C indirectly via unk4, +0x010) and func_8004BD14
 * (+0x000, +0x004). class_3bb8c_b's func_8004D1D0 independently reached
 * the same +0x010 pointer array, walked over the same 0x668 raw bytes,
 * and func_8004C5D0/func_8004C588 the same +0x004 target.
 */
struct Elem {
    u16 flag;                      /* +0x000 */
    /* func_8004B700 (round: charlie/4): copied from `arg3[i].key` (a raw
     * byte, zero-extended then stored as a halfword) and, in a second
     * separate loop over all 7 elements, copied onward into
     * `unk4->unk32` (already established). */
    u16 unk2;                      /* +0x002 */
    ElemTarget *unk4;               /* +0x004 */
    /* func_8004BE54: a per-frame GPU link/load coordinator -- its own
     * vtable slot78 drives that function's whole loop. Full body
     * (`LinkTarget866E8`) kept in class_3bb8c.c, this unit's own reading
     * of a class none of Elem's other established fields touch. */
    LinkTarget866E8 *unk8;           /* +0x008, func_8004BE54 */
    UnkCObj *unkC;                  /* +0x00C, func_8004C470 (via slot118's return) */
    /* func_8004C0AC: array of pointers, walked over 0x668 raw bytes --
     * the true element count is not a round number of elements, so this
     * stays byte-offset arithmetic rather than a sized array. */
    EntryChildObj **unk10;           /* +0x010 */
    u8 pad14[0x1C - 0x14];
};

/*
 * Target of Elem::unk10[i]. TWO runners derived this object independently
 * in round 8 from opposite ends and the head merged them here; the split
 * views are why the offsets below have unrelated provenance:
 *  - class_3bb8c (func_8004C0AC) reached the DATA: it ORs a flag bit into
 *    +0x010 and zeroes +0x018 and +0x020.
 *  - class_3bb8c_b (func_8004D0D0/func_8004D108, via func_8004D1D0)
 *    reached the METHOD TABLE at +0x000 and the one slot those two
 *    dispatch through.
 * They agree on the object's identity (both reach it as Elem::unk10[i]),
 * so this is one type, not two independent views -- the
 * independent-views convention applies ACROSS unit headers, and these two
 * units share this one.
 */

typedef struct EntryChildObjMethods {
    u8 pad00[0x48];
    /* Called by func_8004D0D0 (arg2 = the parent's own unk1E4) and
     * func_8004D108 (arg2 = &D_800869CC). Return value unused by both. */
    void (*slot48)(EntryChildObj *self, s32 arg1, void *arg2); /* +0x048 */
} EntryChildObjMethods;

struct EntryChildObj {
    EntryChildObjMethods *methods;  /* +0x000, func_8004D0D0/func_8004D108 */
    u8 pad04[0x10 - 0x04];
    u32 unk10;                      /* +0x010, func_8004C0AC: OR'd with 0x80000000; func_8004CE24: bit31 set/cleared per its own arg1 */
    /* func_8004BE54: a GsCOORDINATE2-shaped per-entry GPU link record --
     * full body (`EntryGpu`) kept in class_3bb8c.c. Note this sits right
     * where a `GsDOBJ2` embedded at THIS object's own +0x010 would put its
     * `coord2` field (`GsDOBJ2::attribute` at +0x000 lines up with this
     * object's own +0x010 `unk10`, used as GsLinkObject4's `objp`) --
     * consistent, not asserted as the real PSYQ type here. */
    EntryGpu *unk14;                 /* +0x014, func_8004BE54 */
    s32 unk18;                      /* +0x018, func_8004C0AC: zeroed */
    u8 pad1C[0x20 - 0x1C];
    s32 unk20;                      /* +0x020, func_8004C0AC: zeroed */
    u8 pad24[0x36 - 0x24];
    s16 unk36;                       /* +0x036, func_8004BE54 */
    EntryChildObj *unk38;           /* +0x038, func_8004CE24: singly-linked chain, walked while non-NULL */
};

/*
 * Opaque target of self->unk1BC. Only field established: func_8004C3F0
 * reads +0x004, a pointer that turns out to be the same ElemTarget type
 * (its own +0x030 field is read straight afterward, matching ElemTarget's
 * already-established +0x030).
 */
struct Unk1BCObj {
    u8 pad0[0x4];
    ElemTarget *unk4;               /* +0x004, func_8004C3F0 */
};

/*
 * self+0x6C->unk14's pointee (round 13, func_8004C6A8). Only field
 * established: +0x044, a further pointer (Unk6C14SubObj*).
 */
struct Unk6C14Obj {
    u8 pad00[0x44];
    Unk6C14SubObj *unk44;           /* +0x044, func_8004C6A8 */
};

/*
 * Unk6C14Obj::unk44's pointee. `+0x010` is only ever address-taken
 * (forwarded raw to RotMatrix, never dereferenced in this unit);
 * `+0x012` is read BOTH as a signed halfword (to test its sign, adding
 * 0x1000 to the unsigned reading when negative -- a 12-bit two's
 * complement unpack) and, separately, as the resulting unsigned value
 * used for the range dispatch that follows. func_8004C158's own
 * `(u8 *)self->unk6C->unk14 + 0x18` -- one level UP the chain, on
 * Unk6C14Obj's OWN address, not this sub-object -- stays a raw cast in
 * that unit, untouched here.
 */
struct Unk6C14SubObj {
    u8 pad00[0x12];
    u16 unk12;                      /* +0x012, func_8004C6A8 */
};

/*
 * self+0x6C's pointee. func_8004B38C only ever STORES its own arg2 here
 * raw (never dereferences it); func_8004C158 dereferences it and reads
 * +0x014, itself a pointer used only for address-of-plus-offset
 * arithmetic (`+0x018`), never further dereferenced. Retyped this round
 * (round 13, func_8004C6A8) from `void *` to `Unk6C14Obj *` -- the ONLY
 * other reader, func_8004C158, already casts it straight to `(u8 *)`
 * before doing arithmetic, so the retype does not change that already-
 * matched function's bytes (verified: full rebuild stays green).
 */
struct Unk6CObj {
    u8 pad00[0x14];
    Unk6C14Obj *unk14;              /* +0x014, func_8004C158/func_8004C6A8 */
};

/*
 * Template struct copied wholesale by func_8004C6A8 from the constant
 * global `D_8008E98C` into a stack-local descriptor, then partially
 * overwritten (`unk14`/`unk18` zeroed, `unk1C` set from `self->unk74`)
 * before being handed to two uncarved library helpers
 * (`RotMatrix`/`ApplyMatrixLV`) as an in/out parameter block. Field
 * meaning beyond "8 words, offsets 0x00-0x1C" is unestablished; the first
 * five words are read/written only as the opaque whole-struct copy.
 */
struct QueryTemplate866E8 {
    s32 unk0[5];                    /* +0x000..+0x010, opaque (untouched by func_8004C6A8) */
    s32 unk14;                      /* +0x014, func_8004C6A8: zeroed before the call, then an in/out arg to ApplyMatrixLV */
    s32 unk18;                      /* +0x018, func_8004C6A8: zeroed before the call */
    s32 unk1C;                      /* +0x01C, func_8004C6A8: set to self->unk74 before the call */
};

extern QueryTemplate866E8 D_8008E98C;

/* Uncarved library helpers (round 13, func_8004C6A8's only known call
 * site). `RotMatrix`'s first argument is `(u8 *)sub + 0x10` where
 * `sub` is a `Unk6C14SubObj *` -- never dereferenced in this unit, so
 * typed as a raw pointer rather than claiming a struct shape for it.
 * `ApplyMatrixLV` is called with its 2nd and 3rd arguments pointing at
 * the SAME address (`&desc.unk14` passed twice) -- confirmed against the
 * raw disassembly (`$a1`/`$a2` both `sp+0x54`), not a transcription
 * shortcut. */
extern void RotMatrix(void *arg0, QueryTemplate866E8 *arg1);
extern void ApplyMatrixLV(QueryTemplate866E8 *arg0, s32 *arg1, s32 *arg2); /* arity-ok: this IS the callee's real signature (Sony libgte, 0x80015618 reads $a0 matrix / $a1 in / $a2 out); include/code_d294.h's unprototyped copy is round 19's deliberate frame-sizing shape, not a claim about arity */

/*
 * Opaque target of Obj866E8::unk1DC (func_8004CFB0 stores it raw;
 * func_8004CD38 -- a plain, non-virtual helper, NOT a vtable slot, see
 * tools/classtable.py D_800866E8 -- dereferences it as a min/max bounding
 * box against an [x,y] byte pair). Field meaning inferred from the four
 * comparisons in func_8004CD38: `unk0`/`unk2` gate the LOW side, `unk4`/
 * `unk8` the HIGH side, of the point's two axes respectively.
 */
typedef struct Bounds866E8_3bb8c_b {
    s16 unk0;                      /* +0x000, func_8004CD38: point[0] < this -> out of range */
    s16 unk2;                      /* +0x002, func_8004CD38: point[1] < this -> out of range */
    s32 unk4;                      /* +0x004, func_8004CD38: this < point[0] -> out of range */
    s32 unk8;                      /* +0x008, func_8004CD38: this < point[1] -> out of range (also the function's own return value) */
} Bounds866E8_3bb8c_b;

/*
 * self->unk1E4's pointee: one of four static 0xC-byte table entries at
 * D_8008699C/D_800869A8/D_800869B4/D_800869C0 (addresses confirmed 0xC
 * apart), selected by func_8004CFB8 from (rate > 0, flag != 0) and never
 * dereferenced past +0x006. The same 0xC stride lines up with
 * D_800869CC (declared `extern s32 D_800869CC[3]` below, by
 * func_8004D108) as a plausible fifth entry of the same table, but
 * nothing in this unit reaches that entry through THIS pointer type, so
 * the two stay independently declared rather than unified into one
 * array of unproven length.
 */
typedef struct EntryDesc866E8 {
    u8 pad0[0x6];
    s16 unk6;          /* +0x006, func_8004CFB8: multiplied against abs(rate) */
    u8 pad8[0xC - 0x8];
} EntryDesc866E8;

extern EntryDesc866E8 D_8008699C;
extern EntryDesc866E8 D_800869A8;
extern EntryDesc866E8 D_800869B4;
extern EntryDesc866E8 D_800869C0;

/*
 * self+0x8C's array element (`Obj866E8::slots8C`, see below). Established
 * from func_8004CE24, which reads all five fields: `elemIdx` selects
 * `self->arr[elemIdx]`; `h4`/`h6` locate a starting cell in that element's
 * `unk10` pointer grid (row stride 20 cells, confirmed by the `* 20`
 * offset math); `h8`/`hA` are the sub-rectangle's width/height walked
 * from that starting cell. func_8004CDA4 (already matched, a different
 * unit's round) writes a whole one of these via a 3-word block copy using
 * the coarser, already-committed `Unk54Struct` view of the SAME memory --
 * per this project's independent-views convention, that write-side view
 * is left alone; this is a separate, more granular READ-side view of the
 * same 0xC bytes, justified because a whole-struct copy does not care
 * about the internal layout it is copying.
 */
typedef struct GridSlot866E8 {
    s32 elemIdx;   /* +0x0, func_8004CE24: selects self->arr[elemIdx] */
    s16 h4;        /* +0x4, func_8004CE24: starting column */
    s16 h6;        /* +0x6, func_8004CE24: starting row (row stride 20) */
    s16 h8;        /* +0x8, func_8004CE24: sub-rectangle width */
    s16 hA;        /* +0xA, func_8004CE24: sub-rectangle height */
} GridSlot866E8;

/*
 * func_8004CC74's own stack-local query buffer, filled by a call through
 * `Obj866E8Methods::slot10C` and read back at two offsets: `+0x2` (a
 * signed [x,y] byte pair, forwarded to func_8004CD38 as its `point`
 * argument) and `+0x28` (a plain `s32`, read directly by func_8004CC74
 * itself). Everything else is unproven -- this is a local, not part of
 * `Obj866E8`, so it stays a minimal opaque type sized only to cover the
 * two known offsets.
 */
typedef struct CC74QueryBuf {
    u8 pad0[0x2];
    s8 point[2];        /* +0x2, func_8004CC74: forwarded to func_8004CD38 */
    u8 pad4[0x28 - 0x4];
    s32 count;          /* +0x28, func_8004CC74 */
} CC74QueryBuf;

struct Obj866E8 {
    Obj866E8Methods *methods;      /* +0x000 */
    u8 pad04[0x0C - 0x04];
    s32 unkC;                      /* +0x00C, func_8004D678 (compared against 9999999) */
    /*
     * +0x010..+0x048: a "countdown/flush" subsystem, established by
     * class_3bb8c_j (round 15) from four functions (func_80051784,
     * func_800517EC, func_80051814, func_80051858) plus two gp_rel-blocked
     * siblings in the same unit (func_800518F4, func_80051998, both confirmed
     * to touch the SAME unk48/unk18/unk1C fields -- see those functions'
     * stub reports). unk10 is a trip count read once per "flush all" call
     * (func_80051858); unk14 is the countdown's own reset value; unk18 is
     * forwarded as methods->slotA8/slotA4's own arg1; unk1C is the live
     * countdown, decremented per call and reset from unk14 on expiry; unk20
     * is an unrelated boolean toggled independently by func_800517EC; unk48
     * is a readiness/enable gate every function in the group tests non-zero
     * before doing anything, never itself dereferenced in this unit.
     */
    s32 unk10;                     /* +0x010, func_80051858 */
    s32 unk14;                     /* +0x014, func_80051784 */
    s32 unk18;                     /* +0x018, func_80051784/func_80051814/func_80051858 */
    s32 unk1C;                     /* +0x01C, func_80051784/func_80051814/func_80051858 */
    s32 unk20;                     /* +0x020, func_800517EC (xor-toggled) */
    u8 pad24[0x28 - 0x24];
    u8 *unk28;                     /* +0x028, round 45's func_80051998: a per-index byte buffer, `unk28[arg1] = table[idx]` */
    u8 pad2C[0x40 - 0x2C];
    /* +0x040/+0x044, round 45's func_800518F4/func_80051998: opaque
     * resource-handle objects, dispatched only through this unit's own
     * local method-table views (Unk40Obj866E8Methods/Unk44Obj866E8Methods
     * in class_3bb8c_j.c) -- kept `void *` here since nothing outside that
     * unit touches them yet. */
    void *unk40;                   /* +0x040 */
    void *unk44;                   /* +0x044 */
    s32 unk48;                     /* +0x048, func_80051784/func_800517EC/func_80051814/func_80051858 */
    u8 pad4C[0x54 - 0x4C];
    Unk54Struct unk54;             /* +0x054, func_8004B418 (address taken, forwarded opaquely) */
    /* +0x060/+0x064, func_8004BA40 (this round): a callback invoked as
     * `unk60(unk64, value, 0, 0)`, whose result is stored into the
     * SetupEntry866E8 slot being filled. `unk64` is never dereferenced in
     * this unit, only forwarded -- opaque context pointer. Was undifferentiated
     * padding (`pad60[0x68 - 0x60]`) before this round; the split is
     * additive (same total size, same offsets), not a removal. */
    void *(*unk60)(void *arg0, s32 arg1, s32 arg2, s32 arg3); /* +0x060 */
    void *unk64;                                              /* +0x064 */
    Unk68Struct *unk68;            /* +0x068, func_8004B418/func_8004B38C/func_8004B930/func_8004C470 */
    Unk6CObj *unk6C;               /* +0x06C, func_8004B38C stores it raw; func_8004C158 dereferences it */
    s32 unk70;                     /* +0x070, func_8004B570/func_8004B57C */
    s32 unk74;                     /* +0x074, func_8004C6A8: copied into its stack-local QueryTemplate866E8's unk1C before calling RotMatrix */
    s16 unk78;                     /* +0x078, func_8004C620 (halfword, doubled into an index) */
    s16 unk7A;                     /* +0x07A, func_8004C620 (halfword, passed on as an arg) */
    s16 unk7C;                     /* +0x07C, func_8004C93C: a signed sub-cell horizontal offset, clamped into [0,0x14) and combined with unk80 to decide whether the grid footprint spans one or two 20-unit cells; also written directly by func_8004C6A8 */
    s16 unk7E;                     /* +0x07E, func_8004C93C: same convention as unk7C, vertical; also written directly by func_8004C6A8 */
    s32 unk80;                     /* +0x080, func_8004C6A8 (writes arg1 or arg2 depending on its own dispatch), then read/forwarded by func_8004C93C to func_8004CAF0's p7 */
    s32 unk84;                     /* +0x084, func_8004C6A8 (writes the other of arg1/arg2), then read/forwarded by func_8004C93C to func_8004CAF0's p8 */
    s32 unk88;                     /* +0x088, func_8004CE24: loop count over slots8C[] (bounded by slots8C's own 4-element capacity) */
    GridSlot866E8 slots8C[4];      /* +0x08C, func_8004CE24 (reads); func_8004CDA4 (writes, via the coarser Unk54Struct view) -- exactly fills the gap up to the existing unkBC field, so this is a hard capacity, not a guess */
    Descriptor10 unkBC;            /* +0x0BC, func_8004B38C: whole-struct copy from its arg3 */
    u8 padC6[0xEC - 0xC6];
    Elem arr[7];                   /* +0x0EC, func_8004BCE0/func_8004C434/func_8004C588/func_8004C5D0/func_8004BD14/func_8004D1D0 */
    s32 unk1B0;                    /* +0x1B0, func_8004BD14 */
    u16 unk1B4;                    /* +0x1B4, func_8004BD14 */
    u8 pad1B6[0x1B8 - 0x1B6];
    s32 unk1B8;                    /* +0x1B8, func_8004BD14 */
    Unk1BCObj *unk1BC;             /* +0x1BC, func_8004C3F0 */
    u8 pad1C0[0x1CC - 0x1C0];
    s32 unk1CC;                    /* +0x1CC, func_8004CFA8 (address-of only, real type unknown) */
    u8 pad1D0[0x1DC - 0x1D0];
    Bounds866E8_3bb8c_b *unk1DC;   /* +0x1DC, func_8004CFB0 (stores raw)/func_8004CD38 (dereferences) */
    s32 unk1E0;                    /* +0x1E0, func_8004D028/func_8004D088: a countdown gate */
    EntryDesc866E8 *unk1E4;        /* +0x1E4, func_8004D0D0 (forwarded opaquely)/func_8004CFB8 (selects one of four statics and reads +0x6) */
    u8 pad1E8[0x2F4 - 0x1E8];
    s32 unk2F4;                    /* +0x2F4, func_8004D678: zero-checked when unkC > 9999999 */
};

/* Get-vtable helper, same shape and same real function as
 * class_3ac78.h's `func_8004D244` (independent view: this unit names the
 * return type Obj866E8Methods, not Class866E8Methods). It now has a real
 * body in this unit (class_3bb8c_b); class_3ac78 still calls it via `jal`
 * as a raw external. */
extern Obj866E8Methods D_800866E8;

/* Still raw asm in this unit (not this round's target): walks
 * item->unk10[] (an array of EntryChildObj*, up to +0x668 bytes
 * from the base read at item->unk10), calling callback(self, element) for
 * each. Derived to resolve func_8004D0D0/func_8004D108's true call site --
 * see those functions' reports. Not called by name anywhere in this
 * unit's own C (only from within func_8004D140's still-raw body), so this
 * prototype is documentation, not load-bearing. */
extern void func_8004D1D0(Obj866E8 *self, void (*callback)(Obj866E8 *self, EntryChildObj *item), Elem *item);

/* Still raw asm in this unit (not this round's target): iterates
 * self->arr, invoking an optional per-element callback (arg2, called
 * (self, &arr[i]) when non-NULL) and then always forwarding (self, arg1,
 * &arr[i]) to func_8004D1D0. func_8004D028/func_8004D088 both call it
 * with arg2 = NULL (no per-element callback), passing a function POINTER
 * as arg1 instead -- that pointer is consumed further down in
 * func_8004D1D0, not by this function itself. */
extern void func_8004D140(Obj866E8 *self, void (*arg1)(Obj866E8 *self, EntryChildObj *item), void (*arg2)(Obj866E8 *self, Elem *item));

/* 3-word (12-byte) data block, address-of only -- passed to
 * EntryChildObjMethods::slot48 as an opaque arg2 by func_8004D108.
 * Never dereferenced in this unit, so left untyped in size only. */
extern s32 D_800869CC[3];

/* -------------------------------------------------------------------
 * class_3bb8c_c additions below. Two small sibling classes, each built
 * by its own New_X/ctor pair (allocator + base-chain + own-vtable-set,
 * the same shape as func_8004A19C in class_39e08.c). Named by their
 * vtable's address, same convention as Class866E8/Class86668.
 * ------------------------------------------------------------------- */

typedef struct Class869D8 Class869D8;
typedef struct Class869D8Methods Class869D8Methods;

/*
 * Vtable D_800869D8 (asm/data/76DC8.data.s, header word 0x17). Only the
 * slots this unit's own functions reach are typed: +0x008 (ctor,
 * func_8004D2A4, called by New_Class869D8/func_8004D254) and +0x040 (a
 * post-construct hook, func_8004D2F8 -- already matched, empty body).
 */
struct Class869D8Methods {
    u8 pad000[0x008];
    void (*ctor)(Class869D8 *self);            /* +0x008, func_8004D2A4 */
    u8 pad00C[0x040 - 0x00C];
    void (*slot40)(Class869D8 *self);          /* +0x040, func_8004D2F8 */
};

struct Class869D8 {
    Class869D8Methods *methods;                /* +0x000 */
    u8 pad004[0x010 - 0x004];
    s32 unk10;                                  /* +0x010, func_8004D300: gates the slot9C call (nonzero test) */
    u8 pad14[0x070 - 0x014];
    s32 unk70;                                  /* +0x070, func_8004D300: gates the slot9C call (nonzero test) */
    u8 pad74[0x0DC - 0x074];                     /* struct ends at the New_Class869D8 alloc size, 0xDC */
};

extern Class869D8Methods D_800869D8;
extern Class869D8Methods *func_8004D37C(void);

/*
 * Base-class ctor-table getter, chained by func_8004D2A4. Only the ctor
 * slot (+0x008, single `self` argument -- this call site sets up no
 * second argument register) is needed here.
 */
typedef struct BaseCtorTable_3bb8c_c {
    u8 pad0[0x008];
    void (*ctor)(void *self);
    u8 pad00C[0x09C - 0x00C];
    void (*slot9C)(void *self);   /* +0x09C, func_8004D300's forward target (gated by
                                      Class869D8::unk10/unk70 both being nonzero) */
} BaseCtorTable_3bb8c_c;

extern BaseCtorTable_3bb8c_c *func_8003F24C(void);

extern void *func_80017B34(s32 size);

typedef struct Class86AA0 Class86AA0;
typedef struct Class86AA0Methods Class86AA0Methods;

/* Forward declaration: full definition (GenericTagMethods_3bb8c_c /
 * GenericTagInst_3bb8c_c) is below, established from func_8004D434; needed
 * here already for Class86AA0Methods::slotA0's parameter type. */
typedef struct GenericTagInst_3bb8c_c GenericTagInst_3bb8c_c;

/*
 * Vtable D_80086AA0 (asm/data/76DC8.data.s, header word 0x24). Sibling of
 * Class869D8Methods above, same shape: ctor at +0x008 (func_8004D3DC,
 * called by New_Class86AA0/func_8004D38C). +0x0B8 is dispatched by this
 * class's own func_8004D434 (slot +0x09C in the same table); its exact
 * purpose is unestablished beyond "called on self with no other args".
 */
struct Class86AA0Methods {
    u8 pad000[0x008];
    void (*ctor)(Class86AA0 *self);            /* +0x008, func_8004D3DC */
    u8 pad00C[0x0A0 - 0x00C];
    /* Called by func_8004D47C (self's own slot +0x0B8 occupant, see below)
     * when its own arg2 is in [5, 9). arg1 is forwarded opaquely; return
     * value unused. */
    void (*slotA0)(Class86AA0 *self, GenericTagInst_3bb8c_c *arg1, s32 arg2); /* +0x0A0 */
    u8 pad0A4[0x0B8 - 0x0A4];
    /* Declared here 1-argument to match func_8004D434's own call site
     * (`self->methods->slotB8(self)`, already matched) -- but this slot's
     * REAL occupant is func_8004D47C, whose own body reads three args
     * (self, arg1, arg2). Both are right about their own codegen; see
     * func_8004D3DC's report/GetClass6B5CCMethods's declaration below for the
     * identical situation on a different symbol. Not reconciled: widening
     * this field to 3 args would force func_8004D434's call site to
     * synthesize an arg2 it doesn't have, breaking that already-matched
     * function. func_8004D47C's own C definition is typed independently
     * of this field (the vtable's storage is still raw asm data, so
     * nothing here type-checks it either way). */
    void (*slotB8)(Class86AA0 *self);          /* +0x0B8, called by func_8004D434 */
};

struct Class86AA0 {
    Class86AA0Methods *methods;                /* +0x000 */
    u8 pad004[0x034 - 0x004];
    u16 unk34;                                 /* +0x034, func_8004D3DC: zeroed in the ctor */
    u16 unk36;                                 /* +0x036, func_8004D3DC: zeroed in the ctor */
    s32 unk38;                                 /* +0x038, func_8004D3DC: zeroed in the ctor */
};

extern Class86AA0Methods D_80086AA0;
extern Class86AA0Methods *func_8004D508(void);

/* MEASURED, round 9: GetClass6B5CCMethods TAKES NO ARGUMENTS. Its whole body is
 * `lui/addiu %hi/%lo(D_8006B5CC); jr $ra` (asm/code_d294.s) -- it reads
 * neither $a0 nor $a1, and just returns &D_8006B5CC. It is the plain
 * no-parameter vtable getter documented in docs/research/class-framework.md,
 * the same shape as GetClass6D3C8Methods.
 *
 * The arg list below is therefore NOT the callee's signature; it is what THIS
 * call site passes, and it is what this unit's bytes need. src/class_3ac78.c
 * passes TWO args to the same symbol and is equally byte-exact. Both are
 * right about their own codegen and both are wrong about the function.
 * Do not "reconcile" them and do not reduce either to (void) -- that changes
 * the argument setup the caller emits and breaks the match. See
 * docs/match-reports/func_8004D3DC.md.
 *
 * Return type: round 10 split this OFF `BaseCtorTable_3bb8c_c` (func_8003F24C's
 * own return type) into its own `BaseCtorTableB_3bb8c_c`. (The split is round
 * 10's -- round 9 settled only the ARITY question above. The comment here
 * originally credited round 9 with both, which would have made this a settled
 * precedent rather than a fresh judgement open to challenge.)
 *
 * The reason for the split: func_8004D47C (this unit) reaches +0x09C on THIS
 * getter's table with a 3-argument call (self, arg1, arg2) -- a genuine arity
 * conflict with
 * `BaseCtorTable_3bb8c_c::slot9C` (1-argument, established from
 * func_8004D300 via the OTHER getter, func_8003F24C). Same-offset arity
 * conflict means different table/different class, per this project's
 * established split policy (see e.g. TaskCoreObjMethods in
 * include/code_2c054.h). Purely a type-name change here -- func_8004D3DC's
 * own already-matched call (`GetClass6B5CCMethods(self)->ctor(self)`) only touches
 * the +0x008 `ctor` slot, whose layout is identical in both names, so this
 * renaming changes no bytes.
 *
 * Head-verified round 10 by measuring both callees rather than reasoning from
 * the arity conflict: GetClass6B5CCMethods returns &D_8006B5CC (asm/code_d294.s) and
 * func_8003F24C returns &D_8006E8E4 (asm/code_2cc8c_b.s). Different globals,
 * so genuinely different tables -- one type could not have carried both, and
 * the split would have been right even without the arity conflict that
 * prompted it.
 *
 * NOTE: func_8004D47C itself is STALLED, not matched, so the 3-argument
 * `slot9C` below is read off its disassembly rather than proven by a byte
 * match. The offset and the argument count are observed; the parameter TYPES
 * are inferred. */
typedef struct BaseCtorTableB_3bb8c_c BaseCtorTableB_3bb8c_c;
struct BaseCtorTableB_3bb8c_c {
    u8 pad0[0x008];
    void (*ctor)(void *self);                     /* +0x008, func_8004D3DC */
    u8 pad00C[0x09C - 0x00C];
    /* func_8004D47C's forward target (self, arg1 opaque, arg2 int),
     * unconditional first statement of that function. */
    void (*slot9C)(void *self, void *arg1, s32 arg2); /* +0x09C */
};

/* ROUND 59 (extern review): parameter list only, return type untouched.
 * Measured first -- func_8004D3DC's `jal 8001e57c` (0x8004D3E8) carries
 * `move s0,a0` in the delay slot, a callee-save spill and not argument setup,
 * so the `self` this unit passes costs zero bytes. The definition
 * (src/code_d294_b.c:736) is `(void)` and the callee at 0x8001E57C reads no
 * argument register at all, so the one-parameter prototype was simply false.
 * Unspecified parameters is what round 9's own "no prototype in scope" reading
 * means, and it keeps class_3bb8c_c.c's call sites exactly as they are. */
extern BaseCtorTableB_3bb8c_c *GetClass6B5CCMethods();

/*
 * Generic class-instance shape used only to read another object's own
 * vtable header-tag BYTE (the low byte of the header word at the vtable's
 * own +0x000) -- func_8004D434's own second argument is dispatched this
 * way, compared against a literal 0x34.
 */
typedef struct GenericTagMethods_3bb8c_c {
    u8 tag;                                     /* +0x000, low byte of the header word */
} GenericTagMethods_3bb8c_c;

struct GenericTagInst_3bb8c_c {
    GenericTagMethods_3bb8c_c *methods;         /* +0x000 */
};

/*
 * A third small sibling class (New_X/ctor pair, same shape as Class869D8
 * and Class86AA0 above), named by its vtable's address `D_80086B60`
 * (returned by `func_8004E2D0`, still raw asm in the uncarved
 * `asm/class_3bb8c_d.s` -- called directly, not through any vtable).
 * `func_8004D518` is the New_X allocator (alloc size 0xC4); `func_8004D578`
 * is the ctor itself, which SETS `self->methods` directly to this table's
 * own pointer (the base-class constructor chaining pattern already seen
 * in `TaskCoreMethods::slotD8`, include/code_2c054.h) rather than fetching
 * it through another getter first.
 */
typedef struct Class86B60 Class86B60;
typedef struct Class86B60Methods Class86B60Methods;
/* Forward declaration: full definition (DreamSysView_3bb8c_c /
 * DreamSysViewMethods_3bb8c_c) is below, established from func_8004D578;
 * needed early because Class86B60::unkA4 is typed with it. */
typedef struct DreamSysView_3bb8c_c DreamSysView_3bb8c_c;

/*
 * self->unkC's pointee. `unk0` is itself a pointer to a small vtable
 * object (`slot78`, reached by func_8004D898 in a fixed 2-iteration loop
 * alongside a table walk); `unk4` is an opaque value forwarded verbatim
 * by func_8004E054 as an argument to `Class86B60Methods::slot10`.
 */
typedef struct Class86B60UnkCObj_3bb8c_d Class86B60UnkCObj_3bb8c_d;
typedef struct Class86B60UnkC0ObjMethods_3bb8c_d Class86B60UnkC0ObjMethods_3bb8c_d;
typedef struct Class86B60UnkC0Obj_3bb8c_d Class86B60UnkC0Obj_3bb8c_d;

struct Class86B60UnkC0ObjMethods_3bb8c_d {
    u8 pad000[0x078];
    /* +0x078, func_8004D898's own call: `(childObj, &self->unk93,
     * tableEntry)`, where `tableEntry` walks a fixed external table
     * (`D_80086DAC`, stride 0xC) starting fresh each call to this
     * function. */
    void (*slot78)(Class86B60UnkC0Obj_3bb8c_d *self, void *arg1, void *arg2);
};

struct Class86B60UnkC0Obj_3bb8c_d {
    Class86B60UnkC0ObjMethods_3bb8c_d *methods; /* +0x000 */
};

struct Class86B60UnkCObj_3bb8c_d {
    Class86B60UnkC0Obj_3bb8c_d *unk0; /* +0x000, func_8004D898 */
    void *unk4;                        /* +0x004, func_8004E054: opaque, forwarded verbatim */
};

/*
 * self->unk4C's pointee. Only `unk8` is reached, by func_8004D90C, as an
 * opaque value forwarded verbatim to `Class86B60Methods::slotF0`'s 2nd
 * argument.
 */
typedef struct Class86B60Unk4CObj_3bb8c_d Class86B60Unk4CObj_3bb8c_d;
struct Class86B60Unk4CObj_3bb8c_d {
    u8 pad0[0x008];
    void *unk8; /* +0x008, func_8004D90C */
};

/*
 * self->unk60's pointee. Only `unk14` is reached, by func_8004DABC, as a
 * plain `s32` copied into a one-word stack buffer before being forwarded
 * by address to `DreamSysViewMethods_3bb8c_c::slot19C`.
 */
typedef struct Class86B60Unk60Obj_3bb8c_d Class86B60Unk60Obj_3bb8c_d;
struct Class86B60Unk60Obj_3bb8c_d {
    u8 pad0[0x014];
    s32 unk14; /* +0x014, func_8004DABC */
};

/*
 * Generic class-instance shape used only to reach the shared BasicClass-
 * family "release" slot (`+0x004`, same offset as `BasicClassMethods::
 * release` in include/code_8220.h -- "virtual finalize, then free self")
 * on an object whose concrete class this unit does not otherwise need to
 * know. Local, independent view per this project's established
 * multiple-independent-views convention (see e.g. GenericTagInst_3bb8c_c
 * above). func_8004D704 calls this on both `Class86B60::unkA8` and
 * `Class86B60::unkAC`.
 */
typedef struct GenericReleaseMethods_3bb8c_d GenericReleaseMethods_3bb8c_d;
typedef struct GenericReleaseObj_3bb8c_d GenericReleaseObj_3bb8c_d;

struct GenericReleaseMethods_3bb8c_d {
    u8 pad000[0x004];
    void (*release)(GenericReleaseObj_3bb8c_d *self); /* +0x004 */
};

struct GenericReleaseObj_3bb8c_d {
    GenericReleaseMethods_3bb8c_d *methods; /* +0x000 */
};

/*
 * Class86B60::unkB0's pointee. Shares the same `release` slot at `+0x004`
 * as `GenericReleaseObj_3bb8c_d`, but also exposes `+0x04C` (reached by
 * func_8004DC64), so it gets its own local view rather than reusing that
 * minimal type.
 */
typedef struct Class86B60UnkB0ObjMethods_3bb8c_d Class86B60UnkB0ObjMethods_3bb8c_d;
typedef struct Class86B60UnkB0Obj_3bb8c_d Class86B60UnkB0Obj_3bb8c_d;

struct Class86B60UnkB0ObjMethods_3bb8c_d {
    u8 pad000[0x004];
    void (*release)(Class86B60UnkB0Obj_3bb8c_d *self); /* +0x004, func_8004DC08 */
    u8 pad008[0x04C - 0x008];
    /* +0x04C, func_8004DC64's own 2nd call: `(self, arg1, &D_8008A9B4)`. */
    void (*slot4C)(Class86B60UnkB0Obj_3bb8c_d *self, s32 arg1, void *arg2);
    u8 pad050[0x0B8 - 0x050];
    /* +0x0B8, func_8004DCD0's own last call: `(self, buf)` where `buf` is
     * that function's own 3-byte stack buffer. Lands at the SAME offset
     * as `include/class_3bb8c.h`'s own `FieldM7CMethods::slotB8` (a
     * different unit's independent view) and `code_2cc8c.h`'s
     * `Unk64ElemMethods` -- further confirmation (on top of `release`
     * +0x004 and `slot4C` +0x04C already matching both) that `unkB0` is
     * that same real class. */
    void (*slotB8)(Class86B60UnkB0Obj_3bb8c_d *self, void *arg1);
    u8 pad0BC[0x0CC - 0x0BC];
    /* +0x0CC, func_8004DE08's own call: `(self, buf)` where `buf` is a
     * pool-allocated string this function fills with `DecodeFullWidthSjis`
     * before the call and frees right after. */
    void (*slotCC)(Class86B60UnkB0Obj_3bb8c_d *self, void *arg1);
};

struct Class86B60UnkB0Obj_3bb8c_d {
    Class86B60UnkB0ObjMethods_3bb8c_d *methods; /* +0x000 */
    u8 pad004[0x0A9 - 0x004];
    /* +0x0A9, func_8004DE08: read as an unsigned byte and used directly as
     * an allocation SIZE (`func_80017B34`'s own argument). */
    u8 unkA9;
    /* +0x0AA/+0x0AB/+0x0AC, func_8004DB18: three literal byte fields
     * (9/8/4) set right after allocation via `New_Obj6EAC0` -- this is
     * the SAME real object as `code_2cc8c.h`'s `Unk64Elem` (matching
     * slot offsets 0x004/0x04C/0x0B8, see that header's own note), so
     * these three bytes are almost certainly flags/type-tag data on that
     * class; kept opaque byte fields since this unit never reads them
     * back. */
    u8 unkAA;
    u8 unkAB;
    u8 unkAC;
};

/*
 * Class86B60::unkAC's fuller shape -- shares the same `release` slot at
 * `+0x004` as the generic view (established by func_8004D704), but
 * func_8004E054 also reaches `+0x070`. Same reasoning as
 * `Class86B60UnkB0Obj_3bb8c_d` above: kept a dedicated type rather than
 * assuming `unkA8` shares this fuller interface too, since nothing in
 * this unit ever dispatches a second slot on `unkA8`.
 */
typedef struct Class86B60UnkACObjMethods_3bb8c_d Class86B60UnkACObjMethods_3bb8c_d;
typedef struct Class86B60UnkACObj_3bb8c_d Class86B60UnkACObj_3bb8c_d;

struct Class86B60UnkACObjMethods_3bb8c_d {
    u8 pad000[0x004];
    void (*release)(Class86B60UnkACObj_3bb8c_d *self); /* +0x004, func_8004D704 */
    u8 pad008[0x06C - 0x008];
    /* +0x06C, func_8004DF64's own call: `(self, D_8008A9D0, &D_80086D6C,
     * self->unkC->unk4, self->unk10, self->unk14, self->unk48)` -- 7
     * arguments, the last three on the stack. Every pointer beyond `self`
     * is forwarded opaquely (never dereferenced by this slot's own
     * caller), so all stay `void *`/`s32 *` placeholders. */
    void (*slot6C)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, s32 *arg2,
                   void *arg3, void *arg4, void *arg5, void *arg6); /* +0x06C */
    void (*slot70)(Class86B60UnkACObj_3bb8c_d *self); /* +0x070, func_8004E054 */
    /* +0x074, func_8004E1C4's own 2nd call: `(self, D_8008AA10, D_8008AA18,
     * self->unkBC, self->unkC0)` -- the two middle arguments are the
     * VALUES of two `.sdata` globals loaded via `%gp_rel` (not their
     * addresses), each holding a pointer into the still-uncarved rodata
     * block at `D_80011434` (verified in `asm/data/1C34.rodata.s`: no
     * dlabel exists at either byte offset, so they cannot be referenced by
     * name and are forwarded as opaque `void *`). */
    void (*slot74)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, void *arg2, s32 arg3, s32 arg4); /* +0x074 */
    /* +0x078, func_8004E0E4's own call: `(self, D_8008AA10, D_8008AA18,
     * 0xD, 3, self->unkA8, self->unkBC, self->unkC0)` -- eight arguments,
     * the last four on the stack. `arg5` is `Class86B60::unkA8`, forwarded
     * opaquely (never dereferenced by this slot's own caller), so kept
     * `void *` rather than its fuller `GenericReleaseObj_3bb8c_d *` type. */
    void (*slot78)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, void *arg2,
                   s32 arg3, s32 arg4, void *arg5, s32 arg6, s32 arg7); /* +0x078 */
};

struct Class86B60UnkACObj_3bb8c_d {
    Class86B60UnkACObjMethods_3bb8c_d *methods; /* +0x000 */
};

/*
 * Generic class-instance shape used only to read another object's own
 * vtable header WORD (the full `s32` at the vtable's own `+0x000`), the
 * same "arg->methods->header" runtime-type-id shape already documented
 * project-wide. Distinct from `GenericTagInst_3bb8c_c` above, which reads
 * only the LOW BYTE of the same word (a `lbu`) -- func_8004D788 loads and
 * masks the FULL WORD (`lw` then `andi ..,0xF`), so reusing that byte-typed
 * struct here would emit the wrong load width.
 */
typedef struct GenericHeaderMethods_3bb8c_d GenericHeaderMethods_3bb8c_d;
typedef struct GenericHeaderObj_3bb8c_d GenericHeaderObj_3bb8c_d;

struct GenericHeaderMethods_3bb8c_d {
    s32 header; /* +0x000 */
};

struct GenericHeaderObj_3bb8c_d {
    GenericHeaderMethods_3bb8c_d *methods; /* +0x000 */
};

struct Class86B60Methods {
    u8 pad000[0x008];
    void (*ctor)(Class86B60 *self, void *dreamSys);  /* +0x008, func_8004D578 occupies this slot */
    u8 pad00C[0x010 - 0x00C];
    /* +0x010, func_8004E054's own first two calls -- called TWICE with
     * different arguments (`self->unkC->unk4`, then `self->unk10`), both
     * opaque values forwarded verbatim. */
    void (*slot10)(Class86B60 *self, void *arg1);
    /* +0x014, func_8004E054's own 3rd call: `(self, self->unkAC)`, the
     * pointer forwarded opaquely rather than dereferenced by this slot's
     * caller. */
    void (*slot14)(Class86B60 *self, void *arg1);
    u8 pad018[0x040 - 0x018];
    void (*slot40)(Class86B60 *self, void *dreamSys); /* +0x040, func_8004D578's own last call */
    u8 pad044[0x060 - 0x044];
    /* +0x060, func_8004DE08's own two calls, both `(self, literal)` --
     * once with `0xB` right after `self->unk58 = 5`, once with `0xF`
     * later in the same function. */
    void (*slot60)(Class86B60 *self, s32 arg1);
    u8 pad064[0x06C - 0x064];
    void (*slot6C)(Class86B60 *self, s32 arg1); /* +0x06C, func_8004D814's own 2nd call, arg1 = 0xA */
    u8 pad070[0x078 - 0x070];
    void (*slot78)(Class86B60 *self); /* +0x078, func_8004D90C's own last call, `self` only */
    void (*slot7C)(Class86B60 *self); /* +0x07C, func_8004D90C's own 2nd call, `self` only */
    u8 pad080[0x094 - 0x080];
    /* +0x094, func_8004D9D4's shared tail call target for its `unk58==1`
     * and `unk58==4` cases -- a crossjump-merge-safe local function
     * pointer (see round 12's "local function pointer variable" lever)
     * rather than a retyped slot, since `slot130`/`slot134` reach the
     * SAME call site with the same signature. */
    void (*slot94)(Class86B60 *self);
    u8 pad098[0x0D4 - 0x098];
    /* +0x0D4, func_8004D814's own first call, arg1 = &D_800114E8, arg2 = 0. */
    void (*slotD4)(Class86B60 *self, void *arg1, s32 arg2);
    void (*slotD8)(Class86B60 *self, void *arg1);      /* +0x0D8, func_8004D578's own call, arg1 = &D_80086D44 */
    u8 pad0DC[0x0E0 - 0x0DC];
    /* +0x0E0, func_8004DE08's own call: `(self, self->unk14)`, arg1
     * forwarded opaquely. */
    void (*slotE0)(Class86B60 *self, void *arg1);
    u8 pad0E4[0x0F0 - 0x0E4];
    /* +0x0F0, func_8004D90C's own 3rd call: `(self, self->unk4C->unk8,
     * 1)`. Distinct from `DreamSysViewMethods_3bb8c_c::slotF0` (see
     * func_8004D814's report) -- same offset number, unrelated table.
     * func_8004DE08 (this round) also reaches this slot, forwarding an
     * `s32` (its own saved pre-overwrite copy of `self->unk58`) through
     * the SAME `void *arg1` parameter, cast at that call site rather than
     * retyping the slot -- the bit pattern is unchanged either way, and
     * `func_8004D90C`'s own call already established the pointer type. */
    void (*slotF0)(Class86B60 *self, void *arg1, s32 arg2);
    u8 pad0F4[0x11C - 0x0F4];
    /* +0x11C, func_8004DE08's own call: `(self, buf, 1)` where `buf` is
     * the value `DreamSysViewMethods_3bb8c_c::slot19C` (via `self->unkA4`)
     * just filled through a stack out-parameter. */
    void (*slot11C)(Class86B60 *self, s32 arg1, s32 arg2);
    u8 pad120[0x124 - 0x120];
    void (*slot124)(Class86B60 *self, s32 arg1); /* +0x124, func_8004D90C (arg1=0)/func_8004E230 (arg1=0x16) */
    /* +0x128, func_8004E1C4's own first call, `self` only. Return unused. */
    void (*slot128)(Class86B60 *self);
    void (*slot12C)(Class86B60 *self); /* +0x12C, func_8004E230's own first call, `self` only */
    void (*slot130)(Class86B60 *self); /* +0x130, func_8004D9D4's `unk58==2` tail target */
    void (*slot134)(Class86B60 *self); /* +0x134, func_8004D9D4's `unk58==3` tail target */
    /* +0x138, func_8004D788's forward target, only reached when its own
     * arg1's header-word low nibble == 0xB (a runtime-type-id gate) --
     * called with all three of func_8004D788's own parameters verbatim. */
    void (*slot138)(Class86B60 *self, void *arg1, s32 arg2);
};

struct Class86B60 {
    Class86B60Methods *methods;    /* +0x000 */
    u8 pad004[0x00C - 0x004];
    Class86B60UnkCObj_3bb8c_d *unkC; /* +0x00C, func_8004D898/func_8004E054 */
    void *unk10;                     /* +0x010, func_8004E054: opaque, forwarded verbatim */
    void *unk14;                     /* +0x014, func_8004DF64: opaque, forwarded verbatim to unkAC->methods->slot6C */
    u8 pad018[0x02C - 0x018];
    s32 unk2C;                      /* +0x02C, func_8004D814: set to 0x190 */
    u8 pad030[0x034 - 0x030];
    s32 unk34;                      /* +0x034, func_8004D814: zeroed */
    /* +0x038, func_8004D9D4: set to 0 on the `unk58==1` path and to the
     * literal 2 on the `unk58==4` path -- read by nothing else in this
     * unit. The literal 2 is the SAME constant `func_8004D9D4` compares
     * `self->unk58` against for its `case 2`, and retail keeps it
     * resident in one register across the whole function rather than
     * re-materializing it, which is what proves this is a literal `2`
     * and not (as a first reading of the raw asm suggested) `self`
     * re-stored through a leftover register. */
    s32 unk38;
    /* +0x03C, func_8004DCD0: a flag tested `!= 0`, gating whether that
     * function fills its own local 3-byte buffer from `arg1`'s bytes or
     * zeroes it instead. No setter in this unit. */
    s32 unk3C;
    u8 pad040[0x048 - 0x040];
    /* Set up by the base ctor chain (Get_vtable_TaskCore()->slot08 below), read
     * (never written) by func_8004D578 right after. Same offset/shape as
     * `StreamTaskObj::unk48` in include/code_2c054.h (also a base-ctor-
     * chain output), but kept as its own local type since nothing ties
     * the two classes together and the one slot this unit dispatches
     * through (+0x09C) isn't among that type's own known slots. */
    struct Class86B60Unk48Obj *unk48;  /* +0x048, func_8004D578 */
    Class86B60Unk4CObj_3bb8c_d *unk4C; /* +0x04C, func_8004D90C */
    u8 pad050[0x058 - 0x050];
    s32 unk58;                       /* +0x058, func_8004D9D4: 5-valued dispatch (0-4) */
    u8 pad05C[0x060 - 0x05C];
    Class86B60Unk60Obj_3bb8c_d *unk60; /* +0x060, func_8004DABC */
    u8 pad064[0x093 - 0x064];
    /* +0x093, func_8004D898: address-of only, forwarded as
     * `Class86B60UnkC0ObjMethods_3bb8c_d::slot78`'s 2nd argument each
     * loop iteration; real extent beyond one byte unknown. */
    u8 unk93;
    u8 pad094[0x0A4 - 0x094];
    /* +0x0A4, func_8004D578: stores its own dreamSys arg raw. RETYPED this
     * round from a bare `void *` to `DreamSysView_3bb8c_c *` --
     * func_8004D814 (this unit) is the first function to dereference it
     * through its own vtable (`slotF0`) rather than only forwarding it
     * opaquely. Same size (4 bytes), so no layout change; the assignment
     * in func_8004D578 (`self->unkA4 = dreamSys;`, `dreamSys` a `void *`
     * parameter) still compiles under ordinary C pointer conversion
     * rules. */
    DreamSysView_3bb8c_c *unkA4;
    /* +0x0A8/+0x0AC, func_8004D704 (this unit's destructor): two owned
     * sub-objects, each released through their own shared `release` slot.
     * BOTH releases sit inside the SAME `unkAC != NULL` guard -- retail's
     * single branch skips over both calls together, not just the first;
     * there is no separate null check on `unkA8`. `unkAC` was previously
     * typed a bare `s32` from func_8004D578's `zeroed` write alone, which
     * is consistent with either a scalar 0 or a null pointer -- this
     * round's func_8004D704 is what proves it is dereferenced through a
     * vtable, so it is retyped a pointer here (same size, no layout
     * change). */
    GenericReleaseObj_3bb8c_d *unkA8; /* +0x0A8, func_8004D704: released iff unkAC != NULL */
    /* +0x0AC, func_8004D578: zeroed; func_8004D704: guards both releases.
     * RETYPED from the minimal `GenericReleaseObj_3bb8c_d *` to the
     * dedicated `Class86B60UnkACObj_3bb8c_d *` -- func_8004E054 reaches a
     * second slot (`+0x070`) on it. Same size, no layout change. */
    Class86B60UnkACObj_3bb8c_d *unkAC;
    /* +0x0B0, func_8004DC08: a third owned sub-object, released
     * unconditionally (no null check) through the same shared `release`
     * slot as `unkA8`/`unkAC`. Typed its own `Class86B60UnkB0Obj_3bb8c_d`
     * rather than reusing `GenericReleaseObj_3bb8c_d` -- func_8004DC64
     * reaches a SECOND slot (`+0x04C`) on the same pointer that
     * `unkA8`/`unkAC` never do, so it is kept a distinct local view
     * rather than assuming the other two share its fuller shape. */
    Class86B60UnkB0Obj_3bb8c_d *unkB0;
    u8 pad0B4[0x0BC - 0x0B4];
    s32 unkBC;                      /* +0x0BC, func_8004D578: return value of dreamSys->methods->slot1B0 */
    s32 unkC0;                      /* +0x0C0, func_8004D578: output buffer address passed BY REFERENCE
                                        to dreamSys->methods->slot1B0 -- last word of the 0xC4-byte
                                        allocation (0xC0+4 == 0xC4), which is why this field is exactly
                                        one word wide rather than a guess */
};

extern Class86B60Methods D_80086B60;
extern Class86B60Methods *func_8004E2D0(void);   /* still raw asm, asm/class_3bb8c_d.s -- called
                                                       directly (jal), not through any vtable */

/*
 * self->unk48's own pointee (Class86B60Unk48Obj). Only slot9C is reached,
 * by func_8004D578, with a single s32 argument (-1); return value unused.
 */
typedef struct Class86B60Unk48ObjMethods {
    u8 pad000[0x09C];
    void (*slot9C)(struct Class86B60Unk48Obj *self, s32 arg1); /* +0x09C */
} Class86B60Unk48ObjMethods;

typedef struct Class86B60Unk48Obj {
    Class86B60Unk48ObjMethods *methods;   /* +0x000 */
} Class86B60Unk48Obj;

/*
 * Local, opaque view of func_8004D578's `dreamSys` argument -- only the
 * two vtable slots that function reaches (+0x1A0, +0x1B0) are typed. This
 * project already has a much larger, canonical `DreamSys` type
 * (include/DreamSys.h) with its own `vt` field, but neither offset is
 * established there yet and this unit does not edit that header -- kept
 * as an independent local view per this project's established convention
 * (see e.g. Obj866E8 vs. Class866E8 at the top of this file). The `void
 * *dreamSys` parameter type on func_8004D518/func_8004D578 themselves is
 * kept untyped/opaque to match the ALREADY-established external
 * declaration `extern PollTask *func_8004D518(void *dreamSys);` in
 * include/Class6D3C8.h (a different unit's own independent view of this
 * same New_X allocator, used there as a `PollTaskCtor` callback) --
 * this local dispatch type is used only inside func_8004D578's own body.
 */
typedef struct DreamSysViewMethods_3bb8c_c DreamSysViewMethods_3bb8c_c;

struct DreamSysViewMethods_3bb8c_c {
    u8 pad000[0x0F0];
    /* +0x0F0, func_8004D814's own last call: `(dreamSys, 0, 0)`, both
     * trailing arguments literal zero. */
    void (*slotF0)(DreamSysView_3bb8c_c *self, s32 arg1, s32 arg2);
    u8 pad0F4[0x19C - 0x0F4];
    /* +0x19C, func_8004DABC's own call: `arg1` is the address of a
     * one-word stack buffer this function fills from
     * `self->unk60->unk14` before the call. */
    void (*slot19C)(DreamSysView_3bb8c_c *self, s32 *arg1);
    /* Return value forwarded straight to func_8004D6AC's own arg0. */
    s32 (*slot1A0)(DreamSysView_3bb8c_c *self, s32 arg1);      /* +0x1A0 */
    u8 pad1A4[0x1A8 - 0x1A4];
    /* +0x1A8, func_8004E230's own call, `self` only. */
    void (*slot1A8)(DreamSysView_3bb8c_c *self);
    /* +0x1AC, func_8004E0E4's own call: `self->unkA4->methods->slot1AC(
     * self->unkA4)`. Nonzero return gates a one-byte-zero write into
     * `*D_8008AA10` (return type therefore `s32`, not `void` -- the
     * caller's `beqz` on `$v0` tests it directly). */
    s32 (*slot1AC)(DreamSysView_3bb8c_c *self);
    /* Return value stored into Class86B60::unkBC; arg1 is the address of
     * Class86B60::unkC0 (an output buffer this slot presumably fills). */
    s32 (*slot1B0)(DreamSysView_3bb8c_c *self, void *arg1);     /* +0x1B0 */
};

struct DreamSysView_3bb8c_c {
    DreamSysViewMethods_3bb8c_c *methods;   /* +0x000 */
};

/*
 * func_8004D578's own local view of `Get_vtable_TaskCore`'s return type -- ALSO
 * independently declared, with a DIFFERENT 4-argument signature, as
 * `TaskCoreMethods` in include/code_2c054.h (`slot08`, confirmed 3-argument-
 * plus-self there from func_8003B8E4's own byte-exact call). Same real
 * global (`gTaskCoreMethods`) two units deep, two independent arities recorded
 * from two real call sites -- the identical situation already documented
 * for GetClass6B5CCMethods above and for func_8004D3DC's report. Kept local
 * rather than including code_2c054.h, since this unit does not otherwise
 * need that header and each translation unit gets its own extern
 * prototype for a symbol in this project.
 */
typedef struct BaseTaskCtorTable_3bb8c_c BaseTaskCtorTable_3bb8c_c;
struct BaseTaskCtorTable_3bb8c_c {
    u8 pad000[0x008];
    /* func_8004D578's own unconditional first statement:
     * Get_vtable_TaskCore()->slot08(self, &D_80086D44, &D_800114DC, 0). */
    void (*slot08)(void *self, void *arg1, void *arg2, s32 arg3); /* +0x008 */
    /* +0x00C, func_8004D704's own unconditional last call, `self` only.
     * Same offset AND arity as `TaskCoreMethods::slot0C` in
     * include/code_2c054.h (also derived from `gTaskCoreMethods`, the same real
     * global this getter returns) -- independent confirmation, not a
     * coincidence: this is the shared base class's destructor forward,
     * called after `self`'s own two owned sub-objects (`unkA8`/`unkAC`)
     * are released. */
    void (*slot0C)(void *self);
    u8 pad010[0x038 - 0x010];
    /* +0x038, func_8004D788's own unconditional first call, forwarding
     * all three of its own parameters verbatim. */
    void (*slot38)(void *self, void *arg1, s32 arg2);
    u8 pad03C[0x060 - 0x03C];
    /* +0x060, func_8004D90C's own first call, `(self, arg1)` where arg1 is
     * that function's own forwarded 2nd parameter. */
    void (*slot60)(void *self, s32 arg1);
    u8 pad064[0x090 - 0x064];
    /* +0x090, func_8004D9D4's own first, unconditional call, `self` only. */
    void (*slot90)(void *self);
    /* +0x094, func_8004DABC's own first, unconditional call, `self`
     * only. Distinct from `Class86B60Methods::slot94` (see
     * func_8004D9D4's report) -- same offset number, unrelated table. */
    void (*slot94)(void *self);
    u8 pad098[0x0DC - 0x098];
    /* +0x0DC, func_8004DC08's own last call, `self` only, right after
     * releasing `Class86B60::unkB0`. */
    void (*slotDC)(void *self);
    /* +0x0E0, func_8004DC64's own first call: `(self, arg1)`, arg1 its
     * own forwarded 2nd parameter. */
    void (*slotE0)(void *self, s32 arg1);
    /* +0x0E4, func_8004DCD0's own first, unconditional call: `(self,
     * arg1)`, arg1 its own forwarded 2nd parameter (opaque here). */
    void (*slotE4)(void *self, void *arg1);
};

extern BaseTaskCtorTable_3bb8c_c *Get_vtable_TaskCore(void);

/* Address-of only in this unit (func_8004D578 passes &D_80086D44 both as
 * the base ctor's arg1 and, again, as slotD8's own arg1). Placeholder s32
 * type since only the address is taken here. */
extern s32 D_80086D44;

/* Address-of only in this unit (func_8004D578 passes &D_800114DC as the
 * base ctor's arg2). Placeholder s32 type since only the address is taken
 * here. */
extern s32 D_800114DC;

/* Address-of only in this unit (func_8004D814 passes &D_800114E8 to
 * `Class86B60Methods::slotD4`). Placeholder s32 type since only the
 * address is taken here. */
extern s32 D_800114E8;

/* func_8004DF64's own path string, passed to func_8003B39C -- a real
 * dlabel (`asm/data/1C34.rodata.s`: "CARD\FILEICN1.TIM"), so this is the
 * ONLY correct spelling (CLAUDE.md: never re-write a string splat has
 * already emitted as a symbol). */
extern const char D_800114F8[];

/* Address-of only in this unit -- func_8004D898 walks it with an
 * explicit 0xC-byte stride, passing each entry's address on to
 * `Class86B60UnkC0ObjMethods_3bb8c_d::slot78`, but never dereferences it
 * itself. Placeholder s32 type; real element layout unknown. */
extern s32 D_80086DAC;

/* Address-of only in this unit (func_8004DC64 passes &D_8008A9B4 to
 * `Class86B60UnkB0ObjMethods_3bb8c_d::slot4C`). Placeholder s32 type
 * since only the address is taken here. */
extern s32 D_8008A9B4;

/* VALUE-of, not address-of, in this unit -- func_8004E1C4 reaches these
 * through `%gp_rel` loads of the .sdata globals themselves, forwarding
 * whatever they hold. Each holds a pointer into the still-uncarved rodata
 * block at `D_80011434` (`asm/data/1C34.rodata.s`: 0x80011464 and
 * 0x8001149C respectively, neither with its own dlabel), so they cannot
 * be spelled by the address they point to and are typed opaque `void *`
 * instead.
 *
 * `D_8008AA18` is also read by round 43's `func_8004DB18`, which
 * `strcpy`s INTO `(char *)D_8008AA18 + 0x18` and reads it with `strlen` --
 * both require the RUNTIME value to be a writable buffer, not the .rodata
 * address the ROM image happens to initialise it to. Nothing in this unit
 * ever reassigns it, so whatever sets the real (writable) value is outside
 * this unit's own ground; the ROM-image value above is a placeholder only. */
extern void *D_8008AA10;
extern void *D_8008AA18;

/* Same VALUE-of `%gp_rel` pattern, read (and its buffer formatted into via
 * FormatFullWidthNumber) by round 45's `func_8004D6AC` (src/class_3bb8c_c.c). Holds
 * `D_8008AA1C` in the ROM image -- the "7654321" placeholder string
 * (`asm/data/7B12C.sdata.s`) -- so, like `D_8008AA18` above, this is a
 * writable-buffer placeholder rather than the real runtime value. */
extern void *D_8008AA24;

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `func_8004DB18`
 * as `strcpy`'s SOURCE argument. Holds `0x80011474` in the ROM image
 * (immediately past `D_8008AA10`'s own "BISLPS-01556xxx" string, i.e. the
 * start of the font-glyph word table in `D_80011434`) -- likely also a
 * placeholder for the same reason `D_8008AA18` is, since a font-glyph
 * table is not plausible `strcpy` input. */
extern void *D_8008AA14;

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `func_8004DF64`
 * as `Class86B60UnkACObjMethods_3bb8c_d::slot6C`'s own `arg1`. Holds
 * `0x80011454` in the ROM image -- the "BISLPS-01556" string in
 * `D_80011434`, again with no `dlabel` of its own. */
extern void *D_8008A9D0;

/* Address-of only, round 43's `func_8004DF64`
 * (`Class86B60UnkACObjMethods_3bb8c_d::slot6C`'s own `arg2`) -- a real
 * 16-entry pointer table (`asm/data/76DC8.data.s`, `D_8008AA0C` down to
 * `D_8008A9D4` then a NULL terminator), reached only by its own address
 * here, never walked. Placeholder `s32` type since only the address is
 * taken. */
extern s32 D_80086D6C;

/* func_8004DCD0's own rolling byte index (0/1/2, wraps to 0 at 3) into
 * that function's own 3-byte stack buffer -- declared in the ROM image
 * as a full `.word` (`asm/data/7B12C.sdata.s`), but accessed only via
 * `lbu`/`sb` here, so `u8` is the correct C type for this unit's own
 * reference regardless of the underlying storage's full width. */
extern u8 D_8008AA28;

/* func_8004DCD0's own rolling word counter (wraps to 0 at 0x101). */
extern s32 D_8008AA2C;

/* func_8004E34C's own one-shot init guard: read, then unconditionally
 * incremented, before its own body's InitCARD/StartCARD/_bu_init calls
 * run only when the PRE-increment value was 0 (i.e. only on the very
 * first construction of this class). */
extern s32 D_8008AA30;

/* Still raw asm in this unit (gp-relative-blocked, see
 * docs/match-reports/func_8004D6AC.md) -- not this round's function, but
 * func_8004D578 calls it with one forwarded s32 argument (the return
 * value of dreamSys->methods->slot1A0); return value unused there. */
extern void func_8004D6AC(s32 arg0);

/*
 * First argument of func_8004D678: an unrelated, larger caller-side
 * struct (only seen from its one caller, func_8004DE08 in the still-
 * uncarved asm/class_3bb8c_d.s) whose own +0x0BC field is a pointer to
 * the Obj866E8 instance this function actually operates on -- NOT
 * Obj866E8's own +0x0BC (that offset on Obj866E8 itself is the
 * already-documented embedded Descriptor10 `unkBC`). Named distinctly to
 * avoid implying any relation to Obj866E8's own layout.
 */
typedef struct Ctx678_3bb8c_c {
    u8 pad00[0x0BC];
    Obj866E8 *target;                           /* +0x0BC */
} Ctx678_3bb8c_c;

/*
 * Second argument of func_8004D678: holds a pointer at +0x018 to a small
 * result block whose word at +0x004 is the flag func_8004D678 computes.
 */
typedef struct Result678_3bb8c_c {
    u8 pad00[0x018];
    s32 *block;                                 /* +0x018, func_8004D678 writes block[1] */
} Result678_3bb8c_c;

/*
 * func_8004E2E0's own New_X allocator target -- yet another small
 * sibling class (same shape as `Class869D8`/`Class86AA0`/`Class86B60`
 * above): pool-allocate a fixed 0x84-byte block, and if it succeeds,
 * construct it through this table's own `ctor` slot at `+0x008`. Kept
 * fully opaque (no instance type at all) since func_8004E2E0 never
 * dereferences the allocation itself, only forwards it.
 */
typedef struct GenericCtorTable_3bb8c_d GenericCtorTable_3bb8c_d;
struct GenericCtorTable_3bb8c_d {
    u8 pad000[0x008];
    void (*ctor)(void *self, void *arg1, void *arg2); /* +0x008, func_8004E2E0's own call; IS func_8004E34C -- see that function's own, more precise (s32, s32) local declaration in class_3bb8c_d.c, kept separate per the project's independent-arities convention (BaseTaskCtorTable_3bb8c_c/Get_vtable_TaskCore) since nothing here type-checks the two against each other */
    u8 pad00C[0x040 - 0x00C];
    /* +0x040, func_8004E34C's own last call, forwarding its own 3rd
     * parameter verbatim; class_3bb8c_e's independent view (round 14,
     * this same real object) names the concrete function `func_8004E5D4`,
     * still uncarved there. */
    void (*slot40)(void *self, s32 arg1);
};

extern GenericCtorTable_3bb8c_d D_80086DC4;
extern GenericCtorTable_3bb8c_d *func_800507E8(void); /* returns &D_80086DC4; matched in class_3bb8c_g */

/*
 * The object instance itself -- established this round by func_8004E34C,
 * which IS this class's own constructor (verified: `tools/classtable.py
 * 0x80086DC4` places it at the table's own +0x008 ctor slot). Kept
 * minimal (only the one field this unit's ctor writes) since nothing else
 * here dereferences it -- the fuller shape belongs to class_3bb8c_e's own
 * independent view of the SAME real object (`Node3bb8cE`,
 * src/class_3bb8c_e.c), which this unit does not include (per this
 * project's established multiple-independent-local-views convention).
 */
typedef struct GenericCtorObj_3bb8c_d GenericCtorObj_3bb8c_d;
struct GenericCtorObj_3bb8c_d {
    GenericCtorTable_3bb8c_d *methods; /* +0x000, func_8004E34C: self->methods = func_800507E8() -- the base-ctor-chain "sets self->methods directly to this table's own pointer" pattern already seen for Class86B60/func_8004D578 */
};

/*
 * Class86E00 -- a large NEW class table, `D_80086E00` (29 slots + header,
 * resolved with `tools/classtable.py 0x80086E00`), unrelated by
 * inheritance to any of `D_8006B58C`/`D_800866E8`/`D_80086B60`/
 * `D_80086DC4` already known in this header (`--vs` against all four
 * found no matching run of slots -- an independent class, not a
 * subclass of anything else this header names).
 *
 * class_3bb8c_g is the first unit to write any of THIS class's own
 * methods -- specifically slots `+0x048` and up. Slots `+0x004..+0x03C`
 * belong to `class_3bb8c_e`/`class_3bb8c_f`, this round's other two
 * runners sharing this header; they are left fully opaque here since no
 * function in this unit ever dispatches through them.
 *
 * Every type below is suffixed `_3bb8c_g`, INCLUDING the class name
 * itself -- unlike `Class86B60`/`Class869D8`/`Class86AA0` above, which
 * are single-owner. This table is reached by three units at once this
 * round, so an unsuffixed `Class86E00` here could collide at merge with
 * a same-named, differently-shaped definition from `class_3bb8c_e` or
 * `class_3bb8c_f` with no conflict marker to catch it -- the exact
 * round-13 hazard this shared header's rules exist to prevent.
 *
 * Only the offsets this unit's own functions touch are given concrete
 * types; everything else stays opaque padding.
 */
typedef struct Class86E00_3bb8c_g Class86E00_3bb8c_g;
typedef struct Class86E00Methods_3bb8c_g Class86E00Methods_3bb8c_g;

/*
 * self->unk6C's pointee. func_8004FFF4 is the function that PROVES this
 * is a pointer (dereferences its `+0x080` vtable slot) -- before that
 * function was read, `unk6C` looked like a plain `s32` value forwarded
 * opaquely to `Class86E00SubObj_3bb8c_g::slot4C`'s 3rd argument, which is
 * why that parameter is typed with this pointer rather than `s32` below.
 */
typedef struct Class86E00Unk6CObj_3bb8c_g Class86E00Unk6CObj_3bb8c_g;
typedef struct Class86E00Unk6CObjMethods_3bb8c_g Class86E00Unk6CObjMethods_3bb8c_g;

struct Class86E00Unk6CObjMethods_3bb8c_g {
    u8 pad000[0x080];
    /* +0x080, func_8004FFF4's own call: `(self, arg1, 0x7F, 0x7F)`,
     * `arg1` forwarded verbatim from func_8004FFF4's own 2nd parameter. */
    void (*slot80)(Class86E00Unk6CObj_3bb8c_g *self, s32 arg1, s32 arg2, s32 arg3);
};

struct Class86E00Unk6CObj_3bb8c_g {
    Class86E00Unk6CObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * self->unk78's and self->unk7C's shared pointee -- two parallel fields
 * of the SAME sub-object shape (func_80050340/func_80050410 exercise
 * `unk78`; func_800505A8/func_80050670 exercise `unk7C` the identical
 * way), each independently attached via `Class86E00Methods_3bb8c_g::
 * slot10` and torn down via a fixed `slot50`/`slot48`/`release` sequence.
 */
typedef struct Class86E00SubObj_3bb8c_g Class86E00SubObj_3bb8c_g;
typedef struct Class86E00SubObjMethods_3bb8c_g Class86E00SubObjMethods_3bb8c_g;

struct Class86E00SubObjMethods_3bb8c_g {
    u8 pad000[0x004];
    void (*release)(Class86E00SubObj_3bb8c_g *self); /* +0x004, func_80050410/func_80050670 */
    u8 pad008[0x044 - 0x008];
    /* +0x044, func_80050340/func_800505A8's own call: `(self, unk68)`
     * from the OWNING `Class86E00_3bb8c_g`. */
    void (*slot44)(Class86E00SubObj_3bb8c_g *self, s32 arg1);
    void (*slot48)(Class86E00SubObj_3bb8c_g *self); /* +0x048, func_80050410/func_80050670 */
    /* +0x04C, func_80050340/func_800505A8's own call:
     * `(self, unk60, unk64, unk6C)` from the OWNING `Class86E00_3bb8c_g`. */
    void (*slot4C)(Class86E00SubObj_3bb8c_g *self, s32 a1, s32 a2, Class86E00Unk6CObj_3bb8c_g *a3);
    void (*slot50)(Class86E00SubObj_3bb8c_g *self); /* +0x050, func_80050410/func_80050670 */
};

struct Class86E00SubObj_3bb8c_g {
    Class86E00SubObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * self->unk70's pointee -- a DIFFERENT sub-object from
 * `Class86E00SubObj_3bb8c_g` above. It shares the same `+0x004` slot
 * offset only because every BasicClass-family table keeps a slot there
 * (see `BasicClassMethods::release` in code_8220.h) -- the USAGE differs:
 * func_8004FF40 assigns this call's RETURN VALUE back into `unk70` (an
 * "advance" pattern), where `Class86E00SubObj_3bb8c_g::release`'s callers
 * (func_80050410/func_80050670) discard the return and unconditionally
 * null the field afterward instead. Different enough to keep separate
 * rather than unify.
 */
typedef struct Class86E00Unk70Obj_3bb8c_g Class86E00Unk70Obj_3bb8c_g;
typedef struct Class86E00Unk70ObjMethods_3bb8c_g Class86E00Unk70ObjMethods_3bb8c_g;

struct Class86E00Unk70ObjMethods_3bb8c_g {
    u8 pad000[0x004];
    /* +0x004, func_8004FF40's own call: return value stored back into
     * `Class86E00_3bb8c_g::unk70` itself. */
    Class86E00Unk70Obj_3bb8c_g *(*slot4)(Class86E00Unk70Obj_3bb8c_g *self);
    u8 pad008[0x04C - 0x008];
    /* +0x04C, func_8004FE24's own call on a FRESH `unk70` right after
     * assigning it: `(self, self->unk68, &D_8008AA94)`. `unk68` is
     * forwarded verbatim -- kept as the owning struct's established
     * bare `s32` reading of that field, not retyped to a pointer here. */
    void (*slot4C)(Class86E00Unk70Obj_3bb8c_g *self, s32 arg1, void *arg2);
};

struct Class86E00Unk70Obj_3bb8c_g {
    Class86E00Unk70ObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * func_80050730's own `arg1` -- a third, unrelated small object, reached
 * only through its own `+0x09C` slot, whose return value is stored into
 * `Class86E00_3bb8c_g::unk80`.
 */
typedef struct GenericSlot9CObj_3bb8c_g GenericSlot9CObj_3bb8c_g;
typedef struct GenericSlot9CMethods_3bb8c_g GenericSlot9CMethods_3bb8c_g;

struct GenericSlot9CMethods_3bb8c_g {
    u8 pad000[0x09C];
    void *(*slot9C)(GenericSlot9CObj_3bb8c_g *self); /* +0x09C, func_80050730 */
};

struct GenericSlot9CObj_3bb8c_g {
    GenericSlot9CMethods_3bb8c_g *methods; /* +0x000 */
};

struct Class86E00Methods_3bb8c_g {
    u8 pad000[0x010];
    /* +0x010, func_80050340/func_800505A8's own first call: `(self,
     * subObj)`, registering/attaching whichever of `unk78`/`unk7C` that
     * function owns. */
    void (*slot10)(Class86E00_3bb8c_g *self, Class86E00SubObj_3bb8c_g *arg1);
    u8 pad014[0x030 - 0x014];
    /* +0x030, func_8004FBE4's own first call, every path: `(self,
     * arg1)` where `arg1` may already have been forced to `0x17`. */
    void (*slot30)(Class86E00_3bb8c_g *self, s32 arg1);
    u8 pad034[0x050 - 0x034];
    /* +0x050, func_8004FBE4's own `arg1==0x13` case: no extra arguments,
     * return value picks between two literal replacement codes. */
    s32 (*slot50)(Class86E00_3bb8c_g *self);
    u8 pad054[0x058 - 0x054];
    /* +0x058, func_8004FBE4's own `arg1==0x14` case, only when
     * `*(u8 *)self->unk40 == 0`: `(self, unk40, unk30, unk34)`. */
    void (*slot58)(Class86E00_3bb8c_g *self, s32 a1, char *a2, s32 a3);
    u8 pad05C[0x064 - 0x05C];
    /* +0x064, func_8004FBE4's own `arg1==0x15` case: `(self, unk40,
     * unk54, unk58)`, return value picks between two literal
     * replacement codes (same shape as `slot68` just below). */
    s32 (*slot64)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3);
    /* +0x068, func_8004FBE4's own `arg1==0x14` case, unconditional:
     * `(self, unk40, unk44, unk4C, unk50, unk54, unk58)` -- the same
     * six fields as `slot78` below MINUS `unk48`, not a subset call to
     * that slot. Return value picks between two literal replacement
     * codes, same shape as `slot64` above. */
    s32 (*slot68)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
    u8 pad06C[0x074 - 0x06C];
    /* +0x074, func_80050034's own `self->unk24==1` case: 4 extra
     * arguments (`unk40`, `unk44`, `unk54`, `unk58`), same shape as
     * `slot78` just below but with only the last two of that call's
     * trailing four. */
    void (*slot74)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3, s32 a4);
    /* +0x078, func_800504D0's own call for its `arg2==2` case: 7 extra
     * arguments, the last four passed on the stack (`unk4C`, promoted
     * from its native `u8` to a full word, then `unk50`/`unk54`/`unk58`). */
    void (*slot78)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3,
                    s32 a4, s32 a5, s32 a6, s32 a7);
    /* +0x07C, the shared tail call of func_800501F0/func_80050280/
     * func_800504D0/func_80050730: `(self, literal state code)`. */
    void (*slot7C)(Class86E00_3bb8c_g *self, s32 arg1);
    /* +0x080, func_8004FBE4's own 3rd call, every path: `(self, arg1)`,
     * same `arg1` value as `slot30` above. */
    void (*slot80)(Class86E00_3bb8c_g *self, s32 arg1);
    /* +0x084, func_8004FBE4's own 2nd call, every path: `(self)`. */
    void (*slot84)(Class86E00_3bb8c_g *self);
    u8 pad088[0x08C - 0x088];
    void (*slot8C)(Class86E00_3bb8c_g *self, s32 arg1); /* +0x08C, func_800501F0 */
    void (*slot90)(Class86E00_3bb8c_g *self); /* +0x090, func_8004FF90's `arg2==0x19` case */
    void (*slot94)(Class86E00_3bb8c_g *self); /* +0x094, func_8004FF90's `arg2==0x17` case */
    u8 pad098[0x09C - 0x098];
    /* +0x09C, func_8004FBE4's own `arg1==0x11` case: `(self)`, no
     * return value read. */
    void (*slot9C)(Class86E00_3bb8c_g *self);
    void (*slotA0)(Class86E00_3bb8c_g *self); /* +0x0A0, func_800504D0's own first call, both cases */
    u8 pad0A4[0x0A8 - 0x0A4];
    /* +0x0A8, func_8004FBE4's own `arg1==0x12` case: `(self)`, same
     * shape as `slot9C` above. */
    void (*slotA8)(Class86E00_3bb8c_g *self);
    void (*slotAC)(Class86E00_3bb8c_g *self); /* +0x0AC, func_80050730's own 2nd call, both cases */
};

struct Class86E00_3bb8c_g {
    Class86E00Methods_3bb8c_g *methods; /* +0x000 */
    u8 pad004[0x024 - 0x004];
    /* +0x024, func_80050034's own secondary dispatch code (nested inside
     * the `unk28`-driven switch's shared `2`/`4`/`0xA`/`0xE` case),
     * tested against literals `2` and `1`. */
    s32 unk24;
    s32 unk28;   /* +0x028, dispatch/state code tested by several functions */
    /* +0x02C, func_8004FBE4's own loop bound: frees `self->unk38[0..
     * unk2C)` when tearing down (see `unk38`'s own comment below). */
    s32 unk2C;
    /* +0x030, func_80050034's own `strcpy` source into `self->unk40`,
     * in its `self->unk28==0xE` sub-case. */
    char *unk30;
    /* +0x034, func_8004FBE4's own `slot58` arg3, forwarded verbatim
     * alongside `unk30` above. */
    s32 unk34;
    void *unk38; /* +0x038, func_800505A8: forwarded opaquely to `func_80051A5C`'s arg0 */
    /* +0x03C, func_80050034's own `self->unk28==0xE` sub-case: base of a
     * pointer array indexed by `(s32)self->unk80`, `strcat`ed onto
     * `self->unk40` -- same shape as `unk38` just below, indexed the
     * same way for `self->unk44`'s own `strcpy`. */
    void *unk3C;
    s32 unk40;   /* +0x040, func_80050340/func_800504D0 */
    s32 unk44;   /* +0x044, func_80050340/func_800504D0 */
    s32 unk48;   /* +0x048, func_80050340/func_800504D0 */
    u8 unk4C;    /* +0x04C, func_800504D0: read `lbu`, promoted to a full word for `slot78`'s call */
    u8 pad04D[0x050 - 0x04D];
    s32 unk50;   /* +0x050, func_800504D0 */
    s32 unk54;   /* +0x054, func_800504D0 */
    s32 unk58;   /* +0x058, func_800504D0 */
    s32 unk5C;   /* +0x05C, func_80050280: incremented, capped at 6 */
    s32 unk60;   /* +0x060, forwarded to `unk78`/`unk7C`'s own `slot4C` arg1 */
    s32 unk64;   /* +0x064, forwarded to `unk78`/`unk7C`'s own `slot4C` arg2 */
    /* +0x068, a readiness gate checked alongside `unk60` in four
     * functions (func_80050340/func_80050410/func_800505A8/
     * func_80050670) -- both must be non-zero before the body runs.
     * Kept a bare `s32`; never dereferenced in this unit. */
    s32 unk68;
    /* +0x06C, forwarded to `unk78`/`unk7C`'s own `slot4C` arg3.
     * func_8004FFF4 proves this is a pointer (dereferences its `+0x080`
     * vtable slot), not the plain `s32` it looked like from the slot4C
     * call site alone -- retyped here, same size, no layout change. */
    Class86E00Unk6CObj_3bb8c_g *unk6C;
    Class86E00Unk70Obj_3bb8c_g *unk70; /* +0x070, func_8004FF40 */
    /* +0x074, a one-shot flag set to 1 by func_80050340/func_800505A8
     * right after attaching `unk78`/`unk7C`, and consumed (guarding a
     * teardown callback) by func_80050410/func_80050670. */
    s32 unk74;
    Class86E00SubObj_3bb8c_g *unk78; /* +0x078, func_80050340/func_80050410 */
    Class86E00SubObj_3bb8c_g *unk7C; /* +0x07C, func_800505A8/func_80050670 */
    void *unk80; /* +0x080, func_80050730: set from `arg1->methods->slot9C(arg1)`'s return */
};

/* Address-of only in this unit's own screening -- func_800505A8 forwards
 * `self->unk38` and the literal `1` to this external helper; return
 * value stored into `self->unk7C`. Not this round's function here -- it
 * is class_3bb8c_j's New_Class86ED0 (matched round 15, src/class_3bb8c_j.c):
 * `func_80017B34(0x54)` then, on success, its own ctor-table getter's
 * `+0x008` slot called `(self, arg0, arg1)`. This call site's own
 * evidence (arg1 a literal `1`) is what fixed the 2nd parameter as `s32`
 * rather than a pointer. */
extern void *func_80051A5C(void *arg0, s32 arg1);

/* Not this round's function (lives outside this unit's slice) --
 * func_80050340's own external helper, called with `((self->unk48 << 1)
 * + self->unk44, 1)`; return value stored into `self->unk78`. The 2nd
 * argument (a literal `1`) is materialized EARLY, in the delay slot of
 * the guard testing `self->unk78 == NULL` several instructions before
 * this call -- nothing overwrites `$a1` in between, which is what
 * reveals it as a real 2nd argument rather than a scheduling artifact
 * (see func_80050340's report). */
extern void *func_80050BA8(s32 arg0, s32 arg1);

/*
 * class_3bb8c_f: a SEPARATE class from Obj866E8 above -- no evidence unifies
 * them (distinct field layouts), and BuildMemcardPath below writes raw bytes
 * over its own object's first 6 bytes, which would corrupt Obj866E8's own
 * vtable pointer if the two were the same type. This is this unit's own
 * BasicClass-derived (docs/research/class-framework.md, include/code_8220.h)
 * task-ish object: it dispatches through the INHERITED BasicClass
 * addChild/removeChild slots at +0x010/+0x014 (Get_vtable_BasicClass()'s own table
 * establishes those two slots' exact signatures) and adds its own slots
 * from +0x044 on. No FirecatFG name survives, so fields are named by
 * offset. Only the slots/fields this unit's functions actually touch are
 * given concrete types; the rest stays opaque padding. `child`'s type is
 * kept `void *` here (not `BasicClass *`) to avoid pulling in
 * include/code_8220.h just for a parameter type nothing in this unit
 * dereferences.
 */
typedef struct TaskObjF TaskObjF;
typedef struct TaskObjFMethods TaskObjFMethods;

struct TaskObjFMethods {
    s32 header;                                                /* +0x000 */
    void *unk04;                                                /* +0x004 */
    void *ctor;                                                  /* +0x008 */
    void *unk0C;                                                  /* +0x00C */
    void (*addChild)(TaskObjF *self, void *child);                 /* +0x010, func_8004F55C (x2) */
    void (*removeChild)(TaskObjF *self, void *child);               /* +0x014, func_8004F5DC (x2) */
    void *unk18, *unk1C, *unk20, *unk24, *unk28, *unk2C, *unk30, *unk34, *unk38; /* inherited BasicClass slots, untouched by this unit */
    u8 pad3C[0x044 - 0x03C];
    void (*slot44)(TaskObjF *self);                                   /* +0x044, func_8004F9D8 */
    s32 (*slot48)(TaskObjF *self);                                     /* +0x048, func_8004F9D8 */
    s32 (*slot4C)(TaskObjF *self, s32 *out1, s32 *out2, s32 *out3);      /* +0x04C, func_8004F9D8 */
    u8 pad50[0x054 - 0x050];
    s32 (*slot54)(TaskObjF *self, s32 a1, s32 a2);                          /* +0x054, func_8004F8A4 */
    u8 pad58[0x05C - 0x058];
    s32 (*slot5C)(TaskObjF *self, void *a1, void *a2, s32 a3, s32 a4);        /* +0x05C, func_8004F638 */
    s32 (*slot60)(TaskObjF *self, s32 a1, s32 a2);                              /* +0x060, func_8004F8A4 */
    u8 pad64[0x07C - 0x064];
    s32 (*slot7C)(TaskObjF *self, s32 a1);                                        /* +0x07C, func_8004F638/func_8004F8A4/func_8004F9D8 */
    u8 pad80[0x088 - 0x080];
    void (*slot88)(TaskObjF *self, void *arg1, s32 arg2);                          /* +0x088, func_8004FB04 */
    u8 pad8C[0x098 - 0x08C];
    void (*slot98)(TaskObjF *self, void *arg1, s32 arg2);                            /* +0x098, func_8004FB04 */
    u8 pad9C[0x0A4 - 0x09C];
    void (*slotA4)(TaskObjF *self, void *arg1, s32 arg2);                              /* +0x0A4, func_8004FB04 */
    u8 padA8[0x0B0 - 0x0A8];
    void (*slotB0)(TaskObjF *self, void *arg1, s32 arg2);                                /* +0x0B0, func_8004FB04 */
};

struct TaskObjF {
    TaskObjFMethods *methods;   /* +0x000 */
    u8 pad04[0x00C - 0x004];     /* BasicClass::children/parentRefs, untouched by this unit */
    s32 cardSlot;                  /* +0x00C, TaskObjF__TryReadMemcardFile: passed as BuildMemcardPath's "selector" (device slot 0/1) -- RENAMED round 60 (was unk0C) */
    u8 pad10[0x014 - 0x010];
    s32 events[4];                  /* +0x014, TaskObjF__ForEachEvent (walks all 4, early-exit)/TaskObjF__FindReadyEvent (passes &events[0], count 4) -- RENAMED round 60 (was field14): 4 kernel event descriptors, corroborated cross-unit by class_3bb8c_e.c's func_8004E5E4, which fills the identical offset via OpenEvent() then passes the same object to this unit's own EnableEvents wrapper (see TaskObjF__EnableEvents's report) */
    s32 opMode;                       /* +0x024, RENAMED round 60 (was unk24): distinguishes which of this class's two operations is active -- func_8004F638 sets 1, func_8004F8A4 sets 2, func_8004F9D8 reads (==1?); the values' exact meaning is not established */
    s32 statusCode;                    /* +0x028, RENAMED round 60 (was unk28): func_8004F55C/func_8004F638 clear or set it, func_8004F8A4/func_8004F9D8 read it and dispatch it through slot7C -- a status/completion code, not confirmed to be error-only */
    s32 bufCount;                       /* +0x02C, RENAMED round 60 (was unk2C): func_8004F638 (slot5C's return)/func_8004F784/func_8004F810 (loop bound over bufArray) -- the number of bufArray entries actually in use */
    s32 unk30;                          /* +0x030, func_8004F55C (arg1)/func_8004F638 (slot5C's arg3) */
    s32 unk34;                           /* +0x034, func_8004F55C (arg2)/func_8004F638 (slot5C's stack arg4) */
    void **bufArray;                      /* +0x038, RENAMED round 60 (was unk38): a 16-entry pointer array allocated by func_8004F704, torn down by func_8004F810, walked by func_8004F784; also func_8004F638's slot5C arg1 */
    void *scratchBuf;                       /* +0x03C, RENAMED round 60 (was unk3C): a single buffer allocated by func_8004F704, freed by func_8004F810; also func_8004F638's slot5C arg2 */
    s32 unk40;                                /* +0x040, func_8004F638 (arg1)/func_8004F8A4 (arg1, forwarded to slot54 as its own arg2) */
    s32 unk44;                                 /* +0x044, func_8004F638 (arg2)/func_8004F8A4 (arg2) */
    s32 unk48;                                  /* +0x048, func_8004F8A4 (arg3) */
    u8 unk4C;                                     /* +0x04C, func_8004F8A4's 5th (byte) arg; also forwarded live to slot60's arg1 */
    u8 pad4D[0x050 - 0x04D];
    s32 unk50;                                      /* +0x050, func_8004F8A4's 6th arg */
    s32 unk54;                                       /* +0x054, func_8004F638 (arg3)/func_8004F8A4's 7th arg */
    s32 unk58;                                        /* +0x058, func_8004F638's 5th/stack arg/func_8004F8A4's 8th arg; also forwarded live to slot60's arg2 */
    u8 pad5C[0x060 - 0x05C];
    s32 unk60;                                          /* +0x060, func_8004F5DC: removeChild's arg */
    s32 unk64;                                            /* +0x064, func_8004F5DC: removeChild's arg */
    s32 unk68;                                             /* +0x068, func_8004F55C (arg6)/func_8004F5DC (cleared) */
    s32 unk6C;                                              /* +0x06C, func_8004F55C (arg7)/func_8004F5DC (cleared) */
    s32 unk70;                                                /* +0x070, func_8004F55C (cleared) */
};

/* The three library callbacks TaskObjF__EnableEvents/TaskObjF__DisableEvents/TaskObjF__TestEvents
 * forward into TaskObjF__ForEachEvent are EnableEvent/DisableEvent/TestEvent, now
 * linked from the Psy-Q objects libapi/a12, libapi/a13 and libapi/a11.
 * Their declarations live in src/class_3bb8c_f.c, the only unit that uses
 * them: a prototype for a function a Sony object defines does not belong in
 * a header 21 units include, where it would one day collide with the real
 * KERNEL.H. (The old comment here called them "SPU routines" -- they are
 * kernel event-queue calls; only their neighbours in the block are libspu.)
 * TestEvent is also the validity check FindFirstReadyEvent uses on its own array
 * argument. */

/* Generic "find the first of up to `count` entries for which
 * TestEvent accepts it, retrying the whole array forever if none
 * qualify yet" helper -- TaskObjF__FindReadyEvent calls it on this unit's own
 * TaskObjF::events (count 4). D_80086E78 is a small lookup table indexed
 * by the winning slot; bound unknown from this unit alone, left unsized.
 * (Comment updated round 60: the callback was `func_800390F4` before round
 * 34 linked it as Sony's own `TestEvent`; `field14` renamed to `events`.) */
extern s32 D_80086E78[];
extern s32 FindFirstReadyEvent(s32 *arr, s32 count);

/* The generic pool allocator/free pair, already established the same way
 * by include/code_8220.h, include/code_55dd4.h etc -- `func_80017CFC`
 * returning `void *` (not `void`) matches func_8004F784's own use here,
 * which stores its return value back into the freed slot. */
/* func_80017B34/func_80017CFC already declared above in this header. */
extern void *func_80017CFC(void *ptr);

/* A fixed 6-byte memory-card device-name template ("bu00:"/"bu10:", PS-X
 * BIOS device names -- asm/data/7B008.sdata.s). An all-`s8` struct
 * (natural alignment 1) so the whole-struct assignment in BuildMemcardPath
 * reproduces retail's unaligned lwl/lwr + byte-store copy, the same idiom
 * already documented for `Descriptor10` above. */
typedef struct DeviceName866E8 {
    s8 b0, b1, b2, b3, b4, b5;
} DeviceName866E8;

extern DeviceName866E8 D_8008AA9C;   /* "bu10:" */
extern DeviceName866E8 D_8008AAA4;   /* "bu00:" */

/* This project's own strcat (matched elsewhere, src/code_171e0.c) --
 * BuildMemcardPath is this unit's only caller. */
extern char *strcat(char *dest, char *src);

/* BasicClass's own method table getter (include/code_8220.h's
 * `Get_vtable_BasicClass`/`BasicClassMethods`, established there from
 * BASICCLASS_METHODS/D_8006B58C -- see that header for slot38's exact
 * signature, `void (*)(BasicClass *self, void *arg1, s32 arg2)`, which
 * this local view matches). Kept as this unit's own independent local
 * view (same policy as Obj866E8Methods vs. class_3ac78's Class866E8Methods
 * above) rather than including code_8220.h, since nothing here needs any
 * OTHER field of BasicClass.
 *
 * Extended round 14 (class_3bb8c_i) to name the ctor/finalize/addChild/
 * removeChild/removeAllChildren slots -- previously opaque pad, now named
 * from that unit's own functions dispatching through them (func_80050C14's
 * base-ctor call, func_80050CE8/func_80050D30/func_80050DB4/func_80050E34's
 * explicit `Get_vtable_BasicClass()->slotN(...)` base-class calls). Pure pad-to-
 * field split, same total size, offset of the pre-existing `slot38` is
 * unchanged. `void *self` throughout, matching `slot38`'s existing style. */
typedef struct BasicMethods866E8F BasicMethods866E8F;
struct BasicMethods866E8F {
    u8 pad000[0x008];
    /* Named round 14 by class_3bb8c_i and round 15 by class_3bb8c_j, from
     * disjoint call sites that AGREE on every offset and signature -- the
     * complementary-views case in PARALLEL-RUNS collision rule 1, unioned
     * here. These are BasicClassMethods' canonical slots
     * (include/code_8220.h), reached through each unit's own local view of
     * the same real getter/table. Pure pad-to-field split: same total
     * size, `slot38`'s offset unchanged. */
    void (*ctor)(void *self);                    /* +0x008, func_80050C14 (_i) / func_80051AC8 (_j, STALLED) */
    void (*finalize)(void *self);                /* +0x00C, func_80050CE8 (_i) / func_80051C84 (_j) */
    void (*addChild)(void *self, void *child);   /* +0x010, func_80050D30 (_i) / func_80051D1C (_j) */
    void (*removeChild)(void *self, void *child);/* +0x014, func_80050DB4 (_i) / func_80051DA0 (_j) */
    void (*removeAllChildren)(void *self);       /* +0x018, func_80050E34 (_i) / func_80051E20 (_j) */
    u8 pad01C[0x038 - 0x01C];
    void (*slot38)(void *self, void *arg1, s32 arg2); /* +0x038, func_8004FB04's first dispatch */
};
extern BasicMethods866E8F *Get_vtable_BasicClass(void);

/* ROUND 34: five of the six prototypes that used to sit here were the PSX
 * BIOS file trampolines, and they are Sony's -- `open`/`read`/`lseek`/
 * `close`/`delete`, now linked from libapi/a50,a52,a51,a54,a69. They are gone
 * from this SHARED header deliberately, not lost. CLAUDE.md's rule: a
 * prototype for a function ANOTHER unit defines -- here, a Sony object --
 * belongs in the `.c` that calls it. Under Sony's names that matters more,
 * not less: `open`/`read`/`close` are generic enough that a future
 * include/psyq prototype (measured 2026-09-12: none of the shipped headers
 * declares them today, only comments and O_* macros) would collide in
 * whichever of the eleven including units pulled both in first, and the two
 * real callers already disagree about the first argument's type.
 * Each caller now carries its own local `extern` with its own argument shape
 * -- see src/class_3bb8c_f.c and src/class_3bb8c_e.c.
 *
 * func_800507F8 stays: it is game code, defined in src/class_3bb8c_g.c
 * (MATCHED round 45, 60/60 words -- was gp_rel-blocked, resolved round 42). */
extern s32 func_800507F8(s32 arg0, s32 arg1);                /* TaskObjF__WriteMemcardSaveFile's own retry-loop bracket; also called with (arg,0) after the retry loop gives up */

/* -------------------------------------------------------------------
 * class_3bb8c_m additions below (fourth 20-function slice of the tail,
 * 0x44518..0x44F14 -- see src/class_3bb8c_m.c's header comment for the
 * carve provenance). This is a DIFFERENT object graph from Obj866E8
 * above: the field offsets established here (0x10, 0x14, 0x18, 0x20,
 * 0x34, 0x3C, 0x54, 0x80, 0x84) don't correspond to anything already
 * documented on Obj866E8's layout, and this unit's own self-vtable slots
 * (0x10, 0x14, 0x30, 0xB8, 0xD4) collide with nothing already recorded in
 * Obj866E8Methods either. Kept as an entirely independent type rather
 * than folded into Obj866E8, per this project's established
 * multiple-independent-local-views convention -- nothing in this unit's
 * own evidence ties its `self` to that type. Named with an "M" suffix to
 * avoid implying any relationship.
 * ------------------------------------------------------------------- */

typedef struct ObjM ObjM;
typedef struct ObjMMethods ObjMMethods;
typedef struct FieldM14 FieldM14;
typedef struct FieldM14Methods FieldM14Methods;
typedef struct FieldM18 FieldM18;
typedef struct FieldM18Methods FieldM18Methods;
typedef struct FieldM3C FieldM3C;
typedef struct FieldM3CMethods FieldM3CMethods;
typedef struct ChildM_AC ChildM_AC;
typedef struct ChildM_ACMethods ChildM_ACMethods;
typedef struct ChildM114 ChildM114;
typedef struct SubM4 SubM4;
typedef struct SubM4Methods SubM4Methods;
typedef struct ParamM ParamM;
typedef struct ParamMMethods ParamMMethods;
typedef struct FieldM34 FieldM34;
typedef struct FieldM34Methods FieldM34Methods;
typedef struct FieldM50 FieldM50;
typedef struct FieldM50Methods FieldM50Methods;
typedef struct FieldM7C FieldM7C;
typedef struct FieldM7CMethods FieldM7CMethods;

/* self->unk3C's target (func_80053D18/func_80053D9C/func_80053E00/
 * func_80053F84/func_80054120). Rich vtable; only the slots this unit's
 * functions actually dispatch are typed. */
struct FieldM3CMethods {
    u8 pad000[0x0F0];
    void (*slotF0)(FieldM3C *self, s32 *out, s32 arg2); /* +0x0F0, func_80053D18: writes *out */
    void (*slotF4)(FieldM3C *self, s32 arg1);           /* +0x0F4, func_80053D9C/func_80053E00/func_80053F84 */
    u8 pad0F8[0x0FC - 0x0F8];
    void (*slotFC)(FieldM3C *self);                     /* +0x0FC, func_80053D18 */
    u8 pad100[0x13C - 0x100];
    void (*slot13C)(FieldM3C *self, s32 arg1);          /* +0x13C, func_80053E00 */
    u8 pad140[0x17C - 0x140];
    void (*slot17C)(FieldM3C *self, s32 arg1);          /* +0x17C, func_80053F84 */
    u8 pad180[0x1A0 - 0x180];
    void *(*slot1A0)(FieldM3C *self, s32 arg1);         /* +0x1A0, func_80054120 (return discarded there) */
};
struct FieldM3C {
    FieldM3CMethods *methods;   /* +0x000 */
};

/* self->unk18's target (func_80053EB4/func_80053F84). */
struct FieldM18Methods {
    u8 pad000[0x064];
    void (*slot64)(FieldM18 *self, s32 arg1);   /* +0x064, func_80053F84 */
    u8 pad068[0x0AC - 0x068];
    ChildM_AC *(*slotAC)(FieldM18 *self);       /* +0x0AC, func_80053EB4 */
    u8 pad0B0[0x0B4 - 0x0B0];
    void (*slotB4)(FieldM18 *self, s32 arg1);   /* +0x0B4, func_800543FC (not this round's target) */
};
struct FieldM18 {
    FieldM18Methods *methods;   /* +0x000 */
};

/* Returned by FieldM18Methods::slotAC (func_80053EB4). */
struct ChildM_ACMethods {
    u8 pad000[0x0D0];
    void (*slotD0)(ChildM_AC *self, s32 arg1);                      /* +0x0D0 */
    u8 pad0D4[0x0D8 - 0x0D4];
    void (*slotD8)(ChildM_AC *self, s32 arg1, s32 arg2, s32 arg3);  /* +0x0D8 */
};
struct ChildM_AC {
    ChildM_ACMethods *methods;  /* +0x000 */
};

/* self->unk14's target (func_80054120). */
struct FieldM14Methods {
    u8 pad000[0x114];
    ChildM114 *(*slot114)(FieldM14 *self, s32 *out);  /* +0x114, func_80054120 */
};
struct FieldM14 {
    FieldM14Methods *methods;   /* +0x000 */
};

/* Returned by FieldM14Methods::slot114 (func_80054120). */
struct SubM4Methods {
    u8 pad000[0x084];
    void (*slot84)(SubM4 *self);  /* +0x084, func_80054120 */
};
struct SubM4 {
    SubM4Methods *methods;      /* +0x000 */
    u8 pad004[0x034 - 0x004];
    s32 unk34;                   /* +0x034, func_80054120 */
};
struct ChildM114 {
    u8 pad0[0x004];
    SubM4 *unk4;                 /* +0x004, func_80054120 */
    u8 pad8[0x014 - 0x008];
    s32 unk14;                    /* +0x014, func_80054120: set from func_8005C7D4's return */
};

/* func_80053F84's own arg1 parameter -- dispatched via its own vtable
 * slot 0xE4. Independent of FieldM14 above (unrelated numeric slot
 * range, nothing ties the two together). */
struct ParamMMethods {
    u8 pad000[0x0E4];
    s32 (*slotE4)(ParamM *self);  /* +0x0E4, func_80053F84 */
};
struct ParamM {
    ParamMMethods *methods;      /* +0x000 */
};

/* Uncarved sibling (src/code_4cd08.c, still INCLUDE_ASM), func_80054120's
 * only external call. Typed purely from that call site's own register
 * setup: (value, out-pointer, opaque-object) -> s32, whose result is
 * stored into a ChildM114's unk14 and tested for zero. The third arg is
 * FieldM3CMethods::slot1A0's own return value (not `self->unk14`'s
 * child), so it stays void* rather than ChildM114* -- nothing ties the
 * two together. */
extern s32 func_8005C7D4(s32 arg0, s32 *arg1, void *arg2);

/* self->unk34's target (func_800543FC/func_800542D0). */
struct FieldM34Methods {
    u8 pad000[0x088];
    void (*slot88)(FieldM34 *self);  /* +0x088, func_800542D0 */
    void (*slot8C)(FieldM34 *self);  /* +0x08C, func_800543FC */
};
struct FieldM34 {
    FieldM34Methods *methods;   /* +0x000 */
};

/* self->unk10/self->unk54's shared target (func_800543FC/func_800542D0).
 * Distinct from `Unk64Elem` (include/code_2cc8c.h) despite sharing slot
 * NUMBERS with it (0x4C/0x50): self->unk10's own slot4C call site here
 * (func_800542D0) sets up only ONE argument (self), which conflicts with
 * `Unk64ElemMethods::slot4C`'s already-established 3-argument signature
 * (from that unit's own call sites) -- real counter-evidence against
 * unifying the two, per this project's established arity-conflict rule.
 * Kept as its own type. */
struct FieldM50Methods {
    u8 pad000[0x04C];
    void (*slot4C)(FieldM50 *self);  /* +0x04C, func_800542D0 */
    void (*slot50)(FieldM50 *self);  /* +0x050, func_800543FC */
};
struct FieldM50 {
    FieldM50Methods *methods;   /* +0x000 */
};

/* self->unk7C's target (func_800543FC/func_800542D0). Returned by
 * New_Obj6EAC0, matched elsewhere (src/code_2cc8c_f.c) with return type
 * `Unk64Elem *` (include/code_2cc8c.h). This unit keeps its own
 * independent local view rather than including code_2cc8c.h, per the
 * project's established multiple-independent-local-views convention --
 * every slot below (0x004/0x04C/0x0B8) matches `Unk64ElemMethods` exactly
 * in both offset and signature, which is suggestive (not proof) that this
 * IS that same class. A different translation unit is free to type a
 * shared external function's return differently; this does not disturb
 * code_2cc8c_f.c's already-matched bytes. */
struct FieldM7CMethods {
    u8 pad000[0x004];
    void (*slot4)(FieldM7C *self);                              /* +0x004, func_800543FC */
    u8 pad008[0x04C - 0x008];
    void (*slot4C)(FieldM7C *self, FieldM14 *arg1, void *arg2); /* +0x04C, func_800542D0 */
    u8 pad050[0x0B8 - 0x050];
    void (*slotB8)(FieldM7C *self, void *arg1);                  /* +0x0B8, func_800542D0 */
};
struct FieldM7C {
    FieldM7CMethods *methods;   /* +0x000 */
};

/* This unit's own view of the uncarved New_Obj6EAC0 (see FieldM7C's
 * comment above for why the return type differs from the other unit's
 * already-matched view of the same external symbol). */
extern FieldM7C *New_Obj6EAC0(void *ctx, s32 len, char *name);

/* func_800542D0's own literal arguments -- a "Pause" name string plus two
 * small opaque blocks, all reached only by address (never dereferenced in
 * this unit). */
extern char D_8008AB44[];   /* "Pause" (asm/data/7B008.sdata.s) */
extern s32 D_8008AB38;      /* two-word opaque block, address-only here */
extern s32 D_8008AB40;      /* one-word opaque block, address-only here */

/* The self type for this unit's func_80053D18..func_8005426C cluster.
 * Only the slots/fields these functions actually reach are typed. */
struct ObjMMethods {
    u8 pad000[0x010];
    void (*slot10)(ObjM *self, ChildM_AC *arg1);  /* +0x010, func_80053EB4 */
    void (*slot14)(ObjM *self, ParamM *arg1);     /* +0x014, func_80053F84 */
    u8 pad018[0x030 - 0x018];
    void (*slot30)(ObjM *self, s32 arg1);          /* +0x030, func_80053E84/func_80053F84/func_80054208/func_8005426C */
    u8 pad034[0x0B8 - 0x034];
    void (*slotB8)(ObjM *self);                     /* +0x0B8, func_800540E8 */
    u8 pad0BC[0x0D4 - 0x0BC];
    void (*slotD4)(ObjM *self);                     /* +0x0D4, func_80054208/func_8005426C */
};

struct ObjM {
    ObjMMethods *methods;   /* +0x000 */
    u8 pad004[0x010 - 0x004];
    /* RETYPED round 15b (func_800542D0/func_800543FC): was `s32 unk10`,
     * established from func_80053EB4's OWN call site as a plain forwarded
     * register value (never dereferenced there). func_800543FC/
     * func_800542D0 dereference this same field's vtable directly, so it
     * IS a pointer -- func_80053EB4 forwards it opaquely either way, and
     * a pointer-typed argument passed as a raw register value compiles to
     * the identical `lw`/`move` regardless of C-level pointer-vs-s32
     * typing (ABI-neutral retype, zero byte cost -- verified: full
     * rebuild stays whole-image green after this change). */
    FieldM50 *unk10;         /* +0x010, func_80053EB4 (opaque forward)/func_800543FC/func_800542D0 */
    FieldM14 *unk14;         /* +0x014, func_80054120 */
    FieldM18 *unk18;         /* +0x018, func_80053EB4/func_80053F84 */
    u8 pad01C[0x020 - 0x01C];
    s32 unk20;                /* +0x020, func_80053D18/func_80053D9C/func_80053E00/func_80053F84: a mode/state code */
    u8 pad024[0x034 - 0x024];
    FieldM34 *unk34;          /* +0x034, func_800543FC/func_800542D0 */
    u8 pad038[0x03C - 0x038];
    FieldM3C *unk3C;          /* +0x03C, func_80053D18/func_80053D9C/func_80053E00/func_80053F84/func_80054120 */
    u8 pad040[0x054 - 0x040];
    FieldM50 *unk54;          /* +0x054, func_800543FC/func_800542D0 -- same type as unk10 above */
    u8 pad058[0x074 - 0x058];
    void *unk74;               /* +0x074, func_800542D0: forwarded opaquely to New_Obj6EAC0's ctx arg */
    u8 pad078[0x07C - 0x078];
    FieldM7C *unk7C;          /* +0x07C, func_800542D0 (written)/func_800543FC (dispatched) */
    s32 unk80;                 /* +0x080, func_800541D4/func_800542D0 */
    s32 unk84;                 /* +0x084, func_800541D4/func_80054200/func_80054208/func_8005426C */
};

/* -------------------------------------------------------------------
 * HEAD NOTE, round 15 merge: `ObjM` (above, from class_3bb8c_m) and
 * `Obj87034_3bb8c_l` (below, from class_3bb8c_l) are the SAME CLASS.
 * Both units independently reached method table D_80087034, which is
 * exactly the collision runner delta anticipated when it suffixed its
 * type names. The proof is a cross-unit call, not a guess:
 * func_80053EB4 is DEFINED in class_3bb8c_m.c taking `ObjM *self` and
 * CALLED from class_3bb8c_l.c (func_80053C94) passing its own
 * `Obj87034_3bb8c_l *self` as the same first argument.
 *
 * They are deliberately NOT unified yet. Both views are byte-exact as
 * they stand, they were derived from disjoint evidence, and merging two
 * field maps is a struct edit that reaches 24 matched functions across
 * two units -- the round-13 hazard, and not something to do inside a
 * merge resolution. Unify as its own change, with the whole-image SHA1
 * re-verified after, and fold `ObjM`s offsets in as the more complete
 * view. Until then, treat a field present in one view and absent in the
 * other as unknown-but-real rather than contradictory.
 * ------------------------------------------------------------------- */

/*
 * class_3bb8c_l -- the class whose method table is D_80087034 (53 slots,
 * resolved with tools/classtable.py 0x80087034). This unit is the FIRST to
 * write any of this class's own methods, but sibling units class_3bb8c_k
 * and class_3bb8c_m are being carved/worked in the SAME round and may
 * independently reach the SAME table -- per round 14's learning ("suffix
 * new type names with your unit" when other runners are live on the same
 * header), every type below carries the `_3bb8c_l` suffix so a later merge
 * cannot collide on a name. Only the slots/fields this unit's own chosen
 * functions actually dispatch through are given concrete types; everything
 * else stays opaque padding.
 */
typedef struct Obj87034_3bb8c_l Obj87034_3bb8c_l;

/* self->unk50's pointee: a plain (non-vtable) record, read directly by
 * func_800531CC via ordinary field offsets, never through a methods
 * pointer -- so it is NOT another Obj87034_3bb8c_l, just an opaque
 * 3-field descriptor. */
typedef struct Unk50Struct_3bb8c_l {
    s32 unk0;      /* +0x000, round 45's func_800534C8: forwarded opaquely to Obj14Methods_3bb8c_l::slotC4's arg2 */
    s32 unk4;      /* +0x004, round 45's func_800534C8: forwarded opaquely to Obj14Methods_3bb8c_l::slotC4's arg3 */
    s32 unk8;      /* +0x008, round 45's func_800534C8: forwarded opaquely to Obj14Methods_3bb8c_l::slotBC's arg1 */
    void *unkC;   /* +0x00C, func_800531CC (address taken, forwarded opaquely) */
    u8 pad10[0x014 - 0x010];
    s32 unk14;    /* +0x014, func_800531CC: discriminant compared against 2; also func_80053764: discriminant compared against 1 */
    void *unk18;  /* +0x018, func_800531CC (address taken, forwarded opaquely) */
    void *unk1C;  /* +0x01C, func_80053764 (address taken, forwarded opaquely) */
} Unk50Struct_3bb8c_l;

/* Whatever self->unk14 points to: an object of some OTHER, unidentified
 * class -- it has its own methods pointer at +0x000 (func_8005393C
 * dispatches +0x10C on it) AND a plain u16 field at +0x1B4 (func_800531CC
 * reads it directly). Offset +0x10C happens to coincide with a DreamSys
 * vtable offset, but DreamSys's own occupant there (func_8005937C) takes
 * one s32 argument while this call site passes two -- different arities,
 * so this is a different class, not DreamSys; left unnamed. */
typedef struct Obj14Methods_3bb8c_l {
    u8 pad000[0x0BC];
    /* +0x0BC, round 45's func_800534C8: `(self, self->unk50->unk8, 0)`. */
    void (*slotBC)(void *self, s32 arg1, s32 arg2); /* +0x0BC */
    u8 pad0C0[0x0C4 - 0x0C0];
    /* +0x0C4, round 45's func_800534C8: `(self, 3, self->unk50->unk0,
     * self->unk50->unk4)`. */
    void (*slotC4)(void *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0C4 */
    u8 pad0C8[0x0CC - 0x0C8];
    /* +0x0CC, round 45's func_800534C8: `(self, &D_8008710C)`. */
    void (*slotCC)(void *self, void *arg1); /* +0x0CC */
    u8 pad0D0[0x0DC - 0x0D0];
    /* +0x0DC, round 45's func_800534C8: `(self, self->unk48)` on the
     * OWNING Obj87034_3bb8c_l. */
    void (*slotDC)(void *self, s32 arg1); /* +0x0DC */
    /* +0x0E0, round 45's func_800534C8: `(self, GetStageGridDimensions(
     * self->unk38))`. */
    void (*slotE0)(void *self, void *arg1); /* +0x0E0 */
    u8 pad0E4[0x0EC - 0x0E4];
    void (*slotEC)(void *self);                        /* +0x0EC, func_80053764 */
    u8 padF0[0x10C - 0x0F0];
    void *(*slot10C)(void *self, s32 arg1, s32 arg2); /* +0x10C, func_8005393C */
    u8 pad110[0x134 - 0x110];
    void (*slot134)(void *self, void *arg1);           /* +0x134, func_80052F10 */
} Obj14Methods_3bb8c_l;
typedef struct Obj14_3bb8c_l {
    Obj14Methods_3bb8c_l *methods; /* +0x000 */
    u8 pad04[0x1B4 - 0x004];
    u16 unk1B4;                     /* +0x1B4, func_800531CC */
} Obj14_3bb8c_l;

/* Whatever self->unk3C/self->unk18 point to. Resolved by cross-checking
 * the exact offsets this unit's functions dispatch (+0x050, +0x074,
 * +0x0FC, +0x104, +0x108, +0x200) against tools/classtable.py's dump of
 * DREAMSYS_METHODS (0x80087BDC, include/DreamSys.h): every one lands on a
 * real occupant there (func_80058A94, Class6B5CC__GetSetUnk10Field0, func_80059310,
 * DreamSys__GetSetDreamTimeLimit, func_80059360, DreamSys__GetDreamColor
 * respectively), and the two whose header signatures are pinned down
 * (+0x104 `s32(DreamSys*, s32)`, +0x108 `s32(DreamSys*)`) match this unit's
 * own call-site arities exactly. So this almost certainly IS DreamSys, but
 * declared as this unit's own minimal, independent, offset-only view
 * (rather than including DreamSys.h) since none of DreamSys.h's own named
 * fields cover these particular slots (they sit inside its
 * `unknown_functions_0x..` padding arrays) and DreamSys.h is a different
 * unit's header, not this one's to extend. */
typedef struct DreamSysMethods_3bb8c_l {
    u8 pad00[0x04C];
    /* +0x04C, round 45's func_800534C8: `(self, self->unk14)`, dispatched
     * on the OWNING Obj87034_3bb8c_l's own `unk3C` (a DIFFERENT
     * DreamSysObj_3bb8c_l instance from the `self->unk18` this function
     * dispatches every other slot through). */
    void (*slot4C)(void *self, void *arg1); /* +0x04C */
    void (*slot50)(void *self);          /* +0x050, func_800536B0 */
    /* +0x054, round 45's func_800534C8: `(self, val)`, `val` a small
     * derived integer (`(*obj->methods->slot7C(obj, 0)) / 2 * 5 / 3 +
     * D_8008AB34`, `obj` being `*(void **)self->unkC`). */
    void (*slot54)(void *self, s32 arg1); /* +0x054 */
    u8 pad58[0x060 - 0x058];
    void (*slot60)(void *self, s32 arg1);          /* +0x060, func_80053764 */
    void (*slot64)(void *self, void *arg1);        /* +0x064, func_80053764 */
    void (*slot68)(void *self, void *arg1);        /* +0x068, func_80053764 */
    void (*slot6C)(void *self, void *arg1);        /* +0x06C, func_80053764 */
    void (*slot70)(void *self, void *arg1, void *arg2, void *arg3, s32 arg4); /* +0x070, func_80052F10 */
    void (*slot74)(void *self);          /* +0x074, func_800536B0 */
    u8 pad78[0x0AC - 0x078];
    /* Returns another DreamSysObj_3bb8c_l* -- its return value is dispatched
     * through methods->slotF0/slotD4 the same way self->unk3C/self->unk18
     * themselves are, so it is almost certainly a "related instance"
     * accessor rather than a plain getter of scalar data. Named via the
     * elaborated `struct DreamSysObj_3bb8c_l *` (not the typedef, which is
     * not yet in scope this early in the header) to avoid a forward-typedef
     * redefinition -- GCC 2.6.3 rejects `typedef struct X X;` twice even
     * with an identical definition. */
    struct DreamSysObj_3bb8c_l *(*slotAC)(void *self);    /* +0x0AC, func_80053764 */
    void (*slotB0)(void *self, s32 arg1);          /* +0x0B0, func_80053764 */
    void (*slotB4)(void *self, s32 arg1);          /* +0x0B4, func_80053764 */
    u8 padB8[0x0D4 - 0x0B8];
    void (*slotD4)(void *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0D4, func_80053764 */
    u8 padD8[0x0EC - 0x0D8];
    s32 (*slotEC)(void *self, s32 arg1);              /* +0x0EC, func_80052F10 */
    s32 (*slotF0)(void *self, s32 *outBuf, s32 arg2); /* +0x0F0, func_80053ACC (STALLED 28/71 -- offset/signature observed directly from the disassembly, reliable independent of the stall; see docs/match-reports/func_80053ACC.md). ALSO func_80053764, on a DIFFERENT instance (the slotAC return value) with a DIFFERENT 2nd-arg shape (plain s32, not a pointer) -- same slot, two call-site views, per this project's established convention; see that function's report. */
    u8 padF4[0x0F8 - 0x0F4];
    void (*slotF8)(void *self, s32 arg1, s32 arg2); /* +0x0F8, func_80053764 */
    void (*slotFC)(void *self);          /* +0x0FC, func_800536B0/func_80053C94 */
    u8 pad100[0x104 - 0x100];
    s32 (*slot104)(void *self, s32 arg1); /* +0x104, func_800531CC */
    s32 (*slot108)(void *self);           /* +0x108, func_800531CC */
    u8 pad10C[0x1A0 - 0x10C];
    s32 (*slot1A0)(void *self, s32 arg1); /* +0x1A0, func_80052F10 (return value forwarded opaquely to two other calls) */
    u8 pad1A4[0x200 - 0x1A4];
    s32 (*slot200)(void *self);           /* +0x200, func_80053C94 */
} DreamSysMethods_3bb8c_l;
typedef struct DreamSysObj_3bb8c_l {
    DreamSysMethods_3bb8c_l *methods;
    u8 pad04[0x044 - 0x004];
    s32 unk44;               /* +0x044, func_80053984: cleared (only reached when self->unk20 != 0 and the event/code is >= 9) */
    u8 pad48[0x164 - 0x048];
    s32 unk164;             /* +0x164, func_80053BE8: sign-checked gate */
} DreamSysObj_3bb8c_l;

/* Whatever arg1->unkC points to in func_80052DE8 -- a registration sink
 * of some kind (arg1->unkC->methods->slotC8(arg1->unkC, callback,
 * userdata) reads like "subscribe `callback` for `userdata`"). Only the
 * one slot this unit calls through is named. */
typedef struct RegistrantMethods_3bb8c_l {
    u8 pad00[0x0C8];
    void (*slotC8)(void *self, void (*callback)(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3), Obj87034_3bb8c_l *userdata); /* +0x0C8, func_80052DE8 */
} RegistrantMethods_3bb8c_l;
typedef struct RegistrantObj_3bb8c_l {
    RegistrantMethods_3bb8c_l *methods;
} RegistrantObj_3bb8c_l;

typedef struct Obj87034Methods_3bb8c_l {
    s32 header;                                                    /* +0x000 */
    void (*slot04)(Obj87034_3bb8c_l *self);                        /* +0x004, BasicClass generic (func_80017EB0); dispatched directly by func_800531CC on its `other` argument */
    u8 pad08[0x010 - 0x008];
    void (*slot10)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x010, func_80052DE8 */
    void (*slot14)(Obj87034_3bb8c_l *self, void *arg1);            /* +0x014, func_80052EBC/func_800536B0 */
    u8 pad18[0x030 - 0x018];
    void (*slot30)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x030, func_80053ACC (STALLED 28/71 -- offset/signature observed directly from the disassembly, reliable independent of the stall; see docs/match-reports/func_80053ACC.md) */
    u8 pad34[0x048 - 0x034];
    void (*slot48)(Obj87034_3bb8c_l *self);                        /* +0x048, func_80052EBC/func_80053134 (via self->unk54) */
    u8 pad4C[0x05C - 0x04C];
    void (*slot5C)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x05C, func_80052F10 (arg1 is func_80048F84's return value, forwarded opaquely) */
    u8 pad60[0x074 - 0x060];
    void (*slot74)(Obj87034_3bb8c_l *self);                        /* +0x074, func_80053358 (event 0x21) */
    u8 pad78[0x07C - 0x078];
    void (*slot7C)(Obj87034_3bb8c_l *self, void *arg1);            /* +0x07C, func_800531CC */
    void (*slot80)(Obj87034_3bb8c_l *self);                        /* +0x080, func_800531CC */
    void (*slot84)(Obj87034_3bb8c_l *self);                        /* +0x084, func_80053134 */
    void (*slot88)(Obj87034_3bb8c_l *self);                        /* +0x088, func_800531CC */
    void (*slot8C)(Obj87034_3bb8c_l *self);                        /* +0x08C, func_800533F0 */
    u8 pad90[0x094 - 0x090];
    void (*slot94)(Obj87034_3bb8c_l *self);                        /* +0x094, func_80053984 (event/code 0xA, dense switch) */
    void (*slot98)(Obj87034_3bb8c_l *self);                        /* +0x098, func_80053984 (event/code 0xC) */
    void (*slot9C)(Obj87034_3bb8c_l *self);                        /* +0x09C, func_80053BE8; ALSO func_80053984 (event/code 0xD) */
    void (*slotA0)(Obj87034_3bb8c_l *self);                        /* +0x0A0, func_80053984 (event/code 0xE) */
    void (*slotA4)(Obj87034_3bb8c_l *self);                        /* +0x0A4, func_80053984 (event/code 0xF) */
    void (*slotA8)(Obj87034_3bb8c_l *self);                        /* +0x0A8, func_80053984 (event/code 0x10) */
    void (*slotAC)(Obj87034_3bb8c_l *self);                        /* +0x0AC, func_80053984 (event/code 0x11) */
    u8 padB0[0x0C0 - 0x0B0];
    void (*slotC0)(Obj87034_3bb8c_l *self);                        /* +0x0C0, func_80053358 (event 0xC) */
    void (*slotC4)(Obj87034_3bb8c_l *self);                        /* +0x0C4, func_80053358 (event 0x2C)/func_80053458 */
    void (*slotC8)(Obj87034_3bb8c_l *self);                        /* +0x0C8, func_80053358 (event 0x16) */
    u8 padCC[0x0D0 - 0x0CC];
    void (*slotD0)(Obj87034_3bb8c_l *self);                        /* +0x0D0, func_800533F0/func_80053458 */
    void (*slotD4)(Obj87034_3bb8c_l *self);                        /* +0x0D4, func_800536B0/func_80053458 */
} Obj87034Methods_3bb8c_l;

struct Obj87034_3bb8c_l {
    Obj87034Methods_3bb8c_l *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    RegistrantObj_3bb8c_l *unkC;      /* +0x00C, func_80052DE8's `arg1->unkC` */
    s32 unk10;                        /* +0x010, func_80053764 */
    Obj14_3bb8c_l *unk14;              /* +0x014, func_800531CC/func_8005393C/func_800536B0 */
    DreamSysObj_3bb8c_l *unk18;         /* +0x018, func_800536B0 */
    s32 unk1C;                           /* +0x01C, func_800533F0: incremented once per call */
    s32 unk20;                            /* +0x020, func_80053C94: written 6 (a state/phase tag; also written 4 by func_80053ACC (STALLED) and written 5 by func_80053BE8, both round 16 echo) */
    u8 pad24[0x034 - 0x024];
    s32 unk34;                            /* +0x034, round 45's func_800534C8: forwarded opaquely to func_8005C650's own arg3 */
    void *unk38;                          /* +0x038, func_80052E7C: forwarded opaquely to func_80049060/func_80049098 */
    DreamSysObj_3bb8c_l *unk3C;            /* +0x03C, many functions in this unit */
    s32 unk40;                             /* +0x040, func_80053764 */
    s32 unk44;                              /* +0x044, func_80053764 */
    s32 unk48;                               /* +0x048, func_80052F10: set from arg1, or 0xA000 if arg1==0 */
    s32 unk4C;                                /* +0x04C, func_80052F10: set from arg3 (only when self->unk38 != 0) */
    Unk50Struct_3bb8c_l *unk50;             /* +0x050, func_800531CC */
    Obj87034_3bb8c_l *unk54;                 /* +0x054, func_80053134 */
    Obj87034_3bb8c_l *unk58;                  /* +0x058, func_800531A0: forwarded as func_800531CC's `other` */
    u8 pad5C[0x060 - 0x05C];
    s32 unk60;                                 /* +0x060, func_800531CC: has-a-target gate, cleared after detaching */
    s32 unk64;                                  /* +0x064, func_800531CC: set to 1 */
    s32 unk68;                                   /* +0x068, func_800531CC/func_80053358/func_800533F0: zero-checked gate */
    s32 unk6C;                                    /* +0x06C, func_80052F10: out-parameter address passed to func_800544E4, own type unknown */
    u8 pad70[0x078 - 0x070];
    DreamSysObj_3bb8c_l *unk78;                    /* +0x078, func_80052F10: cached copy of self->unk18 */
    u8 pad7C[0x080 - 0x07C];
    s32 unk80;                                    /* +0x080, func_800531CC (on `other`)/func_800533F0/func_80053458: zero-checked gate */
};

/* Global BasicClass-family accessor shared across many classes (see
 * include/class_39e08.h's own fuller `Class86668Methods` view of the SAME
 * table, D_80086668) -- declared here as this unit's own minimal,
 * independent local view rather than including that header, per this
 * project's multiple-independent-local-views convention. Only the one
 * slot func_80052DE8 dispatches through is named. */
/* HEAD NOTE round 15: `BaseMethods87034_3bb8c_l` and its
 * `extern ... *func_8004A4B8(void);` prototype were moved into
 * src/class_3bb8c_l.c. THIRD instance this merge of one rule: a cross-unit
 * prototype in a unit-local type must not live in a shared header. This one
 * was the nastiest, because it did not collide with another RUNNER -- it
 * collided with the pre-existing canonical
 * `extern Class86668Methods *func_8004A4B8(void);` in include/class_39e08.h,
 * and it only surfaced when class_3bb8c_k (which includes BOTH headers) was
 * merged two merges later. class_3bb8c_l includes only class_3bb8c.h, so
 * nothing showed up when delta's own work was verified.
 *
 * Worth unifying deliberately: class_39e08.h's Class86668Methods ALREADY
 * declares slot44 and slot48 at the same offsets with ABI-identical shapes,
 * so delta's local view is a duplicate of a type the project already had.
 * Not done here -- it changes a byte-exact unit's types inside a merge
 * resolution, which is the round-13 hazard. */

/* func_80052DE8's own registered callback -- forward-declared here since
 * func_80052DE8 (ROM order earlier) takes its address before its own
 * definition (ROM order later) is reached. */
extern void func_80052E7C(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3);

/* func_800531A0's tail call -- forward-declared for the same ROM-order
 * reason as func_80052E7C above (func_800531CC is defined later). */
extern void func_800531CC(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *other);

/* func_80052E7C's own two helpers -- still-uncarved ground
 * (asm/psyq_memset.s). func_80049060 is genuinely called at two different
 * arities across the executable (func_80049098 forwards to it with 2 real
 * arguments; func_80052E7C's own `code < 0` branch calls it with only 1,
 * the second register being whatever the caller's own incoming `code`
 * argument left behind -- a "leftover register", not a real second
 * argument), so it is declared K&R/unprototyped here, the documented
 * escape hatch for a genuinely multi-arity call (see
 * DECOMPILATION_LEARNINGS round 14). */
extern s32 func_80049060();
extern void func_80049098(void *arg0, s32 arg1, s32 arg2);

/* func_8005393C's own helper -- still-uncarved ground (asm/class_3bb8c_n.s).
 * Typed purely from this call site's own register usage. */
extern void func_800558F0(void *arg0, void *arg1, s32 arg2);

/* func_80053134's own helpers -- still-uncarved ground
 * (asm/class_3bb8c_n.s), called with no arguments and their return values
 * unused. */
extern void func_8005C76C(void);
extern void func_80054D30(void);


/* -------------------------------------------------------------------
 * HEAD NOTE, round 15 merge: `Obj86ED0` below and `Class86ED0` in
 * src/class_3bb8c_j.c are TWO NAMES for the table at D_80086ED0, given
 * independently by runners alpha and bravo in the same round. Unify
 * deliberately, not inside a merge.
 *
 * Read alongside that, one of the two summaries was wrong and the binary
 * settles it. bravo reported this class as "alloc size 0x54, vtable
 * D_80086ED0"; alpha reported 0x4C. Measured:
 *
 *   func_80050BA8  li a0,0x4c -> func_80017B34, then ctors through
 *                  func_80051A4C(), which returns &D_80086ED0.
 *   func_80051A5C  allocates 0x54 and ctors through func_80052B60(),
 *                  a DIFFERENT table getter living in class_3bb8c_k.
 *
 * So D_80086ED0's class is 0x4C bytes (alpha is right), and bravo's
 * 0x54-byte New_X belongs to a different class whose table it had not yet
 * identified -- it typed that object `Class86ED0` after the getter it did
 * recognise. Neither unit's BYTES are affected; both are byte-exact, and
 * a type name is not codegen. What was at risk was the next reader
 * inheriting "D_80086ED0 == 0x54 bytes" as fact.
 * ------------------------------------------------------------------- */
/*
 * Obj86ED0 -- BasicClass-derived class, vtable D_80086ED0 (resolved with
 * `tools/classtable.py D_80086ED0`, 42 slots; the ONLY class this unit
 * (class_3bb8c_i) itself defines methods for). func_80051A4C (this class's
 * own table getter, `class_3bb8c_j`, still INCLUDE_ASM) returns
 * `&D_80086ED0`; func_80050BA8 is the `New_X`-shaped factory that
 * allocates the 0x4C-byte instance and dispatches its ctor (slot 0x008).
 * Slots 0x004-0x038 line up one-for-one with BasicClass's own 14-slot
 * layout (include/code_8220.h's BasicClassMethods) -- `classtable.py --vs
 * D_8006B58C` confirms release/getNextChild/addParentRef/removeParentRef/
 * clearParentRefs/getNextParentRef/notifyParents/slot34 are UNMODIFIED
 * BasicClass pointers, while ctor/finalize/addChild/removeChild/
 * removeAllChildren/slot38 are all overridden by this unit's own
 * functions. Only the slots this unit's own functions dispatch through
 * SELF (`self->methods->slotN`, as opposed to the explicit
 * `Get_vtable_BasicClass()->slotN` base-table calls, which go through
 * `BasicMethods866E8F` above) are named below; the rest stays opaque
 * padding, same policy as `Obj866E8Methods` elsewhere in this header.
 */
typedef struct Obj86ED0 Obj86ED0;
typedef struct Obj86ED0Methods Obj86ED0Methods;

/* Generic BasicClass-family child object -- only `release` (+0x004,
 * matching BasicClassMethods's own layout) is dispatched on one of
 * these from this unit (func_80051174, on self->unk40/unk44/unk48).
 * Kept minimal/opaque beyond that, same policy as
 * `Class86E00SubObj_3bb8c_g` elsewhere in this header. */
typedef struct ChildObj86ED0 ChildObj86ED0;
typedef struct ChildMethods86ED0 ChildMethods86ED0;
struct ChildMethods86ED0 {
    u8 pad000[0x004];
    void *(*release)(ChildObj86ED0 *self); /* +0x004, func_80051174 */
    u8 pad008[0x04C - 0x008];
    /* +0x04C, func_80050F98 (three call sites, always through self->unk40/
     * unk44/unk48). Same offset/arity as the unrelated FieldM7CMethods::
     * slot4C above -- not unified with it, per this project's established
     * multiple-independent-local-views convention (this unit's own reading
     * from its own call sites). arg1 is func_80050F98's own forwarded
     * parameter; arg2 is a small opaque data blob passed only by address. */
    void (*slot4C)(ChildObj86ED0 *self, void *arg1, void *arg2);
    u8 pad050[0x078 - 0x050];
    void (*slot78)(ChildObj86ED0 *self); /* +0x078, func_80050F98, on the short-lived handle before func_80041C9C/New_Obj6EAC0 consume it */
    u8 pad07C[0x0B8 - 0x07C];
    void (*slotB8)(ChildObj86ED0 *self, void *arg1); /* +0x0B8, func_80050F98, self->unk44 only */
};
struct ChildObj86ED0 {
    ChildMethods86ED0 *methods; /* +0x000 */
};

/* self->unk3C's pointee -- an unrelated class (own vtable, unconnected to
 * D_80086ED0), reached only through its own +0x080 slot by func_8005161C.
 * Field meaning beyond that slot is unestablished. */
typedef struct TargetObj86ED0 TargetObj86ED0;
typedef struct TargetMethods86ED0 TargetMethods86ED0;
struct TargetMethods86ED0 {
    u8 pad000[0x080];
    /* +0x080, func_8005161C: `self->methods->slot80(self, arg1, 0x60, 0x60)`.
     * 3 args, not 2 -- confirmed against this project's established
     * self->methods->slot80(self, arg1, 0x60, 0x60) idiom seen at several
     * other call sites (src/class_3bb8c_k.c, src/code_2cc8c.c,
     * src/class_3bb8c_g.c, src/code_55dd4.c), all forwarding a caller-
     * supplied arg1 alongside a repeated literal. func_8005161C itself
     * takes that arg1 as its own second parameter and forwards it
     * unchanged (same register, no move instruction). */
    void (*slot80)(TargetObj86ED0 *self, s32 arg1, s32 arg2, s32 arg3);
};
struct TargetObj86ED0 {
    TargetMethods86ED0 *methods; /* +0x000 */
};

struct Obj86ED0Methods {
    u8 pad000[0x008];
    /* +0x008, func_80050BA8's own dispatch target -- this class's own
     * ctor, OVERRIDING BasicClass's no-arg ctor with a 2-arg one.
     * func_80050C14 itself, MATCHED round 45; the signature below is
     * the vtable slot's own type (self, arg1, arg2), matching how
     * func_80050BA8 calls it -- func_80050C14's own DEFINITION is typed
     * more precisely (`char *arg1`, since it calls `strlen` on it), which
     * is fine: a data-table vtable slot's declared field type need not
     * match the defining function's own prototype exactly. */
    void (*ctor)(Obj86ED0 *self, s32 arg1, s32 arg2);
    u8 pad00C[0x010 - 0x00C];
    void (*addChild)(Obj86ED0 *self, void *child);    /* +0x010, func_80051200 (OVERRIDES BasicClass's addChild: func_80050D30) */
    void (*removeChild)(Obj86ED0 *self, void *child);  /* +0x014, func_80051270/func_800512C8 (OVERRIDES BasicClass's removeChild: func_80050DB4) */
    u8 pad018[0x030 - 0x018];
    /* +0x030, func_800512C8's own dispatch -- UNMODIFIED BasicClass
     * notifyParents (BasicClass__NotifyParents, code_8220_b), reached through
     * self's own table this one time instead of `Get_vtable_BasicClass()`. */
    void (*notifyParents)(Obj86ED0 *self, s32 arg1);
    u8 pad034[0x040 - 0x034];
    /* +0x040, round 45's func_80050C14 -- its own tail dispatch,
     * `self->methods->slot40(self, arg1, arg2)`, forwarding the ctor's
     * own two arguments unchanged. */
    void (*slot40)(Obj86ED0 *self, s32 arg1, s32 arg2); /* +0x040 */
    u8 pad044[0x048 - 0x044];
    void (*slot48)(Obj86ED0 *self);                      /* +0x048, func_800512C8 -- this class's own slot, func_80051174 */
    u8 pad04C[0x054 - 0x04C];
    void (*slot54)(Obj86ED0 *self, s32 arg1);             /* +0x054, func_80051370 -- this class's own slot, func_800512C8 */
    void (*slot58)(Obj86ED0 *self, void *arg1, s32 arg2);  /* +0x058, func_80050E78's tag==5 case -- this class's own slot, func_80051370 */
    void (*slot5C)(Obj86ED0 *self, void *arg1, s32 arg2);   /* +0x05C, func_80050E78's tag==2 case -- this class's own slot, func_800513D0 */
    /* +0x060, func_800513D0's own `arg2 == 25`/`23` cases: `self->methods->
     * slot60(self, 0x10)`, always with the same literal. */
    void (*slot60)(Obj86ED0 *self, s32 arg1);            /* +0x060 */
    u8 pad064[0x088 - 0x064];
    /* +0x088..+0x0A0, func_800513D0's own dense `arg2` switch: each of
     * these seven slots is resolved into a local function pointer then
     * called as `fn(self)` (no other args) once, after the switch --
     * `arg2 == 21`/`5` -> slot88, `20`/`4` -> slot8C, `18`/`2` -> slot90,
     * `19`/`3` -> slot94 (the two cases per slot gate on `self->unk20`
     * with OPPOSITE polarity, same idiom as func_8004BA40's mask test),
     * `32` -> slotA0, `31` -> slot9C, `28` -> slot98 (these three
     * ungated). All seven share the identical `(Obj86ED0 *self)` shape,
     * confirmed directly off the call site (`jalr $v0; addu $a0,$s0,$zero`,
     * no other register set). */
    void (*slot88)(Obj86ED0 *self);                      /* +0x088 */
    void (*slot8C)(Obj86ED0 *self);                      /* +0x08C */
    void (*slot90)(Obj86ED0 *self);                      /* +0x090 */
    void (*slot94)(Obj86ED0 *self);                      /* +0x094 */
    void (*slot98)(Obj86ED0 *self);                      /* +0x098 */
    void (*slot9C)(Obj86ED0 *self);                      /* +0x09C */
    void (*slotA0)(Obj86ED0 *self);                      /* +0x0A0 */
    void (*slotA4)(Obj86ED0 *self, s32 arg1, s32 arg2);       /* +0x0A4, func_8005165C/func_800516C0 -- func_800518F4, outside this unit's slice */
    /* +0x0A8, func_80051720. 3 args, not 2 -- retail's call sets $a1/$a3
     * (`self->unk18`, `1`) and leaves $a2 holding the just-computed
     * incremented `unk1C` value untouched from a few instructions earlier
     * (no fresh load/li for it), which only makes sense if that register
     * IS the call's own middle argument, forwarded because it was already
     * live there. Same shape as TargetMethods86ED0::slot80 above. */
    void (*slotA8)(Obj86ED0 *self, s32 arg1, s32 arg2, s32 arg3);        /* +0x0A8, func_80051720 -- func_80051998, outside this unit's slice */
};

struct Obj86ED0 {
    Obj86ED0Methods *methods;  /* +0x000 */
    u8 pad004[0x00C - 0x004];   /* inherited BasicClass children/parentRefs, untouched by this unit */
    s32 unkC;                    /* +0x00C, func_80050F28: its own `mode` argument */
    s32 unk10;                   /* +0x010, func_80050F28 (halved when mode==1)/func_8005165C (upper bound tested against unk18+1) */
    s32 unk14;                   /* +0x014, func_80051720 (upper bound tested against unk1C+1) */
    s32 unk18;                   /* +0x018, func_80050F28 (zeroed)/func_8005165C/func_800516C0 (inc/dec counter, capped by unk10) */
    s32 unk1C;                   /* +0x01C, func_80050F28 (zeroed)/func_80051720 (inc counter or reset to 0, capped by unk14) */
    s32 unk20;                   /* +0x020, func_80051200 (zeroed) */
    char *unk24;                 /* +0x024, func_80050F28: its own `arg1` (name string) */
    char *unk28;                 /* +0x028, func_80050CE8 (freed in finalize)/func_80050F28 (DecodeFullWidthSjis/strcpy destination) */
    s32 unk2C;                   /* +0x02C, func_80051200 (zeroed)/func_800512C8 (set to its own arg1 for arg1 in [2,4); read as notifyParents's arg1 for arg1==4)/func_80051370 (range-checked against [2,4)) */
    s32 unk30;                   /* +0x030, func_800512C8 (zeroed)/func_80051370 (incremented; gates the slot54 call on the OLD value being nonzero) */
    void *unk34;                 /* +0x034, func_80050D30/func_80050DB4 (addChild/removeChild target when child's tag==2)/func_800512C8/func_80051270 (removeChild target) */
    void *unk38;                 /* +0x038, func_80050D30/func_80050DB4 (tag==5)/func_80051270 (removeChild target) */
    TargetObj86ED0 *unk3C;        /* +0x03C, func_80051200 (its own arg3)/func_8005161C (dispatch target)/func_80051270 (zeroed) */
    ChildObj86ED0 *unk40;          /* +0x040, func_80051174 (released, no null-back store) */
    ChildObj86ED0 *unk44;           /* +0x044, func_80051174 (released, no null-back store) */
    ChildObj86ED0 *unk48;            /* +0x048, func_80051174 (release+null-back)/func_80050CD8/func_80050E34 (zeroed)/func_8005165C/func_800516C0/func_80051720 (nonzero readiness gate)/func_80050F98 (set from a resolved resource handle) */
};

/* HEAD NOTE round 15: alpha's `extern Obj86ED0Methods *func_80051A4C(void);`
 * also moved into src/class_3bb8c_i.c, for the same reason as the symbol
 * above and with a sharper edge -- runner bravo DEFINES func_80051A4C in
 * src/class_3bb8c_j.c returning its own `Class86ED0Methods *`, so a
 * shared-header prototype in another unit's type is `conflicting types for
 * 'func_80051A4C'`, a hard compile error rather than a warning. A
 * cross-unit prototype for a function ANOTHER unit defines belongs in the
 * caller, not in the shared header, whenever the two units hold different
 * local views of the same class. */
/* HEAD NOTE round 15: the `extern Obj86ED0Methods D_80086ED0;` that stood
 * here was moved into src/class_3bb8c_i.c. src/class_3bb8c_j.c carries its
 * own `extern Class86ED0Methods D_80086ED0;` for the same object, and two
 * incompatible declarations of one symbol in a SHARED header reach both
 * translation units. Unit-local views belong in the unit -- which is what
 * runner bravo did with `Class86ED0` deliberately. The TYPES below stay
 * here because they are the richer, measured view; only the symbol moved. */

/* HEAD NOTE round 15: alpha's `extern char *DecodeFullWidthSjis(char *dest, char
 * *src);` moved into src/class_3bb8c_i.c -- FOURTH instance of the
 * shared-header prototype rule this round. DecodeFullWidthSjis is still
 * INCLUDE_ASM in code_2cc8c_f, so NOBODY has its definition and every
 * declaration is a call-site typing; class_3bb8c_i reads it as
 * `char *(char *, char *)` and class_3bb8c_j as `void (void *, void *)`.
 * Both units are byte-exact with their own reading, because the return
 * type of an INCLUDE_ASM callee only affects the CALLER's codegen. Two
 * call-site typings of one undefined function are exactly the case that
 * must stay unit-local. */

/* -------------------------------------------------------------------
 * HEAD NOTE, round 15 merge: D_80086F88 (below) CLOSES an open question
 * from the alpha/bravo merge, and it is worth reading the two notes
 * together.
 *
 * That note established that bravo's 0x54-byte New_X (func_80051A5C in
 * class_3bb8c_j) ctors through func_80052B60(), "a DIFFERENT table getter
 * living in class_3bb8c_k" that bravo had not identified. charlie has now
 * MATCHED func_80052B60, and it returns &D_80086F88. So the 0x54-byte
 * class's method table is D_80086F88 -- charlie's Class86F88 -- and NOT
 * D_80086ED0.
 *
 * Consequence for the next reader: the type named `Class86ED0` in
 * src/class_3bb8c_j.c is MISNAMED. It is the object of the 0x54-byte
 * class (table D_80086F88); the name came from func_80051A4C, the only
 * getter bravo had resolved at the time, which actually returns
 * D_80086ED0 -- alpha's separate 0x4C-byte class in class_3bb8c_i.
 *
 * Nothing is wrong with the BYTES: class_3bb8c_j is byte-exact and a type
 * name is not codegen. Only the name misleads. Left in place rather than
 * renamed here because runner bravo is still live in that unit as this is
 * written, and PARALLEL-RUNS collision rule 6 says a correction goes down
 * ONE channel -- bravo was messaged, not edited.
 * ------------------------------------------------------------------- */

/*
 * class_3bb8c_k's own view: the class whose method table is D_80086F88
 * (39 slots, tools/classtable.py D_80086F88 -- header word 0x20). Carved
 * round 15; this unit is the first to write any of its methods. Only the
 * slots/fields this unit's functions actually touch are typed; the rest
 * stays opaque. `func_80052644`/`func_800522DC` are two more of this
 * class's own methods (toolchain-blocked, stub reports filed, still
 * INCLUDE_ASM) -- not reflected here since nothing in this unit's C reads
 * through them yet.
 */
typedef struct Class86F88Methods Class86F88Methods;
typedef struct Class86F88 Class86F88;
typedef struct Class86F88ElemMethods Class86F88ElemMethods;
typedef struct Class86F88Elem Class86F88Elem;

struct Class86F88Methods {
    u8 pad000[0x014];
    /* +0x014, func_800521D4's own first dispatch:
     * `self->methods->slot14(self, self->unk34)`. */
    void (*slot14)(Class86F88 *self, s32 arg1);
    u8 pad018[0x030 - 0x018];
    /* +0x030, func_800521D4's `state == 4` path:
     * `self->methods->slot30(self, self->unk2C)`. */
    void (*slot30)(Class86F88 *self, s32 arg1);
    u8 pad034[0x048 - 0x034];
    void (*slot48)(Class86F88 *self); /* +0x048, func_800521D4's own 2nd dispatch, self only */
    u8 pad04C[0x054 - 0x04C];
    /* +0x054 = func_800521D4 itself (this unit, matched). Dispatched by
     * func_8005227C as `self->methods->slot54(self, 4)`. */
    void (*slot54)(Class86F88 *self, s32 state);
    u8 pad058[0x060 - 0x058];
    /* +0x060, func_80052A58/func_8005281C's own trailing dispatch, both
     * conditional on a caller-supplied flag, both `(self, 0)`. */
    void (*slot60)(Class86F88 *self, s32 arg1);
    u8 pad064[0x080 - 0x064];
    /* +0x080 = func_80052498 itself (this unit, matched), whose own body
     * ignores every argument past `self` -- the 3-argument shape below is
     * what func_800523F0's call site (dispatching through a DIFFERENT
     * instance's slot80, `self->unk3C`) actually passes. Per-call-site
     * arity is this project's established convention; it does not
     * contradict func_80052498's own narrower body. */
    void (*slot80)(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3);
    u8 pad084[0x094 - 0x084];
    /* +0x094 = func_8005281C itself (this unit, MATCHED). Signature fixed
     * by three independent callers in this
     * unit (func_80052430, func_80052498, func_800524F8, func_80052598),
     * all of which pass exactly (self, arg1, arg2, arg3, arg4). */
    void (*slot94)(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
    /* +0x098, occupant not in this unit's queue. Called by
     * func_800524F8/func_80052598 with (self, arg1, arg2, arg3). */
    void (*slot98)(Class86F88 *self, s32 arg1, s32 arg2, s32 arg3);
};

/*
 * Each element of Class86F88::unk40[] -- resolved from func_800529FC's own
 * dispatch, `elem->methods->slotB8(elem, &D_8008AB10)`. The `+0x004`
 * "release" slot is the same shared base-class implementation seen at
 * that offset in every vtable this project has resolved so far (compare
 * `GenericReleaseMethods_3bb8c_d` above) -- func_8005278C dispatches
 * through it on each element before clearing the slot.
 */
struct Class86F88ElemMethods {
    u8 pad000[0x004];
    void (*release)(Class86F88Elem *self); /* +0x004 */
    u8 pad008[0x04C - 0x008];
    /* +0x04C, round 45's func_80052644 -- called once per freshly-created
     * element right after `New_Obj6EAC0` returns it, before the same
     * element's own `slotB8` call. `arg2` points at a 2-word stack-local
     * (`D_8008AB00`'s value, then a running `D_8008AB04`-seeded
     * accumulator incremented by 0xA per loop iteration) -- kept opaque
     * `void *` here since only that one call site gives it any shape;
     * see `Elem4CArg_3bb8c_k` in src/class_3bb8c_k.c for the concrete
     * local reading. */
    void (*slot4C)(Class86F88Elem *self, s32 arg1, void *arg2); /* +0x04C */
    u8 pad050[0x0B8 - 0x050];
    void (*slotB8)(Class86F88Elem *self, void *arg1); /* +0x0B8, func_800529FC */
    u8 pad0BC[0x0CC - 0x0BC];
    /* +0x0CC, func_8005281C: called once per active window element with a
     * freshly-formatted (func_8005292C) fixed-width text buffer. */
    void (*slotCC)(Class86F88Elem *self, char *arg1);
};

struct Class86F88Elem {
    Class86F88ElemMethods *methods; /* +0x000 */
};

struct Class86F88 {
    Class86F88Methods *methods;    /* +0x000 */
    u8 pad004[0x010 - 0x004];
    s32 unk10;                     /* +0x010, func_8005278C/func_8005281C: element count, clamped to a max of 4 */
    s32 unk14;                     /* +0x014, func_80052430: upper bound compared against unk24+0x1A */
    /* +0x018, func_8005292C: a table of BYTE OFFSETS (s32 each), added to
     * that function's own `base` (char *) argument to form a source
     * pointer -- `self->unk18[idx]` is never scaled by anything other than
     * its own natural s32 stride, and the resulting sum is used as a plain
     * byte address (strlen/strncpy-style calls), so `base` is a byte
     * pointer and this is an OFFSET table, not a pointer table. */
    s32 *unk18;                    /* +0x018, func_8005292C */
    u8 pad01C[0x020 - 0x01C];
    s32 unk20;                     /* +0x020, func_800523F0(fwd)/func_80052430/func_80052498/func_800524F8/func_80052598/func_800529FC */
    s32 unk24;                     /* +0x024, ditto */
    s32 unk28;                     /* +0x028, ditto; also func_80052B54's own return value */
    s32 unk2C;                     /* +0x02C, func_8005227C/func_800521D4 */
    s32 unk30;                     /* +0x030, func_8005227C/func_800521D4 */
    s32 unk34;                     /* +0x034, func_800521D4's slot14 argument */
    u8 pad038[0x03C - 0x038];
    Class86F88 *unk3C;              /* +0x03C, func_800523F0: another instance of this same class */
    Class86F88Elem *unk40[4];       /* +0x040, func_8005278C/func_8005281C/func_800529FC */
    s32 unk50;                      /* +0x050, enable flag guarding most of this class's dispatch */
};

extern Class86F88Methods D_80086F88;
/* func_80052A58's fixed 2nd argument to Class86F88ElemMethods::slotB8 on
 * its own FIRST dispatch (the element at the "old" index) -- immediately
 * adjacent rodata to D_8008AB10 below (4 bytes before it), never
 * dereferenced by this unit's own code, only its address taken. */
extern s32 D_8008AB0C;
/* func_800529FC's fixed 2nd argument to Class86F88ElemMethods::slotB8 --
 * a 4-byte rodata value (0x00008080), never dereferenced by this unit's
 * own code, only its address taken. Also func_80052A58's own SECOND
 * dispatch (the element at the "new" index, after the increment/decrement). */
extern s32 D_8008AB10;

/* func_800544D4: a plain class-vtable getter (`lui`/`addiu`, no
 * `lw`/`sw`), returns `&D_80087034` verbatim. Confirmed a BasicClass-
 * derived vtable with `tools/classtable.py 0x80087034` (header word then
 * `BasicClass__func_17eb0` at +4, the class-framework fingerprint) --
 * nothing in this unit dereferences it, so it stays untyped beyond the
 * address itself, same convention as `D_80086904` above. */
extern s32 D_80087034;
extern void *func_800544D4(void);

#endif
