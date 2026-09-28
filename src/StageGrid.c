/* StageGrid: the world map's per-stage grid layout, and the two-way lookup
 * between a mood-graph value and the grid cell (stage + chunk) that owns it.
 *
 * Each of the game's stages divides into a `columns` x `rows` grid of chunks
 * (StageGridDimensions, one entry per stage in sStageGridDimensions), and
 * every chunk owns one MoodGraphPoint in a row-major per-stage table
 * (sStageChunkMoods -- 14 pointers to the per-stage STGnn_CHUNK_MOODS
 * arrays). GetMoodFromStageChunk and GetStageChunkFromMood convert between a
 * mood value and its owning (stage, chunk); DreamSys (src/DreamSys.c) is the
 * only caller of either, using them to read the mood at the player's current
 * grid position and to place a mood value back onto the grid.
 * GetStageGridDimensions(Table/Count) are plain accessors over the
 * dimensions table itself.
 *
 * Every function in this unit is matched byte-exact; see
 * docs/match-reports/ for each function's derivation.
 */
#include "common.h"
#include "StageGrid.h"

/* The dimensions table has 14 entries: GetStageGridDimensionsCount returns it,
 * GetStageGridDimensionsTable writes it out, and GetStageChunkFromMood bounds its
 * stage loop by it. */
#define STAGE_GRID_DIMENSIONS_COUNT 14

s32 GetStageGridDimensionsCount(void) {
    return STAGE_GRID_DIMENSIONS_COUNT;
}

StageGridDimensions *GetStageGridDimensionsTable(s32 *count) {
    if (count != NULL) {
        *count = STAGE_GRID_DIMENSIONS_COUNT;
    }
    return sStageGridDimensions;
}

StageGridDimensions *GetStageGridDimensions(s32 stage) {
    return GetStageGridDimensionsTable(NULL) + stage;
}

s32 GetStageChunkFromMood(StageChunk *chunk, MoodGraphPoint *mood) {
    u32 stage;
    s32 row;
    s32 column;
    MoodGraphPoint *chunkMood;
    s32 rows;
    s32 columns;

    for (stage = 0; stage < STAGE_GRID_DIMENSIONS_COUNT; stage++) {
        chunkMood = sStageChunkMoods[stage];
        rows = sStageGridDimensions[stage].rows;
        columns = sStageGridDimensions[stage].columns;
        for (row = 0; row < rows; row++) {
            for (column = 0; column < columns; column++) {
                if (mood->value == chunkMood->value) {
                    chunk->column = column;
                    chunk->row = row;
                    return stage;
                }
                chunkMood++;
            }
        }
    }
    return -1;
}

MoodGraphPoint *GetMoodFromStageChunk(s32 stage, StageChunk *chunk) {
    return sStageChunkMoods[stage] + chunk->row * sStageGridDimensions[stage].columns + chunk->column;
}
