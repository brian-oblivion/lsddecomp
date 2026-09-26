/*
 * code_179d8_d -- PlacementGrid's methods (include/PlacementGrid.h: one
 * grid element's 20 x 20 cells of model placements), then the first seven
 * of VabDriver's empty driver methods (include/VabDriver.h; the rest, and
 * its getter, are in code_179d8_e.c), with func_8002C3B8, a free-standing
 * `return 0` with no caller, between them.
 *
 * Declarations that encode this unit's reading of its callees stay in this
 * file; the code_179d8_* slices share no header.
 */
#include "common.h"
#include "VabDriver.h"
#include "PlacementGrid.h"
#include "LinkResource.h"

/* FileResource's, code_171e0.c: the active driver's table, through which
 * PlacementGrid's ctor, finalize and setFlag reach their parent's. */
extern FileResourceMethods *GetActiveDataSourceMethods(void);

/* The pool allocator (include/class_16334.h, include/code_8220.h, neither
 * included here). */
extern void *BMemPMgrAlloc(s32 size);

/* LinkResource__GetModel as PlacementGrid__ResolveEntry calls it through
 * `linkResource`'s getModel (+0x080). The occupant reads only (self, index);
 * a function-pointer cast, no code.
 * MATCHING: the four arguments keep `placement` in $a3 across the call. */
typedef s32 (*PlacementGridGetModelFn)(LinkResource *self, s32 model, s32 cell, CellPlacement *placement);

PlacementGrid *New_PlacementGrid(char *name) {
    PlacementGrid *self;
    PlacementGridMethods *table;

    self = BMemPMgrAlloc(0x34);
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

    if (cell < 0x190) {
        if (placement->next != 0) {
            rec = (PlacementGridRecord *)((u8 *)self->buffer + placement->next);
            placement->chained = 1;
        } else {
            rec = (PlacementGridRecord *)(cell * 12 + 8 + (u8 *)self->buffer);
            placement->chained = 0;
        }
        placement->next = rec->next;
        if (rec->present != 0) {
            row = cell / 20;
            col = cell - row * 20;
            placement->x = (col << 11) + 0x400;
            placement->y = (s32)rec->y << 11;
            placement->z = (row << 11) + 0x400;
            placement->rotY = rec->rotY << 10;
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

s32 func_8002C3B8(void) {
    return 0;
}

void VabDriver__VabDriver(void) {}

void VabDriver__Destroy(void) {}

void VabDriver__NoOpSlot40(void) {
    char buf[0x40];
}

void VabDriver__Open(void) {
    char buf[0x40];
}

void VabDriver__Close(void) {}

void VabDriver__Seek(void) {}

void VabDriver__NoOpSlot50(void) {}
