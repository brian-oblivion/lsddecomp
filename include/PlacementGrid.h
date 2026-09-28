#ifndef PLACEMENTGRID_H
#define PLACEMENTGRID_H

#include "FileResource.h"

/*
 * PlacementGrid -- the model placements of one grid element's 20 x 20
 * cells: a view over the placements section of the element's LbdFile
 * header block, a list of PlacementGridRecords that ResolveEntry turns,
 * one per call, into a CellPlacement and the model it places. Class id
 * 0xE03, method table gPlacementGridMethods, parent FileResource (the ctor
 * chains to GetActiveDataSourceMethods()->ctor, and Finalize and SetFlag
 * forward to the active driver's, as TimBlockSrc, Tod and ModelData do); no
 * subclasses. Methods in src/sound/PlacementGridVabSound.c. GetPlacementGridMethods is the
 * first entry of sDataSourceClientGetters, so SetActiveDataSource rebinds
 * its file-I/O slots like every client's.
 *
 * Lifecycle: the grid manager (StageMap) is its one user.
 * StageMap__StageMap makes one per grid element, New_PlacementGrid(0),
 * so it never loads a file itself. StageMap__PopulateSlotCells points
 * `buffer` at the element's LbdFile header block + placementsOffset, puts a
 * LinkResource over the models that follow the placements in
 * `linkResource`, and calls +0x078 (FileResource's processBuffer) until it
 * returns 0, filling one GridCell from each CellPlacement.
 * StageMap__UnloadAllSlots releases the LinkResource.
 *
 * +0x078's occupant is PlacementGrid__ResolveEntry(self, placement, cell).
 * With placement->next zero it reads cell `cell`'s own record (12 bytes
 * each from buffer + 8, row-major: cell / 20 is the row, cell % 20 the
 * column); otherwise the record at buffer + next, the cell's next one. It
 * fills the placement at the cell's centre and returns what linkResource's
 * getModel (+0x080, LinkResource__GetModel) gives for the record's model
 * index: -1 for an empty record, 0 once cell reaches 400. Its caller
 * reaches it through PlacementGridResolveEntryFn, a cast of the untyped
 * inherited slot (no code).
 */

typedef struct PlacementGrid PlacementGrid;
typedef struct PlacementGridMethods PlacementGridMethods;

/* One record in the buffer, 12 bytes: PlacementGrid__ResolveEntry steps
 * cell * 12 + 8. */
typedef struct PlacementGridRecord {
    /* +0x0 */ u8 present;   /* zero: the cell is empty, ResolveEntry returns -1 */
    /* +0x1 */ u8 unk1;      /* -> CellPlacement.unk2C */
    /* +0x2 */ u16 model;    /* the index passed to linkResource's +0x080 */
    /* +0x4 */ u8 cellFlags; /* -> CellPlacement.cellFlags, which becomes the GridCell's flags36 */
    /* +0x5 */ u8 rotY;      /* in 0x400 steps: -> CellPlacement.rotY */
    /* +0x6 */ s16 y;        /* in 0x800 units: -> CellPlacement.y */
    /* +0x8 */ s32 next;     /* buffer offset of the cell's next record, 0 for none */
} PlacementGridRecord;

/* What PlacementGrid__ResolveEntry fills in: the caller's stack record, 0x40
 * bytes (StageMap__PopulateSlotCells). x and z are the cell's centre,
 * 0x800 units a cell. */
typedef struct CellPlacement {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 x; /* column * 0x800 + 0x400 */
    /* +0x010 */ s32 y; /* record y * 0x800 */
    /* +0x014 */ s32 z; /* row * 0x800 + 0x400 */
    /* +0x018 */ u8 pad18[0x1A - 0x18];
    /* +0x01A */ u16 rotY; /* record rotY * 0x400 */
    /* +0x01C */ u8 pad1C[0x2C - 0x1C];
    /* +0x02C */ u16 unk2C; /* record unk1 */
    /* +0x02E */ u16 cellFlags; /* the record's cellFlags; the caller copies it to the GridCell's flags36 */
    /* +0x030 */ s32 chained; /* 1: this record came from the previous one's `next` */
    /* +0x034 */ s32 next; /* in: the offset to follow (0 starts at the cell); out: the record's `next` */
    /* +0x038 */ s32 model; /* record model */
    /* +0x03C */ u8 pad3C[4];
} CellPlacement;

typedef s32 (*PlacementGridResolveEntryFn)(PlacementGrid *self, CellPlacement *placement, s32 cell);

struct PlacementGridMethods {
    FILERESOURCE_SLOTS(PlacementGrid, (PlacementGrid * self, char *name));
    /* +0x078 is FileResource's processBuffer; this table's occupant is
     * PlacementGrid__ResolveEntry (PlacementGridResolveEntryFn). */
};

struct PlacementGrid {
    FILERESOURCE_FIELDS(PlacementGridMethods);
    /* +0x02C */ struct LinkResource *linkResource; /* the models' LinkResource (include/LinkResource.h); zeroed by the ctor */
    /* +0x030 */ s32 loaded; /* set by PlacementGrid__OnRequestDone (the driver's read-done callback); zeroed by the ctor; nothing reads it */
}; /* 0x34 bytes: New_PlacementGrid */

extern PlacementGridMethods gPlacementGridMethods;
extern PlacementGridMethods *GetPlacementGridMethods(void);

PlacementGrid *New_PlacementGrid(char *name);
void PlacementGrid__PlacementGrid(PlacementGrid *self, char *name);
void PlacementGrid__Finalize(PlacementGrid *self);
void PlacementGrid__OnRequestDone(PlacementGrid *self);
s32 PlacementGrid__ResolveEntry(PlacementGrid *self, CellPlacement *placement, s32 cell);

#endif
