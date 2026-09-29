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

/* The number of stages, STG00 to STG13 on the disc: one entry each in
 * sStageGridDimensions and sStageChunkMoods (sStage00ChunkMoods ..
 * sStage13ChunkMoods). GetStageGridDimensionsCount returns it,
 * GetStageGridDimensionsTable writes it out, and GetStageChunkFromMood bounds
 * its stage loop by it. */
#define STAGE_COUNT 14

/* clang-format off */
/* Each stage's grid, STG00 to STG13. */
StageGridDimensions sStageGridDimensions[STAGE_COUNT] = {
    /*       cols rows vert */
    /*  0 */ { 1,   5,   1},
    /*  1 */ { 3,   2,   0},
    /*  2 */ { 6,   6,   0},
    /*  3 */ {16,  16,   0},
    /*  4 */ { 6,   5,   0},
    /*  5 */ { 5,   6,   0},
    /*  6 */ { 1,   6,   1},
    /*  7 */ { 5,   1,   0},
    /*  8 */ { 1,   3,   0},
    /*  9 */ { 1,   2,   0},
    /* 10 */ { 3,   1,   0},
    /* 11 */ { 4,   3,   0},
    /* 12 */ { 4,   5,   0},
    /* 13 */ { 2,   2,   0},
};

/* The mood each chunk owns, {dynamic, upper}, one table per stage laid out
 * rows x columns as sStageGridDimensions gives them. */
MoodGraphPoint sStage00ChunkMoods[5 * 1] = {
    {  0, -3},
    {  1,  1},
    { -1,  1},
    {  0,  0},
    { -5, -7},
};

MoodGraphPoint sStage01ChunkMoods[2 * 3] = {
    {  7,  1}, {  1,  2}, {  8, -6},  /* row 0 */
    {  9,  0}, {  0, -9}, { -8, -6},  /* row 1 */
};

MoodGraphPoint sStage02ChunkMoods[6 * 6] = {
    {  0,  9}, {  1,  5}, {  2,  5}, {  2,  4}, {  2,  3}, {  3,  3},  /* row 0 */
    {  3,  2}, {  4,  2}, {  4,  1}, {  3,  1}, {  3,  0}, {  2,  0},  /* row 1 */
    {  2, -1}, {  1, -1}, {  1, -2}, {  1, -4}, {  9,  0}, {  1, -5},  /* row 2 */
    {  0, -5}, { -1, -5}, { -1, -4}, { -1, -2}, { -1, -1}, { -2, -1},  /* row 3 */
    { -2,  0}, { -3,  0}, { -3,  1}, { -4,  1}, { -4,  2}, { -3,  2},  /* row 4 */
    { -3,  3}, { -2,  3}, { -2,  4}, { -2,  5}, { -1,  5}, {  1,  3},  /* row 5 */
};

MoodGraphPoint sStage03ChunkMoods[16 * 16] = {
    {  9,  0}, {  9,  0}, {  6,  7}, {  6,  6}, {  9,  0}, {  9,  0}, {  7,  6}, {  9,  0},  /* row 0 */
    {  9,  0}, {  9,  0}, {  7,  5}, {  9,  0}, {  8,  5}, {  9,  5}, {  9,  0}, {  9,  0},
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  4}, {  9,  3}, {  9,  2}, {  9,  1}, {  9,  0},  /* row 1 */
    {  9, -1}, {  9,  0}, {  9, -2}, {  9, -3}, {  9, -4}, {  9, -5}, {  8, -5}, {  7, -5},
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0}, {  7, -6}, {  6, -6}, {  6, -7},  /* row 2 */
    { -6, -7}, { -6, -6}, {  9,  0}, { -7, -6}, { -7, -5}, { -8, -5}, { -9, -5}, {  9,  0},
    {  9,  0}, {  9,  0}, {  9,  0}, { -9, -4}, { -9, -3}, { -9, -2}, { -9, -1}, { -9,  0},  /* row 3 */
    { -9,  1}, { -9,  2}, { -9,  3}, { -9,  4}, { -9,  5}, { -8,  5}, { -7,  5}, { -7,  6},
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0}, { -6,  6}, { -6,  7}, { -1,  7}, {  0,  7},  /* row 4 */
    {  1,  7}, {  1,  6}, {  2,  6}, {  3,  6}, {  4,  6}, {  5,  6}, {  5,  5}, {  9,  0},
    {  9,  0}, {  5,  4}, {  6,  4}, {  7,  4}, {  8,  4}, {  8,  3}, {  8,  2}, {  8,  1},  /* row 5 */
    {  8,  0}, {  8, -1}, {  8, -2}, {  8, -3}, {  8, -4}, {  7, -4}, {  6, -4}, {  9,  0},
    {  9,  0}, {  5, -4}, {  5, -5}, {  5, -6}, {  4, -6}, {  3, -6}, {  2, -6}, {  1, -6},  /* row 6 */
    { -1, -6}, { -2, -6}, { -3, -6}, { -4, -6}, { -5, -6}, {  9,  0}, { -5, -5}, {  9,  0},
    {  9,  0}, {  9,  0}, { -5, -4}, { -6, -4}, { -7, -4}, { -8, -4}, { -8, -3}, { -8, -2},  /* row 7 */
    { -8, -1}, { -8,  0}, { -8,  1}, { -8,  2}, { -8,  3}, { -8,  4}, { -7,  4}, {  9,  0},
    {  9,  0}, { -6,  4}, { -5,  4}, { -5,  5}, { -5,  6}, { -4,  6}, { -3,  6}, {  9,  0},  /* row 8 */
    { -2,  6}, { -1,  6}, {  3,  5}, {  4,  5}, {  4,  4}, {  4,  3}, {  5,  3}, {  9,  0},
    {  9,  0}, {  9,  0}, {  6,  3}, {  9,  0}, {  7,  3}, {  7,  2}, {  7, -1}, {  7, -2},  /* row 9 */
    {  7, -3}, {  6, -3}, {  5, -3}, {  4, -4}, {  4, -5}, {  3, -5}, {  9,  0}, {  9,  0},
    {  9,  0}, {  2, -5}, { -2, -5}, { -3, -5}, { -4, -5}, { -4, -4}, {  9,  0}, { -5, -3},  /* row 10 */
    { -6, -3}, { -7, -3}, { -7, -2}, { -7, -1}, { -7,  2}, {  9,  0}, {  9,  0}, {  9,  0},
    {  9,  0}, { -7,  3}, { -6,  3}, { -5,  3}, { -4,  3}, { -4,  4}, { -4,  5}, { -3,  5},  /* row 11 */
    {  0,  5}, {  3,  4}, {  5,  2}, {  6,  2}, {  6,  1}, {  6,  0}, {  6, -1}, {  9,  0},
    {  9,  0}, {  6, -2}, {  5, -2}, {  4, -2}, {  3, -2}, {  3, -3}, {  3, -4}, {  2, -4},  /* row 12 */
    { -2, -4}, { -3, -4}, { -3, -3}, {  9,  0}, { -3, -2}, { -4, -2}, { -5, -2}, {  9,  0},
    {  9,  0}, {  9,  0}, { -6, -2}, { -6, -1}, { -6,  0}, { -6,  1}, { -6,  2}, { -5,  2},  /* row 13 */
    { -3,  4}, { -1,  4}, {  0,  4}, {  1,  4}, {  5,  1}, {  5,  0}, {  5, -1}, {  9,  0},
    {  9,  0}, {  4, -1}, {  3, -1}, {  2, -2}, {  2, -3}, {  1, -3}, { -1, -3}, { -2, -3},  /* row 14 */
    { -2, -2}, { -3, -1}, { -4, -1}, { -5, -1}, { -5,  0}, { -5,  1}, {  9,  0}, {  9,  0},
    {  9,  0}, {  9,  0}, {  4,  0}, { -4,  0}, {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0},  /* row 15 */
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0}, {  0,  0}, {  0,  0}, {  9,  0},
};

MoodGraphPoint sStage04ChunkMoods[5 * 6] = {
    {  5,  9}, {  5,  8}, {  7,  7}, {  4,  7}, {  3,  7}, {  9,  0},  /* row 0 */
    {  2,  7}, { -2,  7}, { -3,  7}, { -4,  7}, { -5,  7}, { -5,  8},  /* row 1 */
    { -5,  9}, { -4,  9}, { -3,  9}, { -2,  9}, { -1,  9}, {  9,  0},  /* row 2 */
    {  1,  9}, {  2,  9}, {  3,  9}, {  4,  9}, {  4,  8}, {  3,  8},  /* row 3 */
    {  2,  8}, {  1,  8}, {  0,  8}, { -1,  8}, { -2,  8}, {  9,  0},  /* row 4 */
};

MoodGraphPoint sStage05ChunkMoods[6 * 5] = {
    {  3, -7}, {  3, -8}, {  4, -8}, {  5, -8}, {  5, -9},  /* row 0 */
    {  9,  0}, {  4, -9}, {  3, -9}, {  2, -9}, {  1, -9},  /* row 1 */
    { -1, -8}, { -1, -9}, { -2, -9}, { -3, -9}, { -4, -9},  /* row 2 */
    {  9,  0}, { -5, -9}, { -5, -8}, { -4, -8}, { -3, -8},  /* row 3 */
    {  9,  0}, { -3, -7}, { -2, -7}, { -1, -7}, {  0, -7},  /* row 4 */
    {  1, -7}, {  2, -7}, {  2, -8}, {  1, -8}, {  0, -8},  /* row 5 */
};

MoodGraphPoint sStage06ChunkMoods[6 * 1] = {
    {  0, -4},
    {  0, -2},
    {  0, -1},
    {  0,  1},
    {  0,  2},
    {  1,  2},
};

MoodGraphPoint sStage07ChunkMoods[1 * 5] = {
    {  2,  2}, {  2,  1}, {  1,  0}, { -1,  0}, { -2,  1},
};

MoodGraphPoint sStage08ChunkMoods[3 * 1] = {
    {  7,  0},
    { -7,  0},
    { -7,  1},
};

MoodGraphPoint sStage09ChunkMoods[2 * 1] = {
    {  0, -6},
    { -2, -8},
};

MoodGraphPoint sStage10ChunkMoods[1 * 3] = {
    {  5, -7}, {  4, -7}, { -4, -7},
};

MoodGraphPoint sStage11ChunkMoods[3 * 4] = {
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0},  /* row 0 */
    {  9,  0}, { -4,  8}, { -3,  8}, {  9,  0},  /* row 1 */
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0},  /* row 2 */
};

MoodGraphPoint sStage12ChunkMoods[5 * 4] = {
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0},  /* row 0 */
    {  9,  0}, {  6,  5}, {  6, -5}, {  9,  0},  /* row 1 */
    { -6, -5}, { -6,  5}, {  0,  6}, {  9,  0},  /* row 2 */
    {  9,  0}, {  4, -3}, { -4, -3}, {  9,  0},  /* row 3 */
    {  9,  0}, {  9,  0}, {  9,  0}, {  9,  0},  /* row 4 */
};

MoodGraphPoint sStage13ChunkMoods[2 * 2] = {
    { -1,  3}, {  9,  0},  /* row 0 */
    { -1,  2}, { -2,  2},  /* row 1 */
};

/* Each stage's chunk-mood table, by stage index. */
MoodGraphPoint *sStageChunkMoods[STAGE_COUNT] = {
    sStage00ChunkMoods, sStage01ChunkMoods, sStage02ChunkMoods, sStage03ChunkMoods,
    sStage04ChunkMoods, sStage05ChunkMoods, sStage06ChunkMoods, sStage07ChunkMoods,
    sStage08ChunkMoods, sStage09ChunkMoods, sStage10ChunkMoods, sStage11ChunkMoods,
    sStage12ChunkMoods, sStage13ChunkMoods,
};
/* clang-format on */

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
