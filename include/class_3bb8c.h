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
 * `$s4`/`$s3`, both `+= 0xC` per iteration), not one indexed base -- no
 * source shape tried this round (plain `arr1[i].field`, an explicit
 * `&arr1[i]` element pointer, a nested sub-struct accessed via
 * `arr1[i].sub.field`, or manual byte-cast pointer walking) reproduced
 * this; every attempt landed on ONE induction variable and was exactly 4
 * bytes short (a missing `addiu $s4,$s4,0xc`), or regressed further. See
 * the match report for the full attempt log before re-deriving this.
 */
typedef struct SetupEntry866E8 {
    void *ptr0;     /* +0x0 */
    s16 rate;       /* +0x4 */
    u8 pad6[0x8 - 0x6];
    s32 id;         /* +0x8 */
} SetupEntry866E8;

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

/* Uncarved sibling in this same unit (asm/class_3bb8c.s past this slice),
 * called once per element from func_8004B700's outer loop (this round) with
 * seven arguments: `self`, the stack-buffer slot being filled
 * (`&stackBuf[count]`, a `SetupEntry866E8*`), `self->unk68->divisor`, the
 * `val / divisor` quotient's low bit, `val` itself, the earlier
 * `func_8004B930(self, val, flag)` result, and `arg3[i].key`. Not this
 * round's function to match -- the call site establishes only its
 * ARGUMENT shape, not its body. */
extern void func_8004BA40(Obj866E8 *self, SetupEntry866E8 *arg1, s32 divisor, s32 flag, s32 val, s32 savedResult, s32 key);

/* Only the slots this unit's functions dispatch through (via
 * self->methods->slotNN) are typed; everything else stays opaque so the
 * struct keeps the right size/offsets without requiring every method to be
 * typed up front (same policy as include/class_39e08.h). */
typedef struct Obj866E8Methods {
    u8 pad000[0x88];
    /* Called by func_8004BD14 with a literal 7, one of the object's own
     * Elem array slots, and the loop index. */
    void (*slot88)(Obj866E8 *self, s32 arg1, Elem *entry, s32 arg3); /* +0x088 */
    u8 pad08C[0xC0 - 0x8C];
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
     * func_8004B700 (this round, MATCHED) at the end of its own loop with
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
     * (see `CC74QueryBuf`) and a literal 0; return value unused there. */
    s32 (*slot10C)(Obj866E8 *self, void *outBuf, s32 arg2); /* +0x10C */
    /* = func_8004C1C0 (this round, MATCHED). Resolves `in` (may be NULL at
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
} Obj866E8Methods;

/*
 * Opaque object pointed to by UnkCObj::unk14. `unk1C` established by this
 * round's func_8004C1C0 (plain s32, single-width read). `unk18`/`unk20`
 * were originally typed plain s32 from func_8004C470 (still INCLUDE_ASM,
 * so provisional); func_8004C1C0 (MATCHED) reads them BOTH as a full s32
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
    u8 pad004[0x02A - 0x004];
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
    u8 pad08[0x0C - 0x08];
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
    u8 pad14[0x18 - 0x14];
    s32 unk18;                      /* +0x018, func_8004C0AC: zeroed */
    u8 pad1C[0x20 - 0x1C];
    s32 unk20;                      /* +0x020, func_8004C0AC: zeroed */
    u8 pad24[0x38 - 0x24];
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
 * self+0x6C's pointee. func_8004B38C only ever STORES its own arg2 here
 * raw (never dereferences it); func_8004C158 dereferences it and reads
 * +0x014, itself a pointer used only for address-of-plus-offset
 * arithmetic (`+0x018`), never further dereferenced.
 */
struct Unk6CObj {
    u8 pad00[0x14];
    void *unk14;                    /* +0x014, func_8004C158 */
};

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
    u8 pad10[0x54 - 0x10];
    Unk54Struct unk54;             /* +0x054, func_8004B418 (address taken, forwarded opaquely) */
    u8 pad60[0x68 - 0x60];
    Unk68Struct *unk68;            /* +0x068, func_8004B418/func_8004B38C/func_8004B930/func_8004C470 */
    Unk6CObj *unk6C;               /* +0x06C, func_8004B38C stores it raw; func_8004C158 dereferences it */
    s32 unk70;                     /* +0x070, func_8004B570/func_8004B57C */
    u8 pad74[0x78 - 0x74];
    s16 unk78;                     /* +0x078, func_8004C620 (halfword, doubled into an index) */
    s16 unk7A;                     /* +0x07A, func_8004C620 (halfword, passed on as an arg) */
    u8 pad7C[0x88 - 0x7C];
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
} BaseCtorTable_3bb8c_c;

extern BaseCtorTable_3bb8c_c *func_8003F24C(void);

extern void *func_80017B34(s32 size);

typedef struct Class86AA0 Class86AA0;
typedef struct Class86AA0Methods Class86AA0Methods;

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
    u8 pad00C[0x0B8 - 0x00C];
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

/* MEASURED, round 9: func_8001E57C TAKES NO ARGUMENTS. Its whole body is
 * `lui/addiu %hi/%lo(D_8006B5CC); jr $ra` (asm/code_d294.s) -- it reads
 * neither $a0 nor $a1, and just returns &D_8006B5CC. It is the plain
 * no-parameter vtable getter documented in docs/research/class-framework.md,
 * the same shape as func_800269E0.
 *
 * The arg list below is therefore NOT the callee's signature; it is what THIS
 * call site passes, and it is what this unit's bytes need. src/class_3ac78.c
 * passes TWO args to the same symbol and is equally byte-exact. Both are
 * right about their own codegen and both are wrong about the function.
 * Do not "reconcile" them and do not reduce either to (void) -- that changes
 * the argument setup the caller emits and breaks the match. See
 * docs/match-reports/func_8004D3DC.md. */
extern BaseCtorTable_3bb8c_c *func_8001E57C(void *self);

/*
 * Generic class-instance shape used only to read another object's own
 * vtable header-tag BYTE (the low byte of the header word at the vtable's
 * own +0x000) -- func_8004D434's own second argument is dispatched this
 * way, compared against a literal 0x34.
 */
typedef struct GenericTagMethods_3bb8c_c {
    u8 tag;                                     /* +0x000, low byte of the header word */
} GenericTagMethods_3bb8c_c;

typedef struct GenericTagInst_3bb8c_c {
    GenericTagMethods_3bb8c_c *methods;         /* +0x000 */
} GenericTagInst_3bb8c_c;

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

#endif
