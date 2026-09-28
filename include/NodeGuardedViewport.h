#ifndef NODEGUARDEDVIEWPORT_H
#define NODEGUARDEDVIEWPORT_H

#include "Viewport.h"

/*
 * NodeGuardedViewport -- a Viewport that skips its frame while no view node
 * is attached (class id 0x17, method table gNodeGuardedViewportMethods). Its
 * ctor chains to Viewport's (GetViewportMethods()->ctor) first. Methods in
 * src/TitleMenuTaskObjF.c. No class derives from it.
 *
 * What it changes (`classtable.py gNodeGuardedViewportMethods --vs
 * gViewportMethods`):
 *  - update (+0x09C, NodeGuardedViewport__Update) calls Viewport__Update
 *    only when viewNode and otReady are both set. Viewport__Update checks
 *    otReady itself but dereferences viewNode and refView.super, which
 *    RemoveChild clears, so the override is what lets the viewport run
 *    detached;
 *  - initDefaults (+0x040) is empty. The Viewport ctor runs under
 *    Viewport's own table, so Viewport__InitDefaults has already written
 *    every default by the time this ctor installs its table and calls
 *    initDefaults again; the override makes that second call a no-op;
 *  - four own slots, +0x0B8..+0x0C4, hold empty functions nothing calls.
 *
 * Lifecycle: DayTask__DayTask (src/DayTaskStageMap.c), the one
 * construction site, builds it as its IntermediateBaseInitArgs' viewport.
 * DayTask__Init hands it to the DreamSys (setViewport), and ObjM, built
 * from the same init args, drives it through Viewport's slots: InitStyleAndWorld
 * and SetupSceneStyle detach and re-attach the DreamSys as the view node,
 * EnterStyleSession sets its light mode, clear colour, fog and far colour and
 * enables drawing, and ExitSceneStyle detaches the view node and leaves it
 * detached -- the state the update guard covers.
 *
 * The object is 0xDC bytes (New_NodeGuardedViewport); nothing reads or
 * writes the own range +0x0BC..+0x0DC.
 */

typedef struct NodeGuardedViewport NodeGuardedViewport;
typedef struct NodeGuardedViewportMethods NodeGuardedViewportMethods;

/* Viewport's slots, then this class's own. The overrides of inherited
 * slots are the ctor, initDefaults and update (see the banner). */
/* clang-format off */
#define NODEGUARDEDVIEWPORT_SLOTS(Self, CtorParams)                                                   \
    VIEWPORT_SLOTS(Self, CtorParams);                                                                 \
    /* +0x0B8 */ void (*slotB8)(void); /* NodeGuardedViewport__NoOpSlotB8, empty; nothing calls it */ \
    /* +0x0BC */ void (*slotBC)(void); /* NodeGuardedViewport__NoOpSlotBC, empty; nothing calls it */ \
    /* +0x0C0 */ void (*slotC0)(void); /* NodeGuardedViewport__NoOpSlotC0, empty; nothing calls it */ \
    /* +0x0C4 */ void (*slotC4)(void)  /* NodeGuardedViewport__NoOpSlotC4, empty; nothing calls it */
/* clang-format on */

/* clang-format off */
#define NODEGUARDEDVIEWPORT_FIELDS(Methods)                                           \
    VIEWPORT_FIELDS(Methods);                                                         \
    /* +0x0BC */ u8 pad0BC[0x0DC - 0x0BC] /* no accessor; the object is 0xDC bytes */
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
void NodeGuardedViewport__NoOpSlotBC(void);
void NodeGuardedViewport__NoOpSlotC0(void);
void NodeGuardedViewport__NoOpSlotC4(void);

#endif
