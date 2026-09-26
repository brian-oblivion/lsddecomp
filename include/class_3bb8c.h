#ifndef CLASS_3BB8C_H
#define CLASS_3BB8C_H

#include "common.h"
#include "BasicClass.h"
#include "TaskCore.h"

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
typedef struct EntryChildObj EntryChildObj;
typedef struct Elem Elem;
typedef struct UnkCObj UnkCObj;
typedef struct Unk14Obj Unk14Obj;
typedef struct Unk1BCObj Unk1BCObj;
typedef struct Unk6CObj Unk6CObj;
typedef struct Unk6C14Obj Unk6C14Obj;
typedef struct Unk6C14SubObj Unk6C14SubObj;
typedef struct QueryTemplate866E8 QueryTemplate866E8;
typedef struct EntryGpu EntryGpu;

/* self+0x54: an inline (not pointer) 3-word sub-struct, dereferenced by
 * ComputeCellWorldOffsets (arg3) and also matches Class866E8__FindElementForPosition's `arg1` descriptor
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
 *  - Class866E8__ComputeDivisorSplit reads +0x000 (s16 divisor) as a `div`/`%` operand.
 *  - Class866E8__ComputeRateFlags reads +0x000 (same divisor) and +0x002 (s16 count,
 *    used as a loop trip count) and +0x004 (s32, gates the whole
 *    function between two totally different code paths).
 *  - ComputeCellWorldOffsets's own `arg2` parameter is fed this exact pointer at
 *    its one call site (Class866E8__ComputeCellOffsets) and reads all three fields
 *    (+0x000 s16, +0x002 s16, +0x004 s32) the same way.
 *  - Class866E8__FindElementForPosition reads +0x004 alone (a boolean-ish gate).
 */
typedef struct Unk68Struct {
    s16 divisor;   /* +0x000 */
    s16 count;     /* +0x002 */
    s32 unk4;      /* +0x004 */
} Unk68Struct;

/*
 * A 10-byte "descriptor" struct, passed by pointer. Established from TWO
 * independent functions:
 *  - ComputeCellWorldOffsets's `arg4` (5th/stack argument) reads it as four signed
 *    bytes (+0x0..+0x3) followed by three signed halfwords (+0x4, +0x6,
 *    +0x8).
 *  - Class866E8__SetTargetAndBuildRates block-copies a whole one of these (its own `arg3`)
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
 * from functions that STALLED -- Class866E8__ComputeFootprintDescriptor (72/106), Class866E8__BuildRateEntries
 * (125/140), Class866E8__ApplyRateEntries (structurally 104/105). None of it is backed by a
 * byte-exact match. The offsets and access widths are OBSERVED from the
 * disassembly and are reliable; the TYPES and the names are inferred, and the
 * grouping into structs is a hypothesis.
 *
 * (These comments originally said "MATCHED" for Class866E8__ComputeFootprintDescriptor and
 * Class866E8__BuildRateEntries. That was wrong -- the runner matched nothing in this unit
 * this round -- and it mattered, because it presented inferred structure as
 * byte-verified and would have discouraged the next reader from questioning
 * it. Relabelled by the head at merge.)
 *
 * Output struct of Class866E8__ComputeFootprintDescriptor (Obj866E8Methods::slot110). A `Descriptor10`
 * embedded at +0x000 (natural alignment 2, so the next member falls at the
 * next 4-byte boundary, +0x00C -- matches exactly) followed by 8 more s32-
 * sized fields the function fills from a resolved Elem/Unk14Obj pair.
 * `base.b0`/`base.b1` are filled by the callee `Class866E8__ComputeDivisorSplit` (mod/div of
 * the raw rate); `base.b2`/`base.b3`/`h4`/`h6`/`h8` are computed in
 * Class866E8__ComputeFootprintDescriptor itself. Field meaning beyond that is unestablished -- named
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
    Elem *unk24;         /* +0x024, Class866E8__ComputeFootprintDescriptor: the Elem it resolved via slot11C */
    s32 unk28;           /* +0x028, Class866E8__ComputeFootprintDescriptor: the raw Class81940::ownerRate, sign-extended */
} Descriptor10Ext;

/*
 * "in" struct of Class866E8__ComputeFootprintDescriptor (Obj866E8Methods::slot110's 3rd param).
 * Three fields, each read BOTH as a full s32 (for a coarse cell-index
 * computation) and, separately and later, as just the low `u16` half (for a
 * fine sub-cell offset computation) -- the same memory, two widths, at
 * non-adjacent points in the function, which is why each is a union here
 * rather than a plain s32: writing it as a plain field and casting at the
 * use site would not force retail's observed re-load-at-the-narrower-width
 * behaviour. Provenance: Class866E8__GetTargetDescriptor (this unit) passes
 * `(u8 *)self->unk6C->unk14 + 0x18` as this pointer.
 */
typedef struct QueryPos866E8 {
    union { s32 w; u16 h; } unk0;   /* +0x000 */
    union { s32 w; u16 h; } unk4;   /* +0x004 */
    union { s32 w; u16 h; } unk8;   /* +0x008 */
} QueryPos866E8;

/*
 * Class866E8__ApplyRateEntries's 2nd parameter: a 0xC-byte-strided array, one entry per
 * loop iteration. Established from that function alone: `ptr0` is tested
 * for NULL to pick a branch and then, on the non-NULL branch, forwarded
 * VERBATIM (untouched) to the element's Class81940 +0x078 (loadHeader's name) -- consistent with a
 * pointer, though its pointee is never dereferenced in this unit; `rate`
 * is read as a plain halfword and copied into the resolved Elem's
 * `unk4->ownerRate` (an `s16` there); `id` is read as a full word and
 * passed as `Obj866E8Methods::slot118`'s index argument.
 *
 * NOTE for whoever revisits Class866E8__ApplyRateEntries: retail walks `ptr0` and the
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
 *
 * MATCHED round 73: the second walker is `sp = (SetupSub866E8 *)&arr1->rate`
 * assigned INSIDE the loop, with `arr1` itself advanced (no `ep` copy);
 * loop.c's strength reduction then produces the `+4` register. The note
 * above is kept as history.
 */
typedef struct SetupEntry866E8 {
    void *ptr0;     /* +0x0 */
    s16 rate;       /* +0x4 */
    u8 pad6[0x8 - 0x6];
    s32 id;         /* +0x8 */
} SetupEntry866E8;

/* A SECOND view of the SAME 0xC-byte stride, based at `+0x4` instead of
 * `+0x0`. Exists only as the target type of Class866E8__ApplyRateEntries's `rate`/`id`
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
 * called by Class866E8__ComputeCellOffsets and Class866E8__SetTargetAndBuildRates (both already matched) and
 * itself attempted-but-stalled this round (58/73, see
 * docs/match-reports/ComputeCellWorldOffsets.md) -- not a byte-exact match, so its
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
extern s32 ComputeCellWorldOffsets(s32 *arg0, s32 *outBuf, Unk68Struct *arg2, Unk54Struct *arg3, Descriptor10 *arg4);

/* Constant `Unk54Struct` (unk0=-1, unk4=0, unk8=0x140014) whole-struct-copied
 * by Class866E8__InitFootprintSlot into self+0x8C+key*0xC. */
extern Unk54Struct gDefaultElemRateOffset;

/*
 * Class866E8__BuildRateEntries's per-outer-loop-iteration key/enable pair, read from its
 * own `arg3` parameter (a 2-byte-strided array, one entry per element of
 * `self->arr`). `key` is copied into the resolved Elem's own `unk2` and
 * doubles as an index into `sRateOffsetTable` (`key * 0xC`, i.e. `sRateOffsetTable
 * + key`, since that table's own stride is 0xC == sizeof(Unk54Struct));
 * `flag` gates the whole per-element body (skip if 0).
 */
typedef struct TargetSpec866E8 {
    u8 key;   /* +0x0 */
    u8 flag;  /* +0x1 */
} TargetSpec866E8;

/* Retyped this round from `extern s32` (address-only placeholder) to a real
 * 7-entry `TargetSpec866E8` array: Class866E8__SetTargetAndBuildRates's only use
 * of `&sDefaultTargetSpecs` is as the `arg3` it forwards to its own slotF8
 * (Class866E8__BuildRateEntries), whose real, already-matched signature reads
 * exactly `arg3[i].key`/`arg3[i].flag` for `i < 7`. Every entry's `flag` byte
 * is nonzero in the data (`asm/data/76DC8.data.s`), i.e. this is the default
 * "enable every element" spec table. */
extern TargetSpec866E8 sDefaultTargetSpecs[7];

/* Data table, 0xC-byte stride, indexed by `TargetSpec866E8::key` in
 * Class866E8__BuildRateEntries -- reuses `Unk54Struct`'s shape (three consecutive `s32`
 * words) since that is exactly how Class866E8__BuildRateEntries reads it (offsets
 * 0x0/0x4/0x8, all as plain `s32`, no evidence of any other width). Bound
 * unknown from this unit alone (`key` is an arbitrary byte from the
 * caller), so left unsized. */
extern Unk54Struct sRateOffsetTable[];

/* Called once per element from Class866E8__BuildRateEntries's outer loop with seven
 * arguments: `self`, the stack-buffer slot being filled (`&stackBuf[count]`,
 * a `SetupEntry866E8*`), `self->unk68->divisor`, the `val / divisor`
 * quotient's low bit, `val` itself, the earlier
 * `Class866E8__ComputeRateFlags(self, val, flag)` result, and `arg3[i].key`. Matched this
 * round -- see docs/match-reports/Class866E8__ComputeRateEntry.md. */
extern s32 Class866E8__ComputeRateEntry(Obj866E8 *self, SetupEntry866E8 *arg1, s32 divisor, s32 flag, s32 val, s32 savedResult, s32 key);
/* NOTE: return type corrected this round from `void` to `s32` (0 or 1,
 * whether the mask test at the top passed) -- retail explicitly sets
 * $v0 to 0 or 1 on every path before returning, which a `void` function
 * would never do. Class866E8__BuildRateEntries, the only caller, discards it. */

/* `key`-indexed bitmask table, one bit per key (`1 << key`), tested against
 * `savedResult` (Class866E8__ComputeRateFlags's return, forwarded through Class866E8__BuildRateEntries) by
 * Class866E8__ComputeRateEntry. Bound is PROVEN, not guessed: the data file places exactly
 * 7 words here (0x8008688C-0800868A8) before sRateEntryTable starts, and
 * sRateEntryTable below is independently proven to hold exactly 7 `Unk54Struct`
 * entries (0x800868A8-0x800868FC) -- same key domain, consistent. */
extern const s32 sRateKeyMask[7];

/* `key`-indexed, reuses `Unk54Struct`'s 3-`s32` shape (same evidence as
 * `sRateOffsetTable` above: offsets 0x0/0x4/0x8, plain words). Read by
 * Class866E8__ComputeRateEntry as: `unk0` gates a `divisor * unk0` multiply (its low 32
 * bits used, `unk4` or `unk8` added depending on a caller-supplied `flag`);
 * when `unk0 == 0` the multiply is skipped entirely and `unk4` alone is
 * used. Sized at 7 (see sRateKeyMask's comment for why this one is provable
 * where `sRateOffsetTable` above is not). */
extern const Unk54Struct sRateEntryTable[7];

/* `Class81940::ownerKey`-indexed remap table, read by Class866E8__UpdateFootprintTracking as a
 * signed byte (`lb`). 8-entry bound is PROVEN, not guessed: the data file
 * (`asm/data/76DC8.data.s`) places exactly 8 bytes here (values
 * `01 02 03 00 04 05 06 00`) before `sDefaultTargetSpecs` starts. The fetched byte
 * (range 0..6) doubles as Class866E8__UpdateFootprintTracking's own return value and, scaled by
 * 4, as the index into `sFootprintResultPtrTable` below. */
extern const s8 sFootprintResultRemap[8];

/* 7-entry pointer table, first entry NULL, indexed by `sFootprintResultRemap`'s
 * fetched byte in Class866E8__UpdateFootprintTracking. Bound PROVEN by the data file: exactly 7
 * words at `sFootprintResultPtrTable` (one NULL, six pointers into the 4-word tables
 * `D_80086914`..`D_80086964`) before the next symbol starts. Element type
 * `s32 *` matches `Obj866E8Methods::slotF8`'s own `arg3` (already `s32 *`
 * from `Class866E8__SetTargetAndBuildRates`'s call site) -- Class866E8__UpdateFootprintTracking forwards a
 * `sFootprintResultPtrTable` entry there unchanged. */
extern s32 *sFootprintResultPtrTable[7];

/* Only the slots this unit's functions dispatch through (via
 * self->methods->slotNN) are typed; everything else stays opaque so the
 * struct keeps the right size/offsets without requiring every method to be
 * typed up front (same policy as include/class_39e08.h). */
typedef struct Obj866E8Methods {
    u8 pad000[0x30];
    /* = BasicClass::notifyParents (`BasicClass__NotifyParents`, classtable-
     * verified against `D_800866E8`'s own +0x030 entry) -- this class
     * inherits the base BasicClassMethods layout for its low slots (see
     * `include/code_8220.h`). Called by Class866E8__UpdateFootprintTracking as
     * `self->methods->slot30(self, 5)` when the just-copied descriptor's
     * leading raw halfword differs from what was there before. */
    void (*slot30)(Obj866E8 *self, s32 arg1);  /* +0x030 */
    u8 pad034[0x60 - 0x34];
    /* Called by round 45's TextEntry__SetCursorPos/TextEntry__SetCharAt as
     * `self->methods->slot60(self, 0)`, tail of the countdown/flush
     * "record + notify" path, when their own trailing flag argument is
     * set. Return unused. */
    void (*slot60)(Obj866E8 *self, s32 arg1);  /* +0x060 */
    u8 pad064[0x88 - 0x64];
    /* Called by Class866E8__OnNotifyTag1 with a literal 7, one of the object's own
     * Elem array slots, and the loop index. */
    void (*slot88)(Obj866E8 *self, s32 arg1, Elem *entry, s32 arg3); /* +0x088 */
    u8 pad08C[0xA4 - 0x8C];
    /* = the "flush all" finalizer, class_3bb8c_j round 15: called once by
     * TextEntry__ResetAllChars right after its countdown-flush loop, with (self,
     * self->unk18, literal 1). Return unused. */
    void (*slotA4)(Obj866E8 *self, s32 arg1, s32 arg2);   /* +0x0A4, TextEntry__ResetAllChars */
    /* = the countdown/flush callback, class_3bb8c_j round 15: called by
     * TextEntry__PrevChar (self, self->unk18, decremented countdown, literal 1),
     * TextEntry__ResetChar (self, self->unk18, literal 0, literal 1) and
     * TextEntry__ResetAllChars's own loop (self, loop index, self->unk1C, literal 0).
     * Argument MEANING differs per call site (index vs. self->unk18) but
     * all three pass exactly 3 args beyond self; return unused. */
    void (*slotA8)(Obj866E8 *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0A8, TextEntry__PrevChar/TextEntry__ResetChar/TextEntry__ResetAllChars */
    u8 pad0AC[0xC0 - 0xAC];
    /* Called by Class866E8__Disable right before it clears self->enabled. */
    void (*slotC0)(Obj866E8 *self);            /* +0x0C0 */
    u8 pad0C4[0xF8 - 0xC4];
    /* = Class866E8__BuildRateEntries. This IS Class866E8__BuildRateEntries's own identity slot
     * (verified via classtable, `D_800866E8`'s own +0x0F8 entry) -- its real
     * signature (self, s32 val, Unk54Struct *arg2, TargetSpec866E8 *arg3, void
     * return) is established by that function's own body, not by this call
     * site's placeholder types. Called by Class866E8__SetTargetAndBuildRates with
     * ComputeCellWorldOffsets's own return value, the SAME stack buffer that was
     * ComputeCellWorldOffsets's `outBuf` argument (as `Unk54Struct *`), and
     * `sDefaultTargetSpecs` (as `TargetSpec866E8 *`). Left `s32`/`s32 *`/`s32 *`
     * here rather than retyped to match: `Class866E8__UpdateFootprintTracking`'s OWN
     * call through this same slot passes `&buf.unkC` (`s32 *`) and a
     * `sFootprintResultPtrTable` entry (`s32 *`) instead, and retyping the field would need
     * that already-matched call site re-cast rather than left alone -- the
     * two call sites' real argument types genuinely differ, which is exactly
     * what a `void *`-shaped vtable slot type papers over on this project. */
    s32 (*slotF8)(Obj866E8 *self, s32 arg1, s32 *arg2, s32 *arg3);   /* +0x0F8 */
    /* = Class866E8__ApplyRateEntries. This IS Class866E8__ApplyRateEntries's own identity slot (verified
     * via classtable), not something Class866E8__ApplyRateEntries calls -- its actual
     * signature is `(self, arr1, count)`, matching a `SetupEntry866E8`
     * array and a count, per Class866E8__ApplyRateEntries's own stalled-but-structurally-
     * derived body (see docs/match-reports/Class866E8__ApplyRateEntries.md). Called by
     * Class866E8__BuildRateEntries (this round: STALLED at 125/140, register identity only)
     * at the end of its own loop with
     * a 7-slot stack buffer it filled and the number of slots actually
     * used. */
    void (*slotFC)(Obj866E8 *self, SetupEntry866E8 *arr1, s32 count); /* +0x0FC */
    u8 pad100[0x104 - 0x100];
    /* Called by Class866E8__OnNotifyTag1 with one of the object's own Elem array
     * slots. */
    void (*slot104)(Obj866E8 *self, Elem *entry);  /* +0x104 */
    /* = called by Class866E8__ApplyRateEntries (this round) with one of the object's own
     * Elem array slots, in BOTH branches of an `arr1[i].ptr0 != 0` test --
     * guarded by the SAME `entry->unk4->unk2C != 0` condition each time.
     * Return value unused. Distinct from slot104 above, which
     * Class866E8__OnNotifyTag1 dispatches under a different (mode==1) condition. */
    void (*slot108)(Obj866E8 *self, Elem *entry); /* +0x108 */
    /* Called by Class866E8__SetFootprintFromQuery with its own stack-local query buffer
     * (see `CC74QueryBuf`) and a literal 0; return value unused there.
     * Also called by Class866E8__ComputeFootprintFromRotation with the identical (own stack-local
     * CC74QueryBuf, 0) shape; return value unused there either. */
    s32 (*slot10C)(Obj866E8 *self, void *outBuf, s32 arg2); /* +0x10C */
    /* = Class866E8__ComputeFootprintDescriptor (this round: STALLED at 72/106). Resolves `in` (may be NULL at
     * other call sites; Class866E8__ComputeFootprintDescriptor itself never null-checks it) via
     * slot11C, then fills `out`. Returns 0 on success, 1 if the slot11C
     * lookup misses. Matches class_3ac78's independent view of the same
     * slot -- `self->methods->slot110(self, &buf, gateArg)` in that unit's
     * Class866E8__ApplyToSenderFootprint uses the identical (out-pointer, in-pointer) argument
     * order, which is what fixed these two params as pointers rather than
     * the previous round's placeholder (s32, void*). Class866E8__GetTargetDescriptor (this
     * unit) passes its own `arg1` param through as `out` and a computed
     * pointer as `in` -- see Class866E8__GetTargetDescriptor's retyped signature below. */
    s32 (*slot110)(Obj866E8 *self, Descriptor10Ext *out, QueryPos866E8 *in);   /* +0x110 */
    u8 pad114[0x118 - 0x114];
    /* Called by Class866E8__FindElementForPosition with a plain array index (0..6); the
     * returned pointer is subsequently read like an Elem slot accessor,
     * so this is almost certainly `return &self->arr[index];` -- not
     * this round's function to match. */
    Elem *(*slot118)(Obj866E8 *self, s32 index);   /* +0x118 */
    /* Called by Class866E8__ComputeFootprintDescriptor with its own `in` (QueryPos866E8*) param,
     * forwarded opaquely; return value dereferenced exactly like an Elem
     * (->unk4, ->unkC), so this is almost certainly an Elem-lookup sibling
     * to slot118 (Class866E8__FindElemByUnk32) -- not this round's function to match
     * (Class866E8__FindElementForPosition, still INCLUDE_ASM in this unit). */
    Elem *(*slot11C)(Obj866E8 *self, QueryPos866E8 *arg1);   /* +0x11C */
    /* Called twice by Class866E8__SplitFootprintSlot, each time with a small offset off
     * its own arg3; the return value is stored as a freshly-created
     * GridSlot866E8's `elemIdx`. */
    s32 (*slot120)(Obj866E8 *self, s32 arg1);      /* +0x120 */
    /* Called by Class866E8__InitFootprintSlot with its own arg3 (unmodified); the return
     * value is stored into the first word of a freshly-copied 3-word
     * slot at self+0x8C+key*0xC (see Class866E8__InitFootprintSlot). */
    s32 (*slot124)(Obj866E8 *self, s32 arg1);      /* +0x124 */
    /* = Class866E8__RefreshFootprint (class_3bb8c_b, already matched: `void
     * Class866E8__RefreshFootprint(Obj866E8 *self)`). Called by Class866E8__UpdateFootprintTracking as
     * `self->methods->slot128(self)`, return unused. Classtable-verified
     * (`D_800866E8`'s own +0x128 entry). */
    void (*slot128)(Obj866E8 *self);               /* +0x128 */
} Obj866E8Methods;

/*
 * Opaque object pointed to by UnkCObj::unk14. `unk1C` established by this
 * round's Class866E8__ComputeFootprintDescriptor (plain s32, single-width read). `unk18`/`unk20`
 * were originally typed plain s32 from Class866E8__FindElementForPosition (still INCLUDE_ASM,
 * so provisional); Class866E8__ComputeFootprintDescriptor (STALLED, 72/106) reads them BOTH as a full s32
 * (coarse) and, separately and later in the function, as just the low
 * `u16` half (fine) -- retail re-loads from memory at the narrower width
 * rather than deriving it from the already-loaded s32, so each is a union
 * here rather than a plain field (same reasoning as `QueryPos866E8`
 * above). Class866E8__FindElementForPosition's own s32-only reads remain valid against the
 * `.w` member, so this is additive, not a contradiction of what it
 * established.
 */
struct Unk14Obj {
    /* Class866E8__BuildRateEntries (round: charlie/4): cleared to 0 right after unk18/
     * unk1C/unk20 are filled -- a genuine RELOAD of the same Unk14Obj*
     * (not the same register kept live), so it is a real memory write, not
     * dead code. */
    s32 unk0;                         /* +0x000 */
    u8 pad4[0x18 - 0x4];
    union { s32 w; u16 h; } unk18;   /* +0x018 */
    s32 unk1C;                        /* +0x01C, Class866E8__ComputeFootprintDescriptor */
    union { s32 w; u16 h; } unk20;    /* +0x020 */
};

/*
 * Opaque object returned by Obj866E8Methods::slot118 -- read as +0x00C is
 * a pointer to a Unk14Obj. Deliberately a DIFFERENT top-level type from
 * `Elem` even though slot118's return value is later read exactly like an
 * Elem-array-slot accessor would suggest, because this round's evidence
 * (Class866E8__FindElementForPosition) only reaches the +0x00C field, never Elem's own +0x000
 * or +0x004 -- unifying them would be guessing past the evidence.
 */
struct UnkCObj {
    u8 pad00[0x14];
    Unk14Obj *unk14;    /* +0x014, Class866E8__FindElementForPosition */
};

/*
 * self+0xEC's array element, one of Obj866E8::arr[7]. Established from
 * Class866E8__CountFlaggedElements (+0x000), Class866E8__FindElemByUnk32 (+0x004), and this round's
 * Class866E8__ResetElementCells (+0x00C indirectly via unk4, +0x010) and Class866E8__OnNotifyTag1
 * (+0x000, +0x004). class_3bb8c_b's Class866E8__ForEachEntryChild independently reached
 * the same +0x010 pointer array, walked over the same 0x668 raw bytes,
 * and Class866E8__FindElemIndexByUnk30/Class866E8__FindElemIndexByUnk32 the same +0x004 target.
 */
struct Elem {
    u16 flag;                      /* +0x000 */
    /* Class866E8__BuildRateEntries (round: charlie/4): copied from `arg3[i].key` (a raw
     * byte, zero-extended then stored as a halfword) and, in a second
     * separate loop over all 7 elements, copied onward into
     * `unk4->unk32` (already established). */
    u16 unk2;                      /* +0x002 */
    struct Class81940 *unk4;        /* +0x004, New_Class81940 (Class866E8__Class866E8); include/Class81940.h */
    /* New_Class6D940(0) (Class866E8__Class866E8): the element's placement
     * grid; Class866E8__LoadElementResources points its buffer into the
     * element's resource and walks its +0x078 (Class6D940__ResolveEntry)
     * over every cell. include/Class6D940.h. */
    struct Class6D940 *unk8;         /* +0x008, Class866E8__LoadElementResources */
    UnkCObj *unkC;                  /* +0x00C, Class866E8__FindElementForPosition (via slot118's return) */
    /* Class866E8__ResetElementCells: array of pointers, walked over 0x668 raw bytes --
     * the true element count is not a round number of elements, so this
     * stays byte-offset arithmetic rather than a sized array. */
    EntryChildObj **unk10;           /* +0x010 */
    u8 pad14[0x1C - 0x14];
};

/*
 * Target of Elem::unk10[i]. TWO runners derived this object independently
 * in round 8 from opposite ends and the head merged them here; the split
 * views are why the offsets below have unrelated provenance:
 *  - class_3bb8c (Class866E8__ResetElementCells) reached the DATA: it ORs a flag bit into
 *    +0x010 and zeroes +0x018 and +0x020.
 *  - class_3bb8c_b (Class866E8__ApplyRateToChild/Class866E8__ResetChildRate, via Class866E8__ForEachEntryChild)
 *    reached the METHOD TABLE at +0x000 and the one slot those two
 *    dispatch through.
 * They agree on the object's identity (both reach it as Elem::unk10[i]),
 * so this is one type, not two independent views -- the
 * independent-views convention applies ACROSS unit headers, and these two
 * units share this one.
 */

typedef struct EntryChildObjMethods {
    u8 pad00[0x48];
    /* Called by Class866E8__ApplyRateToChild (arg2 = the parent's own rateEntry) and
     * Class866E8__ResetChildRate (arg2 = &D_800869CC). Return value unused by both. */
    void (*slot48)(EntryChildObj *self, s32 arg1, void *arg2); /* +0x048 */
} EntryChildObjMethods;

struct EntryChildObj {
    EntryChildObjMethods *methods;  /* +0x000, Class866E8__ApplyRateToChild/Class866E8__ResetChildRate */
    u8 pad04[0x10 - 0x04];
    u32 unk10;                      /* +0x010, Class866E8__ResetElementCells: OR'd with 0x80000000; Class866E8__SetFootprintCellFlag: bit31 set/cleared per its own arg1 */
    /* Class866E8__LoadElementResources: a GsCOORDINATE2-shaped per-entry GPU link record --
     * full body (`EntryGpu`) kept in class_3bb8c.c. Note this sits right
     * where a `GsDOBJ2` embedded at THIS object's own +0x010 would put its
     * `coord2` field (`GsDOBJ2::attribute` at +0x000 lines up with this
     * object's own +0x010 `unk10`, used as GsLinkObject4's `objp`) --
     * consistent, not asserted as the real PSYQ type here. */
    EntryGpu *unk14;                 /* +0x014, Class866E8__LoadElementResources */
    s32 unk18;                      /* +0x018, Class866E8__ResetElementCells: zeroed */
    u8 pad1C[0x20 - 0x1C];
    s32 unk20;                      /* +0x020, Class866E8__ResetElementCells: zeroed */
    u8 pad24[0x36 - 0x24];
    s16 unk36;                       /* +0x036, Class866E8__LoadElementResources */
    EntryChildObj *unk38;           /* +0x038, Class866E8__SetFootprintCellFlag: singly-linked chain, walked while non-NULL */
};

/*
 * Opaque target of self->unk1BC. Only field established: Class866E8__GetLastTargetRateSplit
 * reads +0x004, an element's Class81940 (its +0x030 ownerRate is read
 * straight afterward).
 */
struct Unk1BCObj {
    u8 pad0[0x4];
    struct Class81940 *unk4;        /* +0x004, Class866E8__GetLastTargetRateSplit */
};

/*
 * self+0x6C->unk14's pointee (round 13, Class866E8__ComputeFootprintFromRotation). Only field
 * established: +0x044, a further pointer (Unk6C14SubObj*).
 */
struct Unk6C14Obj {
    u8 pad00[0x44];
    Unk6C14SubObj *unk44;           /* +0x044, Class866E8__ComputeFootprintFromRotation */
};

/*
 * Unk6C14Obj::unk44's pointee. `+0x010` is only ever address-taken
 * (forwarded raw to RotMatrix, never dereferenced in this unit);
 * `+0x012` is read BOTH as a signed halfword (to test its sign, adding
 * 0x1000 to the unsigned reading when negative -- a 12-bit two's
 * complement unpack) and, separately, as the resulting unsigned value
 * used for the range dispatch that follows. Class866E8__GetTargetDescriptor's own
 * `(u8 *)self->unk6C->unk14 + 0x18` -- one level UP the chain, on
 * Unk6C14Obj's OWN address, not this sub-object -- stays a raw cast in
 * that unit, untouched here.
 */
struct Unk6C14SubObj {
    u8 pad00[0x12];
    u16 unk12;                      /* +0x012, Class866E8__ComputeFootprintFromRotation */
};

/*
 * self+0x6C's pointee. Class866E8__SetTargetAndBuildRates only ever STORES its own arg2 here
 * raw (never dereferences it); Class866E8__GetTargetDescriptor dereferences it and reads
 * +0x014, itself a pointer used only for address-of-plus-offset
 * arithmetic (`+0x018`), never further dereferenced. Retyped this round
 * (round 13, Class866E8__ComputeFootprintFromRotation) from `void *` to `Unk6C14Obj *` -- the ONLY
 * other reader, Class866E8__GetTargetDescriptor, already casts it straight to `(u8 *)`
 * before doing arithmetic, so the retype does not change that already-
 * matched function's bytes (verified: full rebuild stays green).
 */
struct Unk6CObj {
    u8 pad00[0x14];
    Unk6C14Obj *unk14;              /* +0x014, Class866E8__GetTargetDescriptor/Class866E8__ComputeFootprintFromRotation */
};

/*
 * Template struct copied wholesale by Class866E8__ComputeFootprintFromRotation from the constant
 * global `D_8008E98C` into a stack-local descriptor, then partially
 * overwritten (`unk14`/`unk18` zeroed, `unk1C` set from `self->unk74`)
 * before being handed to two uncarved library helpers
 * (`RotMatrix`/`ApplyMatrixLV`) as an in/out parameter block. Field
 * meaning beyond "8 words, offsets 0x00-0x1C" is unestablished; the first
 * five words are read/written only as the opaque whole-struct copy.
 */
struct QueryTemplate866E8 {
    s32 unk0[5];                    /* +0x000..+0x010, opaque (untouched by Class866E8__ComputeFootprintFromRotation) */
    s32 unk14;                      /* +0x014, Class866E8__ComputeFootprintFromRotation: zeroed before the call, then an in/out arg to ApplyMatrixLV */
    s32 unk18;                      /* +0x018, Class866E8__ComputeFootprintFromRotation: zeroed before the call */
    s32 unk1C;                      /* +0x01C, Class866E8__ComputeFootprintFromRotation: set to self->unk74 before the call */
};

extern QueryTemplate866E8 D_8008E98C;

/* Uncarved library helpers (round 13, Class866E8__ComputeFootprintFromRotation's only known call
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
 * Opaque target of Obj866E8::bounds (Class866E8__SetBounds stores it raw;
 * IsPointOutOfBounds -- a plain, non-virtual helper, NOT a vtable slot, see
 * tools/classtable.py D_800866E8 -- dereferences it as a min/max bounding
 * box against an [x,y] byte pair). Field meaning inferred from the four
 * comparisons in IsPointOutOfBounds: `unk0`/`unk2` gate the LOW side, `unk4`/
 * `unk8` the HIGH side, of the point's two axes respectively.
 */
typedef struct Bounds866E8_3bb8c_b {
    s16 minX;                      /* +0x000, IsPointOutOfBounds: point[0] < this -> out of range */
    s16 minY;                      /* +0x002, IsPointOutOfBounds: point[1] < this -> out of range */
    s32 maxX;                      /* +0x004, IsPointOutOfBounds: this < point[0] -> out of range */
    s32 maxY;                      /* +0x008, IsPointOutOfBounds: this < point[1] -> out of range (also the function's own return value) */
} Bounds866E8_3bb8c_b;

/*
 * self->rateEntry's pointee: one of four static 0xC-byte table entries at
 * D_8008699C/D_800869A8/D_800869B4/D_800869C0 (addresses confirmed 0xC
 * apart), selected by Class866E8__ConfigureRateEntry from (rate > 0, flag != 0) and never
 * dereferenced past +0x006. The same 0xC stride lines up with
 * D_800869CC (declared `extern s32 D_800869CC[3]` below, by
 * Class866E8__ResetChildRate) as a plausible fifth entry of the same table, but
 * nothing in this unit reaches that entry through THIS pointer type, so
 * the two stay independently declared rather than unified into one
 * array of unproven length.
 */
typedef struct EntryDesc866E8 {
    u8 pad0[0x6];
    s16 scale;          /* +0x006, Class866E8__ConfigureRateEntry: multiplied against abs(rate) */
    u8 pad8[0xC - 0x8];
} EntryDesc866E8;

extern EntryDesc866E8 D_8008699C;
extern EntryDesc866E8 D_800869A8;
extern EntryDesc866E8 D_800869B4;
extern EntryDesc866E8 D_800869C0;

/*
 * self+0x8C's array element (`Obj866E8::gridSlots`, see below). Established
 * from Class866E8__SetFootprintCellFlag, which reads all five fields: `elemIdx` selects
 * `self->arr[elemIdx]`; `col`/`row` locate a starting cell in that element's
 * `unk10` pointer grid (row stride 20 cells, confirmed by the `* 20`
 * offset math); `width`/`height` are the sub-rectangle's width/height walked
 * from that starting cell. Class866E8__InitFootprintSlot (already matched, a different
 * unit's round) writes a whole one of these via a 3-word block copy using
 * the coarser, already-committed `Unk54Struct` view of the SAME memory --
 * per this project's independent-views convention, that write-side view
 * is left alone; this is a separate, more granular READ-side view of the
 * same 0xC bytes, justified because a whole-struct copy does not care
 * about the internal layout it is copying.
 */
typedef struct GridSlot866E8 {
    s32 elemIdx;   /* +0x0, Class866E8__SetFootprintCellFlag: selects self->arr[elemIdx] */
    s16 col;        /* +0x4, Class866E8__SetFootprintCellFlag: starting column */
    s16 row;        /* +0x6, Class866E8__SetFootprintCellFlag: starting row (row stride 20) */
    s16 width;        /* +0x8, Class866E8__SetFootprintCellFlag: sub-rectangle width */
    s16 height;        /* +0xA, Class866E8__SetFootprintCellFlag: sub-rectangle height */
} GridSlot866E8;

/*
 * Class866E8__SetFootprintFromQuery's own stack-local query buffer, filled by a call through
 * `Obj866E8Methods::slot10C` and read back at two offsets: `+0x2` (a
 * signed [x,y] byte pair, forwarded to IsPointOutOfBounds as its `point`
 * argument) and `+0x28` (a plain `s32`, read directly by Class866E8__SetFootprintFromQuery
 * itself). Everything else is unproven -- this is a local, not part of
 * `Obj866E8`, so it stays a minimal opaque type sized only to cover the
 * two known offsets.
 */
typedef struct CC74QueryBuf {
    u8 pad0[0x2];
    s8 point[2];        /* +0x2, Class866E8__SetFootprintFromQuery: forwarded to IsPointOutOfBounds */
    u8 pad4[0x28 - 0x4];
    s32 count;          /* +0x28, Class866E8__SetFootprintFromQuery */
} CC74QueryBuf;

struct Obj866E8 {
    Obj866E8Methods *methods;      /* +0x000 */
    u8 pad04[0x0C - 0x04];
    s32 unkC;                      /* +0x00C, CheckObj866E8CountFlag (compared against 9999999) */
    /*
     * +0x010..+0x048: a "countdown/flush" subsystem, established by
     * class_3bb8c_j (round 15) from four functions (TextEntry__PrevChar,
     * TextEntry__ToggleAltCommands, TextEntry__ResetChar, TextEntry__ResetAllChars) plus two gp_rel-blocked
     * siblings in the same unit (TextEntry__SetCursorPos, TextEntry__SetCharAt, both confirmed
     * to touch the SAME unk48/unk18/unk1C fields -- see those functions'
     * stub reports). unk10 is a trip count read once per "flush all" call
     * (TextEntry__ResetAllChars); unk14 is the countdown's own reset value; unk18 is
     * forwarded as methods->slotA8/slotA4's own arg1; unk1C is the live
     * countdown, decremented per call and reset from unk14 on expiry; unk20
     * is an unrelated boolean toggled independently by TextEntry__ToggleAltCommands; unk48
     * is a readiness/enable gate every function in the group tests non-zero
     * before doing anything, never itself dereferenced in this unit.
     */
    s32 unk10;                     /* +0x010, TextEntry__ResetAllChars */
    s32 unk14;                     /* +0x014, TextEntry__PrevChar */
    s32 unk18;                     /* +0x018, TextEntry__PrevChar/TextEntry__ResetChar/TextEntry__ResetAllChars */
    s32 unk1C;                     /* +0x01C, TextEntry__PrevChar/TextEntry__ResetChar/TextEntry__ResetAllChars */
    s32 unk20;                     /* +0x020, TextEntry__ToggleAltCommands (xor-toggled) */
    u8 pad24[0x28 - 0x24];
    u8 *unk28;                     /* +0x028, round 45's TextEntry__SetCharAt: a per-index byte buffer, `unk28[arg1] = table[idx]` */
    u8 pad2C[0x40 - 0x2C];
    /* +0x040/+0x044, round 45's TextEntry__SetCursorPos/TextEntry__SetCharAt: opaque
     * resource-handle objects, dispatched only through this unit's own
     * local method-table views (Unk40Obj866E8Methods/Unk44Obj866E8Methods
     * in class_3bb8c_j.c) -- kept `void *` here since nothing outside that
     * unit touches them yet. */
    void *unk40;                   /* +0x040 */
    void *unk44;                   /* +0x044 */
    s32 unk48;                     /* +0x048, TextEntry__PrevChar/TextEntry__ToggleAltCommands/TextEntry__ResetChar/TextEntry__ResetAllChars */
    u8 pad4C[0x54 - 0x4C];
    Unk54Struct unk54;             /* +0x054, Class866E8__ComputeCellOffsets (address taken, forwarded opaquely) */
    /* +0x060/+0x064, Class866E8__ComputeRateEntry (this round): a callback invoked as
     * `unk60(unk64, value, 0, 0)`, whose result is stored into the
     * SetupEntry866E8 slot being filled. `unk64` is never dereferenced in
     * this unit, only forwarded -- opaque context pointer. Was undifferentiated
     * padding (`pad60[0x68 - 0x60]`) before this round; the split is
     * additive (same total size, same offsets), not a removal. */
    void *(*unk60)(void *arg0, s32 arg1, s32 arg2, s32 arg3); /* +0x060 */
    void *unk64;                                              /* +0x064 */
    Unk68Struct *unk68;            /* +0x068, Class866E8__ComputeCellOffsets/Class866E8__SetTargetAndBuildRates/Class866E8__ComputeRateFlags/Class866E8__FindElementForPosition */
    Unk6CObj *target;              /* +0x06C, Class866E8__SetTargetAndBuildRates stores it raw; Class866E8__GetTargetDescriptor dereferences it; round 78 head: renamed from unk6C by type scope (bravo proposed posSource; `target` matches the two functions that set and read it) */
    s32 enabled;                   /* +0x070, Class866E8__Enable/Class866E8__Disable. TIER A: class_3ac78's
                                     * independent local view of this SAME field already reached "enabled"
                                     * from the identical evidence (its own Class866E8__UpdateIfEnabled report,
                                     * round 67) -- set 1 by Class866E8__Enable, cleared 0 by Class866E8__Disable
                                     * after a teardown dispatch, matching a plain enable/disable pair. */
    s32 gridSpan;                  /* +0x074, world span of the grid; gDefaultGridSpan = 0xA000. Class866E8__ComputeFootprintFromRotation copies it into its stack-local QueryTemplate866E8's unk1C before calling RotMatrix */
    s16 gridHalfCells;             /* +0x078, gridSpan >> 12 = 10; Class866E8__RefreshFootprint doubles it into an index (giving gridCells) */
    s16 gridCells;                 /* +0x07A, gridSpan >> 11 = 20, the row stride byte-matched Class866E8__SetFootprintCellFlag uses; Class866E8__RefreshFootprint passes it on as an arg */
    s16 footprintCol;                     /* +0x07C, Class866E8__BuildFootprintSlots: a signed sub-cell horizontal offset, clamped into [0,0x14) and combined with footprintWidth to decide whether the grid footprint spans one or two 20-unit cells; also written directly by Class866E8__ComputeFootprintFromRotation */
    s16 footprintRow;                     /* +0x07E, Class866E8__BuildFootprintSlots: same convention as footprintCol, vertical; also written directly by Class866E8__ComputeFootprintFromRotation */
    s32 footprintWidth;                     /* +0x080, Class866E8__ComputeFootprintFromRotation (writes arg1 or arg2 depending on its own dispatch), then read/forwarded by Class866E8__BuildFootprintSlots to Class866E8__SplitFootprintSlot's p7 */
    s32 footprintHeight;                     /* +0x084, Class866E8__ComputeFootprintFromRotation (writes the other of arg1/arg2), then read/forwarded by Class866E8__BuildFootprintSlots to Class866E8__SplitFootprintSlot's p8 */
    s32 gridSlotCount;                     /* +0x088, Class866E8__SetFootprintCellFlag: loop count over gridSlots[] (bounded by gridSlots's own 4-element capacity) */
    GridSlot866E8 gridSlots[4];      /* +0x08C, Class866E8__SetFootprintCellFlag (reads); Class866E8__InitFootprintSlot (writes, via the coarser Unk54Struct view) -- exactly fills the gap up to the existing unkBC field, so this is a hard capacity, not a guess */
    Descriptor10 unkBC;            /* +0x0BC, Class866E8__SetTargetAndBuildRates: whole-struct copy from its arg3 */
    u8 padC6[0xEC - 0xC6];
    Elem arr[7];                   /* +0x0EC, Class866E8__CountFlaggedElements/Class866E8__FindElemByUnk32/Class866E8__FindElemIndexByUnk32/Class866E8__FindElemIndexByUnk30/Class866E8__OnNotifyTag1/Class866E8__ForEachEntryChild */
    s32 unk1B0;                    /* +0x1B0, Class866E8__OnNotifyTag1 */
    u16 unk1B4;                    /* +0x1B4, Class866E8__OnNotifyTag1 */
    u8 pad1B6[0x1B8 - 0x1B6];
    s32 unk1B8;                    /* +0x1B8, Class866E8__OnNotifyTag1 */
    Unk1BCObj *unk1BC;             /* +0x1BC, Class866E8__GetLastTargetRateSplit */
    u8 pad1C0[0x1CC - 0x1C0];
    s32 unk1CC;                    /* +0x1CC, Class866E8__GetUnk1CC (address-of only, real type unknown) */
    u8 pad1D0[0x1DC - 0x1D0];
    Bounds866E8_3bb8c_b *bounds;   /* +0x1DC, Class866E8__SetBounds (stores raw)/IsPointOutOfBounds (dereferences) */
    s32 rateCountdown;                    /* +0x1E0, Class866E8__AdvanceRateCountdown/Class866E8__FlushRateLatch: a countdown gate */
    EntryDesc866E8 *rateEntry;        /* +0x1E4, Class866E8__ApplyRateToChild (forwarded opaquely)/Class866E8__ConfigureRateEntry (selects one of four statics and reads +0x6) */
    u8 pad1E8[0x2F4 - 0x1E8];
    s32 unk2F4;                    /* +0x2F4, CheckObj866E8CountFlag: zero-checked when unkC > 9999999 */
};

/* Get-vtable helper, same shape and same real function as
 * class_3ac78.h's `GetClass866E8Methods` (independent view: this unit names the
 * return type Obj866E8Methods, not Class866E8Methods). It now has a real
 * body in this unit (class_3bb8c_b); class_3ac78 still calls it via `jal`
 * as a raw external. */
extern Obj866E8Methods D_800866E8;

/* Still raw asm in this unit (not this round's target): walks
 * item->unk10[] (an array of EntryChildObj*, up to +0x668 bytes
 * from the base read at item->unk10), calling callback(self, element) for
 * each. Derived to resolve Class866E8__ApplyRateToChild/Class866E8__ResetChildRate's true call site --
 * see those functions' reports. Not called by name anywhere in this
 * unit's own C (only from within Class866E8__ForEachElem's still-raw body), so this
 * prototype is documentation, not load-bearing. */
extern void Class866E8__ForEachEntryChild(Obj866E8 *self, void (*callback)(Obj866E8 *self, EntryChildObj *item), Elem *item);

/* Still raw asm in this unit (not this round's target): iterates
 * self->arr, invoking an optional per-element callback (arg2, called
 * (self, &arr[i]) when non-NULL) and then always forwarding (self, arg1,
 * &arr[i]) to Class866E8__ForEachEntryChild. Class866E8__AdvanceRateCountdown/Class866E8__FlushRateLatch both call it
 * with arg2 = NULL (no per-element callback), passing a function POINTER
 * as arg1 instead -- that pointer is consumed further down in
 * Class866E8__ForEachEntryChild, not by this function itself. */
extern void Class866E8__ForEachElem(Obj866E8 *self, void (*arg1)(Obj866E8 *self, EntryChildObj *item), void (*arg2)(Obj866E8 *self, Elem *item));

/* 3-word (12-byte) data block, address-of only -- passed to
 * EntryChildObjMethods::slot48 as an opaque arg2 by Class866E8__ResetChildRate.
 * Never dereferenced in this unit, so left untyped in size only. */
extern s32 D_800869CC[3];

/* -------------------------------------------------------------------
 * class_3bb8c_c additions below. Small sibling classes, each built by
 * its own New_X/ctor pair (allocator + base-chain + own-vtable-set, the
 * same shape as Class86668__Class86668 in class_39e08.c). Named by their
 * vtable's address, same convention as Class866E8/Class86668. The first
 * of them, Class869D8 (gClass869D8Methods, a Viewport), is defined in
 * include/Class869D8.h (round 87, track 4).
 * ------------------------------------------------------------------- */

extern void *BMemPMgrAlloc(s32 size);

/* Class86AA0 (gClass86AA0Methods, a Class6B5CC) is defined in
 * include/Class86AA0.h (round 88, track 4). */

/* GetClass6B5CCMethods and its table: include/Class6B5CC.h (track 4, round
 * 81). The local BaseCtorTableB_3bb8c_c view and the unprototyped getter that
 * lived here are gone; round 59 measured both arguments class_3bb8c_c.c passed
 * to the no-argument getter as zero-cost (docs/match-reports/Class86AA0__Class86AA0.md). */


/* Class86B60 (gClass86B60Methods, a TaskCore) is defined in
 * include/Class86B60.h (round 88, track 4). Two views of OTHER classes
 * its methods call stay here, named for the Class86B60 field that holds
 * them: */

/* The class of IntermediateBaseInitArgs::unk0 (a BasicClass * there), as
 * Class86B60__OnDeinit calls its +0x078; TaskCore__OnDeinit makes the same
 * call through code_2c054.h's TaskTextObj. */
typedef struct Class86B60UnkC0ObjMethods_3bb8c_d Class86B60UnkC0ObjMethods_3bb8c_d;
typedef struct Class86B60UnkC0Obj_3bb8c_d Class86B60UnkC0Obj_3bb8c_d;

struct Class86B60UnkC0ObjMethods_3bb8c_d {
    u8 pad000[0x078];
    /* +0x078, Class86B60__OnDeinit's own call: `(childObj, &self->unk93,
     * tableEntry)`, where `tableEntry` walks a fixed external table
     * (`D_80086DAC`, stride 0xC) starting fresh each call to this
     * function. */
    void (*slot78)(Class86B60UnkC0Obj_3bb8c_d *self, void *arg1, void *arg2);
};

struct Class86B60UnkC0Obj_3bb8c_d {
    Class86B60UnkC0ObjMethods_3bb8c_d *methods; /* +0x000 */
};

/* Class86B60::saveCtrl's class: New_TaskObjF's object (gTaskObjFMethods,
 * class id 0xB), as Class86B60's methods call it. A view of TaskObjF, not
 * of Class86B60; include/Class86B60.h refers to it by tag. */
typedef struct Class86B60UnkACObjMethods_3bb8c_d Class86B60UnkACObjMethods_3bb8c_d;
typedef struct Class86B60UnkACObj_3bb8c_d Class86B60UnkACObj_3bb8c_d;

struct Class86B60UnkACObjMethods_3bb8c_d {
    u8 pad000[0x004];
    void (*release)(Class86B60UnkACObj_3bb8c_d *self); /* +0x004, Class86B60__Finalize */
    u8 pad008[0x06C - 0x008];
    /* +0x06C, Class86B60__BeginMemcardSave's own call: `(self, D_8008A9D0, &D_80086D6C,
     * self->initArgs->unk4, self->unk10, self->unk14, self->sound)` -- 7
     * arguments, the last three on the stack. Every pointer beyond `self`
     * is forwarded opaquely (never dereferenced by this slot's own
     * caller), so all stay `void *`/`s32 *` placeholders. */
    void (*slot6C)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, s32 *arg2,
                   void *arg3, void *arg4, void *arg5, void *arg6); /* +0x06C */
    void (*slot70)(Class86B60UnkACObj_3bb8c_d *self); /* +0x070, Class86B60__EndMemcardSave */
    /* +0x074, Class86B60__UpdateMemcardSaveStatus's own 2nd call: `(self, D_8008AA10, D_8008AA18,
     * self->saveBlock, self->saveBlockSize)` -- the two middle arguments are the
     * VALUES of two `.sdata` globals loaded via `%gp_rel` (not their
     * addresses), each holding a pointer into the still-uncarved rodata
     * block at `D_80011434` (verified in `asm/data/1C34.rodata.s`: no
     * dlabel exists at either byte offset, so they cannot be referenced by
     * name and are forwarded as opaque `void *`). */
    void (*slot74)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, void *arg2, s32 arg3, s32 arg4); /* +0x074 */
    /* +0x078, Class86B60__UpdateMemcardSaveWithIcon's own call: `(self, D_8008AA10, D_8008AA18,
     * 0xD, 3, self->iconHandle, self->saveBlock, self->saveBlockSize)` --
     * eight arguments, the last four on the stack. `arg5` is
     * `Class86B60::iconHandle` (a TimImage), forwarded opaquely (never
     * dereferenced by this slot's own caller), so kept `void *`. */
    void (*slot78)(Class86B60UnkACObj_3bb8c_d *self, void *arg1, void *arg2,
                   s32 arg3, s32 arg4, void *arg5, s32 arg6, s32 arg7); /* +0x078 */
};

struct Class86B60UnkACObj_3bb8c_d {
    Class86B60UnkACObjMethods_3bb8c_d *methods; /* +0x000 */
};


/* Class86B60's menu description (include/TaskCore.h): Class86B60__Class86B60
 * passes &D_80086D44 as TaskCore's ctor's `target` and again to setTarget.
 * 0x28 bytes in asm/data/76DC8.data.s, a path word, three zero words, the two
 * colour triples and four pointers: the TaskCoreTarget layout. RETYPED from a
 * placeholder `s32` in round 88 (address-of only; no byte change). */
extern TaskCoreTarget D_80086D44;

/* "ETC\ETCSE" (asm/data/1C34.rodata.s), Class86B60__Class86B60's
 * soundBankPath for TaskCore's ctor. RETYPED from a placeholder `s32` in
 * round 88 (address-of only); the ctor casts away the const for the
 * ctor's `char *`. */
extern const char D_800114DC[];

/* "ETC\TITLE.TIM" (asm/data/1C34.rodata.s), Class86B60__Reset's path for
 * setSubHandle. RETYPED from a placeholder `s32` in round 88 (address-of
 * only). */
extern const char D_800114E8[];

/* Class86B60__BeginMemcardSave's own path string, passed to New_TimImage -- a real
 * dlabel (`asm/data/1C34.rodata.s`: "CARD\FILEICN1.TIM"), so this is the
 * ONLY correct spelling (CLAUDE.md: never re-write a string splat has
 * already emitted as a symbol). */
extern const char D_800114F8[];

/* Address-of only in this unit -- Class86B60__OnDeinit walks it with an
 * explicit 0xC-byte stride, passing each entry's address on to
 * `Class86B60UnkC0ObjMethods_3bb8c_d::slot78`, but never dereferences it
 * itself. Placeholder s32 type; real element layout unknown. */
extern s32 D_80086DAC;

/* Address-of only in this unit (Class86B60__ForwardToNameField passes &D_8008A9B4 to
 * the name field's attachToParent, TextRow +0x04C, as its Vec3_d294 offset;
 * the words are -4, -23, ...). Placeholder s32 type, cast at the call. */
extern s32 D_8008A9B4;

/* VALUE-of, not address-of, in this unit -- Class86B60__UpdateMemcardSaveStatus reaches these
 * through `%gp_rel` loads of the .sdata globals themselves, forwarding
 * whatever they hold. Each holds a pointer into the still-uncarved rodata
 * block at `D_80011434` (`asm/data/1C34.rodata.s`: 0x80011464 and
 * 0x8001149C respectively, neither with its own dlabel), so they cannot
 * be spelled by the address they point to and are typed opaque `void *`
 * instead.
 *
 * `D_8008AA18` is also read by round 43's `Class86B60__CreateNameField`, which
 * `strcpy`s INTO `(char *)D_8008AA18 + 0x18` and reads it with `strlen` --
 * both require the RUNTIME value to be a writable buffer, not the .rodata
 * address the ROM image happens to initialise it to. Nothing in this unit
 * ever reassigns it, so whatever sets the real (writable) value is outside
 * this unit's own ground; the ROM-image value above is a placeholder only. */
extern void *D_8008AA10;
extern void *D_8008AA18;

/* Same VALUE-of `%gp_rel` pattern, read (and its buffer formatted into via
 * FormatFullWidthNumber) by round 45's `FormatNumberIntoBuffer` (src/class_3bb8c_c.c). Holds
 * `D_8008AA1C` in the ROM image -- the "7654321" placeholder string
 * (`asm/data/7B12C.sdata.s`) -- so, like `D_8008AA18` above, this is a
 * writable-buffer placeholder rather than the real runtime value. */
extern void *D_8008AA24;

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `Class86B60__CreateNameField`
 * as `strcpy`'s SOURCE argument. Holds `0x80011474` in the ROM image
 * (immediately past `D_8008AA10`'s own "BISLPS-01556xxx" string, i.e. the
 * start of the font-glyph word table in `D_80011434`) -- likely also a
 * placeholder for the same reason `D_8008AA18` is, since a font-glyph
 * table is not plausible `strcpy` input. */
extern void *D_8008AA14;

/* Same VALUE-of `%gp_rel` pattern, read only by round 43's `Class86B60__BeginMemcardSave`
 * as `Class86B60UnkACObjMethods_3bb8c_d::slot6C`'s own `arg1`. Holds
 * `0x80011454` in the ROM image -- the "BISLPS-01556" string in
 * `D_80011434`, again with no `dlabel` of its own. */
extern void *D_8008A9D0;

/* Address-of only, round 43's `Class86B60__BeginMemcardSave`
 * (`Class86B60UnkACObjMethods_3bb8c_d::slot6C`'s own `arg2`) -- a real
 * 16-entry pointer table (`asm/data/76DC8.data.s`, `D_8008AA0C` down to
 * `D_8008A9D4` then a NULL terminator), reached only by its own address
 * here, never walked. Placeholder `s32` type since only the address is
 * taken. */
extern s32 D_80086D6C;

/* Class86B60__TickNameFieldCursor's own rolling byte index (0/1/2, wraps to 0 at 3) into
 * that function's own 3-byte stack buffer -- declared in the ROM image
 * as a full `.word` (`asm/data/7B12C.sdata.s`), but accessed only via
 * `lbu`/`sb` here, so `u8` is the correct C type for this unit's own
 * reference regardless of the underlying storage's full width. */
extern u8 D_8008AA28;

/* Class86B60__TickNameFieldCursor's own rolling word counter (wraps to 0 at 0x101). */
extern s32 D_8008AA2C;

/* TaskObjF__TaskObjF's own one-shot init guard: read, then unconditionally
 * incremented, before its own body's InitCARD/StartCARD/_bu_init calls
 * run only when the PRE-increment value was 0 (i.e. only on the very
 * first construction of this class). */
extern s32 D_8008AA30;

/* Still raw asm in this unit (gp-relative-blocked, see
 * docs/match-reports/FormatNumberIntoBuffer.md) -- not this round's function, but
 * Class86B60__Class86B60 calls it with one forwarded s32 argument (the return
 * value of dreamSys->methods->slot1A0); return value unused there. */
extern void FormatNumberIntoBuffer(s32 arg0);

/*
 * First argument of CheckObj866E8CountFlag: an unrelated, larger caller-side
 * struct (only seen from its one caller, Class86B60__CommitNameEntry in the still-
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
 * Second argument of CheckObj866E8CountFlag: holds a pointer at +0x018 to a small
 * result block whose word at +0x004 is the flag CheckObj866E8CountFlag computes.
 */
typedef struct Result678_3bb8c_c {
    u8 pad00[0x018];
    s32 *block;                                 /* +0x018, CheckObj866E8CountFlag writes block[1] */
} Result678_3bb8c_c;

/*
 * New_TaskObjF's own New_X allocator target -- yet another small
 * sibling class (same shape as `Class869D8`/`Class86AA0`/`Class86B60`
 * above): pool-allocate a fixed 0x84-byte block, and if it succeeds,
 * construct it through this table's own `ctor` slot at `+0x008`. Kept
 * fully opaque (no instance type at all) since New_TaskObjF never
 * dereferences the allocation itself, only forwards it.
 */
typedef struct GenericCtorTable_3bb8c_d GenericCtorTable_3bb8c_d;
struct GenericCtorTable_3bb8c_d {
    u8 pad000[0x008];
    void (*ctor)(void *self, void *arg1, void *arg2); /* +0x008, New_TaskObjF's own call; IS TaskObjF__TaskObjF -- see that function's own, more precise (s32, s32) local declaration in class_3bb8c_d.c, kept separate per the project's independent-arities convention since nothing here type-checks the two against each other */
    u8 pad00C[0x040 - 0x00C];
    /* +0x040, TaskObjF__TaskObjF's own last call, forwarding its own 3rd
     * parameter verbatim; class_3bb8c_e's independent view (round 14,
     * this same real object) names the concrete function `TaskObjF__SetCardSlot`,
     * still uncarved there. Round 78: renamed slot40 -> setCardSlot
     * (classtable-confirmed occupant TaskObjF__SetCardSlot). */
    void (*setCardSlot)(void *self, s32 arg1);
};

extern GenericCtorTable_3bb8c_d gTaskObjFMethods;
extern GenericCtorTable_3bb8c_d *GetTaskObjFMethods(void); /* returns &gTaskObjFMethods; matched in class_3bb8c_g */

/*
 * The object instance itself -- established this round by TaskObjF__TaskObjF,
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
    GenericCtorTable_3bb8c_d *methods; /* +0x000, TaskObjF__TaskObjF: self->methods = GetTaskObjFMethods() -- the base-ctor-chain "sets self->methods directly to this table's own pointer" pattern already seen for Class86B60/Class86B60__Class86B60 */
};

/*
 * Class86E00 -- NOT a separate class (round 73 head correction). The name
 * came from reading `tools/classtable.py 0x80086E00` as a table start, but
 * nothing in the image addresses 0x80086E00: it is +0x03C INSIDE
 * `gTaskObjFMethods` (0x80086DC4, 44 slots, header word 0xB), whose +0x064..
 * +0x078 are the `TaskObjF__*` methods of class_3bb8c_f and whose +0x07C..
 * +0x0B0 are class_3bb8c_g's. So `Class86E00_3bb8c_g` below is class_3bb8c_g's
 * unit-local view of `TaskObjF`, and its methods are named `TaskObjF__*`.
 * Slot offsets quoted as "Class86E00 +N" in older comments are 0x3C short of
 * the real table offset. Track 4 merges these views into one TaskObjF.
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
 * self->childC's pointee. TaskObjF__SetChildFlag8 is the function that PROVES this
 * is a pointer (dereferences its `+0x080` vtable slot) -- before that
 * function was read, `childC` looked like a plain `s32` value forwarded
 * opaquely to `Class86E00SubObj_3bb8c_g::slot4C`'s 3rd argument, which is
 * why that parameter is typed with this pointer rather than `s32` below.
 */
typedef struct Class86E00Unk6CObj_3bb8c_g Class86E00Unk6CObj_3bb8c_g;
typedef struct Class86E00Unk6CObjMethods_3bb8c_g Class86E00Unk6CObjMethods_3bb8c_g;

struct Class86E00Unk6CObjMethods_3bb8c_g {
    u8 pad000[0x080];
    /* +0x080, TaskObjF__SetChildFlag8's own call: `(self, arg1, 0x7F, 0x7F)`,
     * `arg1` forwarded verbatim from TaskObjF__SetChildFlag8's own 2nd parameter. */
    void (*slot80)(Class86E00Unk6CObj_3bb8c_g *self, s32 arg1, s32 arg2, s32 arg3);
};

struct Class86E00Unk6CObj_3bb8c_g {
    Class86E00Unk6CObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * self->childA's and self->childB's shared pointee -- two parallel fields
 * of the SAME sub-object shape (TaskObjF__AttachChildA/TaskObjF__DetachChildA exercise
 * `childA`; TaskObjF__AttachChildB/TaskObjF__DetachChildB exercise `childB` the identical
 * way), each independently attached via `Class86E00Methods_3bb8c_g::
 * slot10` and torn down via a fixed `slot50`/`slot48`/`release` sequence.
 */
typedef struct Class86E00SubObj_3bb8c_g Class86E00SubObj_3bb8c_g;
typedef struct Class86E00SubObjMethods_3bb8c_g Class86E00SubObjMethods_3bb8c_g;

struct Class86E00SubObjMethods_3bb8c_g {
    u8 pad000[0x004];
    void (*release)(Class86E00SubObj_3bb8c_g *self); /* +0x004, TaskObjF__DetachChildA/TaskObjF__DetachChildB */
    u8 pad008[0x044 - 0x008];
    /* +0x044, TaskObjF__AttachChildA/TaskObjF__AttachChildB's own call: `(self, childReady)`
     * from the OWNING `Class86E00_3bb8c_g`. */
    void (*slot44)(Class86E00SubObj_3bb8c_g *self, s32 arg1);
    void (*slot48)(Class86E00SubObj_3bb8c_g *self); /* +0x048, TaskObjF__DetachChildA/TaskObjF__DetachChildB */
    /* +0x04C, TaskObjF__AttachChildA/TaskObjF__AttachChildB's own call:
     * `(self, unk60, unk64, childC)` from the OWNING `Class86E00_3bb8c_g`. */
    void (*slot4C)(Class86E00SubObj_3bb8c_g *self, s32 a1, s32 a2, Class86E00Unk6CObj_3bb8c_g *a3);
    void (*slot50)(Class86E00SubObj_3bb8c_g *self); /* +0x050, TaskObjF__DetachChildA/TaskObjF__DetachChildB */
};

struct Class86E00SubObj_3bb8c_g {
    Class86E00SubObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * self->cardIcon's pointee -- a DIFFERENT sub-object from
 * `Class86E00SubObj_3bb8c_g` above. It shares the same `+0x004` slot
 * offset only because every BasicClass-family table keeps a slot there
 * (see `BasicClassMethods::release` in code_8220.h) -- the USAGE differs:
 * TaskObjF__TickCardIcon assigns this call's RETURN VALUE back into `cardIcon` (an
 * "advance" pattern), where `Class86E00SubObj_3bb8c_g::release`'s callers
 * (TaskObjF__DetachChildA/TaskObjF__DetachChildB) discard the return and unconditionally
 * null the field afterward instead. Different enough to keep separate
 * rather than unify.
 */
typedef struct Class86E00Unk70Obj_3bb8c_g Class86E00Unk70Obj_3bb8c_g;
typedef struct Class86E00Unk70ObjMethods_3bb8c_g Class86E00Unk70ObjMethods_3bb8c_g;

struct Class86E00Unk70ObjMethods_3bb8c_g {
    u8 pad000[0x004];
    /* +0x004, TaskObjF__TickCardIcon's own call: return value stored back into
     * `Class86E00_3bb8c_g::cardIcon` itself. */
    Class86E00Unk70Obj_3bb8c_g *(*slot4)(Class86E00Unk70Obj_3bb8c_g *self);
    u8 pad008[0x04C - 0x008];
    /* +0x04C, TaskObjF__LoadCardIcon's own call on a FRESH `cardIcon` right after
     * assigning it: `(self, self->childReady, &D_8008AA94)`. `childReady` is
     * forwarded verbatim -- kept as the owning struct's established
     * bare `s32` reading of that field, not retyped to a pointer here. */
    void (*slot4C)(Class86E00Unk70Obj_3bb8c_g *self, s32 arg1, void *arg2);
};

struct Class86E00Unk70Obj_3bb8c_g {
    Class86E00Unk70ObjMethods_3bb8c_g *methods; /* +0x000 */
};

/*
 * TaskObjF__OnItemSelected's own `arg1` -- a third, unrelated small object, reached
 * only through its own `+0x09C` slot, whose return value is stored into
 * `Class86E00_3bb8c_g::selectedItem`.
 */
typedef struct GenericSlot9CObj_3bb8c_g GenericSlot9CObj_3bb8c_g;
typedef struct GenericSlot9CMethods_3bb8c_g GenericSlot9CMethods_3bb8c_g;

struct GenericSlot9CMethods_3bb8c_g {
    u8 pad000[0x09C];
    void *(*slot9C)(GenericSlot9CObj_3bb8c_g *self); /* +0x09C, TaskObjF__OnItemSelected */
};

struct GenericSlot9CObj_3bb8c_g {
    GenericSlot9CMethods_3bb8c_g *methods; /* +0x000 */
};

struct Class86E00Methods_3bb8c_g {
    u8 pad000[0x010];
    /* +0x010, TaskObjF__AttachChildA/TaskObjF__AttachChildB's own first call: `(self,
     * subObj)`, registering/attaching whichever of `childA`/`childB` that
     * function owns. */
    void (*slot10)(Class86E00_3bb8c_g *self, Class86E00SubObj_3bb8c_g *arg1);
    u8 pad014[0x030 - 0x014];
    /* +0x030, TaskObjF__SetState's own first call, every path: `(self,
     * arg1)` where `arg1` may already have been forced to `0x17`. */
    void (*slot30)(Class86E00_3bb8c_g *self, s32 arg1);
    u8 pad034[0x050 - 0x034];
    /* +0x050, TaskObjF__SetState's own `arg1==0x13` case: no extra arguments,
     * return value picks between two literal replacement codes. */
    s32 (*slot50)(Class86E00_3bb8c_g *self);
    u8 pad054[0x058 - 0x054];
    /* +0x058, TaskObjF__SetState's own `arg1==0x14` case, only when
     * `*(u8 *)self->unk40 == 0`: `(self, unk40, unk30, unk34)`. */
    void (*slot58)(Class86E00_3bb8c_g *self, s32 a1, char *a2, s32 a3);
    u8 pad05C[0x064 - 0x05C];
    /* +0x064, TaskObjF__SetState's own `arg1==0x15` case: `(self, unk40,
     * unk54, unk58)`, return value picks between two literal
     * replacement codes (same shape as `slot68` just below). */
    s32 (*slot64)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3);
    /* +0x068, TaskObjF__SetState's own `arg1==0x14` case, unconditional:
     * `(self, unk40, unk44, unk4C, unk50, unk54, unk58)` -- the same
     * six fields as `slot78` below MINUS `unk48`, not a subset call to
     * that slot. Return value picks between two literal replacement
     * codes, same shape as `slot64` above. */
    s32 (*slot68)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3, s32 a4, s32 a5, s32 a6);
    u8 pad06C[0x074 - 0x06C];
    /* +0x074, TaskObjF__AdvanceState's own `self->secondaryMode==1` case: 4 extra
     * arguments (`unk40`, `unk44`, `unk54`, `unk58`), same shape as
     * `slot78` just below but with only the last two of that call's
     * trailing four. */
    void (*slot74)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3, s32 a4);
    /* +0x078, TaskObjF__OnCommand's own call for its `arg2==2` case: 7 extra
     * arguments, the last four passed on the stack (`unk4C`, promoted
     * from its native `u8` to a full word, then `unk50`/`unk54`/`unk58`). */
    void (*slot78)(Class86E00_3bb8c_g *self, s32 a1, s32 a2, s32 a3,
                    s32 a4, s32 a5, s32 a6, s32 a7);
    /* +0x07C, the shared tail call of TaskObjF__ForceIdleFromState/TaskObjF__TickStateDelay/
     * TaskObjF__OnCommand/TaskObjF__OnItemSelected: `(self, literal state code)`. */
    void (*slot7C)(Class86E00_3bb8c_g *self, s32 arg1);
    /* +0x080, TaskObjF__SetState's own 3rd call, every path: `(self, arg1)`,
     * same `arg1` value as `slot30` above. */
    void (*slot80)(Class86E00_3bb8c_g *self, s32 arg1);
    /* +0x084, TaskObjF__SetState's own 2nd call, every path: `(self)`. */
    void (*slot84)(Class86E00_3bb8c_g *self);
    u8 pad088[0x08C - 0x088];
    void (*slot8C)(Class86E00_3bb8c_g *self, s32 arg1); /* +0x08C, TaskObjF__ForceIdleFromState */
    void (*slot90)(Class86E00_3bb8c_g *self); /* +0x090, TaskObjF__OnNotify's `arg2==0x19` case */
    void (*slot94)(Class86E00_3bb8c_g *self); /* +0x094, TaskObjF__OnNotify's `arg2==0x17` case */
    u8 pad098[0x09C - 0x098];
    /* +0x09C, TaskObjF__SetState's own `arg1==0x11` case: `(self)`, no
     * return value read. */
    void (*slot9C)(Class86E00_3bb8c_g *self);
    void (*slotA0)(Class86E00_3bb8c_g *self); /* +0x0A0, TaskObjF__OnCommand's own first call, both cases */
    u8 pad0A4[0x0A8 - 0x0A4];
    /* +0x0A8, TaskObjF__SetState's own `arg1==0x12` case: `(self)`, same
     * shape as `slot9C` above. */
    void (*slotA8)(Class86E00_3bb8c_g *self);
    void (*slotAC)(Class86E00_3bb8c_g *self); /* +0x0AC, TaskObjF__OnItemSelected's own 2nd call, both cases */
};

struct Class86E00_3bb8c_g {
    Class86E00Methods_3bb8c_g *methods; /* +0x000 */
    u8 pad004[0x024 - 0x004];
    /* +0x024, TaskObjF__AdvanceState's own secondary dispatch code (nested inside
     * the `state`-driven switch's shared `2`/`4`/`0xA`/`0xE` case),
     * tested against literals `2` and `1`. */
    s32 secondaryMode;
    s32 state;   /* +0x028, dispatch/state code tested by several functions */
    /* +0x02C, TaskObjF__SetState's own loop bound: frees `self->unk38[0..
     * unk2C)` when tearing down (see `unk38`'s own comment below). */
    s32 unk2C;
    /* +0x030, TaskObjF__AdvanceState's own `strcpy` source into `self->unk40`,
     * in its `self->state==0xE` sub-case. */
    char *unk30;
    /* +0x034, TaskObjF__SetState's own `slot58` arg3, forwarded verbatim
     * alongside `unk30` above. */
    s32 unk34;
    void *unk38; /* +0x038, TaskObjF__AttachChildB: forwarded opaquely to `New_Class86F88`'s arg0 */
    /* +0x03C, TaskObjF__AdvanceState's own `self->state==0xE` sub-case: base of a
     * pointer array indexed by `(s32)self->selectedItem`, `strcat`ed onto
     * `self->unk40` -- same shape as `unk38` just below, indexed the
     * same way for `self->unk44`'s own `strcpy`. */
    void *unk3C;
    s32 unk40;   /* +0x040, TaskObjF__AttachChildA/TaskObjF__OnCommand */
    s32 unk44;   /* +0x044, TaskObjF__AttachChildA/TaskObjF__OnCommand */
    s32 unk48;   /* +0x048, TaskObjF__AttachChildA/TaskObjF__OnCommand */
    u8 unk4C;    /* +0x04C, TaskObjF__OnCommand: read `lbu`, promoted to a full word for `slot78`'s call */
    u8 pad04D[0x050 - 0x04D];
    s32 unk50;   /* +0x050, TaskObjF__OnCommand */
    s32 unk54;   /* +0x054, TaskObjF__OnCommand */
    s32 unk58;   /* +0x058, TaskObjF__OnCommand */
    s32 waitCounter;   /* +0x05C, TaskObjF__TickStateDelay: incremented, capped at 6 */
    s32 unk60;   /* +0x060, forwarded to `childA`/`childB`'s own `slot4C` arg1 */
    s32 unk64;   /* +0x064, forwarded to `childA`/`childB`'s own `slot4C` arg2 */
    /* +0x068, a readiness gate checked alongside `unk60` in four
     * functions (TaskObjF__AttachChildA/TaskObjF__DetachChildA/TaskObjF__AttachChildB/
     * TaskObjF__DetachChildB) -- both must be non-zero before the body runs.
     * Kept a bare `s32`; never dereferenced in this unit. */
    s32 childReady;
    /* +0x06C, forwarded to `childA`/`childB`'s own `slot4C` arg3.
     * TaskObjF__SetChildFlag8 proves this is a pointer (dereferences its `+0x080`
     * vtable slot), not the plain `s32` it looked like from the slot4C
     * call site alone -- retyped here, same size, no layout change. */
    Class86E00Unk6CObj_3bb8c_g *childC;
    Class86E00Unk70Obj_3bb8c_g *cardIcon; /* +0x070, TaskObjF__TickCardIcon */
    /* +0x074, a one-shot flag set to 1 by TaskObjF__AttachChildA/TaskObjF__AttachChildB
     * right after attaching `childA`/`childB`, and consumed (guarding a
     * teardown callback) by TaskObjF__DetachChildA/TaskObjF__DetachChildB. */
    s32 childAttached;
    struct TextEntry *childA;         /* +0x078, TaskObjF__AttachChildA/TaskObjF__DetachChildA: New_TextEntry (include/TextEntry.h) */
    Class86E00SubObj_3bb8c_g *childB; /* +0x07C, TaskObjF__AttachChildB/TaskObjF__DetachChildB */
    void *selectedItem; /* +0x080, TaskObjF__OnItemSelected: set from `arg1->methods->slot9C(arg1)`'s return */
};

/* New_Class86F88 (TaskObjF__AttachChildB's childB) is declared in
 * include/Class86F88.h. */

/* New_TextEntry (TaskObjF__AttachChildA's childA) is declared in
 * include/TextEntry.h. */

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
    void (*addChild)(TaskObjF *self, void *child);                 /* +0x010, TaskObjF__Init (x2) */
    void (*removeChild)(TaskObjF *self, void *child);               /* +0x014, TaskObjF__Deinit (x2) */
    void *unk18, *unk1C, *unk20, *unk24, *unk28, *unk2C, *unk30, *unk34, *unk38; /* inherited BasicClass slots, untouched by this unit */
    u8 pad3C[0x044 - 0x03C];
    void (*slot44)(TaskObjF *self);                                   /* +0x044, TaskObjF__Validate */
    s32 (*slot48)(TaskObjF *self);                                     /* +0x048, TaskObjF__Validate */
    s32 (*slot4C)(TaskObjF *self, s32 *out1, s32 *out2, s32 *out3);      /* +0x04C, TaskObjF__Validate */
    u8 pad50[0x054 - 0x050];
    s32 (*slot54)(TaskObjF *self, s32 a1, s32 a2);                          /* +0x054, TaskObjF__func_8004F8A4 */
    u8 pad58[0x05C - 0x058];
    s32 (*slot5C)(TaskObjF *self, void *a1, void *a2, s32 a3, s32 a4);        /* +0x05C, TaskObjF__func_8004F638 */
    s32 (*slot60)(TaskObjF *self, s32 a1, s32 a2);                              /* +0x060, TaskObjF__func_8004F8A4 */
    u8 pad64[0x07C - 0x064];
    s32 (*slot7C)(TaskObjF *self, s32 a1);                                        /* +0x07C, TaskObjF__func_8004F638/TaskObjF__func_8004F8A4/TaskObjF__Validate */
    u8 pad80[0x088 - 0x080];
    void (*slot88)(TaskObjF *self, void *arg1, s32 arg2);                          /* +0x088, TaskObjF__Notify */
    u8 pad8C[0x098 - 0x08C];
    void (*slot98)(TaskObjF *self, void *arg1, s32 arg2);                            /* +0x098, TaskObjF__Notify */
    u8 pad9C[0x0A4 - 0x09C];
    void (*slotA4)(TaskObjF *self, void *arg1, s32 arg2);                              /* +0x0A4, TaskObjF__Notify */
    u8 padA8[0x0B0 - 0x0A8];
    void (*slotB0)(TaskObjF *self, void *arg1, s32 arg2);                                /* +0x0B0, TaskObjF__Notify */
};

struct TaskObjF {
    TaskObjFMethods *methods;   /* +0x000 */
    u8 pad04[0x00C - 0x004];     /* BasicClass::children/parentRefs, untouched by this unit */
    s32 cardSlot;                  /* +0x00C, TaskObjF__TryReadMemcardFile: passed as BuildMemcardPath's "selector" (device slot 0/1) -- RENAMED round 60 (was unk0C) */
    u8 pad10[0x014 - 0x010];
    s32 events[4];                  /* +0x014, TaskObjF__ForEachEvent (walks all 4, early-exit)/TaskObjF__WaitForReadyEvent (passes &events[0], count 4) -- RENAMED round 60 (was field14): 4 kernel event descriptors, corroborated cross-unit by class_3bb8c_e.c's TaskObjF__OpenEvents, which fills the identical offset via OpenEvent() then passes the same object to this unit's own EnableEvents wrapper (see TaskObjF__EnableEvents's report) */
    s32 opMode;                       /* +0x024, RENAMED round 60 (was unk24): distinguishes which of this class's two operations is active -- TaskObjF__func_8004F638 sets 1, TaskObjF__func_8004F8A4 sets 2, TaskObjF__Validate reads (==1?); the values' exact meaning is not established */
    s32 statusCode;                    /* +0x028, RENAMED round 60 (was unk28): TaskObjF__Init/TaskObjF__func_8004F638 clear or set it, TaskObjF__func_8004F8A4/TaskObjF__Validate read it and dispatch it through slot7C -- a status/completion code, not confirmed to be error-only */
    s32 bufCount;                       /* +0x02C, RENAMED round 60 (was unk2C): TaskObjF__func_8004F638 (slot5C's return)/TaskObjF__FreeUnusedBuffers/TaskObjF__FreeBuffers (loop bound over bufArray) -- the number of bufArray entries actually in use */
    s32 unk30;                          /* +0x030, TaskObjF__Init (arg1)/TaskObjF__func_8004F638 (slot5C's arg3) */
    s32 unk34;                           /* +0x034, TaskObjF__Init (arg2)/TaskObjF__func_8004F638 (slot5C's stack arg4) */
    void **bufArray;                      /* +0x038, RENAMED round 60 (was unk38): a 16-entry pointer array allocated by TaskObjF__AllocBuffers, torn down by TaskObjF__FreeBuffers, walked by TaskObjF__FreeUnusedBuffers; also TaskObjF__func_8004F638's slot5C arg1 */
    void *scratchBuf;                       /* +0x03C, RENAMED round 60 (was unk3C): a single buffer allocated by TaskObjF__AllocBuffers, freed by TaskObjF__FreeBuffers; also TaskObjF__func_8004F638's slot5C arg2 */
    s32 unk40;                                /* +0x040, TaskObjF__func_8004F638 (arg1)/TaskObjF__func_8004F8A4 (arg1, forwarded to slot54 as its own arg2) */
    s32 unk44;                                 /* +0x044, TaskObjF__func_8004F638 (arg2)/TaskObjF__func_8004F8A4 (arg2) */
    s32 unk48;                                  /* +0x048, TaskObjF__func_8004F8A4 (arg3) */
    u8 unk4C;                                     /* +0x04C, TaskObjF__func_8004F8A4's 5th (byte) arg; also forwarded live to slot60's arg1 */
    u8 pad4D[0x050 - 0x04D];
    s32 unk50;                                      /* +0x050, TaskObjF__func_8004F8A4's 6th arg */
    s32 unk54;                                       /* +0x054, TaskObjF__func_8004F638 (arg3)/TaskObjF__func_8004F8A4's 7th arg */
    s32 unk58;                                        /* +0x058, TaskObjF__func_8004F638's 5th/stack arg/TaskObjF__func_8004F8A4's 8th arg; also forwarded live to slot60's arg2 */
    u8 pad5C[0x060 - 0x05C];
    s32 unk60;                                          /* +0x060, TaskObjF__Deinit: removeChild's arg */
    s32 unk64;                                            /* +0x064, TaskObjF__Deinit: removeChild's arg */
    s32 unk68;                                             /* +0x068, TaskObjF__Init (arg6)/TaskObjF__Deinit (cleared) */
    s32 unk6C;                                              /* +0x06C, TaskObjF__Init (arg7)/TaskObjF__Deinit (cleared) */
    s32 unk70;                                                /* +0x070, TaskObjF__Init (cleared) */
};

/* The three library callbacks TaskObjF__EnableEvents/TaskObjF__DisableEvents/TaskObjF__TestEvents
 * forward into TaskObjF__ForEachEvent are EnableEvent/DisableEvent/TestEvent, now
 * linked from the Psy-Q objects libapi/a12, libapi/a13 and libapi/a11.
 * Their declarations live in src/class_3bb8c_f.c, the only unit that uses
 * them: a prototype for a function a Sony object defines does not belong in
 * a header 21 units include, where it would one day collide with the real
 * KERNEL.H. (The old comment here called them "SPU routines" -- they are
 * kernel event-queue calls; only their neighbours in the block are libspu.)
 * TestEvent is also the validity check WaitForReadyEvent uses on its own array
 * argument. */

/* Generic "find the first of up to `count` entries for which
 * TestEvent accepts it, retrying the whole array forever if none
 * qualify yet" helper -- TaskObjF__WaitForReadyEvent calls it on this unit's own
 * TaskObjF::events (count 4). D_80086E78 is a small lookup table indexed
 * by the winning slot; bound unknown from this unit alone, left unsized.
 * (Comment updated round 60: the callback was `func_800390F4` before round
 * 34 linked it as Sony's own `TestEvent`; `field14` renamed to `events`.) */
extern s32 D_80086E78[];
extern s32 WaitForReadyEvent(s32 *arr, s32 count);

/* The generic pool allocator/free pair, already established the same way
 * by include/code_8220.h, include/code_55dd4.h etc -- `BMemPMgrFree`
 * returning `void *` (not `void`) matches TaskObjF__FreeUnusedBuffers's own use here,
 * which stores its return value back into the freed slot. */
/* BMemPMgrAlloc/BMemPMgrFree already declared above in this header. */
extern void *BMemPMgrFree(void *ptr);

/* A fixed 6-byte memory-card device-name template ("bu00:"/"bu10:", PS-X
 * BIOS device names -- asm/data/7B008.sdata.s). An all-`s8` struct
 * (natural alignment 1) so the whole-struct assignment in BuildMemcardPath
 * reproduces retail's unaligned lwl/lwr + byte-store copy, the same idiom
 * already documented for `Descriptor10` above. */
typedef struct DeviceName866E8 {
    s8 b0, b1, b2, b3, b4, b5;
} DeviceName866E8;

extern DeviceName866E8 gMcDevicePath1;   /* "bu10:" */
extern DeviceName866E8 gMcDevicePath0;   /* "bu00:" */

/* This project's own strcat (matched elsewhere, src/code_171e0.c) --
 * BuildMemcardPath is this unit's only caller. */
extern char *strcat(char *dest, char *src);


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
 * CopyMemcardIconTemplate stays: it is game code, defined in src/class_3bb8c_g.c
 * (MATCHED round 45, 60/60 words -- was gp_rel-blocked, resolved round 42). */
extern s32 CopyMemcardIconTemplate(s32 arg0, s32 arg1);                /* TaskObjF__WriteMemcardSaveFile's own retry-loop bracket; also called with (arg,0) after the retry loop gives up */

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
typedef struct ChildM_AC ChildM_AC;
typedef struct ChildM_ACMethods ChildM_ACMethods;
typedef struct ChildM114 ChildM114;
typedef struct ParamM ParamM;
typedef struct ParamMMethods ParamMMethods;
typedef struct FieldM34 FieldM34;
typedef struct FieldM34Methods FieldM34Methods;
typedef struct FieldM50 FieldM50;
typedef struct FieldM50Methods FieldM50Methods;
typedef struct FieldM7C FieldM7C;
typedef struct FieldM7CMethods FieldM7CMethods;

/* self->unk18's target (ObjM__ForwardToSubChild/ObjM__HandleEvent5Or6). */
struct FieldM18Methods {
    u8 pad000[0x064];
    void (*slot64)(FieldM18 *self, s32 arg1);   /* +0x064, ObjM__HandleEvent5Or6 */
    u8 pad068[0x0AC - 0x068];
    ChildM_AC *(*slotAC)(FieldM18 *self);       /* +0x0AC, ObjM__ForwardToSubChild */
    u8 pad0B0[0x0B4 - 0x0B0];
    void (*slotB4)(FieldM18 *self, s32 arg1);   /* +0x0B4, ObjM__TeardownPauseOverlay (not this round's target) */
};
struct FieldM18 {
    FieldM18Methods *methods;   /* +0x000 */
};

/* Returned by FieldM18Methods::slotAC (ObjM__ForwardToSubChild). */
struct ChildM_ACMethods {
    u8 pad000[0x0D0];
    void (*slotD0)(ChildM_AC *self, s32 arg1);                      /* +0x0D0 */
    u8 pad0D4[0x0D8 - 0x0D4];
    void (*slotD8)(ChildM_AC *self, s32 arg1, s32 arg2, s32 arg3);  /* +0x0D8 */
};
struct ChildM_AC {
    ChildM_ACMethods *methods;  /* +0x000 */
};

/* self->unk14's target (ObjM__CheckAuxTrigger). */
struct FieldM14Methods {
    u8 pad000[0x114];
    ChildM114 *(*slot114)(FieldM14 *self, s32 *out);  /* +0x114, ObjM__CheckAuxTrigger */
};
struct FieldM14 {
    FieldM14Methods *methods;   /* +0x000 */
};

/* Returned by FieldM14Methods::slot114 (ObjM__CheckAuxTrigger). */
struct ChildM114 {
    u8 pad0[0x004];
    struct Class81940 *unk4;     /* +0x004, ObjM__CheckAuxTrigger: an element's Class81940 (include/Class81940.h) */
    u8 pad8[0x014 - 0x008];
    s32 unk14;                    /* +0x014, ObjM__CheckAuxTrigger: set from TryDreamAuxTrigger's return */
};

/* ObjM__HandleEvent5Or6's own arg1 parameter -- dispatched via its own vtable
 * slot 0xE4. Independent of FieldM14 above (unrelated numeric slot
 * range, nothing ties the two together). */
struct ParamMMethods {
    u8 pad000[0x0E4];
    s32 (*slotE4)(ParamM *self);  /* +0x0E4, ObjM__HandleEvent5Or6 */
};
struct ParamM {
    ParamMMethods *methods;      /* +0x000 */
};

/* Uncarved sibling (src/code_4cd08.c, still INCLUDE_ASM), ObjM__CheckAuxTrigger's
 * only external call. Typed purely from that call site's own register
 * setup: (value, out-pointer, opaque-object) -> s32, whose result is
 * stored into a ChildM114's unk14 and tested for zero. The third arg is
 * DreamSys's getCurrentDayAndYear (+0x1A0) return value (not `self->unk14`'s
 * child), so it stays void* rather than ChildM114* -- nothing ties the
 * two together. */
extern s32 TryDreamAuxTrigger(s32 arg0, s32 *arg1, void *arg2);

/* self->unk34's target (ObjM__TeardownPauseOverlay/ObjM__AdvancePauseSetup). */
struct FieldM34Methods {
    u8 pad000[0x088];
    void (*slot88)(FieldM34 *self);  /* +0x088, ObjM__AdvancePauseSetup */
    void (*slot8C)(FieldM34 *self);  /* +0x08C, ObjM__TeardownPauseOverlay */
};
struct FieldM34 {
    FieldM34Methods *methods;   /* +0x000 */
};

/* self->unk10/self->unk54's shared target (ObjM__TeardownPauseOverlay/ObjM__AdvancePauseSetup).
 * Distinct from TextRow (include/TextRow.h) despite sharing slot
 * NUMBERS with it (0x4C/0x50): self->unk10's own slot4C call site here
 * (ObjM__AdvancePauseSetup) sets up only ONE argument (self), which conflicts with
 * TextRow's attachToParent (+0x04C), a 3-argument slot
 * (from that unit's own call sites) -- real counter-evidence against
 * unifying the two, per this project's established arity-conflict rule.
 * Kept as its own type. */
struct FieldM50Methods {
    u8 pad000[0x04C];
    void (*slot4C)(FieldM50 *self);  /* +0x04C, ObjM__AdvancePauseSetup */
    void (*slot50)(FieldM50 *self);  /* +0x050, ObjM__TeardownPauseOverlay */
};
struct FieldM50 {
    FieldM50Methods *methods;   /* +0x000 */
};

/* self->unk7C's target (ObjM__TeardownPauseOverlay/ObjM__AdvancePauseSetup). Returned by
 * New_TextRow (src/code_2cc8c_f.c), which returns `TextRow *`
 * (include/TextRow.h); the call site casts. The slots below (0x004/0x04C/
 * 0x0B8) are TextRow's release/attachToParent/setColor; the view is kept
 * (ObjM's, not TextRow's job). */
struct FieldM7CMethods {
    u8 pad000[0x004];
    void (*slot4)(FieldM7C *self);                              /* +0x004, ObjM__TeardownPauseOverlay */
    u8 pad008[0x04C - 0x008];
    void (*slot4C)(FieldM7C *self, FieldM14 *arg1, void *arg2); /* +0x04C, ObjM__AdvancePauseSetup */
    u8 pad050[0x0B8 - 0x050];
    void (*slotB8)(FieldM7C *self, void *arg1);                  /* +0x0B8, ObjM__AdvancePauseSetup */
};
struct FieldM7C {
    FieldM7CMethods *methods;   /* +0x000 */
};

/* New_TextRow: include/TextRow.h (track 4, round 88), included by the units
 * that call it; its callers cast the TextRow to their own view. */

/* ObjM__AdvancePauseSetup's own literal arguments -- a "Pause" name string plus two
 * small opaque blocks, all reached only by address (never dereferenced in
 * this unit). */
extern char D_8008AB44[];   /* "Pause" (asm/data/7B008.sdata.s) */
extern s32 D_8008AB38;      /* two-word opaque block, address-only here */
extern s32 D_8008AB40;      /* one-word opaque block, address-only here */

/* The self type for this unit's ObjM__EnterState7..ObjM__CloseAndNotifyC cluster.
 * Only the slots/fields these functions actually reach are typed. */
struct ObjMMethods {
    u8 pad000[0x010];
    void (*slot10)(ObjM *self, ChildM_AC *arg1);  /* +0x010, ObjM__ForwardToSubChild */
    void (*slot14)(ObjM *self, ParamM *arg1);     /* +0x014, ObjM__HandleEvent5Or6 */
    u8 pad018[0x030 - 0x018];
    /* +0x030 == BasicClass__NotifyParents's own slot (confirmed with
     * `tools/classtable.py 0x80087034`, same table as the rest of ObjMMethods
     * -- self->methods IS D_80087034), so this is a plain notify-parents
     * dispatch with a class-specific event code, not an unknown slot. */
    void (*notifyParents)(ObjM *self, s32 code);   /* +0x030, ObjM__NotifyParentsCodeB/ObjM__HandleEvent5Or6/ObjM__CloseAndNotifyD/ObjM__CloseAndNotifyC */
    u8 pad034[0x0B8 - 0x034];
    void (*checkAuxTrigger)(ObjM *self);            /* +0x0B8, ObjM__HandleEvent7: this class's OWN ObjM__CheckAuxTrigger, confirmed via classtable.py against D_80087034 */
    u8 pad0BC[0x0D4 - 0x0BC];
    void (*teardownPauseOverlay)(ObjM *self);       /* +0x0D4, ObjM__CloseAndNotifyD/ObjM__CloseAndNotifyC: this class's OWN ObjM__TeardownPauseOverlay, confirmed via classtable.py */
};

struct ObjM {
    ObjMMethods *methods;   /* +0x000 */
    u8 pad004[0x010 - 0x004];
    /* RETYPED round 15b (ObjM__AdvancePauseSetup/ObjM__TeardownPauseOverlay): was `s32 unk10`,
     * established from ObjM__ForwardToSubChild's OWN call site as a plain forwarded
     * register value (never dereferenced there). ObjM__TeardownPauseOverlay/
     * ObjM__AdvancePauseSetup dereference this same field's vtable directly, so it
     * IS a pointer -- ObjM__ForwardToSubChild forwards it opaquely either way, and
     * a pointer-typed argument passed as a raw register value compiles to
     * the identical `lw`/`move` regardless of C-level pointer-vs-s32
     * typing (ABI-neutral retype, zero byte cost -- verified: full
     * rebuild stays whole-image green after this change). */
    FieldM50 *unk10;         /* +0x010, ObjM__ForwardToSubChild (opaque forward)/ObjM__TeardownPauseOverlay/ObjM__AdvancePauseSetup */
    FieldM14 *unk14;         /* +0x014, ObjM__CheckAuxTrigger */
    FieldM18 *unk18;         /* +0x018, ObjM__ForwardToSubChild/ObjM__HandleEvent5Or6 */
    u8 pad01C[0x020 - 0x01C];
    s32 mode;                 /* +0x020, ObjM__EnterState7/ObjM__EnterState8/ObjM__EnterStateA/ObjM__HandleEvent5Or6: a mode/state code (already documented as such before this round; renamed from unk20 since every accessor is exclusively this unit's -- called `mode` rather than `state` to avoid colliding, in a reader's head, with ObjM__AdvancePauseSetup's own local `state` variable, which is `self->unk80`'s step counter, an unrelated field) */
    u8 pad024[0x034 - 0x024];
    FieldM34 *unk34;          /* +0x034, ObjM__TeardownPauseOverlay/ObjM__AdvancePauseSetup */
    u8 pad038[0x03C - 0x038];
    struct DreamSys *dreamSys; /* +0x03C, ObjM__EnterState7/ObjM__EnterState8/ObjM__EnterStateA/ObjM__HandleEvent5Or6/ObjM__CheckAuxTrigger: the six slots called on it (+0x0F0/+0x0F4/+0x0FC/+0x13C/+0x17C/+0x1A0) are DreamSys occupants (include/DreamSys.h) */
    u8 pad040[0x054 - 0x040];
    FieldM50 *unk54;          /* +0x054, ObjM__TeardownPauseOverlay/ObjM__AdvancePauseSetup -- same type as unk10 above */
    u8 pad058[0x074 - 0x058];
    void *unk74;               /* +0x074, ObjM__AdvancePauseSetup: forwarded opaquely to New_TextRow's ctx arg */
    u8 pad078[0x07C - 0x078];
    FieldM7C *unk7C;          /* +0x07C, ObjM__AdvancePauseSetup (written)/ObjM__TeardownPauseOverlay (dispatched) */
    s32 pauseSetupStep;                 /* +0x080, ObjM__UpdateCloseReadyFlag/ObjM__AdvancePauseSetup */
    s32 closeReady;                   /* +0x084, ObjM__UpdateCloseReadyFlag/ObjM__ClearCloseReadyFlag/ObjM__CloseAndNotifyD/ObjM__CloseAndNotifyC */
};

/* -------------------------------------------------------------------
 * HEAD NOTE, round 15 merge: `ObjM` (above, from class_3bb8c_m) and
 * `Obj87034_3bb8c_l` (below, from class_3bb8c_l) are the SAME CLASS.
 * Both units independently reached method table D_80087034, which is
 * exactly the collision runner delta anticipated when it suffixed its
 * type names. The proof is a cross-unit call, not a guess:
 * ObjM__ForwardToSubChild is DEFINED in class_3bb8c_m.c taking `ObjM *self` and
 * CALLED from class_3bb8c_l.c (ObjM__EnterState6) passing its own
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

/* self->styleConfig's pointee: a plain (non-vtable) record, read directly by
 * ObjM__TransferToOther via ordinary field offsets, never through a methods
 * pointer -- so it is NOT another Obj87034_3bb8c_l, just an opaque
 * 3-field descriptor. */
typedef struct Unk50Struct_3bb8c_l {
    s32 unk0;      /* +0x000, round 45's ObjM__SetupSceneStyle: forwarded opaquely to Obj14Methods_3bb8c_l::slotC4's arg2 */
    s32 unk4;      /* +0x004, round 45's ObjM__SetupSceneStyle: forwarded opaquely to Obj14Methods_3bb8c_l::slotC4's arg3 */
    s32 unk8;      /* +0x008, round 45's ObjM__SetupSceneStyle: forwarded opaquely to Obj14Methods_3bb8c_l::slotBC's arg1 */
    void *unkC;   /* +0x00C, ObjM__TransferToOther (address taken, forwarded opaquely) */
    u8 pad10[0x014 - 0x010];
    s32 unk14;    /* +0x014, ObjM__TransferToOther: discriminant compared against 2; also ObjM__EnterStyleSession: discriminant compared against 1 */
    void *unk18;  /* +0x018, ObjM__TransferToOther (address taken, forwarded opaquely) */
    void *unk1C;  /* +0x01C, ObjM__EnterStyleSession (address taken, forwarded opaquely) */
} Unk50Struct_3bb8c_l;

/* Whatever self->unk14 points to: an object of some OTHER, unidentified
 * class -- it has its own methods pointer at +0x000 (ObjM__TickStyle
 * dispatches +0x10C on it) AND a plain u16 field at +0x1B4 (ObjM__TransferToOther
 * reads it directly). Offset +0x10C happens to coincide with a DreamSys
 * vtable offset, but DreamSys's own occupant there (DreamSys__SetSoundObj) takes
 * one s32 argument while this call site passes two -- different arities,
 * so this is a different class, not DreamSys; left unnamed. */
typedef struct Obj14Methods_3bb8c_l {
    u8 pad000[0x0BC];
    /* +0x0BC, round 45's ObjM__SetupSceneStyle: `(self, self->styleConfig->unk8, 0)`. */
    void (*slotBC)(void *self, s32 arg1, s32 arg2); /* +0x0BC */
    u8 pad0C0[0x0C4 - 0x0C0];
    /* +0x0C4, round 45's ObjM__SetupSceneStyle: `(self, 3, self->styleConfig->unk0,
     * self->styleConfig->unk4)`. */
    void (*slotC4)(void *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0C4 */
    u8 pad0C8[0x0CC - 0x0C8];
    /* +0x0CC, round 45's ObjM__SetupSceneStyle: `(self, &D_8008710C)`. */
    void (*slotCC)(void *self, void *arg1); /* +0x0CC */
    u8 pad0D0[0x0DC - 0x0D0];
    /* +0x0DC, round 45's ObjM__SetupSceneStyle: `(self, self->unk48)` on the
     * OWNING Obj87034_3bb8c_l. */
    void (*slotDC)(void *self, s32 arg1); /* +0x0DC */
    /* +0x0E0, round 45's ObjM__SetupSceneStyle: `(self, GetStageGridDimensions(
     * self->unk38))`. */
    void (*slotE0)(void *self, void *arg1); /* +0x0E0 */
    u8 pad0E4[0x0EC - 0x0E4];
    void (*slotEC)(void *self);                        /* +0x0EC, ObjM__EnterStyleSession */
    u8 padF0[0x10C - 0x0F0];
    void *(*slot10C)(void *self, s32 arg1, s32 arg2); /* +0x10C, ObjM__TickStyle */
    u8 pad110[0x134 - 0x110];
    void (*slot134)(void *self, void *arg1);           /* +0x134, ObjM__InitStyleAndWorld */
} Obj14Methods_3bb8c_l;
typedef struct Obj14_3bb8c_l {
    Obj14Methods_3bb8c_l *methods; /* +0x000 */
    u8 pad04[0x1B4 - 0x004];
    u16 unk1B4;                     /* +0x1B4, ObjM__TransferToOther */
} Obj14_3bb8c_l;

/* What self->world points to (and what its +0x0AC returns): an
 * unidentified class dispatched through style/world-setup slots. Until
 * round 88 this type was also self->target's, as DreamSysObj_3bb8c_l; the
 * target is the game's DreamSys (track 4: its +0x04C/+0x050/+0x0EC/+0x0F0/
 * +0x0F8/+0x0FC/+0x104/+0x108/+0x1A0/+0x200 calls are DreamSys occupants,
 * and +0x044/+0x164 are Actor's state and DreamSys's currentStage), and
 * those slots left with it. Nothing ties the world to DreamSys. */
typedef struct StyleWorldMethods_3bb8c_l {
    u8 pad00[0x054];
    /* +0x054, round 45's ObjM__SetupSceneStyle: `(self, val)`, `val` a small
     * derived integer (`(*obj->methods->slot7C(obj, 0)) / 2 * 5 / 3 +
     * D_8008AB34`, `obj` being `*(void **)self->unkC`). */
    void (*slot54)(void *self, s32 arg1); /* +0x054 */
    u8 pad58[0x060 - 0x058];
    void (*slot60)(void *self, s32 arg1);          /* +0x060, ObjM__EnterStyleSession */
    void (*slot64)(void *self, void *arg1);        /* +0x064, ObjM__EnterStyleSession */
    void (*slot68)(void *self, void *arg1);        /* +0x068, ObjM__EnterStyleSession */
    void (*slot6C)(void *self, void *arg1);        /* +0x06C, ObjM__EnterStyleSession */
    void (*slot70)(void *self, void *arg1, void *arg2, void *arg3, s32 arg4); /* +0x070, ObjM__InitStyleAndWorld: (target, &D_8008715C, &D_80087168, 0) */
    void (*slot74)(void *self);          /* +0x074, ObjM__ExitSceneStyle */
    u8 pad78[0x0AC - 0x078];
    /* Returns another object of this view, dispatched through slotF0/slotD4
     * (elaborated tag: the typedef is not in scope yet). */
    struct StyleWorldObj_3bb8c_l *(*slotAC)(void *self);    /* +0x0AC, ObjM__EnterStyleSession */
    void (*slotB0)(void *self, s32 arg1);          /* +0x0B0, ObjM__EnterStyleSession */
    void (*slotB4)(void *self, s32 arg1);          /* +0x0B4, ObjM__EnterStyleSession */
    u8 padB8[0x0D4 - 0x0B8];
    void (*slotD4)(void *self, s32 arg1, s32 arg2, s32 arg3); /* +0x0D4, ObjM__EnterStyleSession (on slotAC's result) */
    u8 padD8[0x0F0 - 0x0D8];
    s32 (*slotF0)(void *self, s32 *outBuf, s32 arg2); /* +0x0F0, ObjM__EnterStyleSession (on slotAC's result, as (s32 *)ret, 0 or 3) */
} StyleWorldMethods_3bb8c_l;
typedef struct StyleWorldObj_3bb8c_l {
    StyleWorldMethods_3bb8c_l *methods;
} StyleWorldObj_3bb8c_l;

/* Whatever arg1->unkC points to in ObjM__AttachTarget -- a registration sink
 * of some kind (arg1->unkC->methods->slotC8(arg1->unkC, callback,
 * userdata) reads like "subscribe `callback` for `userdata`"). Only the
 * one slot this unit calls through is named. */
typedef struct RegistrantMethods_3bb8c_l {
    u8 pad00[0x0C8];
    void (*slotC8)(void *self, void (*callback)(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3), Obj87034_3bb8c_l *userdata); /* +0x0C8, ObjM__AttachTarget */
} RegistrantMethods_3bb8c_l;
typedef struct RegistrantObj_3bb8c_l {
    RegistrantMethods_3bb8c_l *methods;
} RegistrantObj_3bb8c_l;

typedef struct Obj87034Methods_3bb8c_l {
    s32 header;                                                    /* +0x000 */
    void (*slot04)(Obj87034_3bb8c_l *self);                        /* +0x004, BasicClass generic (func_80017EB0); dispatched directly by ObjM__TransferToOther on its `other` argument */
    u8 pad08[0x010 - 0x008];
    void (*slot10)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x010, ObjM__AttachTarget */
    void (*slot14)(Obj87034_3bb8c_l *self, void *arg1);            /* +0x014, ObjM__DetachTarget/ObjM__ExitSceneStyle */
    u8 pad18[0x030 - 0x018];
    void (*slot30)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x030, ObjM__EnterState4 (STALLED 28/71 -- offset/signature observed directly from the disassembly, reliable independent of the stall; see docs/match-reports/ObjM__EnterState4.md) */
    u8 pad34[0x048 - 0x034];
    void (*slot48)(Obj87034_3bb8c_l *self);                        /* +0x048, ObjM__DetachTarget/ObjM__TeardownStyle (via self->unk54) */
    u8 pad4C[0x05C - 0x04C];
    void (*slot5C)(Obj87034_3bb8c_l *self, s32 arg1);              /* +0x05C, ObjM__InitStyleAndWorld (arg1 is PickVariant's return value, forwarded opaquely) */
    u8 pad60[0x074 - 0x060];
    void (*slot74)(Obj87034_3bb8c_l *self);                        /* +0x074, ObjM__DispatchEvent (event 0x21) */
    u8 pad78[0x07C - 0x078];
    void (*slot7C)(Obj87034_3bb8c_l *self, void *arg1);            /* +0x07C, ObjM__TransferToOther */
    void (*slot80)(Obj87034_3bb8c_l *self);                        /* +0x080, ObjM__TransferToOther */
    void (*slot84)(Obj87034_3bb8c_l *self);                        /* +0x084, ObjM__TeardownStyle */
    void (*slot88)(Obj87034_3bb8c_l *self);                        /* +0x088, ObjM__TransferToOther */
    void (*slot8C)(Obj87034_3bb8c_l *self);                        /* +0x08C, ObjM__TickTarget */
    u8 pad90[0x094 - 0x090];
    void (*slot94)(Obj87034_3bb8c_l *self);                        /* +0x094, ObjM__HandleStateCode (event/code 0xA, dense switch) */
    void (*slot98)(Obj87034_3bb8c_l *self);                        /* +0x098, ObjM__HandleStateCode (event/code 0xC) */
    void (*slot9C)(Obj87034_3bb8c_l *self);                        /* +0x09C, ObjM__EnterState5; ALSO ObjM__HandleStateCode (event/code 0xD) */
    void (*slotA0)(Obj87034_3bb8c_l *self);                        /* +0x0A0, ObjM__HandleStateCode (event/code 0xE) */
    void (*slotA4)(Obj87034_3bb8c_l *self);                        /* +0x0A4, ObjM__HandleStateCode (event/code 0xF) */
    void (*slotA8)(Obj87034_3bb8c_l *self);                        /* +0x0A8, ObjM__HandleStateCode (event/code 0x10) */
    void (*slotAC)(Obj87034_3bb8c_l *self);                        /* +0x0AC, ObjM__HandleStateCode (event/code 0x11) */
    u8 padB0[0x0C0 - 0x0B0];
    void (*slotC0)(Obj87034_3bb8c_l *self);                        /* +0x0C0, ObjM__DispatchEvent (event 0xC) */
    void (*slotC4)(Obj87034_3bb8c_l *self);                        /* +0x0C4, ObjM__DispatchEvent (event 0x2C)/ObjM__DispatchActiveState */
    void (*slotC8)(Obj87034_3bb8c_l *self);                        /* +0x0C8, ObjM__DispatchEvent (event 0x16) */
    u8 padCC[0x0D0 - 0x0CC];
    void (*slotD0)(Obj87034_3bb8c_l *self);                        /* +0x0D0, ObjM__TickTarget/ObjM__DispatchActiveState */
    void (*slotD4)(Obj87034_3bb8c_l *self);                        /* +0x0D4, ObjM__ExitSceneStyle/ObjM__DispatchActiveState */
} Obj87034Methods_3bb8c_l;

struct Obj87034_3bb8c_l {
    Obj87034Methods_3bb8c_l *methods; /* +0x000 */
    u8 pad04[0x00C - 0x004];
    RegistrantObj_3bb8c_l *unkC;      /* +0x00C, ObjM__AttachTarget's `arg1->unkC` */
    s32 unk10;                        /* +0x010, ObjM__EnterStyleSession */
    Obj14_3bb8c_l *unk14;              /* +0x014, ObjM__TransferToOther/ObjM__TickStyle/ObjM__ExitSceneStyle */
    StyleWorldObj_3bb8c_l *world;         /* +0x018, ObjM__ExitSceneStyle; cached into `cachedWorld` by ObjM__InitStyleAndWorld -- an object distinct from `target`, dispatched through style/world-setup slots (slot74/slotAC/slot60/slot64/slot6C/slot68/slotB0/slotB4) */
    s32 unk1C;                           /* +0x01C, ObjM__TickTarget: incremented once per call */
    s32 phase;                            /* +0x020, ObjM__EnterState6: written 6 (a state/phase tag; also written 4 by ObjM__EnterState4 and 5 by ObjM__EnterState5; read by ObjM__HandleStateCode, which is 0-gated) */
    u8 pad24[0x034 - 0x024];
    s32 unk34;                            /* +0x034, round 45's ObjM__SetupSceneStyle: forwarded opaquely to SetDreamAuxWorld's own arg3 */
    void *unk38;                          /* +0x038, ObjM__OnRegistrantEvent: forwarded opaquely to GetGridRecordAt/GetGridRecordXY */
    struct DreamSys *target;            /* +0x03C, many functions in this unit: the game's DreamSys (include/DreamSys.h), which SetDreamAuxWorld installs as gDreamAuxWorld */
    s32 unk40;                             /* +0x040, ObjM__EnterStyleSession */
    s32 unk44;                              /* +0x044, ObjM__EnterStyleSession */
    s32 unk48;                               /* +0x048, ObjM__InitStyleAndWorld: set from arg1, or 0xA000 if arg1==0 */
    s32 unk4C;                                /* +0x04C, ObjM__InitStyleAndWorld: set from arg3 (only when self->unk38 != 0) */
    Unk50Struct_3bb8c_l *styleConfig;             /* +0x050, ObjM__TransferToOther; set from RegisterStyleConfig's return in ObjM__InitStyleAndWorld (or an explicit `arg2` override) */
    Obj87034_3bb8c_l *unk54;                 /* +0x054, ObjM__TeardownStyle */
    Obj87034_3bb8c_l *pendingOther;                  /* +0x058, ObjM__OnSelectTransfer: forwarded as ObjM__TransferToOther's `other` */
    u8 pad5C[0x060 - 0x05C];
    s32 hasTarget;                                 /* +0x060, ObjM__TransferToOther: has-a-target gate, cleared after detaching */
    s32 unk64;                                  /* +0x064, ObjM__TransferToOther: set to 1 */
    s32 attached;                                   /* +0x068, ObjM__TransferToOther/ObjM__DispatchEvent/ObjM__TickTarget: zero-checked gate */
    s32 unk6C;                                    /* +0x06C, ObjM__InitStyleAndWorld: out-parameter address passed to RegisterStyleConfig, own type unknown */
    u8 pad70[0x078 - 0x070];
    StyleWorldObj_3bb8c_l *cachedWorld;                    /* +0x078, ObjM__InitStyleAndWorld: cached copy of self->world */
    u8 pad7C[0x080 - 0x07C];
    s32 unk80;                                    /* +0x080, ObjM__TransferToOther (on `other`)/ObjM__TickTarget/ObjM__DispatchActiveState: zero-checked gate */
};

/* ObjM's parent class, Class86668 (gClass86668Methods), is declared in
 * include/Class86668.h (track 4, round 84); class_3bb8c_k and class_3bb8c_l
 * include it for their base-table calls. The unit-local
 * `BaseMethods87034_3bb8c_l` view the round-15 head note here described is
 * gone. */

/* ObjM__AttachTarget's own registered callback -- forward-declared here since
 * ObjM__AttachTarget (ROM order earlier) takes its address before its own
 * definition (ROM order later) is reached. */
extern void ObjM__OnRegistrantEvent(Obj87034_3bb8c_l *self, s32 code, s32 arg2, s32 arg3);

/* ObjM__OnSelectTransfer's tail call -- forward-declared for the same ROM-order
 * reason as ObjM__OnRegistrantEvent above (ObjM__TransferToOther is defined later). */
extern void ObjM__TransferToOther(Obj87034_3bb8c_l *self, Obj87034_3bb8c_l *other);

/* ObjM__OnRegistrantEvent's own two helpers -- MATCHED, src/code_39094.c.
 * GetGridRecordAt(index, sub) reads both $a0 and $a1. ObjM__OnRegistrantEvent's
 * `code >= 0` branch leaves its own incoming `code` in $a1 at the jal (no
 * write to $a1 before it), so `code` IS the second argument: a non-negative
 * code is a linear cell index, a negative one sends x/y to GetGridRecordXY.
 * (Was declared K&R/unprototyped and called with one argument until round
 * 82's externcheck pass; the forwarded form is byte-identical.) */
extern s32 GetGridRecordAt(void *arg0, s32 sub);
extern void GetGridRecordXY(void *arg0, s32 arg1, s32 arg2);

/* ObjM__TickStyle's own helper -- MATCHED, src/class_3bb8c_n.c. Typed purely
 * from this call site's own register usage. */
extern void TickStyle(void *arg0, void *arg1, s32 arg2);

/* ObjM__TeardownStyle's own helpers -- MATCHED, src/class_3bb8c_n.c (StyleTeardown)
 * and src/code_4cd08.c (TickDreamAuxSlots2), both matched, called with no arguments
 * and their return values unused. */
extern void TickDreamAuxSlots2(void);
extern void StyleTeardown(void);


/*
 * TextEntry (gTextEntryMethods, class_3bb8c_i/j) is declared in
 * include/TextEntry.h (FINISHING-PLAN track 4, round 87). What stays here are
 * two helper views of objects it HOLDS, which are not its class: the
 * resources `cursorSprite`/`textRow`/`panelSprite` and the TIM handles
 * (ChildObj86ED0), and `target` (TargetObj86ED0). The names are kept.
 */
/* TextEntry's `textRow` (a TextRow, include/TextRow.h): a view of the slots
 * its calls use, not one class. The TIM handles TextEntry__LoadCardResources
 * and TaskObjF__LoadCardIcon load through New_TimImage used this view too
 * until round 88; they are `TimImage *` now (include/TimImage.h). */
typedef struct ChildObj86ED0 ChildObj86ED0;
typedef struct ChildMethods86ED0 ChildMethods86ED0;
struct ChildMethods86ED0 {
    u8 pad000[0x004];
    void *(*release)(ChildObj86ED0 *self); /* +0x004, TextEntry__ReleaseCardResources */
    u8 pad008[0x04C - 0x008];
    /* +0x04C, textRow: (parent, &D_8008AAD4), the attachToParent shape
     * ScreenSprite's slot has (include/ScreenSprite.h). */
    void (*slot4C)(ChildObj86ED0 *self, void *arg1, void *arg2);
    u8 pad050[0x078 - 0x050];
    void (*slot78)(ChildObj86ED0 *self); /* +0x078; no accessor since round 88 (it was the TIM handles' TimImage__Upload) */
    u8 pad07C[0x0B8 - 0x07C];
    void (*slotB8)(ChildObj86ED0 *self, void *arg1); /* +0x0B8, TextEntry__LoadCardResources, textRow */
    u8 pad0BC[0x0C4 - 0x0BC];
    void (*slotC4)(ChildObj86ED0 *self, s32 arg1, s32 arg2); /* +0x0C4, TextEntry__SetCharAt, textRow: (char, pos); TextRow__SetCellAt */
};
struct ChildObj86ED0 {
    ChildMethods86ED0 *methods; /* +0x000 */
};

/* TextEntry's `target` (+0x03C) -- an unrelated class (own vtable, unconnected to
 * gTextEntryMethods; TaskObjF passes its childC), reached only through its own +0x080 slot by TextEntry__NotifyTarget.
 * Field meaning beyond that slot is unestablished. */
typedef struct TargetObj86ED0 TargetObj86ED0;
typedef struct TargetMethods86ED0 TargetMethods86ED0;
struct TargetMethods86ED0 {
    u8 pad000[0x080];
    /* +0x080, TextEntry__NotifyTarget: `self->methods->slot80(self, arg1, 0x60, 0x60)`.
     * 3 args, not 2 -- confirmed against this project's established
     * self->methods->slot80(self, arg1, 0x60, 0x60) idiom seen at several
     * other call sites (src/class_3bb8c_k.c, src/code_2cc8c.c,
     * src/class_3bb8c_g.c, src/code_55dd4.c), all forwarding a caller-
     * supplied arg1 alongside a repeated literal. TextEntry__NotifyTarget itself
     * takes that arg1 as its own second parameter and forwards it
     * unchanged (same register, no move instruction). */
    void (*slot80)(TargetObj86ED0 *self, s32 arg1, s32 arg2, s32 arg3);
};
struct TargetObj86ED0 {
    TargetMethods86ED0 *methods; /* +0x000 */
};

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

/* Class86F88 (gClass86F88Methods, the 0x54-byte list selector whose methods
 * are in class_3bb8c_j and class_3bb8c_k) is include/Class86F88.h (track 4,
 * round 89). */

/* GetObjMMethods: a plain class-vtable getter (`lui`/`addiu`, no
 * `lw`/`sw`), returns `&D_80087034` verbatim. Confirmed a BasicClass-
 * derived vtable with `tools/classtable.py 0x80087034` (header word then
 * `BasicClass__Release` at +4, the class-framework fingerprint) --
 * nothing in this unit dereferences it, so it stays untyped beyond the
 * address itself, same convention as `sDefaultTargetSpecs` above. */
extern s32 D_80087034;
extern void *GetObjMMethods(void);

#endif
