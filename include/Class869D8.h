#ifndef CLASS869D8_H
#define CLASS869D8_H

#include "Viewport.h"

/*
 * Class869D8 -- class id 0x17, method table gClass869D8Methods: Viewport's
 * subclass (its ctor chains to GetViewportMethods()->ctor first, so the id
 * tree 0x7 -> 0x17 is the ctor chain). Methods in src/class_3bb8c_c.c. No
 * class derives from it. Its one construction site is Class865C8__Class865C8
 * (src/class_39e08.c), which stores it as the viewport of an
 * IntermediateBase init argument block (IntermediateBaseInitArgs +0x010;
 * which class_39e08.h once viewed as Obj0C::unk10); IntermediateBase reaches it only through
 * Viewport's slots.
 *
 * What it changes, from its own methods (`classtable.py gClass869D8Methods
 * --vs gViewportMethods`):
 *  - +0x040 initDefaults is empty (Class869D8__InitDefaults), so the
 *    Viewport ctor's closing initDefaults call writes none of Viewport's
 *    defaults for this class;
 *  - +0x09C update (Class869D8__Update) calls Viewport__Update only when
 *    viewNode and otReady are both set. Viewport__Update checks otReady
 *    itself but dereferences viewNode unguarded, so the override's addition
 *    is the NULL viewNode guard;
 *  - four own slots, +0x0B8..+0x0C4, hold empty functions with no known
 *    caller.
 * That is not enough to say what the viewport is for in the game, so the
 * name stays the table's address.
 *
 * The object is 0xDC bytes (New_Class869D8); nothing reads or writes the
 * own range +0x0BC..+0x0DC in any C or carved asm that types it.
 */

typedef struct Class869D8 Class869D8;
typedef struct Class869D8Methods Class869D8Methods;

/* Viewport's slots, then this class's own. The overrides of inherited
 * slots are the ctor, initDefaults and update (see the banner). */
/* clang-format off */
#define CLASS869D8_SLOTS(Self, CtorParams)                                                         \
    VIEWPORT_SLOTS(Self, CtorParams);                                                              \
    /* +0x0B8 */ void (*slotB8)(void); /* func_8004D35C, empty; no known caller */                 \
    /* +0x0BC */ void (*slotBC)(void); /* func_8004D364, empty; no known caller */                 \
    /* +0x0C0 */ void (*slotC0)(void); /* func_8004D36C, empty; no known caller */                 \
    /* +0x0C4 */ void (*slotC4)(void)  /* func_8004D374, empty; no known caller */
/* clang-format on */

/* clang-format off */
#define CLASS869D8_FIELDS(Methods)                                                                 \
    VIEWPORT_FIELDS(Methods);                                                                      \
    /* +0x0BC */ u8 pad0BC[0x0DC - 0x0BC] /* no accessor; the object is 0xDC bytes (New_Class869D8) */
/* clang-format on */

struct Class869D8Methods {
    CLASS869D8_SLOTS(Class869D8, (Class869D8 * self));
};

struct Class869D8 {
    CLASS869D8_FIELDS(Class869D8Methods);
};

extern Class869D8Methods gClass869D8Methods;
extern Class869D8Methods *GetClass869D8Methods(void); /* returns &gClass869D8Methods */

/* The class's own methods, in address order. */
Class869D8 *New_Class869D8(void); /* BMemPMgrAlloc(0xDC), then ctor */
void Class869D8__Class869D8(Class869D8 *self);
void Class869D8__InitDefaults(void); /* +0x040; empty, reads no argument */
void Class869D8__Update(Class869D8 *self);
void func_8004D35C(void);
void func_8004D364(void);
void func_8004D36C(void);
void func_8004D374(void);

#endif
