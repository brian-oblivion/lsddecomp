#ifndef GRIDCELL_H
#define GRIDCELL_H

#include "SceneNode.h"

/*
 * GridCell -- class id 0x24, method table gGridCellMethods: SceneNode's
 * subclass (its ctor chains to GetSceneNodeMethods()->ctor first, so the id
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
 * What it changes, from its own methods (`classtable.py gGridCellMethods
 * --vs gSceneNodeMethods`):
 *  - +0x040 reset is empty (GridCell__Reset). The SceneNode ctor's own
 *    reset call runs before the table is switched, so SceneNode__Reset
 *    still runs once at construction;
 *  - +0x09C dispatchLinkCommand (GridCell__DispatchLinkCommand) routes a
 *    sender whose class id byte is 0x34 (an Actor) to onActorLinkCommand
 *    and ignores every other sender: the mirror of Actor's override, which
 *    routes a 0x24 sender (this class) to Actor's onGridCellLinkCommand;
 *  - two own slots: +0x0B8 onActorLinkCommand, which chains SceneNode's
 *    dispatchLinkCommand and runs tryAttachNearby for events 5..8 (the same
 *    body as Actor__OnActorLinkCommand), and +0x0BC returnSelf, no known
 *    caller.
 * That says how it links, not what a grid cell is in the game, so the name
 * stays the table's address.
 *
 * Fields: none of its own. The ctor zeroes SceneNode's +0x034, +0x036 and
 * +0x038 (unk34, flags36, nextInCell). The object is 0x3C bytes
 * (New_GridCell), SHORTER than SceneNode's 0x44: the struct below
 * expands SCENENODE_FIELDS whole, so sizeof overstates the allocation by
 * SceneNode's trailing pad3C[8]. No code takes sizeof(GridCell).
 */

typedef struct GridCell GridCell;
typedef struct GridCellMethods GridCellMethods;

/* SceneNode's slots, then this class's own. The overrides of inherited
 * slots are the ctor, reset and dispatchLinkCommand (see the banner). The
 * ctor returns nothing, but the slot keeps SceneNode's `void *` ctor type:
 * New_GridCell ignores the value (as LightRig's ctor, include/LightRig.h). */
/* clang-format off */
#define GRIDCELL_SLOTS(Self, CtorParams)                                                         \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*onActorLinkCommand)(Self *self, void *sender, s32 event); /* GridCell__OnActorLinkCommand; GridCell__DispatchLinkCommand's 0x34 case */ \
    /* +0x0BC */ void *(*returnSelf)(Self *self) /* GridCell__ReturnSelf; no known caller */
/* clang-format on */

/* clang-format off */
#define GRIDCELL_FIELDS(Methods)                                                                 \
    SCENENODE_FIELDS(Methods) /* no own fields; the object is 0x3C bytes (New_GridCell), see the banner */
/* clang-format on */

struct GridCellMethods {
    GRIDCELL_SLOTS(GridCell, (GridCell * self));
};

struct GridCell {
    GRIDCELL_FIELDS(GridCellMethods);
};

extern GridCellMethods gGridCellMethods;
extern GridCellMethods *GetGridCellMethods(void); /* returns &gGridCellMethods */

/* The class's own methods, in address order. */
GridCell *New_GridCell(void); /* BMemPMgrAlloc(0x3C), then ctor */
void GridCell__GridCell(GridCell *self);
void GridCell__Reset(void); /* +0x040; empty, reads no argument */
void GridCell__DispatchLinkCommand(GridCell *self, BasicClass *sender, s32 event);
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event);
void *GridCell__ReturnSelf(GridCell *self);

#endif
