#ifndef PLACEMENTGRID_H
#define PLACEMENTGRID_H

#include "FileResource.h"

/*
 * PlacementGrid -- a FileResource data source (class id 0xE03, method table
 * gPlacementGridMethods) over a 20x20 grid of placement records. Methods in
 * src/code_179d8_d.c. No classes derive from it. Its getter is the first
 * entry of gDataSourceClientGetters, so SetActiveDataSource rebinds its
 * interface slots like every other client's.
 *
 * PARENT BY CTOR CHAIN: PlacementGrid__PlacementGrid's first call is
 * GetActiveDataSourceMethods()->ctor, and PlacementGrid__Finalize and
 * PlacementGrid__SetFlag forward to the active driver's finalize and setFlag,
 * as TimBlockSrc, Tod and ModelData do.
 *
 * Its one user is the grid manager (Class866E8): Class866E8__Class866E8
 * makes one per grid element with New_PlacementGrid(0), so nothing is
 * loaded; Class866E8__LoadElementResources points `buffer` into an already
 * loaded resource, puts a LinkResource (gLinkResourceMethods) of the element's models
 * in `linkResource`, then calls +0x078 once per cell until it returns 0.
 *
 * +0x078 is FileResource's slot78 (NULL there): this table's occupant is
 * PlacementGrid__ResolveEntry(self, placement, cell), s32. It reads cell
 * `cell`'s PlacementGridRecord (12 bytes each, from buffer +8; cell / 20 is
 * the row, cell % 20 the column), follows `next` for a second record in
 * the same cell, fills a PlacementGridPlacement and returns the model
 * linkResource's +0x080 (LinkResource__GetModel) gives for the record's
 * model index: 0 past the last cell (400), -1 for an empty record.
 * Class866E8__LoadElementResources calls it through
 * PlacementGridResolveEntryFn, a cast of the inherited slot (no code).
 */

typedef struct PlacementGrid PlacementGrid;
typedef struct PlacementGridMethods PlacementGridMethods;

/* One record in the buffer, 12 bytes: PlacementGrid__ResolveEntry steps
 * cell * 12 + 8. */
typedef struct PlacementGridRecord {
    /* +0x0 */ u8 present; /* zero: the cell is empty, ResolveEntry returns -1 */
    /* +0x1 */ u8 unk1;    /* -> PlacementGridPlacement.unk2C */
    /* +0x2 */ u16 model;  /* the index passed to linkResource's +0x080 */
    /* +0x4 */ u8 unk4;    /* -> PlacementGridPlacement.unk2E */
    /* +0x5 */ u8 rotY;    /* in 0x400 steps: -> PlacementGridPlacement.rotY */
    /* +0x6 */ s16 y;      /* in 0x800 units: -> PlacementGridPlacement.y */
    /* +0x8 */ s32 next;   /* buffer offset of the cell's next record, 0 for none */
} PlacementGridRecord;

/* What PlacementGrid__ResolveEntry fills in: the caller's stack record, 0x40
 * bytes (Class866E8__LoadElementResources). x and z are the cell's centre,
 * 0x800 units a cell. */
typedef struct PlacementGridPlacement {
    /* +0x000 */ u8 pad0[0xC];
    /* +0x00C */ s32 x; /* column * 0x800 + 0x400 */
    /* +0x010 */ s32 y; /* record y * 0x800 */
    /* +0x014 */ s32 z; /* row * 0x800 + 0x400 */
    /* +0x018 */ u8 pad18[0x1A - 0x18];
    /* +0x01A */ u16 rotY; /* record rotY * 0x400 */
    /* +0x01C */ u8 pad1C[0x2C - 0x1C];
    /* +0x02C */ u16 unk2C;   /* record unk1 */
    /* +0x02E */ u16 unk2E;   /* record unk4 */
    /* +0x030 */ s32 chained; /* 1: this record came from the previous one's `next` */
    /* +0x034 */ s32 next; /* in: the offset to follow (0 starts at the cell); out: the record's `next` */
    /* +0x038 */ s32 model; /* record model */
    /* +0x03C */ u8 pad3C[4];
} PlacementGridPlacement;

typedef s32 (*PlacementGridResolveEntryFn)(PlacementGrid *self, PlacementGridPlacement *placement, s32 cell);

struct PlacementGridMethods {
    FILERESOURCE_SLOTS(PlacementGrid, (PlacementGrid * self, char *name));
    /* +0x078 is FileResource's slot78; this table's occupant is
     * PlacementGrid__ResolveEntry (PlacementGridResolveEntryFn). */
};

struct PlacementGrid {
    FILERESOURCE_FIELDS(PlacementGridMethods);
    /* +0x02C */ struct LinkResource *linkResource; /* the models' LinkResource (include/LinkResource.h); zeroed by the ctor */
    /* +0x030 */ s32 loaded; /* set by PlacementGrid__SetFlag; zeroed by the ctor */
}; /* 0x34 bytes: New_PlacementGrid */

extern PlacementGridMethods gPlacementGridMethods;
extern PlacementGridMethods *GetPlacementGridMethods(void);

PlacementGrid *New_PlacementGrid(char *name);
void PlacementGrid__PlacementGrid(PlacementGrid *self, char *name);
void PlacementGrid__Finalize(PlacementGrid *self);
void PlacementGrid__SetFlag(PlacementGrid *self);
s32 PlacementGrid__ResolveEntry(PlacementGrid *self, PlacementGridPlacement *placement, s32 cell);

#endif
