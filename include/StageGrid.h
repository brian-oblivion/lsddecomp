#ifndef STAGE_GRID
#define STAGE_GRID

#include "common.h"

typedef struct StageGridDimensions {
    s16 columns;
    s16 rows;
    bool isVertical;
} StageGridDimensions;

typedef struct StageChunk {
    s8 column;
    s8 row;
} StageChunk;

struct simplePair {
    s8 x;
    s8 y;
};

/* @brief Number of stages, the length of both per-stage tables (14). */
extern s32 GetStageGridDimensionsCount(void);

/* @brief Gets the stage-dimensions table, optionally writing its length out. */
/* @param count Optional out-parameter; unwritten if NULL. */
extern StageGridDimensions *GetStageGridDimensionsTable(s32 *count);

/* @brief Gets the dimensions entry for a single stage. */
extern StageGridDimensions *GetStageGridDimensions(s32 stage);

/* @brief Gets the stage and chunk associated with a given point on the mood graph. */
/* @param chunk Pointer where the found chunk will be written to */
/* @param mood The mood point to be checked */
/* @return Stage index the found chunk belongs to */
extern s32 GetStageChunkFromMood(StageChunk *chunk, MoodGraphPoint *mood);

/* @brief Gets the mood contribution associated with a given stage and chunk. */
/* @param stage The stage index the chunk belongs to. */
/* @param chunk The chunk coordinates to be checked */
/* @return Mood graph point of the given chunk's mood */
extern MoodGraphPoint *GetMoodFromStageChunk(s32 stage, StageChunk *chunk);

#endif