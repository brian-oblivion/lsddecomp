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

/* self+0x54: an inline (not pointer) 3-word sub-struct, dereferenced by
 * func_8004B44C (uncarved helper in this same unit, not this round's own
 * target -- called by func_8004B418). Field meaning unknown beyond "3
 * words, read/written as a group". */
typedef struct Unk54Struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Unk54Struct;

/* Only the slots this unit's functions dispatch through (via
 * self->methods->slotNN) are typed; everything else stays opaque so the
 * struct keeps the right size/offsets without requiring every method to be
 * typed up front (same policy as include/class_39e08.h). */
typedef struct Obj866E8Methods {
    u8 pad00[0xC0];
    /* Called by func_8004B57C right before it zeroes self->unk70. */
    void (*slotC0)(Obj866E8 *self);            /* +0x0C0 */
} Obj866E8Methods;

typedef struct ElemTarget {
    u8 pad00[0x32];
    s16 unk32;                     /* +0x032, compared by func_8004C434 */
} ElemTarget;

/*
 * Object pointed to by Elem::unk10[i] (func_8004D1D0, class_3bb8c_b). Same
 * real memory as class_3ac78.h's independent `GenericObject`/
 * `GenericMethodsHeader` view of the same array (its own comment: "array of
 * GenericObject*, scanned up to +0x668 bytes") -- named distinctly here per
 * the project's multiple-independent-local-views convention (see e.g.
 * class_3ac78/class_3bb8c both describing D_800866E8). Only the one slot
 * func_8004D0D0/func_8004D108 dispatch through is typed.
 */
typedef struct Unk10ChildObj_3bb8c_b Unk10ChildObj_3bb8c_b;

typedef struct Unk10ChildMethods_3bb8c_b {
    u8 pad00[0x48];
    /* Called by func_8004D0D0 (arg2 = self's own unk1E4) and
     * func_8004D108 (arg2 = &D_800869CC). Return value unused by both. */
    void (*slot48)(Unk10ChildObj_3bb8c_b *self, s32 arg1, void *arg2); /* +0x048 */
} Unk10ChildMethods_3bb8c_b;

struct Unk10ChildObj_3bb8c_b {
    Unk10ChildMethods_3bb8c_b *methods; /* +0x000 */
};

/* self+0xEC: a 7-element array, each element 0x1C bytes. Established from
 * multiple independent functions:
 *  - func_8004BCE0 reads each element's own +0x000 (u16 flag, nonzero-
 *    ness only).
 *  - func_8004C434 reads each element's own +0x004 (a pointer), then
 *    dereferences THAT pointer's +0x032 (s16) to compare against a search
 *    key.
 *  - func_8004D1D0 reads the element's own +0x010 (a pointer to an array
 *    of Unk10ChildObj_3bb8c_b*, walked up to +0x668 bytes -- the SAME
 *    field class_3ac78.h's independent view names `unk10`).
 */
typedef struct Elem {
    u16 flag;                      /* +0x000 */
    u8 pad02[0x04 - 0x02];
    ElemTarget *unk4;              /* +0x004 */
    u8 pad08[0x10 - 0x08];
    Unk10ChildObj_3bb8c_b **unk10; /* +0x010, func_8004D1D0 */
    u8 pad14[0x1C - 0x14];
} Elem;

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

struct Obj866E8 {
    Obj866E8Methods *methods;      /* +0x000 */
    u8 pad04[0x54 - 0x04];
    Unk54Struct unk54;             /* +0x054, func_8004B418 (address taken, forwarded opaquely) */
    u8 pad60[0x68 - 0x60];
    void *unk68;                   /* +0x068, func_8004B418 (forwarded opaquely, never dereferenced here) */
    u8 pad6C[0x70 - 0x6C];
    s32 unk70;                     /* +0x070, func_8004B570/func_8004B57C */
    u8 pad74[0xEC - 0x74];
    Elem arr[7];                   /* +0x0EC, func_8004BCE0/func_8004C434 */
    u8 pad1B0[0x1CC - 0x1B0];
    s32 unk1CC;                    /* +0x1CC, func_8004CFA8 (address-of only, real type unknown) */
    u8 pad1D0[0x1DC - 0x1D0];
    Bounds866E8_3bb8c_b *unk1DC;   /* +0x1DC, func_8004CFB0 (stores raw)/func_8004CD38 (dereferences) */
    u8 pad1E0[0x1E4 - 0x1E0];
    void *unk1E4;                  /* +0x1E4, func_8004D0D0: forwarded opaquely to Unk10ChildMethods_3bb8c_b::slot48 */
};

/* Get-vtable helper, same shape and same real function as
 * class_3ac78.h's `func_8004D244` (independent view: this unit names the
 * return type Obj866E8Methods, not Class866E8Methods). It now has a real
 * body in this unit (class_3bb8c_b); class_3ac78 still calls it via `jal`
 * as a raw external. */
extern Obj866E8Methods D_800866E8;

/* Still raw asm in this unit (not this round's target): walks
 * item->unk10[] (an array of Unk10ChildObj_3bb8c_b*, up to +0x668 bytes
 * from the base read at item->unk10), calling callback(self, element) for
 * each. Derived to resolve func_8004D0D0/func_8004D108's true call site --
 * see those functions' reports. Not called by name anywhere in this
 * unit's own C (only from within func_8004D140's still-raw body), so this
 * prototype is documentation, not load-bearing. */
extern void func_8004D1D0(Obj866E8 *self, void (*callback)(Obj866E8 *self, Unk10ChildObj_3bb8c_b *item), Elem *item);

/* 3-word (12-byte) data block, address-of only -- passed to
 * Unk10ChildMethods_3bb8c_b::slot48 as an opaque arg2 by func_8004D108.
 * Never dereferenced in this unit, so left untyped in size only. */
extern s32 D_800869CC[3];

/* Uncarved helper in this same unit (asm/class_3bb8c.s past this slice),
 * called only by func_8004B418. Declared locally with the minimal
 * signature that call site demonstrates; not this round's function to
 * match. Returns a plain scalar (its own body computes a value into $v0
 * with ordinary integer ops, no pointer arithmetic on the result). */
extern s32 func_8004B44C(void *arg0, void *outBuf, void *arg2, Unk54Struct *arg3, void *arg4);

#endif
