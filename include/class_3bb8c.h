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

/* Only the slots this unit's functions dispatch through (via
 * self->methods->slotNN) are typed; everything else stays opaque so the
 * struct keeps the right size/offsets without requiring every method to be
 * typed up front (same policy as include/class_39e08.h). */
typedef struct Obj866E8Methods {
    u8 pad00[0xC0];
    /* Called by func_8004B57C right before it zeroes self->unk70. */
    void (*slotC0)(Obj866E8 *self);            /* +0x0C0 */
} Obj866E8Methods;

/* self+0xEC: a 7-element array, each element 0x1C bytes. Established by
 * func_8004BCE0, which reads each element's own +0x000 (u16 flag,
 * nonzero-ness only). Nothing else about an element's shape is known yet. */
typedef struct Elem {
    u16 flag;                      /* +0x000 */
    u8 pad02[0x1C - 0x02];
} Elem;

struct Obj866E8 {
    Obj866E8Methods *methods;      /* +0x000 */
    u8 pad04[0x70 - 0x04];
    s32 unk70;                     /* +0x070, func_8004B570/func_8004B57C */
    u8 pad74[0xEC - 0x74];
    Elem arr[7];                   /* +0x0EC, func_8004BCE0 */
};

#endif
