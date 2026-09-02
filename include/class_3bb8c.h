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

/* self+0xEC: a 7-element array, each element 0x1C bytes. Established from
 * TWO independent functions:
 *  - func_8004BCE0 reads each element's own +0x000 (u16 flag, nonzero-
 *    ness only).
 *  - func_8004C434 reads each element's own +0x004 (a pointer), then
 *    dereferences THAT pointer's +0x032 (s16) to compare against a search
 *    key.
 * Nothing else about an element's shape is known yet. */
typedef struct Elem {
    u16 flag;                      /* +0x000 */
    u8 pad02[0x04 - 0x02];
    ElemTarget *unk4;              /* +0x004 */
    u8 pad08[0x1C - 0x08];
} Elem;

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
};

/* Uncarved helper in this same unit (asm/class_3bb8c.s past this slice),
 * called only by func_8004B418. Declared locally with the minimal
 * signature that call site demonstrates; not this round's function to
 * match. Returns a plain scalar (its own body computes a value into $v0
 * with ordinary integer ops, no pointer arithmetic on the result). */
extern s32 func_8004B44C(void *arg0, void *outBuf, void *arg2, Unk54Struct *arg3, void *arg4);

#endif
