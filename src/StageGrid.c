/* 2 queued (GetStageChunkFromMood / GetMoodFromStageChunk).
 * DELIBERATELY UNWORKED in round 2026-08-29-a: a 2-function queue is too thin
 * to be worth a runner's provisioning cost. Both need the STAGE_CHUNK_MOODS
 * data slot understood first; that is head or warm-session work.
 */
#include "common.h"
#include "StageGrid.h"

/* The dimensions table has 14 entries; func_800494B4 and the guard in
 * GetStageGridDimensionsTable are the only two places that constant appears. */
#define STAGE_GRID_DIMENSIONS_COUNT 14

s32 func_800494B4(void) {
    return STAGE_GRID_DIMENSIONS_COUNT;
}

StageGridDimensions *GetStageGridDimensionsTable(s32 *count) {
    if (count != NULL) {
        *count = STAGE_GRID_DIMENSIONS_COUNT;
    }
    return STAGE_GRID_DIMENSIONS;
}

StageGridDimensions *GetStageGridDimensions(s32 index) {
    return GetStageGridDimensionsTable(NULL) + index;
}

INCLUDE_ASM("asm/nonmatchings/StageGrid", GetStageChunkFromMood);

MoodGraphPoint *GetMoodFromStageChunk(s32 stage, StageChunk *chunk) {
    return STAGE_CHUNK_MOODS[stage] + chunk->row * STAGE_GRID_DIMENSIONS[stage].columns + chunk->column;
}
