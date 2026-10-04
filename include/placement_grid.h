#ifndef PLACEMENT_GRID_H
#define PLACEMENT_GRID_H

#include "file_resource.h"

/**
 * @file placement_grid.h
 * @brief PlacementGrid, the model placements of one map-grid element, and
 * the placement records it walks.
 */

typedef struct PlacementGrid PlacementGrid;
typedef struct PlacementGridMethods PlacementGridMethods;

/** PlacementGrid's class id (gPlacementGridMethods word +0x000). */
#define PLACEMENTGRID_CLASS_ID 0xE03

/**
 * @brief One placement record in a PlacementGrid's buffer, 12 bytes: each
 * cell's first record sits at buffer + 8 + cell * 12, and further records
 * of the same cell are chained by `next`.
 */
typedef struct PlacementGridRecord {
    /* +0x0 */ u8 present; /**< Zero: the cell is empty, and ResolveEntry returns -1. */
    /* +0x1 */ u8 unused1; /**< Copied to CellPlacement::unused2C; nothing else reads it. */
    /* +0x2 */ u16 model;  /**< The model index passed to the LinkResource's getModel. */
    /* +0x4 */ u8 cellFlags; /**< Copied to CellPlacement::cellFlags, which becomes the GridCell's flags36. */
    /* +0x5 */ u8 rotY;  /**< Y rotation in quarter turns. */
    /* +0x6 */ s16 y;    /**< Height in cells (0x800 units). */
    /* +0x8 */ s32 next; /**< Buffer offset of the cell's next record, 0 for none. */
} PlacementGridRecord;

/**
 * @brief What PlacementGrid__ResolveEntry fills in: the caller's 0x40-byte
 * stack record (StageMap__PopulateSlotCells). x and z are the cell's centre,
 * 0x800 units a cell.
 */
typedef struct CellPlacement {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 x; /**< column * 0x800 + 0x400. */
    /* +0x010 */ s32 y; /**< The record's y * 0x800. */
    /* +0x014 */ s32 z; /**< row * 0x800 + 0x400. */
    /* +0x018 */ u8 pad18[0x1A - 0x18];
    /* +0x01A */ u16 rotY; /**< The record's rotY * 0x400. */
    /* +0x01C */ u8 pad1C[0x2C - 0x1C];
    /* +0x02C */ u16 unused2C; /**< The record's unused1; nothing reads it. */
    /* +0x02E */ u16 cellFlags; /**< The record's cellFlags; the caller copies it to the GridCell's flags36. */
    /* +0x030 */ s32 chained; /**< 1 when this record came from the previous one's `next`. */
    /* +0x034 */ s32 next; /**< In: the offset to follow (0 starts at the cell); out: the record's `next`. */
    /* +0x038 */ s32 model; /**< The record's model. */
    /* +0x03C */ u8 pad3C[4];
} CellPlacement;

/**
 * @brief The type PlacementGrid's processBuffer slot (+0x078) is called
 * through: FileResource declares that slot untyped, and this class's
 * occupant is PlacementGrid__ResolveEntry.
 */
typedef intptr_t (*PlacementGridResolveEntryFn)(PlacementGrid *self, CellPlacement *placement, s32 cell);

/**
 * @brief PlacementGrid's method table: FileResource's slots, with a ctor
 * that takes a file name; +0x078 (processBuffer) is
 * PlacementGrid__ResolveEntry, called through PlacementGridResolveEntryFn.
 */
struct PlacementGridMethods {
    FILERESOURCE_SLOTS(PlacementGrid, (PlacementGrid * self, char *name));
};

/**
 * @brief The model placements of one grid element's 20 x 20 cells (class id
 * 0xE03), a FileResource subclass with no subclasses.
 *
 * A view over the placements section of the element's LbdFile header block:
 * a list of PlacementGridRecords that ResolveEntry turns, one per call, into
 * a CellPlacement and the model it places. Like every FileResource client it
 * runs on the active driver: the ctor, Finalize and OnRequestDone chain to
 * GetActiveDataSourceMethods()'s, and GetPlacementGridMethods is in
 * sDataSourceClientGetters, so SetActiveDataSource rebinds its file-I/O
 * slots.
 *
 * Lifecycle: the grid manager (StageMap) is its one user.
 * StageMap__StageMap makes one per grid element with New_PlacementGrid(NULL),
 * so it never loads a file itself. StageMap__PopulateSlotCells points
 * `buffer` at the element's LbdFile header block + placementsOffset, puts a
 * LinkResource over the models that follow the placements in `linkResource`,
 * and calls ResolveEntry until it returns 0, filling one GridCell from each
 * CellPlacement. StageMap__UnloadAllSlots releases the LinkResource.
 * Methods in src/sound/placement_grid.c.
 */
struct PlacementGrid {
    FILERESOURCE_FIELDS(PlacementGridMethods);
    /* +0x02C */ struct LinkResource *linkResource; /**< The models' LinkResource; zeroed by the ctor. */
    /* +0x030 */ s32 loaded; /**< Set by PlacementGrid__OnRequestDone; zeroed by the ctor; nothing reads it. */
}; /* 0x34 bytes: New_PlacementGrid */

/** @brief PlacementGrid's method table (see PlacementGridMethods). */
extern PlacementGridMethods gPlacementGridMethods;

/**
 * @brief Returns PlacementGrid's method table.
 * @return &gPlacementGridMethods.
 */
extern PlacementGridMethods *GetPlacementGridMethods(void);

/**
 * @brief Allocates a PlacementGrid from the BMemPMgr pool and constructs it.
 * @param name File to request, or NULL for none.
 * @return The new object, or NULL when the pool is exhausted.
 */
PlacementGrid *New_PlacementGrid(char *name);

/**
 * @brief Constructor (slot +0x008): the active driver's ctor, this class's
 * table, `linkResource` and `loaded` cleared, and requestLoadFile(name) when
 * `name` is not NULL.
 * @param self The object being constructed.
 * @param name File to request, or NULL.
 */
void PlacementGrid__PlacementGrid(PlacementGrid *self, char *name);

/**
 * @brief Finalizer (slot +0x00C): the active driver's finalize.
 * @param self The object being finalized.
 */
void PlacementGrid__Finalize(PlacementGrid *self);

/**
 * @brief Slot +0x064, onRequestDone: sets `loaded`, then calls the active
 * driver's onRequestDone.
 * @param self The object whose request completed.
 */
void PlacementGrid__OnRequestDone(PlacementGrid *self);

/**
 * @brief Slot +0x078 (processBuffer): resolves one placement record of cell
 * `cell` into `placement` and returns its model.
 *
 * With placement->next zero it reads the cell's own record (row-major:
 * `cell / 20` is the row, `cell % 20` the column); otherwise the record at
 * buffer + next, the cell's next one. It fills the placement at the cell's
 * centre and returns what linkResource's getModel gives for the record's
 * model index.
 * @param self      The grid.
 * @param placement In: `next` says which record; out: the placement.
 * @param cell      Cell index, 0..399.
 * @return The model, -1 for an empty record, or 0 once `cell` reaches 400.
 */
intptr_t PlacementGrid__ResolveEntry(PlacementGrid *self, CellPlacement *placement, s32 cell);

#endif
