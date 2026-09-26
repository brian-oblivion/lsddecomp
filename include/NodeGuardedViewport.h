#ifndef NODEGUARDEDVIEWPORT_H
#define NODEGUARDEDVIEWPORT_H

#include "Viewport.h"

/*
 * NodeGuardedViewport -- class id 0x17, method table gNodeGuardedViewportMethods: Viewport's
 * subclass (its ctor chains to GetViewportMethods()->ctor first, so the id
 * tree 0x7 -> 0x17 is the ctor chain). Methods in src/class_3bb8c_c.c. No
 * class derives from it. Its one construction site is Class865C8__Class865C8
 * (src/class_39e08.c), which stores it as the viewport of an
 * IntermediateBase init argument block (IntermediateBaseInitArgs +0x010;
 * which class_39e08.h once viewed as Obj0C::unk10); IntermediateBase reaches it only through
 * Viewport's slots.
 *
 * What it changes, from its own methods (`classtable.py gNodeGuardedViewportMethods
 * --vs gViewportMethods`):
 *  - +0x040 initDefaults is empty (NodeGuardedViewport__InitDefaults), so the
 *    Viewport ctor's closing initDefaults call writes none of Viewport's
 *    defaults for this class;
 *  - +0x09C update (NodeGuardedViewport__Update) calls Viewport__Update only when
 *    viewNode and otReady are both set. Viewport__Update checks otReady
 *    itself but dereferences viewNode unguarded, so the override's addition
 *    is the NULL viewNode guard;
 *  - four own slots, +0x0B8..+0x0C4, hold empty functions with no known
 *    caller.
 * That is not enough to say what the viewport is for in the game, so the
 * name stays the table's address.
 *
 * The object is 0xDC bytes (New_NodeGuardedViewport); nothing reads or writes the
 * own range +0x0BC..+0x0DC in any C or carved asm that types it.
 */

typedef struct NodeGuardedViewport NodeGuardedViewport;
typedef struct NodeGuardedViewportMethods NodeGuardedViewportMethods;

/* Viewport's slots, then this class's own. The overrides of inherited
 * slots are the ctor, initDefaults and update (see the banner). */
/* clang-format off */
#define NODEGUARDEDVIEWPORT_SLOTS(Self, CtorParams)                                                         \
    VIEWPORT_SLOTS(Self, CtorParams);                                                              \
    /* +0x0B8 */ void (*slotB8)(void); /* NodeGuardedViewport__NoOpSlotB8, empty; no known caller */                 \
    /* +0x0BC */ void (*slotBC)(void); /* func_8004D364, empty; no known caller */                 \
    /* +0x0C0 */ void (*slotC0)(void); /* func_8004D36C, empty; no known caller */                 \
    /* +0x0C4 */ void (*slotC4)(void)  /* func_8004D374, empty; no known caller */
/* clang-format on */

/* clang-format off */
#define NODEGUARDEDVIEWPORT_FIELDS(Methods)                                                                 \
    VIEWPORT_FIELDS(Methods);                                                                      \
    /* +0x0BC */ u8 pad0BC[0x0DC - 0x0BC] /* no accessor; the object is 0xDC bytes (New_NodeGuardedViewport) */
/* clang-format on */

struct NodeGuardedViewportMethods {
    NODEGUARDEDVIEWPORT_SLOTS(NodeGuardedViewport, (NodeGuardedViewport * self));
};

struct NodeGuardedViewport {
    NODEGUARDEDVIEWPORT_FIELDS(NodeGuardedViewportMethods);
};

extern NodeGuardedViewportMethods gNodeGuardedViewportMethods;
extern NodeGuardedViewportMethods *GetNodeGuardedViewportMethods(void); /* returns &gNodeGuardedViewportMethods */

/* The class's own methods, in address order. */
NodeGuardedViewport *New_NodeGuardedViewport(void); /* BMemPMgrAlloc(0xDC), then ctor */
void NodeGuardedViewport__NodeGuardedViewport(NodeGuardedViewport *self);
void NodeGuardedViewport__InitDefaults(void); /* +0x040; empty, reads no argument */
void NodeGuardedViewport__Update(NodeGuardedViewport *self);
void NodeGuardedViewport__NoOpSlotB8(void);
void func_8004D364(void);
void func_8004D36C(void);
void func_8004D374(void);

#endif
