/*
 * vab_sound.c -- PlacementGrid, then NullDriver, with ReturnZero between
 * them. VabStreamObj and the SoundCueSet functions follow in
 * vab_stream_obj.c and sound_cue_set.c.
 *
 * PlacementGrid (include/placement_grid.h), New_PlacementGrid to
 * GetPlacementGridMethods: the model placements of one map chunk's 20 x 20
 * cells: its allocator, ctor, finalize, read-done flag, the processBuffer
 * occupant that turns one placement record per call into a CellPlacement and
 * its model, and the table getter. ReturnZero follows; nothing calls it or
 * points at it.
 *
 * The type LinkResource's getModel is called through,
 * PlacementGridGetModelFn, is this file's own. The two classes' method
 * tables end the file.
 */
#include "common.h"
#include <libgte.h>
#include "null_driver.h"
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

void NullDriver__NullDriver(void) {}

void NullDriver__Destroy(void) {}

void NullDriver__NoOpSlot40(void) {
    /* MATCHING: retail reserves a 64-byte frame it never touches. */
    char unused[64];
}

void NullDriver__Open(void) {
    /* MATCHING: the same unused 64-byte frame. */
    char unused[64];
}

void NullDriver__Close(void) {}

void NullDriver__Seek(void) {}

void NullDriver__NoOpSlot50(void) {}

/*
 * The rest of NullDriver's empty slots and its mode accessors
 * (include/null_driver.h).
 *
 * NullDriver is the data source data_source.c selects when it is not reading
 * the CD. GetNullDriverMode, SetNullDriverMode and
 * GetNullDriverUseVSyncCallback answer the queries data_source.c's
 * GetActiveDataSource* functions otherwise forward to CdDriver: they keep
 * the two mode words and report no VSync callback.
 */

/* SetNullDriverMode's two words, read back by GetNullDriverMode. */
extern s32 sNullDriverMode;
extern s32 sNullDriverModeArg;

s32 NullDriver__Read(void) {
    return 0;
}

void NullDriver__LoadFile(void) {}

void NullDriver__RunRequestQueue(void) {}

void NullDriver__RequestLoadFile(void) {}

void NullDriver__StopService(void) {}

void NullDriver__CancelRequests(void) {}

NullDriverMethods *GetNullDriverMethods(void) {
    return &gNullDriverMethods;
}

s32 GetNullDriverMode(s32 *outMode2) {
    if (outMode2 != NULL) {
        *outMode2 = sNullDriverModeArg;
    }
    return sNullDriverMode;
}

s32 SetNullDriverMode(s32 async, s32 mode2) {
    sNullDriverMode = async;
    sNullDriverModeArg = mode2;
    return 1;
}

s32 GetNullDriverUseVSyncCallback(void) {
    return 0;
}

/* The file's two method tables, in the order the image keeps them. A
 * (void *) entry is a function whose declared type differs from its slot's:
 * a method inherited from a parent class and declared on the parent's type,
 * or an empty method declared (void). */

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

/* NullDriver (include/null_driver.h): FileResource's slots up to +0x074,
 * the eleven data-source slots all empty. */
NullDriverMethods gNullDriverMethods = {
    /* +0x000 header */ NULLDRIVER_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ (void *)NullDriver__NullDriver,
    /* +0x00C finalize */ (void *)NullDriver__Destroy,
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
    /* +0x040 slot40 */ NullDriver__NoOpSlot40,
    /* +0x044 open */ (void *)NullDriver__Open,
    /* +0x048 close */ (void *)NullDriver__Close,
    /* +0x04C seek */ (void *)NullDriver__Seek,
    /* +0x050 slot50 */ NullDriver__NoOpSlot50,
    /* +0x054 read */ (void *)NullDriver__Read,
    /* +0x058 loadFile */ (void *)NullDriver__LoadFile,
    /* +0x05C freeBuffer */ (void *)FileResource__FreeBuffer,
    /* +0x060 slot60 */ NoOp,
    /* +0x064 onRequestDone */ (void *)FileResource__OnRequestDone,
    /* +0x068 runRequestQueue */ NullDriver__RunRequestQueue,
    /* +0x06C requestLoadFile */ (void *)NullDriver__RequestLoadFile,
    /* +0x070 stopService */ (void *)NullDriver__StopService,
    /* +0x074 cancelRequests */ (void *)NullDriver__CancelRequests,
};
