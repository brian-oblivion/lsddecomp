/**
 * @file tile_map.h
 * @brief TileMap, the FileResource whose fields are one libgs GsMAP over a
 *        TileAtlas's cells, and its method table.
 *
 * GsMAP is <libgs.h>'s, so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */
#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "file_resource.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

struct TileAtlas;

typedef struct TileMap TileMap;
typedef struct TileMapMethods TileMapMethods;

/** TileMap's class id (gTileMapMethods word +0x000). */
#define TILEMAP_CLASS_ID 0x203

/** @name The background grid
 * TileMap's default grid and TileAtlas's cells (include/tile_atlas.h):
 * 20 x 15 cells of 16 x 16 texels, one atlas cell per map cell.
 * @{ */
#define TILEMAP_COLS 20 /**< cells across */
#define TILEMAP_ROWS 15 /**< cells down */
#define TILE_SIZE 16    /**< a cell's width and height, in texels */

/** @} */

/**
 * @brief TileMap's method table, gTileMapMethods: FileResource's slots, with
 *        no new ones.
 *
 * It overrides +0x008 ctor (TileMap__TileMap), +0x00C finalize
 * (TileMap__Finalize) and +0x064 onRequestDone (TileMap__Load). The inherited
 * +0x078 processBuffer holds TileMap__BuildMap, called through
 * TileMapBuildMapFn.
 */
struct TileMapMethods {
    FILERESOURCE_SLOTS(TileMap, (TileMap * self, s32 source, struct TileAtlas *atlas));
};

/**
 * @brief A tile map (class id 0x203): a libgs GsMAP indexing a TileAtlas's
 *        cells, by default a 20 x 15 grid of 16 x 16 cells (one 320 x 240
 *        screen) with cell i at index i.
 *
 * BgLayer__Reset points a GsBG's map at `map` and sizes the layer from it.
 * Parent FileResource, through the active data-source driver; no subclasses.
 * Methods in src/graphics/tile_map.c. The object is 0x44 bytes
 * (New_TileMap).
 *
 * Its one builder is TaskCore__TaskCore (src/app/task.c): New_TileAtlas(0),
 * then New_TileMap(0, atlas), then New_BgLayer(tileMap, BGLAYER_MODE_SCREEN); TaskCore__Finalize
 * releases the three.
 */
struct TileMap {
    FILERESOURCE_FIELDS(TileMapMethods);
    /* +0x02C */ GsMAP map; /**< the map TileMap__BuildMap fills; BgLayer__Reset points its GsBG here */
    /* +0x03C */ struct TileAtlas *atlas; /**< the ctor's atlas; BuildMap takes its cells as map.base */
    /* +0x040 */ u16 defaultGrid; /**< 1 from the ctor when `source` is 0; BuildMap lays out the 20 x 15 grid only when set */
    /* +0x042 */ u16 loaded; /**< 0 from the ctor, 1 from TileMap__Load after BuildMap */
};

/** @brief TileMap__BuildMap as TileMap__Load calls it through the
 *         untyped processBuffer slot. */
typedef void (*TileMapBuildMapFn)(TileMap *self);

/** TileMap's method table. */
extern TileMapMethods gTileMapMethods;

/**
 * @brief Returns TileMap's method table.
 * @return &gTileMapMethods.
 */
extern TileMapMethods *GetTileMapMethods(void);

/**
 * @brief Allocates a TileMap from the pool and constructs it over an atlas.
 * @param source 0 to build the default grid at once (the one caller's); what
 *               a nonzero value means, no caller shows.
 * @param atlas  The atlas whose cells the map indexes.
 * @return The new map, or NULL when the pool is exhausted.
 */
TileMap *New_TileMap(s32 source, struct TileAtlas *atlas);

/**
 * @brief Constructor (slot +0x008): the active driver's, the atlas kept; with
 *        `source` 0, marks the default grid and runs onRequestDone to build it.
 * @param self   The object to construct.
 * @param source 0 for the default grid.
 * @param atlas  The atlas whose cells the map indexes.
 */
void TileMap__TileMap(TileMap *self, s32 source, struct TileAtlas *atlas);

/**
 * @brief Finalizer (slot +0x00C): frees the map's index table, then the active
 *        driver's finalizer.
 * @param self The object being destroyed.
 */
void TileMap__Finalize(TileMap *self);

/**
 * @brief Slot +0x064 (onRequestDone): when no load is in progress, builds the
 *        map and sets `loaded`.
 * @param self The map.
 */
void TileMap__Load(TileMap *self);

/**
 * @brief Slot +0x078 (processBuffer): takes the atlas's cells as the map's
 *        base and, with the default grid, lays out the 20 x 15 index table
 *        0..299. Without the default grid, or when the table cannot be
 *        allocated, it frees the buffer instead.
 * @param self The map. TileMap__Load calls it with no argument, which works
 *             because Load's own `self` is still in place.
 */
void TileMap__BuildMap(TileMap *self);

#endif
