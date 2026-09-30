/**
 * @file grid_cell.h
 * @brief GridCell, one cell of StageMap's grid: a SceneNode carrying the model placed there.
 *
 * Declares the GridCell class (object, method table, and its slot and field
 * macros), its class id, size and flag constants, and its methods, which
 * are defined in src/world/grid_cell.c.
 */
#ifndef GRID_CELL_H
#define GRID_CELL_H

#include "scene_node.h"

typedef struct GridCell GridCell;
typedef struct GridCellMethods GridCellMethods;

/** GridCell's class id (gGridCellMethods word +0x000). Two nibbles, so
 * `(header & CLASS_ID_LEVEL2_MASK) == GRIDCELL_CLASS_ID` is its is-kind-of test
 * (Actor__DispatchLinkCommand). */
#define GRIDCELL_CLASS_ID 0x24

/** SceneNode's slots, then GridCell's own. GridCell overrides the inherited
 * ctor, reset (+0x040, empty) and dispatchLinkCommand (+0x09C). The ctor
 * returns nothing, but the slot keeps SceneNode's `void *` ctor type:
 * New_GridCell ignores the value (as LightRig's ctor, include/light_rig.h). */
/* clang-format off */
#define GRIDCELL_SLOTS(Self, CtorParams)                                                         \
    SCENENODE_SLOTS(Self, CtorParams);                                                            \
    /* +0x0B8 */ void (*onActorLinkCommand)(Self *self, void *sender, s32 event); /* @see GridCell__OnActorLinkCommand */ \
    /* +0x0BC */ void *(*returnSelf)(Self *self) /* @see GridCell__ReturnSelf; no known caller */
/* clang-format on */

/** SceneNode's fields; GridCell adds none. */
/* clang-format off */
#define GRIDCELL_FIELDS(Methods)                                                                 \
    SCENENODE_FIELDS(Methods) /* no own fields; the object is 0x3C bytes (New_GridCell) */
/* clang-format on */

/** The object's size, New_GridCell's allocation. sizeof(GridCell) overstates
 * it by SceneNode's trailing pad3C[8] (see GridCell). */
#define GRIDCELL_SIZE 60

/** A flags36 bit (a placement record's cellFlags, PopulateSlotCells): the
 * cell takes the commands StageMap hands the cells under a sender's
 * footprint (NotifyGridCell skips a cell without it).
 * DreamSys__NotifyLinkAttempt reads the low seven bits of the same word as a
 * voice index. */
#define GRIDCELL_FLAG_TAKES_COMMANDS 0x80

/**
 * @brief GridCell's method table: SceneNode's slots, then GridCell's own.
 */
struct GridCellMethods {
    GRIDCELL_SLOTS(GridCell, (GridCell * self));
};

/**
 * @brief One cell of StageMap's grid: a SceneNode carrying the model placed in it.
 *
 * Class id 0x24 (GRIDCELL_CLASS_ID), method table gGridCellMethods, parent
 * SceneNode (the ctor chains to SceneNode's first); no class below it.
 * Methods in src/world/grid_cell.c.
 *
 * Lifecycle: only StageMap__StageMap (src/world/stage_map.c) creates them.
 * For each of its seven elements it makes one GridCell as the element's
 * `cellParent`, attached to the StageMap at `origin`, and 410 more as the
 * element's `cells` (a 20 x 20 lattice, row stride 20, then 10 overflow
 * cells), each attached to the cellParent 0x800 units apart in x and z,
 * lighting mode 1, `attribute` bit 31 set. StageMap__Finalize releases them
 * all.
 *
 * Use: StageMap__PopulateSlotCells fills a cell from the element's placement
 * records, linking the placement's TMD onto it (`model`, `tmd`,
 * GsLinkObject4 on `attribute`), setting its coord2 translation and y
 * rotation and its `flags36`; a record flagged as chained goes into one of
 * the overflow cells and is hung off the lattice cell through `nextInCell`.
 * StageMap__ClearSlotCells clears them again. Two walks read a cell and its
 * chain: StageMap__DispatchToRectCells hands a command to every cell under a
 * footprint rectangle (NotifyGridCell: only a cell with flags36 bit 0x80),
 * and Actor__ScanGridWindow raycasts vertically against each
 * (SceneNode__RaycastVertical).
 *
 * Linking:
 *  - +0x040 reset is empty. SceneNode's ctor calls reset before this ctor
 *    switches the table, so SceneNode__Reset still runs once.
 *  - +0x09C dispatchLinkCommand passes only an Actor sender (class id byte
 *    0x34) on, to onActorLinkCommand; Actor__DispatchLinkCommand is the
 *    mirror, routing a GridCell sender (0x24) to onGridCellLinkCommand
 *    (DreamSys__WallLink reads the grid's current cell there).
 *  - Own slots: +0x0B8 onActorLinkCommand chains SceneNode's
 *    dispatchLinkCommand and runs tryAttachNearby for events 5..8 (the body
 *    of Actor__OnActorLinkCommand); +0x0BC returnSelf, no caller.
 *
 * Fields: none of its own. The ctor zeroes SceneNode's unk34, flags36 and
 * nextInCell. The object is 0x3C bytes (New_GridCell), shorter than
 * SceneNode's 0x44: the struct expands SCENENODE_FIELDS whole, so sizeof
 * overstates the allocation by SceneNode's trailing pad3C[8]. No code takes
 * sizeof(GridCell).
 */
struct GridCell {
    GRIDCELL_FIELDS(GridCellMethods);
};

/** GridCell's method table (class id GRIDCELL_CLASS_ID). */
extern GridCellMethods gGridCellMethods;

/**
 * @brief Returns GridCell's method table.
 * @return &gGridCellMethods.
 */
extern GridCellMethods *GetGridCellMethods(void);

/* The class's own methods, in address order. */

/**
 * @brief Allocates a GridCell (GRIDCELL_SIZE bytes) and runs its ctor through the method table.
 * @return the new cell, or NULL when the allocation fails.
 */
GridCell *New_GridCell(void);

/**
 * @brief Constructs a GridCell: SceneNode's ctor, GridCell's table, and an empty cell.
 *
 * Zeroes unk34, flags36 and nextInCell.
 * @param self the object to construct.
 */
void GridCell__GridCell(GridCell *self);

/**
 * @brief The +0x040 reset override: does nothing.
 */
void GridCell__Reset(void);

/**
 * @brief Passes a link command on to onActorLinkCommand when the sender is an Actor.
 *
 * An Actor or a class below it: the sender's class-id low byte is
 * ACTOR_CLASS_ID. Other senders are ignored.
 * @param self   the cell.
 * @param sender the object sending the command.
 * @param event  the link command.
 */
void GridCell__DispatchLinkCommand(GridCell *self, BasicClass *sender, s32 event);

/**
 * @brief Handles an Actor's link command: SceneNode's handling, then tryAttachNearby on a move.
 *
 * Runs SceneNode's dispatchLinkCommand, then tryAttachNearby(self, sender,
 * event) for the Actor move events ACTOR_EVENT_UNSWEPT..ACTOR_EVENT_MOVED_Y.
 * @param self   the cell.
 * @param sender the Actor.
 * @param event  the link command.
 */
void GridCell__OnActorLinkCommand(GridCell *self, void *sender, s32 event);

/**
 * @brief Returns the cell itself (the +0x0BC slot; nothing calls it).
 * @param self the cell.
 * @return `self`.
 */
void *GridCell__ReturnSelf(GridCell *self);

#endif
