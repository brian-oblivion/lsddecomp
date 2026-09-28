#ifndef TILEMAP_H
#define TILEMAP_H

#include "FileResource.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

/*
 * TileMap -- a FileResource data source (class id 0x203, method table
 * gTileMapMethods) whose own fields, +0x02C..+0x03B, are exactly libgs's GsMAP
 * (LIBGS.H: cellw, cellh, ncellw, ncellh, base, index). Methods in
 * src/graphics/graphics_resources.c. No classes derive from it (`typeviews.py --tree`), so
 * there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TileMap__TileMap's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize forwards to the active
 * driver's, as TimImage and TimBlockSrc do.
 *
 * The name is the GsMAP's: TileMap__BuildMap fills cellw/cellh = 16,
 * ncellw = 20, ncellh = 15 (a 320 x 240 screen of 16 x 16 cells),
 * allocates the ncellw * ncellh u16 index table and fills it 0..n-1, and
 * takes `base` from its atlas's cells; BgLayer__Reset
 * (include/BgLayer.h) points a GsBG's map at &map and sizes the layer from
 * cellw * ncellw by cellh * ncellh.
 *
 * How it is used, at the one New_TileMap call site (TaskCore__TaskCore,
 * src/app/task.c): New_TileAtlas(0), then New_TileMap(0, atlas), then
 * New_BgLayer(tileMap, 1); TaskCore__Finalize releases the three (+0x004).
 * The atlas is a TileAtlas (gTileAtlasMethods, include/TileAtlas.h, by tag here):
 * its `cells` is the 300-GsCELL array BuildMap copies into map.base.
 *
 * SLOTS (`classtable.py gTileMapMethods --vs gFileResourceMethods`, 30 against 30):
 *  - +0x008 ctor, TileMap__TileMap(self, arg1, atlas): the active driver's
 *    ctor, this table, atlas at +0x03C, loaded = 0; with arg1 == 0,
 *    defaultGrid = 1, loadState = 0 and onRequestDone (+0x064). What a nonzero arg1
 *    means is not shown: the one caller passes 0;
 *  - +0x00C finalize, TileMap__Finalize: frees map.index, then the active
 *    driver's finalize;
 *  - +0x064 onRequestDone, TileMap__Load: unless loadState is set, +0x078 and
 *    loaded = 1. It calls +0x078 with NO argument ($a0 is never set up;
 *    TileMap__Load's report), through TileMapBuildMapFn;
 *  - +0x078 is FileResource's `processBuffer` (NULL there); this table's
 *    occupant is TileMap__BuildMap. No own slots past it.
 *
 * FIELDS: the GsMAP at +0x02C, then the atlas, then two u16 flags; the
 * object is 0x44 bytes (New_TileMap).
 *
 * GsMAP is <libgs.h>'s, so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */

struct TileAtlas;

typedef struct TileMap TileMap;
typedef struct TileMapMethods TileMapMethods;

struct TileMapMethods {
    FILERESOURCE_SLOTS(TileMap, (TileMap * self, s32 arg1, struct TileAtlas *atlas));
    /* +0x078 is FileResource's processBuffer; this table's occupant is
     * TileMap__BuildMap, called through TileMapBuildMapFn. */
}; /* 30 slots, 0x7C bytes */

struct TileMap {
    FILERESOURCE_FIELDS(TileMapMethods);
    /* +0x02C */ GsMAP map; /* TileMap__BuildMap fills it; BgLayer__Reset points its GsBG here */
    /* +0x03C */ struct TileAtlas *atlas; /* include/TileAtlas.h: the ctor's third argument; BuildMap reads its cells */
    /* +0x040 */ u16 defaultGrid; /* 1 from the ctor when arg1 == 0; BuildMap lays out the 20 x 15 grid only when set */
    /* +0x042 */ u16 loaded; /* 0 from the ctor, 1 from TileMap__Load after BuildMap */
}; /* 0x44 bytes: New_TileMap */

/* TileMap__BuildMap as TileMap__Load calls it: no argument (see the
 * banner). */
typedef void (*TileMapBuildMapFn)();

extern TileMapMethods gTileMapMethods;
extern TileMapMethods *GetTileMapMethods(void); /* returns &gTileMapMethods */

TileMap *New_TileMap(s32 arg0, struct TileAtlas *atlas); /* BMemPMgrAlloc(0x44), then ctor */
void TileMap__TileMap(TileMap *self, s32 arg1, struct TileAtlas *atlas);
void TileMap__Finalize(TileMap *self);
void TileMap__Load(TileMap *self);
void TileMap__BuildMap(TileMap *self);

#endif
