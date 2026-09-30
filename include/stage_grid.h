#ifndef STAGE_GRID_H
#define STAGE_GRID_H

#include "common.h"

/**
 * @file stage_grid.h
 * @brief Each stage's grid of map chunks, and the lookup between a chunk and
 *        the mood-graph point it owns (src/world/stage_grid.c).
 *
 * Every stage divides into a `columns` x `rows` grid of chunks, and every
 * chunk owns one MoodGraphPoint in a row-major per-stage table. DreamSys is
 * the only caller of the mood lookups: it logs the mood of the player's
 * current chunk (DreamSys__LogChunkMood) and finds the stage and chunk that
 * own a mood to choose a day's first spawn (GenerateInitialSpawn).
 */

/** The game's stages, each one area of the dream, by the names players know
 * them by. The value is the stage's index in every per-stage table and the
 * number of its directory on the disc (STG00..STG13). */
enum Stage {
    STAGE_BRIGHT_MOON_COTTAGE = 0, /**< the house a dream can begin in, and its courtyard */
    STAGE_PIT_AND_TEMPLE = 1,      /**< grass, brick temples with faces, the marching elephant */
    STAGE_KYOTO = 2,               /**< traditional buildings, shrines and animal statues */
    STAGE_NATURAL_WORLD = 3,       /**< meadow and horses under a blue sky */
    STAGE_HAPPY_TOWN = 4,          /**< the face-pattern floor and floating blocks */
    STAGE_VIOLENCE_DISTRICT = 5,   /**< dark painted city blocks, cars and docks */
    STAGE_MOONLIGHT_TOWER = 6,     /**< a rooftop under the moon */
    STAGE_TEMPLE_DOJO = 7,         /**< a temple interior with an attacking dog */
    STAGE_FLESH_TUNNELS = 8,       /**< the flesh walls, podium and lettered walkway */
    STAGE_CLOCKWORK_MACHINES = 9,  /**< the round mechanical room with the pendulum */
    STAGE_LONG_HALLWAY = 10,       /**< the long green tunnel from the cottage */
    STAGE_SUN_FACES_HEAVE = 11,    /**< the rotating open-mouthed sun */
    STAGE_BLACK_SPACE = 12,        /**< the black-and-white platform and floating blocks */
    STAGE_MONUMENT_PARK = 13,      /**< miniature world landmarks */
    STAGE_COUNT = 14               /**< the number of stages */
};

/** @brief One stage's chunk grid: its size, and how StageMap lays it out. */
typedef struct StageGridDimensions {
    s16 columns; /**< chunks per row; also the row stride of the stage's mood and map-chunk tables */
    s16 rows;    /**< rows of chunks */
    bool isVertical; /**< StageMap's layout switch: 0 takes a chunk's footprint from its cell
                          (StageMap__SetFootprintFromCell), non-zero sets a rectangle */
} StageGridDimensions;

/** @brief A chunk's position in its stage's grid. */
typedef struct StageChunk {
    s8 column; /**< the chunk's column */
    s8 row;    /**< the chunk's row */
} StageChunk;

/**
 * @brief The number of stages, the length of both per-stage tables.
 * @return 14.
 */
extern s32 GetStageGridDimensionsCount(void);

/**
 * @brief The stage-dimensions table, one entry per stage.
 * @param count Where to write the table's length (14), or NULL.
 * @return The table's first entry.
 */
extern StageGridDimensions *GetStageGridDimensionsTable(s32 *count);

/**
 * @brief One stage's grid dimensions.
 * @param stage The stage index; not range-checked.
 * @return That stage's entry in the dimensions table.
 */
extern StageGridDimensions *GetStageGridDimensions(s32 stage);

/**
 * @brief Finds the first chunk, over every stage in order and each stage's
 *        chunks row by row, whose mood equals `mood`.
 * @param chunk Where to write the chunk found; unwritten when none is.
 * @param mood  The mood-graph point to look for (compared whole).
 * @return The stage the chunk belongs to, or -1 when no chunk has that mood.
 */
extern s32 GetStageChunkFromMood(StageChunk *chunk, MoodGraphPoint *mood);

/**
 * @brief The mood-graph point a chunk owns.
 * @param stage The stage index the chunk belongs to.
 * @param chunk The chunk's column and row; not range-checked.
 * @return The chunk's entry in the stage's mood table.
 */
extern MoodGraphPoint *GetMoodFromStageChunk(s32 stage, StageChunk *chunk);

#endif
