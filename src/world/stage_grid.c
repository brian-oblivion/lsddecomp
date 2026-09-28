/* stage_grid.c: each stage's grid of map chunks, and the two-way lookup between
 * a mood-graph value and the grid cell (stage + chunk) that owns it.
 *
 * Each of the STAGE_COUNT stages divides into a `columns` x `rows` grid of
 * chunks (StageGridDimensions, one entry per stage in sStageGridDimensions),
 * and every chunk owns one MoodGraphPoint in a row-major per-stage table
 * (sStageChunkMoods: one pointer per stage, to sStage00ChunkMoods ..
 * sStage13ChunkMoods). GetMoodFromStageChunk and GetStageChunkFromMood
 * convert between a chunk and its mood value; DreamSys (src/world/dream_sys.c) is
 * the only caller of either, reading the mood of the player's current chunk
 * (DreamSys__LogChunkMood) and finding the stage and chunk that own a given
 * mood value to choose a spawn point (GenerateInitialSpawn).
 * GetStageGridDimensionsCount, GetStageGridDimensionsTable and
 * GetStageGridDimensions are plain accessors over the dimensions table;
 * GetStageMapChunkRecordXY (src/cd/game_files.c) turns a chunk's (x, y) into the
 * index of its map-chunk file with the same row-major `columns` stride.
 */
#include "common.h"
#include "stage_grid.h"

/* The two per-stage tables (splat data), read only here. */
extern StageGridDimensions sStageGridDimensions[];
extern MoodGraphPoint *sStageChunkMoods[];

/* The number of stages, STG00 to STG13 on the disc: one entry each in
 * sStageGridDimensions and sStageChunkMoods (sStage00ChunkMoods ..
 * sStage13ChunkMoods). GetStageGridDimensionsCount returns it,
 * GetStageGridDimensionsTable writes it out, and GetStageChunkFromMood bounds
 * its stage loop by it. */
#define STAGE_COUNT 14

s32 GetStageGridDimensionsCount(void) {
    return STAGE_COUNT;
}

StageGridDimensions *GetStageGridDimensionsTable(s32 *count) {
    if (count != NULL) {
        *count = STAGE_COUNT;
    }
    return sStageGridDimensions;
}

StageGridDimensions *GetStageGridDimensions(s32 stage) {
    return GetStageGridDimensionsTable(NULL) + stage;
}

/* Searches every stage's chunks in row-major order for the first whose mood
 * equals *mood. Returns -1, leaving *chunk unwritten, when none does. */
s32 GetStageChunkFromMood(StageChunk *chunk, MoodGraphPoint *mood) {
    u32 stage; /* MATCHING: a signed counter compiles slti, not sltiu */
    s32 row;
    s32 column;
    MoodGraphPoint *chunkMood;
    /* MATCHING: s32, not the fields' s16, which re-sign-extends per use */
    s32 rows;
    s32 columns;

    for (stage = 0; stage < STAGE_COUNT; stage++) {
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
    /* MATCHING: pointer additions, not &moods[row * columns + column], which
     * scales the summed index once (one word short) */
    return sStageChunkMoods[stage] + chunk->row * sStageGridDimensions[stage].columns + chunk->column;
}
