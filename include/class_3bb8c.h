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

/* Uncarved helper in this same unit (asm/class_3bb8c.s past this slice),
 * called only by func_8004B418. Declared locally with the minimal
 * signature that call site demonstrates; not this round's function to
 * match. Returns a plain scalar (its own body computes a value into $v0
 * with ordinary integer ops, no pointer arithmetic on the result).
 *
 * Parameter types were tightened this round (all pointer-type-only
 * changes, so this does not disturb func_8004B418's already-matched
 * bytes): `arg2` is `self->unk68` at the one known call site (Unk68Struct
 * *, see above); `arg4` is the 5th/stack argument (Descriptor10 *, see
 * above); `outBuf` is fed a 3-word local array by that same call site, so
 * it is typed `s32 *` rather than opaque `void *`. */
extern s32 func_8004B44C(void *arg0, s32 *outBuf, Unk68Struct *arg2, Unk54Struct *arg3, Descriptor10 *arg4);

/* Unidentified global, address-only use (func_8004B38C passes `&D_80086904`
 * as an argument, never reads it directly here). Typed `s32` purely as a
 * placeholder since only its address is taken. */
extern s32 D_80086904;

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
    u8 pad0FC[0x104 - 0xFC];
    /* Called by func_8004BD14 with one of the object's own Elem array
     * slots. */
    void (*slot104)(Obj866E8 *self, Elem *entry);  /* +0x104 */
    u8 pad108[0x110 - 0x108];
    /* Called by func_8004C158 with its own arg1 and a computed pointer;
     * matches class_3ac78's independent view of the same slot
     * (`slot110`/func_8004C1C0, s32 return, not this round's function). */
    s32 (*slot110)(Obj866E8 *self, s32 arg1, void *arg2);   /* +0x110 */
    u8 pad114[0x118 - 0x114];
    /* Called by func_8004C470 with a plain array index (0..6); the
     * returned pointer is subsequently read like an Elem slot accessor,
     * so this is almost certainly `return &self->arr[index];` -- not
     * this round's function to match. */
    Elem *(*slot118)(Obj866E8 *self, s32 index);   /* +0x118 */
} Obj866E8Methods;

/*
 * Opaque object pointed to by ElemTarget::unk4::unk14 (func_8004C470).
 * Only the two s32 fields that function reads are typed.
 */
struct Unk14Obj {
    u8 pad00[0x18];
    s32 unk18;
    u8 pad1C[0x20 - 0x1C];
    s32 unk20;
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
    u8 pad000[0x7C];
    /* Called by func_8004C0AC as `entry->unk4->methods->slot7C(entry->unk4,
     * entry)` -- dispatch target is the ElemTarget itself, not self. */
    void (*slot7C)(ElemTarget *self, Elem *entry);  /* +0x07C */
};

struct ElemTarget {
    ElemTargetMethods *methods;    /* +0x000, func_8004C0AC */
    u8 pad004[0x02A - 0x004];
    u16 unk2A;                     /* +0x02A, func_8004BD14 */
    s16 unk2C;                     /* +0x02C, func_8004BD14 */
    s16 unk2E;                     /* +0x02E, func_8004BD14 */
    s16 unk30;                     /* +0x030, func_8004C0AC/func_8004C3F0 */
    s16 unk32;                     /* +0x032, func_8004C434 */
};

/*
 * self+0xEC's array element, one of Obj866E8::arr[7]. Established from
 * func_8004BCE0 (+0x000), func_8004C434 (+0x004), and this round's
 * func_8004C0AC (+0x00C indirectly via unk4, +0x010) and func_8004BD14
 * (+0x000, +0x004).
 */
struct Elem {
    u16 flag;                      /* +0x000 */
    u8 pad02[0x04 - 0x02];
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
 * Opaque target of Elem::unk10[i]. func_8004C0AC OR's a flag bit into
 * +0x010 and zeroes +0x018 and +0x020; nothing else about it is known.
 */
struct EntryChildObj {
    u8 pad00[0x10];
    u32 unk10;                      /* +0x010, func_8004C0AC: OR'd with 0x80000000 */
    u8 pad14[0x18 - 0x14];
    s32 unk18;                      /* +0x018, func_8004C0AC: zeroed */
    u8 pad1C[0x20 - 0x1C];
    s32 unk20;                      /* +0x020, func_8004C0AC: zeroed */
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

struct Obj866E8 {
    Obj866E8Methods *methods;      /* +0x000 */
    u8 pad04[0x54 - 0x04];
    Unk54Struct unk54;             /* +0x054, func_8004B418 (address taken, forwarded opaquely) */
    u8 pad60[0x68 - 0x60];
    Unk68Struct *unk68;            /* +0x068, func_8004B418/func_8004B38C/func_8004B930/func_8004C470 */
    Unk6CObj *unk6C;               /* +0x06C, func_8004B38C stores it raw; func_8004C158 dereferences it */
    s32 unk70;                     /* +0x070, func_8004B570/func_8004B57C */
    u8 pad74[0xBC - 0x74];
    Descriptor10 unkBC;            /* +0x0BC, func_8004B38C: whole-struct copy from its arg3 */
    u8 padC6[0xEC - 0xC6];
    Elem arr[7];                   /* +0x0EC, func_8004BCE0/func_8004C434/func_8004BD14 */
    s32 unk1B0;                    /* +0x1B0, func_8004BD14 */
    u16 unk1B4;                    /* +0x1B4, func_8004BD14 */
    u8 pad1B6[0x1B8 - 0x1B6];
    s32 unk1B8;                    /* +0x1B8, func_8004BD14 */
    Unk1BCObj *unk1BC;              /* +0x1BC, func_8004C3F0 */
};

#endif
