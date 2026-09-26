#ifndef GRIDCELL_H
#define GRIDCELL_H

#include "SceneNode.h"

/*
 * GridCell -- one cell of StageMap's grid: a SceneNode that carries the
 * model placed in that cell. Class id 0x24, method table gGridCellMethods;
 * SceneNode's subclass (the ctor chains to SceneNode's first), no class
 * below it. Methods in src/class_3bb8c_c.c.
 *
 * Lifecycle: only StageMap__StageMap (src/class_3ac78.c) creates them.
 * For each of its seven elements it makes one GridCell as the element's
 * `cellParent`, attached to the StageMap at `origin`, and 410 more as
 * the element's `cells` (a 20 x 20 lattice, row stride 20, then 10
 * overflow cells), each attached to the cellParent 0x800 units apart in x
 * and z, lighting mode 1, `attribute` bit 31 set. StageMap__Finalize
 * releases them all.
 *
 * Use: StageMap__LoadElementResources fills a cell from the element's
 * placement records, linking the placement's TMD onto it (`model`, `tmd`,
 * GsLinkObject4 on `attribute`), setting its coord2 translation and y
 * rotation and its `flags36`; a record flagged as chained goes into one of
 * the overflow cells and is hung off the lattice cell through `nextInCell`.
 * StageMap__ResetElementCells clears them again. Two walks read a cell
 * and its chain: StageMap__DispatchToRectCells hands a command to every
 * cell under a footprint rectangle (NotifyGridCell: only a cell with
 * flags36 bit 0x80), and Actor__ScanGridWindow raycasts vertically against
 * each (SceneNode__RaycastVertical).
 *
 * Linking (`classtable.py gGridCellMethods --vs gSceneNodeMethods`):
 *  - +0x040 reset is empty. SceneNode's ctor calls reset before this ctor
 *    switches the table, so SceneNode__Reset still runs once;
 *  - +0x09C dispatchLinkCommand passes only an Actor sender (class id byte
 *    0x34) on, to onActorLinkCommand; Actor__DispatchLinkCommand is the
 *    mirror, routing a GridCell sender (0x24) to onGridCellLinkCommand
 *    (DreamSys__WallLink reads the grid's current cell there);
 *  - own slots: +0x0B8 onActorLinkCommand chains SceneNode's
 *    dispatchLinkCommand and runs tryAttachNearby for events 5..8 (the
 *    body of Actor__OnActorLinkCommand); +0x0BC returnSelf, no caller.
 *
 * Fields: none of its own. The ctor zeroes SceneNode's unk34, flags36 and
 * nextInCell. The object is 0x3C bytes (New_GridCell), shorter than
 * SceneNode's 0x44: the struct expands SCENENODE_FIELDS whole, so sizeof
 * overstates the allocation by SceneNode's trailing pad3C[8]. No code takes
 * sizeof(GridCell).
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
