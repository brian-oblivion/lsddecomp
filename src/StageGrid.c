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

INCLUDE_ASM("asm/nonmatchings/StageGrid", GetMoodFromStageChunk);
