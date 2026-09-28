#ifndef TILEATLAS_H
#define TILEATLAS_H

#include "FileResource.h"

/*
 * TileAtlas -- a FileResource data source (class id 0x303, method table
 * gTileAtlasMethods) that builds, instead of loading, an array of 300 libgs
 * GsCELLs: the cell atlas a TileMap's GsMAP indexes (include/TileMap.h).
 * Methods in src/graphics/GraphicsResources.c. No classes derive from it (`typeviews.py
 * --tree`), so there are no FIELDS/SLOTS macros.
 *
 * The ctor chain agrees with the id: TileAtlas__TileAtlas's first call is
 * GetActiveDataSourceMethods()->ctor, and finalize forwards to the active
 * driver's, as TileMap, TimImage and TimBlockSrc do.
 *
 * The name is the cells': TileAtlas__BuildCells allocates 300 libgs
 * GsCELLs (8 bytes each) and fills them as 16 x 16-texel cells over VRAM
 * from x 0x280, u,v stepping by 16, a new texture page every 64 x;
 * TileMap__BuildMap takes `cells` as its GsMAP's base and lays out a 20 x 15
 * index table 0..299 over it.
 *
 * How it is used, at the one New_TileAtlas call site (TaskCore__TaskCore,
 * src/app/Task.c): New_TileAtlas(0), then New_TileMap(0, atlas), then
 * New_BgLayer(tileMap, 1); TaskCore__Finalize releases the three (+0x004).
 *
 * SLOTS (`classtable.py gTileAtlasMethods --vs gFileResourceMethods`, 30 against 30; the
 * words from +0x07C on are gDataSourceClientGetters, not this table):
 *  - +0x008 ctor, TileAtlas__TileAtlas(self, arg1): the active driver's
 *    ctor, this table, unk34 = 0, loaded = 0; with arg1 == 0,
 *    defaultCells = 1, unk2A = 0 and onRequestDone (+0x064). What a nonzero arg1
 *    means is not shown: the one caller passes 0;
 *  - +0x00C finalize, TileAtlas__Finalize: frees unk34 and cells, then the
 *    active driver's finalize;
 *  - +0x058 loadFile is NULL in this table (a TileAtlas loads no file);
 *  - +0x064 onRequestDone, TileAtlas__Load: unless unk2A is set, +0x078 and
 *    loaded = 1. It calls +0x078 with NO argument ($a0 is never set up,
 *    as in TileMap__Load), through TileAtlasBuildCellsFn;
 *  - +0x078 is FileResource's `void *slot78` (NULL there); this table's
 *    occupant is TileAtlas__BuildCells. No own slots past it.
 *
 * FIELDS: the cell array, two u16 flags and a word only Finalize frees;
 * the object is 0x38 bytes (New_TileAtlas).
 *
 * GsCELL is <libgs.h>'s, so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */

typedef struct TileAtlas TileAtlas;
typedef struct TileAtlasMethods TileAtlasMethods;

struct TileAtlasMethods {
    FILERESOURCE_SLOTS(TileAtlas, (TileAtlas * self, s32 arg1));
    /* +0x078 is FileResource's slot78; this table's occupant is
     * TileAtlas__BuildCells, called through TileAtlasBuildCellsFn. */
}; /* 30 slots, 0x7C bytes */

struct TileAtlas {
    FILERESOURCE_FIELDS(TileAtlasMethods);
    /* +0x02C */ GsCELL *cells; /* 300 of them, from TileAtlas__BuildCells; TileMap__BuildMap's map.base */
    /* +0x030 */ u16 defaultCells; /* 1 from the ctor when arg1 == 0; BuildCells builds only when set */
    /* +0x032 */ u16 loaded;       /* 0 from the ctor, 1 from TileAtlas__Load after BuildCells */
    /* +0x034 */ void *unk34; /* 0 from the ctor, freed by Finalize; no TileAtlas method sets it */
}; /* 0x38 bytes: New_TileAtlas */

/* TileAtlas__BuildCells as TileAtlas__Load calls it: no argument (see the
 * banner). */
typedef void (*TileAtlasBuildCellsFn)();

extern TileAtlasMethods gTileAtlasMethods;
extern TileAtlasMethods *GetTileAtlasMethods(void); /* returns &gTileAtlasMethods */

TileAtlas *New_TileAtlas(s32 arg0); /* BMemPMgrAlloc(0x38), then ctor */
void TileAtlas__TileAtlas(TileAtlas *self, s32 arg1);
void TileAtlas__Finalize(TileAtlas *self);
void TileAtlas__Load(TileAtlas *self);
void TileAtlas__BuildCells(TileAtlas *self);

#endif
