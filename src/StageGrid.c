/* COMPLETE as of round 23 (2026-09-07) -- no queue left in this unit.
 *
 * The last two functions (GetStageChunkFromMood / GetMoodFromStageChunk) were
 * banked from round 2026-08-29-a (with the banked marker `progress.py` keys on,
 * deliberately not spelled out again here -- see below) on the grounds that both
 * needed the STAGE_CHUNK_MOODS data slot understood first. That was correct: the
 * slot is 14 pointers to per-stage row-major arrays of the 2-byte MoodGraphPoint
 * union, dimensioned by STAGE_GRID_DIMENSIONS (14 x 8-byte entries, `bool` = int).
 * Once read out of the executable both functions were shape-correct on the first
 * attempt; see docs/match-reports/ for the two local-type residues that remained.
 *
 * The phrase above is NOT written out in full on purpose. `progress.py` matches
 * the banked marker as a bare whole-file substring, so a sentence merely
 * DESCRIBING the marker banks the unit exactly as the marker would -- round 46
 * lost a whole unit's `fresh` count to that. This unit has no queue today, so
 * quoting it here would have cost nothing; it would have cost the next person
 * who adds one. Refer to the marker, do not quote it.
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

s32 GetStageChunkFromMood(StageChunk *ret, MoodGraphPoint *mood) {
    u32 stage;
    s32 row;
    s32 col;
    MoodGraphPoint *p;
    s32 rows;
    s32 columns;

    for (stage = 0; stage < STAGE_GRID_DIMENSIONS_COUNT; stage++) {
        p = STAGE_CHUNK_MOODS[stage];
        rows = STAGE_GRID_DIMENSIONS[stage].rows;
        columns = STAGE_GRID_DIMENSIONS[stage].columns;
        for (row = 0; row < rows; row++) {
            for (col = 0; col < columns; col++) {
                if (mood->value == p->value) {
                    ret->column = col;
                    ret->row = row;
                    return stage;
                }
                p++;
            }
        }
    }
    return -1;
}

MoodGraphPoint *GetMoodFromStageChunk(s32 stage, StageChunk *chunk) {
    return STAGE_CHUNK_MOODS[stage] + chunk->row * STAGE_GRID_DIMENSIONS[stage].columns + chunk->column;
}
