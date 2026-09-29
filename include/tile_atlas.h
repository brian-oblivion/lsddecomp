/**
 * @file tile_atlas.h
 * @brief TileAtlas, the FileResource that builds the 300 libgs GsCELLs a
 *        TileMap indexes, and its method table.
 *
 * GsCELL is <libgs.h>'s, so an includer takes Sony's headers first
 * (`common.h`, <libgte.h>, <libgpu.h>, <libgs.h>).
 */
#ifndef TILE_ATLAS_H
#define TILE_ATLAS_H

#include "file_resource.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>

typedef struct TileAtlas TileAtlas;
typedef struct TileAtlasMethods TileAtlasMethods;

/** TileAtlas's class id (gTileAtlasMethods word +0x000). */
#define TILEATLAS_CLASS_ID 0x303

/**
 * @brief TileAtlas's method table, gTileAtlasMethods: FileResource's slots,
 *        with no new ones.
 *
 * It overrides +0x008 ctor (TileAtlas__TileAtlas), +0x00C finalize
 * (TileAtlas__Finalize) and +0x064 onRequestDone (TileAtlas__Load); +0x058
 * loadFile is NULL, as a TileAtlas loads no file. The inherited +0x078
 * processBuffer holds TileAtlas__BuildCells, called through
 * TileAtlasBuildCellsFn.
 */
struct TileAtlasMethods {
    FILERESOURCE_SLOTS(TileAtlas, (TileAtlas * self, s32 source));
};

/**
 * @brief A tile atlas (class id 0x303): 300 GsCELLs of 16 x 16 texels over
 *        VRAM from x 640, built rather than loaded.
 *
 * TileMap__BuildMap takes `cells` as its GsMAP's base. Parent FileResource,
 * through the active data-source driver; no subclasses. Methods in
 * src/graphics/graphics_resources.c. The object is 0x38 bytes
 * (New_TileAtlas).
 *
 * Its one builder is TaskCore__TaskCore (src/app/task.c): New_TileAtlas(0),
 * then New_TileMap(0, atlas), then New_BgLayer(tileMap, 1); TaskCore__Finalize
 * releases the three.
 */
struct TileAtlas {
    FILERESOURCE_FIELDS(TileAtlasMethods);
    /* +0x02C */ GsCELL *cells; /**< 300 of them, from TileAtlas__BuildCells; a TileMap's map.base */
    /* +0x030 */ u16 defaultCells; /**< 1 from the ctor when `source` is 0; BuildCells builds only when set */
    /* +0x032 */ u16 loaded; /**< 0 from the ctor, 1 from TileAtlas__Load after BuildCells */
    /* +0x034 */ void *unk34; /**< 0 from the ctor and freed by Finalize; no TileAtlas method sets it */
};

/** @brief TileAtlas__BuildCells as TileAtlas__Load calls it through the
 *         processBuffer slot: with no argument. */
typedef void (*TileAtlasBuildCellsFn)();

/** TileAtlas's method table. */
extern TileAtlasMethods gTileAtlasMethods;

/**
 * @brief Returns TileAtlas's method table.
 * @return &gTileAtlasMethods.
 */
extern TileAtlasMethods *GetTileAtlasMethods(void);

/**
 * @brief Allocates a TileAtlas from the pool and constructs it.
 * @param source 0 to build the default cells at once (the one caller's);
 *               what a nonzero value means, no caller shows.
 * @return The new atlas, or NULL when the pool is exhausted.
 */
TileAtlas *New_TileAtlas(s32 source);

/**
 * @brief Constructor (slot +0x008): the active driver's; with `source` 0, marks
 *        the default cells and runs onRequestDone to build them.
 * @param self   The object to construct.
 * @param source 0 for the default cells.
 */
void TileAtlas__TileAtlas(TileAtlas *self, s32 source);

/**
 * @brief Finalizer (slot +0x00C): frees `unk34` and the cells, then the
 *        active driver's finalizer.
 * @param self The object being destroyed.
 */
void TileAtlas__Finalize(TileAtlas *self);

/**
 * @brief Slot +0x064 (onRequestDone): when no load is in progress, builds the
 *        cells and sets `loaded`.
 * @param self The atlas.
 */
void TileAtlas__Load(TileAtlas *self);

/**
 * @brief Slot +0x078 (processBuffer): with the default cells, allocates the
 *        300 GsCELLs and lays them out row by row across VRAM x 640..960, u
 *        and v restarting at each 15-bit texture page.
 * @param self The atlas. TileAtlas__Load calls it with no argument, which
 *             works because Load's own `self` is still in place.
 */
void TileAtlas__BuildCells(TileAtlas *self);

#endif
