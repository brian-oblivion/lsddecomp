/*
 * PlacementGrid's methods (include/placement_grid.h: the model placements
 * of one map chunk's 20 x 20 cells), New_PlacementGrid to
 * GetPlacementGridMethods: its allocator, ctor, finalize, read-done flag,
 * the processBuffer occupant that turns one placement record per call into
 * a CellPlacement and its model, and the table getter. ReturnZero follows;
 * nothing calls it or points at it. NullDriver, VabStreamObj and the
 * SoundCueSet functions follow in null_driver.c, vab_stream_obj.c and
 * sound_cue_set.c.
 *
 * The type LinkResource's getModel is called through,
 * PlacementGridGetModelFn, is this file's own. The method table ends the
 * file.
 */
#include "common.h"
#include "placement_grid.h"
#include "link_resource.h"
#include "stage_map.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/** @brief What `buffer` points at: 8 bytes nothing here reads, then each
 * cell's first record, one per cell of the chunk's 20 x 20 lattice,
 * row-major. A cell's further records are reached by the byte offset in
 * `next`. */
typedef struct PlacementGridBuffer {
    u8 pad0[8];
    PlacementGridRecord cells[STAGE_SLOT_LATTICE_CELLS]; /**< each cell's first record, row-major */
} PlacementGridBuffer;

/* LinkResource__GetModel as PlacementGrid__ResolveEntry calls it through
 * `linkResource`'s getModel (+0x080). The occupant reads only (self, index). */
/* MATCHING: all four arguments, as retail passes them, though the occupant reads two */
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

void PlacementGrid__OnRequestDone(PlacementGrid *self) {
    self->loaded = 1;
    GetActiveDataSourceMethods()->onRequestDone((FileResource *)self);
}

/* Fill `placement` from cell `cell`'s first record, or, when
 * placement->next is set, from the record it points at, and return the
 * record's model from linkResource. -1: the record is empty; 0: `cell` is
 * past the lattice, the caller's end of the walk. */
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
            placement->unused2C = rec->unused1;
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

/* The method table. A (void *) entry is a function whose declared type
 * differs from its slot's: a method inherited from a parent class and
 * declared on the parent's type. */

/* PlacementGrid (include/placement_grid.h): FileResource's table with the
 * ctor, finalize and onRequestDone, and processBuffer's occupant,
 * ResolveEntry. */
PlacementGridMethods gPlacementGridMethods = {
    /* +0x000 header */ PLACEMENTGRID_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ PlacementGrid__PlacementGrid,
    /* +0x00C finalize */ PlacementGrid__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)BasicClass__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 slot40 */ NULL,
    /* +0x044 open */ NULL,
    /* +0x048 close */ NULL,
    /* +0x04C seek */ NULL,
    /* +0x050 slot50 */ NULL,
    /* +0x054 read */ NULL,
    /* +0x058 loadFile */ NULL,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ PlacementGrid__OnRequestDone,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ PlacementGrid__ResolveEntry,
};
