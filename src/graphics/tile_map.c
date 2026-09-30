/*
 * TileMap's methods (include/tile_map.h: the GsMAP of the background grid
 * over a TileAtlas's cells, built rather than loaded), in ROM order: the
 * allocator, ctor and finalize, onRequestDone (Load) and BuildMap, ending
 * with its getter GetTileMapMethods; its method table closes the file. A
 * (void *) entry in it is a method whose declared parameters differ from
 * the slot's, usually one inherited from a parent class and declared on
 * the parent's type.
 */
#include "common.h"
#include "tile_map.h"
#include "tile_atlas.h"
#include "bmem_pmgr.h"
#include "data_source.h"

/* Allocate and construct a TileMap over `atlas`. */
TileMap *New_TileMap(s32 source, TileAtlas *atlas) {
    TileMap *obj = BMemPMgrAlloc(sizeof(TileMap));

    if (obj != NULL) {
        GetTileMapMethods()->ctor(obj, source, atlas);
        return obj;
    }
    return NULL;
}

/* ctor (+0x008): with no `source` (the one caller's), build the default
 * grid at once. What a nonzero `source` would be, no caller shows. */
void TileMap__TileMap(TileMap *self, s32 source, TileAtlas *atlas) {
    s32 unused[8]; /* MATCHING: never used; it gives retail's 0x40-byte stack */

    GetActiveDataSourceMethods()->ctor((FileResource *)self);
    self->methods = GetTileMapMethods();
    self->atlas = atlas;
    self->loaded = 0;
    if (source == 0) {
        self->defaultGrid = 1;
        self->loadState = 0;
        self->methods->onRequestDone(self);
    }
}

/* finalize (+0x00C): free the map's index table. */
void TileMap__Finalize(TileMap *self) {
    BMemPMgrFree(self->map.index);
    GetActiveDataSourceMethods()->finalize((FileResource *)self);
}

/* onRequestDone (+0x064): when idle, BuildMap. */
void TileMap__Load(TileMap *self) {
    if (self->loadState == 0) {
        ((TileMapBuildMapFn)self->methods->processBuffer)(self);
        self->loaded = 1;
    }
}

/* +0x078: map the atlas's cells, the default grid indexing them in order;
 * without the default grid, or when the index table cannot be had, free
 * the buffer instead. */
void TileMap__BuildMap(TileMap *self) {
    s32 n;
    s32 i;
    u16 *p;

    self->map.base = self->atlas->cells;
    if (self->defaultGrid != 0) {
        self->map.ncellw = TILEMAP_COLS;
        self->map.cellw = TILE_SIZE;
        self->map.cellh = TILE_SIZE;
        self->map.ncellh = TILEMAP_ROWS;
        n = self->map.ncellw * self->map.ncellh;
        self->map.index = BMemPMgrAlloc(n * sizeof(*self->map.index));
        if (self->map.index != NULL) {
            p = self->map.index;
            for (i = 0; i < n; i++) {
                *p++ = i;
            }
            return;
        }
    }
    self->methods->freeBuffer(self);
}

TileMapMethods *GetTileMapMethods(void) {
    return &gTileMapMethods;
}

/* TileMap: load, then build the GsMAP. */
TileMapMethods gTileMapMethods = {
    /* +0x000 header */ TILEMAP_CLASS_ID,
    /* +0x004 release */ (void *)FileResource__Release,
    /* +0x008 ctor */ TileMap__TileMap,
    /* +0x00C finalize */ TileMap__Finalize,
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
    /* +0x064 onRequestDone */ TileMap__Load,
    /* +0x068 runRequestQueue */ NULL,
    /* +0x06C requestLoadFile */ NULL,
    /* +0x070 stopService */ NULL,
    /* +0x074 cancelRequests */ NULL,
    /* +0x078 processBuffer */ TileMap__BuildMap,
};
