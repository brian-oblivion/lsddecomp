/*
 * code_179d8_d -- PlacementGrid's methods (include/PlacementGrid.h: one
 * grid element's 20 x 20 cells of model placements), then the first seven
 * of VabDriver's empty driver methods (include/VabDriver.h; the rest, and
 * its getter, are in code_179d8_e.c), with ReturnZero, a free-standing
 * `return 0` with no caller, between them.
 *
 * Declarations that encode this unit's reading of its callees stay in this
 * file; the code_179d8_* slices share no header.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "VabDriver.h"
#include "PlacementGrid.h"
#include "LinkResource.h"
#include "StageMap.h"

/* FileResource's, code_171e0.c: the active driver's table, through which
 * PlacementGrid's ctor, finalize and setFlag reach their parent's. */
extern FileResourceMethods *GetActiveDataSourceMethods(void);

/* The pool allocator (include/class_16334.h, include/BMemPMgr.h, neither
 * included here). */
extern void *BMemPMgrAlloc(s32 size);

/* What `buffer` points at: 8 bytes nothing here reads, then each cell's
 * first record, one per cell of the chunk's 20 x 20 lattice, row-major. A
 * cell's further records are reached by the byte offset in `next`. */
typedef struct PlacementGridBuffer {
    u8 pad0[8];
    PlacementGridRecord cells[STAGE_SLOT_LATTICE_CELLS];
} PlacementGridBuffer;

/* LinkResource__GetModel as PlacementGrid__ResolveEntry calls it through
 * `linkResource`'s getModel (+0x080). The occupant reads only (self, index);
 * a function-pointer cast, no code.
 * MATCHING: the four arguments keep `placement` in $a3 across the call. */
typedef s32 (*PlacementGridGetModelFn)(LinkResource *self, s32 model, s32 cell, CellPlacement *placement);

PlacementGrid *New_PlacementGrid(char *name) {
    PlacementGrid *self;
    PlacementGridMethods *table;

    self = BMemPMgrAlloc(sizeof(PlacementGrid));
    if (self != NULL) {
        table = GetPlacementGridMethods();
        table->ctor(self, name);
        return self;
    }
    return NULL;
}

void PlacementGrid__PlacementGrid(PlacementGrid *self, char *name) {
    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetPlacementGridMethods();
    self->linkResource = NULL;
    self->loaded = 0;
    if (name != NULL) {
        self->methods->requestLoadFile(self, name);
    }
}

void PlacementGrid__Finalize(PlacementGrid *self) {
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

void PlacementGrid__SetFlag(PlacementGrid *self) {
    self->loaded = 1;
    GetActiveDataSourceMethods()->setFlag((FileResource *)self);
}

s32 PlacementGrid__ResolveEntry(PlacementGrid *self, CellPlacement *placement, s32 cell) {
    PlacementGridRecord *rec;
    LinkResource *link;
    s32 row;
    s32 col;
    s32 model;

    if (cell < STAGE_SLOT_LATTICE_CELLS) {
        if (placement->next != 0) {
            rec = (PlacementGridRecord *)((u8 *)self->buffer + placement->next);
            placement->chained = 1;
        } else {
            rec = &((PlacementGridBuffer *)self->buffer)->cells[cell];
            placement->chained = 0;
        }
        placement->next = rec->next;
        if (rec->present != 0) {
            row = cell / STAGE_CHUNK_CELLS;
            col = cell - row * STAGE_CHUNK_CELLS;
            placement->x = (col << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2;
            placement->y = (s32)rec->y << STAGE_CELL_SHIFT;
            placement->z = (row << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2;
            placement->rotY = rec->rotY * ANGLE_DEG(90);
            placement->unk2C = rec->unk1;
            placement->cellFlags = rec->cellFlags;
            model = rec->model;
            placement->model = model;
            link = self->linkResource;
            return ((PlacementGridGetModelFn)link->methods->getModel)(link, model, cell, placement);
        }
        return -1;
    }
    return 0;
}

PlacementGridMethods *GetPlacementGridMethods(void) {
    return &gPlacementGridMethods;
}

s32 ReturnZero(void) {
    return 0;
}

void VabDriver__VabDriver(void) {}

void VabDriver__Destroy(void) {}

void VabDriver__NoOpSlot40(void) {
    char unused[64];
}

void VabDriver__Open(void) {
    char unused[64];
}

void VabDriver__Close(void) {}

void VabDriver__Seek(void) {}

void VabDriver__NoOpSlot50(void) {}
