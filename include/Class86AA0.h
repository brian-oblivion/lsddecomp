#ifndef CLASS86AA0_H
#define CLASS86AA0_H

#include "Class6B5CC.h"

/*
 * Class86AA0 -- class id 0x24, method table gClass86AA0Methods: Class6B5CC's
 * subclass (its ctor chains to GetClass6B5CCMethods()->ctor first, so the id
 * tree 0x4 -> 0x24 is the ctor chain). Methods in src/class_3bb8c_c.c. No
 * class derives from it.
 *
 * Construction: Class866E8__Class866E8 (src/class_3ac78.c) makes one per
 * grid element as the element's `cellParent`, attached to the Class866E8,
 * and one per grid cell (0x668 / 4 = 410 of them), each attached to that
 * cellParent at a 0x800-unit x/z spacing, lighting-mode 1, with
 * attribute bit 31 set. The grid dispatch (Class866E8__DispatchToRectCells,
 * NotifyGridCell) walks a cell and its `nextInCell` chain.
 *
 * What it changes, from its own methods (`classtable.py gClass86AA0Methods
 * --vs gClass6B5CCMethods`):
 *  - +0x040 reset is empty (Class86AA0__Reset). The Class6B5CC ctor's own
 *    reset call runs before the table is switched, so Class6B5CC__Reset
 *    still runs once at construction;
 *  - +0x09C dispatchLinkCommand (Class86AA0__DispatchLinkCommand) routes a
 *    sender whose class id byte is 0x34 (an Actor) to onActorLinkCommand
 *    and ignores every other sender: the mirror of Actor's override, which
 *    routes a 0x24 sender (this class) to Actor's onClass86AA0LinkCommand;
 *  - two own slots: +0x0B8 onActorLinkCommand, which chains Class6B5CC's
 *    dispatchLinkCommand and runs tryAttachNearby for events 5..8 (the same
 *    body as Actor__OnActorLinkCommand), and +0x0BC returnSelf, no known
 *    caller.
 * That says how it links, not what a grid cell is in the game, so the name
 * stays the table's address.
 *
 * Fields: none of its own. The ctor zeroes Class6B5CC's +0x034, +0x036 and
 * +0x038 (unk34, flags36, nextInCell). The object is 0x3C bytes
 * (New_Class86AA0), SHORTER than Class6B5CC's 0x44: the struct below
 * expands CLASS6B5CC_FIELDS whole, so sizeof overstates the allocation by
 * Class6B5CC's trailing pad3C[8]. No code takes sizeof(Class86AA0).
 */

typedef struct Class86AA0 Class86AA0;
typedef struct Class86AA0Methods Class86AA0Methods;

/* Class6B5CC's slots, then this class's own. The overrides of inherited
 * slots are the ctor, reset and dispatchLinkCommand (see the banner). The
 * ctor returns nothing, but the slot keeps Class6B5CC's `void *` ctor type:
 * New_Class86AA0 ignores the value (as LightRig's ctor, include/LightRig.h). */
/* clang-format off */
#define CLASS86AA0_SLOTS(Self, CtorParams)                                                         \
    CLASS6B5CC_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*onActorLinkCommand)(Self *self, void *sender, s32 event); /* Class86AA0__OnActorLinkCommand; Class86AA0__DispatchLinkCommand's 0x34 case */ \
    /* +0x0BC */ void *(*returnSelf)(Self *self) /* Class86AA0__ReturnSelf; no known caller */
/* clang-format on */

/* clang-format off */
#define CLASS86AA0_FIELDS(Methods)                                                                 \
    CLASS6B5CC_FIELDS(Methods) /* no own fields; the object is 0x3C bytes (New_Class86AA0), see the banner */
/* clang-format on */

struct Class86AA0Methods {
    CLASS86AA0_SLOTS(Class86AA0, (Class86AA0 * self));
};

struct Class86AA0 {
    CLASS86AA0_FIELDS(Class86AA0Methods);
};

extern Class86AA0Methods gClass86AA0Methods;
extern Class86AA0Methods *GetClass86AA0Methods(void); /* returns &gClass86AA0Methods */

/* The class's own methods, in address order. */
Class86AA0 *New_Class86AA0(void); /* BMemPMgrAlloc(0x3C), then ctor */
void Class86AA0__Class86AA0(Class86AA0 *self);
void Class86AA0__Reset(void); /* +0x040; empty, reads no argument */
void Class86AA0__DispatchLinkCommand(Class86AA0 *self, BasicClass *sender, s32 event);
void Class86AA0__OnActorLinkCommand(Class86AA0 *self, void *sender, s32 event);
void *Class86AA0__ReturnSelf(Class86AA0 *self);

#endif
