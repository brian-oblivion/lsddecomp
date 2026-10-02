/*
 * TileAtlas's methods (include/tile_atlas.h: the GsCELLs of the background
 * grid, one per map cell, laid out across 15-bit texture pages; built
 * rather than loaded), in ROM order: the allocator, ctor and finalize,
 * onRequestDone (Load) and BuildCells, ending with its getter
 * GetTileAtlasMethods; its method table closes the file. A (void *) entry
 * in it is a method whose declared parameters differ from the slot's,
 * usually one inherited from a parent class and declared on the parent's
 * type.
 */
#include "common.h"
#include "tile_map.h"
#include "tile_atlas.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/* TileAtlas's cells, one per cell of the background grid (TILEMAP_COLS x
 * TILEMAP_ROWS, include/tile_map.h), over 15-bit texture pages (GetTPage
 * tp 2) from VRAM x 640 to 960. */
#define TILE_ATLAS_CELLS (TILEMAP_COLS * TILEMAP_ROWS)
#define TILE_ATLAS_X 640
#define TILE_ATLAS_X_END 960
#define TPAGE_15BIT 2    /* GetTPage's tp: 15-bit direct */
#define TPAGE_WIDTH 64   /* a texture page's VRAM width */
#define TPAGE_HEIGHT 256 /* ... and height */
#define TPAGE_LOWER 0x10 /* the tpage word's page-y bit: pages from VRAM y 256 */

/* Allocate and construct a TileAtlas. */
TileAtlas *New_TileAtlas(s32 source) {
    TileAtlas *obj = BMemPMgrAlloc(sizeof(TileAtlas));

    if (obj != NULL) {
        GetTileAtlasMethods()->ctor(obj, source);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): with no `source` (the one caller's), build the default
 * cells at once. */
void TileAtlas__TileAtlas(TileAtlas *self, s32 source) {
    s32 unused[8]; /* MATCHING: never used; it gives retail's 0x40-byte stack */

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTileAtlasMethods();
    self->unused34 = 0;
    self->loaded = 0;
    if (source == 0) {
        self->defaultCells = 1;
        self->loadState = 0;
        self->methods->onRequestDone(self);
    }
}

/* finalize (+0x00C): free unused34 (which no method here sets) and the
 * cells. */
void TileAtlas__Finalize(TileAtlas *self) {
    BMemPMgrFree(self->unused34);
    BMemPMgrFree(self->cells);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): when idle, BuildCells. */
void TileAtlas__Load(TileAtlas *self) {
    s32 unused[8]; /* MATCHING: never used; it gives retail's 0x38-byte stack */

    if (self->loadState == 0) {
        ((TileAtlasBuildCellsFn)self->methods->processBuffer)(self);
        self->loaded = 1;
    }
}

/* +0x078: with the default cells, lay the atlas's cells out row by row
 * across VRAM x TILE_ATLAS_X..TILE_ATLAS_X_END, u and v restarting at each
 * texture page. */
void TileAtlas__BuildCells(TileAtlas *self) {
    GsCELL *c;
    s32 x = TILE_ATLAS_X;
    s32 u;
    s32 v;
    s32 tpage;
    s32 i;
    s32 n;

    if (self->defaultCells != 0) {
        v = 0;
        u = 0;
        tpage = GetTPage(TPAGE_15BIT, 0, TILE_ATLAS_X, 0);
        self->cells = BMemPMgrAlloc(TILE_ATLAS_CELLS * sizeof(GsCELL));
        if (self->cells != NULL) {
            i = 0;
            c = self->cells;
            n = TILE_ATLAS_CELLS;
            for (; i < n; i++, c++) {
                c->u = u;
                c->tpage = tpage;
                c->v = v;
                c->cba = 0;
                c->flag = 0;
                u += TILE_SIZE;
                x += TILE_SIZE;
                if (x >= TILE_ATLAS_X_END) {
                    u = 0;
                    x = TILE_ATLAS_X;
                    v += TILE_SIZE;
                }
                if ((x & (TPAGE_WIDTH - 1)) == 0) {
                    tpage = x >> 6; /* x / TPAGE_WIDTH */
                    if (v >= TPAGE_HEIGHT) {
                        tpage += TPAGE_LOWER;
                    }
                    u = 0;
                }
            }
        }
    }
}

TileAtlasMethods *GetTileAtlasMethods(void) {
    return &gTileAtlasMethods;
}

/* TileAtlas: load, then build the GsCELLs. */
TileAtlasMethods gTileAtlasMethods = {
    /* +0x000 header */ TILEATLAS_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ TileAtlas__TileAtlas,
    /* +0x00C finalize */ TileAtlas__Finalize,
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
    /* +0x064 onRequestDone */ TileAtlas__Load,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TileAtlas__BuildCells,
};
