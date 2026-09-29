/**
 * @file node_guarded_viewport.h
 * @brief NodeGuardedViewport, a Viewport that skips its frame while no view node is attached.
 *
 * Declares the NodeGuardedViewport class (object, method table, and its
 * slot and field macros) and its methods, which are defined in
 * src/ui/title_menu.c.
 */
#ifndef NODE_GUARDED_VIEWPORT_H
#define NODE_GUARDED_VIEWPORT_H

#include "viewport.h"

typedef struct NodeGuardedViewport NodeGuardedViewport;
typedef struct NodeGuardedViewportMethods NodeGuardedViewportMethods;

/** NodeGuardedViewport's class id (gNodeGuardedViewportMethods word +0x000). */
#define NODEGUARDEDVIEWPORT_CLASS_ID 0x17

/** Viewport's slots, then NodeGuardedViewport's own. It overrides the
 * inherited ctor, initDefaults (+0x040) and update (+0x09C). */
/* clang-format off */
#define NODEGUARDEDVIEWPORT_SLOTS(Self, CtorParams)                                                   \
    VIEWPORT_SLOTS(Self, CtorParams);                                                                 \
    /* +0x0B8 */ void (*slotB8)(void); /* @see NodeGuardedViewport__NoOpSlotB8; nothing calls it */ \
    /* +0x0BC */ void (*slotBC)(void); /* @see NodeGuardedViewport__NoOpSlotBC; nothing calls it */ \
    /* +0x0C0 */ void (*slotC0)(void); /* @see NodeGuardedViewport__NoOpSlotC0; nothing calls it */ \
    /* +0x0C4 */ void (*slotC4)(void)  /* @see NodeGuardedViewport__NoOpSlotC4; nothing calls it */
/* clang-format on */

/** Viewport's fields, then an unused range to 0xDC bytes. */
/* clang-format off */
#define NODEGUARDEDVIEWPORT_FIELDS(Methods)                                           \
    VIEWPORT_FIELDS(Methods);                                                         \
    /* +0x0BC */ u8 pad0BC[0x0DC - 0x0BC] /* no accessor; the object is 0xDC bytes */
/* clang-format on */

/**
 * @brief NodeGuardedViewport's method table: Viewport's slots, then four empty ones.
 */
struct NodeGuardedViewportMethods {
    NODEGUARDEDVIEWPORT_SLOTS(NodeGuardedViewport, (NodeGuardedViewport * self));
};

/**
 * @brief A Viewport that skips its frame while no view node is attached.
 *
 * Class id 0x17, method table gNodeGuardedViewportMethods, parent Viewport:
 * its ctor chains to Viewport's (GetViewportMethods()->ctor) first. Methods
 * in src/ui/title_menu.c. No class derives from it.
 *
 * What it changes:
 *  - update (+0x09C, NodeGuardedViewport__Update) calls Viewport__Update
 *    only when viewNode and otReady are both set. Viewport__Update checks
 *    otReady itself but dereferences viewNode and refView.super, which
 *    RemoveChild clears, so the override is what lets the viewport run
 *    detached.
 *  - initDefaults (+0x040) is empty. The Viewport ctor runs under Viewport's
 *    own table, so Viewport__InitDefaults has already written every default
 *    by the time this ctor installs its table and calls initDefaults again;
 *    the override makes that second call a no-op.
 *  - Four own slots, +0x0B8..+0x0C4, hold empty functions nothing calls.
 *
 * Lifecycle: DayTask__DayTask (src/world/day_task.c), the one construction
 * site, builds it as its IntermediateBaseInitArgs' viewport. DayTask__Init
 * hands it to the DreamSys (setViewport), and ObjM, built from the same init
 * args, drives it through Viewport's slots: InitStyleAndWorld and
 * SetupSceneStyle detach and re-attach the DreamSys as the view node,
 * EnterStyleSession sets its light mode, clear colour, fog and far colour
 * and enables drawing, and ExitSceneStyle detaches the view node and leaves
 * it detached, the state the update guard covers.
 *
 * The object is 0xDC bytes (New_NodeGuardedViewport); nothing reads or
 * writes the own range +0x0BC..+0x0DC.
 */
struct NodeGuardedViewport {
    NODEGUARDEDVIEWPORT_FIELDS(NodeGuardedViewportMethods);
};

/** NodeGuardedViewport's method table (class id 0x17). */
extern NodeGuardedViewportMethods gNodeGuardedViewportMethods;

/**
 * @brief Returns NodeGuardedViewport's method table.
 * @return &gNodeGuardedViewportMethods.
 */
extern NodeGuardedViewportMethods *GetNodeGuardedViewportMethods(void);

/* The class's own methods, in address order. */

/**
 * @brief Allocates a NodeGuardedViewport (0xDC bytes) and runs its ctor through the method table.
 * @return the new viewport, or NULL when the allocation fails.
 */
NodeGuardedViewport *New_NodeGuardedViewport(void);

/**
 * @brief Constructs a NodeGuardedViewport: Viewport's ctor, then its own table.
 *
 * Calls initDefaults once more under the new table, which is empty.
 * @param self the object to construct.
 */
void NodeGuardedViewport__NodeGuardedViewport(NodeGuardedViewport *self);

/**
 * @brief The +0x040 initDefaults override: does nothing.
 */
void NodeGuardedViewport__InitDefaults(void);

/**
 * @brief Runs Viewport's update only while a view node is attached and the ordering table is ready.
 * @param self the viewport.
 */
void NodeGuardedViewport__Update(NodeGuardedViewport *self);

/** @brief The empty +0x0B8 slot. */
void NodeGuardedViewport__NoOpSlotB8(void);
/** @brief The empty +0x0BC slot. */
void NodeGuardedViewport__NoOpSlotBC(void);
/** @brief The empty +0x0C0 slot. */
void NodeGuardedViewport__NoOpSlotC0(void);
/** @brief The empty +0x0C4 slot. */
void NodeGuardedViewport__NoOpSlotC4(void);

#endif
