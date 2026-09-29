/*
 * dream_aux.c -- the dream's auxiliary entities: one resident Entity kept
 * near the player, and the chunk triggers that spawn Entities as the
 * StageMap loads chunks. include/dream_aux.h describes the lifecycle and
 * the trigger flow; this file holds the tables' walkers in the order the
 * flow runs them: load and release, the world, the lookup, the day and
 * condition tests, and the spawn.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "entity.h"
#include "dream_aux.h"
#include "scene_node.h"
#include "model_data.h"
#include "trigger_world.h"
#include "dream_sys.h"
#include "stage_map.h"
#include <rand.h>

/** @brief One placement: the cell (column, row) inside the chunk, a yaw
 * from sDreamAuxSpawnRotations and an offset inside the cell from
 * sDreamAuxPosTable. */
typedef struct {
    u16 cell;         /**< the cell's key inside the chunk */
    s8 rotationIndex; /**< the yaw, an index into sDreamAuxSpawnRotations */
    s8 offsetIndex;   /**< the offset in the cell, an index into sDreamAuxPosTable */
} DreamAuxSpawnInfo;

/* A trigger entry's chunk, a MapChunk (column, then row) read as one s16. */
#define CHUNK_KEY(col, row) ((row) << 8 | (col))

/* dream_aux.c's data, in address order. */
/* clang-format off */
/* The dream colour each colour condition asks for (TRIGGER_COND_DREAM_COLOR_FIRST on). */
s8 sSpecialColors[8] = {6, 4, 3, 7, 0, 2, 1, 5};

/* The resident Entity, kept {0, -200, 8000} from the player. */
DreamAuxSlot sDreamAuxSlots[1] = {{NULL, NULL, {0, -200, 8000}}};

/* The offsets inside a cell a placement lands at (DreamAuxSpawnInfo::offsetIndex). */
CellOffset sDreamAuxPosTable[79] = {
    {{0, -200}, 0}, {{0, -2048}, 0}, {{0, -4096}, 0}, {{0, -6144}, 0}, /* 0 */
    {{0, -9216}, 0}, {{0, -10240}, 0}, {{0, -512}, 0}, {{0, -1024}, 0}, /* 4 */
    {{0, -1024}, 0}, {{1024, 0}, 1024}, {{1024, 0}, -1024}, {{1024, 0}, 0}, /* 8 */
    {{1024, -6144}, 1024}, {{0, 0}, 2048}, {{512, -256}, 0}, {{0, 0}, 0}, /* 12 */
    {{-740, -6016}, 0}, {{0, -6144}, -512}, {{2048, 4096}, -20480}, {{0, 0}, 20480}, /* 16 */
    {{1024, -300}, 0}, {{-1024, -300}, 0}, {{0, -300}, 0}, {{0, -6444}, -512}, /* 20 */
    {{1024, -712}, 0}, {{0, 2048}, 0}, {{-976, -7600}, 956}, {{0, -7912}, 0}, /* 24 */
    {{0, 10240}, 0}, {{128, -1224}, 0}, {{0, -400}, 0}, {{0, -200}, 512}, /* 28 */
    {{512, 0}, 512}, {{512, -200}, 0}, {{3072, -712}, -2048}, {{0, 0}, -2048}, /* 32 */
    {{0, 0}, 1024}, {{256, -200}, -256}, {{-256, -200}, -256}, {{768, -200}, -512}, /* 36 */
    {{0, -2048}, 0}, {{-2048, -9216}, 0}, {{0, -200}, -30720}, {{0, -200}, -28672}, /* 40 */
    {{0, -200}, -26624}, {{-1024, -200}, -512}, {{512, -200}, -1024}, {{2048, -200}, 1024}, /* 44 */
    {{1024, -200}, 1024}, {{0, -900}, 1400}, {{0, -400}, 0}, {{1024, -400}, 0}, /* 48 */
    {{1024, 400}, 0}, {{-1024, 3896}, -512}, {{512, 3896}, -1024}, {{2048, 3896}, 1024}, /* 52 */
    {{1024, 3896}, 1024}, {{20480, -200}, 0}, {{-1024, -200}, 0}, {{-256, 0}, 0}, /* 56 */
    {{256, 0}, 0}, {{-512, 0}, 0}, {{-20480, -4096}, 0}, {{0, -2048}, -20480}, /* 60 */
    {{-14336, -200}, 0}, {{0, 1024}, -12288}, {{2048, 512}, -14336}, {{-976, -7400}, 956}, /* 64 */
    {{1024, -100}, 0}, {{1024, -100}, -14336}, {{0, -200}, 700}, {{-30720, -200}, 0}, /* 68 */
    {{1024, 512}, -22528}, {{0, 15360}, -18432}, {{-1024, 14336}, -30720}, {{-2048, -200}, -24576}, /* 72 */
    {{-30720, -400}, 0}, {{-14336, -2200}, -18432}, {{500, -300}, -1300}, /* 76 */
};

/* The placements' yaws, in degrees: 0, -90, +90 and 180. */
Ratio16 sDreamAuxSpawnRotations[4][3] = {
    {{0, 1}, {0, 1}, {0, 1}},
    {{0, 1}, {-90, 1}, {0, 1}},
    {{0, 1}, {90, 1}, {0, 1}},
    {{0, 1}, {180, 1}, {0, 1}},
};

/* Every placement a TriggerRecord can name (TriggerRecord::spawnIndices):
 * the cell, the yaw and the offset. */
DreamAuxSpawnInfo sDreamAuxSpawnInfo[229] = {
    {0x0A0A, 0,  0}, {0x0D0A, 0, 29}, {0x0004, 0, 15}, {0x0900, 1, 15}, {0x090E, 0,  0}, /* 0 */
    {0x0611, 1,  7}, {0x0A11, 1,  7}, {0x0E11, 1,  7}, {0x0602, 2,  8}, {0x0A02, 2,  8}, /* 5 */
    {0x0E02, 2,  8}, {0x0809, 0,  9}, {0x0A09, 3, 15}, {0x070C, 0, 35}, {0x1305, 0, 11}, /* 10 */
    {0x090D, 2,  0}, {0x0D00, 0, 12}, {0x0C08, 0,  9}, {0x0C05, 0, 15}, {0x0A0F, 2, 15}, /* 15 */
    {0x0909, 0,  9}, {0x1005, 2,  0}, {0x0504, 2, 15}, {0x0C05, 3,  0}, {0x0211, 0, 15}, /* 20 */
    {0x0504, 0,  0}, {0x0309, 0,  0}, {0x0F0D, 0, 15}, {0x0E0B, 0, 13}, {0x0705, 3, 31}, /* 25 */
    {0x0905, 0, 22}, {0x0508, 0, 15}, {0x1107, 0, 32}, {0x010E, 3, 18}, {0x1303, 0, 19}, /* 30 */
    {0x0A0F, 0,  0}, {0x100E, 0, 14}, {0x080A, 1, 36}, {0x0509, 0,  0}, {0x0804, 3,  1}, /* 35 */
    {0x0804, 3,  2}, {0x0805, 3, 16}, {0x0803, 1,  4}, {0x0807, 3, 17}, {0x0807, 3,  1}, /* 40 */
    {0x0805, 3,  1}, {0x0509, 1, 20}, {0x0509, 2, 21}, {0x0A0B, 3,  0}, {0x0A09, 0, 22}, /* 45 */
    {0x0B0A, 0, 22}, {0x0A0B, 0, 22}, {0x0A0C, 0, 22}, {0x0C08, 0, 15}, {0x0806, 3, 17}, /* 50 */
    {0x030D, 0, 15}, {0x0C13, 0, 34}, {0x080D, 1,  0}, {0x0008, 3, 15}, {0x0007, 1,  1}, /* 55 */
    {0x0300, 1,  0}, {0x0300, 3, 51}, {0x0A0D, 3,  0}, {0x0F01, 0,  0}, {0x0204, 1, 25}, /* 60 */
    {0x0C0C, 1, 15}, {0x070D, 0, 26}, {0x0A0A, 0, 15}, {0x0A0A, 0,  4}, {0x0A0A, 0,  1}, /* 65 */
    {0x0804, 0,  0}, {0x0A0A, 0, 27}, {0x0D0A, 0, 28}, {0x030E, 0,  0}, {0x030D, 0,  0}, /* 70 */
    {0x030C, 0,  0}, {0x030B, 0,  0}, {0x050C, 0,  0}, {0x050B, 0,  0}, {0x050A, 0,  0}, /* 75 */
    {0x0509, 0,  0}, {0x0D09, 0, 51}, {0x0D03, 0,  0}, {0x0C0F, 0,  0}, {0x080B, 0,  0}, /* 80 */
    {0x0C0E, 0,  0}, {0x090D, 2, 15}, {0x1309, 0,  2}, {0x0C0A, 0, 51}, {0x0A0A, 0, 30}, /* 85 */
    {0x0808, 0, 30}, {0x0603, 1, 15}, {0x0408, 3, 33}, {0x0D0D, 2,  0}, {0x070F, 0, 41}, /* 90 */
    {0x0A01, 0,  5}, {0x0D07, 0, 14}, {0x0F0F, 0,  1}, {0x0E13, 2,  0}, {0x0D13, 2,  0}, /* 95 */
    {0x0C13, 2,  0}, {0x080A, 2, 36}, {0x1006, 0,  0}, {0x1006, 0, 37}, {0x1006, 0, 38}, /* 100 */
    {0x0807, 2, 39}, {0x0800, 1, 36}, {0x0800, 2, 36}, {0x090A, 0,  0}, {0x0807, 3, 40}, /* 105 */
    {0x0012, 3, 42}, {0x0012, 3, 43}, {0x0012, 3, 44}, {0x0A0A, 0, 45}, {0x0A0A, 1, 46}, /* 110 */
    {0x0A0A, 2, 47}, {0x0A0A, 3, 48}, {0x080B, 1, 45}, {0x0A0B, 3, 46}, {0x0A0B, 3, 47}, /* 115 */
    {0x0A0B, 0, 48}, {0x0D05, 0, 49}, {0x0400, 3, 51}, {0x0500, 3, 51}, {0x0600, 3, 51}, /* 120 */
    {0x0700, 3, 51}, {0x0B0E, 0,  0}, {0x0C10, 0,  0}, {0x0D09, 0, 52}, {0x0B04, 0, 45}, /* 125 */
    {0x0B04, 1, 46}, {0x0B04, 2, 47}, {0x0B04, 3, 48}, {0x0A04, 0, 45}, {0x0804, 1, 46}, /* 130 */
    {0x0A04, 2, 47}, {0x0A04, 3, 48}, {0x110A, 0, 53}, {0x110A, 1, 54}, {0x110A, 2, 55}, /* 135 */
    {0x110A, 3, 56}, {0x0713, 2, 57}, {0x1303, 0,  0}, {0x1307, 1,  0}, {0x080E, 3, 58}, /* 140 */
    {0x060C, 3,  0}, {0x0905, 0, 15}, {0x030D, 0, 15}, {0x090E, 0, 15}, {0x090E, 3, 59}, /* 145 */
    {0x090E, 3, 60}, {0x090E, 0, 61}, {0x0A00, 1,  0}, {0x0C05, 2,  0}, {0x0F14, 2,  0}, /* 150 */
    {0x0F14, 0, 45}, {0x0F13, 0, 48}, {0x0E13, 2, 48}, {0x0A07, 0,  0}, {0x2507, 3, 22}, /* 155 */
    {0x2506, 3, 22}, {0x2505, 3, 22}, {0x2504, 3, 22}, {0x0E0C, 2, 22}, {0x0A10, 3, 30}, /* 160 */
    {0x0C09, 0, 28}, {0x0E13, 0, 28}, {0x1113, 0, 28}, {0x0313, 0, 28}, {0x090C, 0, 28}, /* 165 */
    {0x0B0A, 0,  4}, {0x0D0A, 0,  4}, {0x0A09, 0,  4}, {0x0809, 0,  4}, {0x0A09, 1,  5}, /* 170 */
    {0x0809, 2,  3}, {0x030A, 1, 25}, {0x0000, 1, 25}, {0x0712, 2, 15}, {0x0004, 3,  0}, /* 175 */
    {0x0B00, 2,  0}, {0x0A00, 2,  0}, {0x0900, 2,  0}, {0x0504, 2,  3}, {0x1005, 1, 62}, /* 180 */
    {0x0805, 0,  0}, {0x0904, 0,  0}, {0x0603, 0,  0}, {0x0A1C, 3, 63}, {0x1100, 1, 64}, /* 185 */
    {0x1303, 1,  0}, {0x0013, 0,  0}, {0x000D, 0, 65}, {0x000D, 1, 66}, {0x080D, 0, 67}, /* 190 */
    {0x060D, 0, 26}, {0x070C, 0, 67}, {0x070B, 0, 67}, {0x0C10, 0,  0}, {0x0117, 1, 68}, /* 195 */
    {0x1505, 0, 22}, {0x000A, 3, 69}, {0x1602, 1,  2}, {0x1702, 2,  2}, {0x1802, 3,  2}, /* 200 */
    {0x1313, 0,  7}, {0x0013, 0,  7}, {0x0302, 2,  0}, {0x0302, 2, 70}, {0x0400, 1, 71}, /* 205 */
    {0x0005, 0, 72}, {0x1D05, 0, 11}, {0x100F, 2,  0}, {0x000C, 2, 73}, {0x0013, 0, 74}, /* 210 */
    {0x0000, 3, 75}, {0x1301, 0, 50}, {0x1200, 0, 50}, {0x0700, 1, 76}, {0x0713, 2, 50}, /* 215 */
    {0x0000, 1, 77}, {0x020F, 2,  2}, {0x010F, 2,  2}, {0x0913, 0,  0}, {0x0914, 0,  0}, /* 220 */
    {0x0915, 0,  0}, {0x0916, 0,  0}, {0x110B, 0, 48}, {0x0509, 2, 78}, /* 225 */
};

/* Each stage's chunk triggers: the TriggerRecords (the latch, the condition,
 * the model, the spawned Entities' mood row and up to four placements,
 * -1 ending the list), then the trigger entries (a chunk, the day parity
 * and up to three of the stage's records, -1 ending the list). */
TriggerRecord sStg00AuxRecords[15] = {
    /* trig cond model mood  spawns */
    {0,  -1,  0,   0, { 42,  -1,   0,   0}}, /* 0 */
    {0,   1,  0, 255, {  0,  -1,   0,   0}}, /* 1 */
    {0,  -9,  1,  92, { 54,  -1,   0,   0}}, /* 2 */
    {0,   5,  0,   1, { 41,  -1,   0,   0}}, /* 3 */
    {0,  -8,  1,  91, { 43,  -1,   0,   0}}, /* 4 */
    {0,   2,  0,   2, { 40,  -1,   0,   0}}, /* 5 */
    {0,   1,  0,   5, { 39,  -1,   0,   0}}, /* 6 */
    {0,   1,  1,  90, { 44,  -1,   0,   0}}, /* 7 */
    {0,   1,  2,  77, { 45, 109,  -1,   0}}, /* 8 */
    {0,  -1,  0,  93, { 38,  -1,   0,   0}}, /* 9 */
    {0,   3,  1,   3, { 40,  -1,   0,   0}}, /* 10 */
    {0,   4,  2,   4, { 40,  -1,   0,   0}}, /* 11 */
    {0,   5,  1, 105, { 40,  -1,   0,   0}}, /* 12 */
    {0,   8,  0, 107, { 38,  -1,   0,   0}}, /* 13 */
    {0,   9,  0, 106, {105,  -1,   0,   0}}, /* 14 */
};
DreamAuxTriggerEntry sStg00AuxTriggers[5] = {
    /* chunk          day  records */
    {CHUNK_KEY( 0,  0), 2, { 9, 13, 14}},
    {CHUNK_KEY( 0,  1), 1, { 6,  7,  8}},
    {CHUNK_KEY( 0,  2), 2, { 5, 10, 11}},
    {CHUNK_KEY( 0,  3), 1, { 3,  4,  2}},
    {CHUNK_KEY( 0,  4), 2, { 0, -1,  0}},
};
TriggerRecord sStg01AuxRecords[3] = {
    /* trig cond model mood  spawns */
    {0,   1,  0,   7, {  0,  -1,   0,   0}}, /* 0 */
    {0,   1,  0,   8, { 24,  -1,   0,   0}}, /* 1 */
    {0,  10,  0, 109, {110, 111, 112,  -1}}, /* 2 */
};
DreamAuxTriggerEntry sStg01AuxTriggers[2] = {
    {CHUNK_KEY( 1,  0), 0, { 0, -1,  0}},
    {CHUNK_KEY( 1,  1), 0, { 1,  2, -1}},
};
TriggerRecord sStg02AuxRecords[30] = {
    /* trig cond model mood  spawns */
    {0,   1,  0,   9, { 57, 141, 142, 143}}, /* 0 */
    {0,  -1,  0,  10, {  0,  -1,   0,   0}}, /* 1 */
    {0,  20,  0,  11, { 58,  -1,   0,   0}}, /* 2 */
    {0,   6,  0,  12, { 55,  -1,   0,   0}}, /* 3 */
    {0,   1,  0,  13, { 13,  -1,   0,   0}}, /* 4 */
    {0,   6,  0,  14, { 14,  -1,   0,   0}}, /* 5 */
    {0,   6,  0,  94, { 62,  -1,   0,   0}}, /* 6 */
    {0,   1,  0,  15, { 15,  -1,   0,   0}}, /* 7 */
    {0,   1,  0,  16, { 36,  96,  97,  -1}}, /* 8 */
    {0,   1,  0,  17, { 16,  -1,   0,   0}}, /* 9 */
    {0,   6,  0,  18, { 59,  -1,   0,   0}}, /* 10 */
    {0,   1,  0,  80, { 63,  -1,   0,   0}}, /* 11 */
    {0,   1,  0,  19, { 60, 215,  -1,   0}}, /* 12 */
    {0,  20,  0,  20, { 61,  -1,   0,   0}}, /* 13 */
    {0,   1,  0,  21, { 35,  -1,   0,   0}}, /* 14 */
    {0,   1,  0,  22, { 56,  -1,   0,   0}}, /* 15 */
    {0,   1,  0,  23, {  0,  -1,   0,   0}}, /* 16 */
    {0,  11,  0, 110, {121,  -1,   0,   0}}, /* 17 */
    {0,  15,  0,  20, {122, 123, 124, 125}}, /* 18 */
    {0,  10,  0,  21, {126, 127,  -1,   0}}, /* 19 */
    {0,  13,  0,  94, {144, 145,  -1,   0}}, /* 20 */
    {0,  21,  0, 125, {207, 208, 209, 210}}, /* 21 */
    {0,   8,  0, 127, {212,  -1,   0,   0}}, /* 22 */
    {0,   5,  0, 126, {213, 214,  -1,   0}}, /* 23 */
    {0,   1,  0,  14, {211,  -1,   0,   0}}, /* 24 */
    {0,  21,  0,  20, {216, 217, 218, 219}}, /* 25 */
    {0,  -5,  0,  18, {221, 222,  -1,   0}}, /* 26 */
    {0,  -8,  0,  22, {223, 224, 225, 226}}, /* 27 */
    {0,  -8,  0,  80, {227,  -1,   0,   0}}, /* 28 */
    {0,   5,  0,  12, {220,  -1,   0,   0}}, /* 29 */
};
DreamAuxTriggerEntry sStg02AuxTriggers[17] = {
    /* chunk          day  records */
    {CHUNK_KEY( 1,  0), 0, { 4, -1,  0}},
    {CHUNK_KEY( 4,  0), 1, { 7, -1,  0}},
    {CHUNK_KEY( 3,  1), 0, {13, 18, 25}},
    {CHUNK_KEY( 4,  1), 1, { 5, 24, -1}},
    {CHUNK_KEY( 5,  1), 2, {11, 28, -1}},
    {CHUNK_KEY( 0,  2), 1, {14, 19, -1}},
    {CHUNK_KEY( 1,  2), 2, { 8, -1,  0}},
    {CHUNK_KEY( 2,  2), 1, {16, 22, -1}},
    {CHUNK_KEY( 0,  3), 2, { 6, 20, 23}},
    {CHUNK_KEY( 1,  3), 1, {12, -1,  0}},
    {CHUNK_KEY( 4,  3), 0, { 2, 17, 21}},
    {CHUNK_KEY( 0,  4), 1, { 0, -1,  0}},
    {CHUNK_KEY( 2,  4), 1, {10, 26, -1}},
    {CHUNK_KEY( 4,  4), 2, { 3, 29, -1}},
    {CHUNK_KEY( 2,  5), 2, {15, 27, -1}},
    {CHUNK_KEY( 3,  5), 1, { 9, -1,  0}},
    {CHUNK_KEY( 4,  5), 2, { 1, -1,  0}},
};
TriggerRecord sStg03AuxRecords[33] = {
    /* trig cond model mood  spawns */
    {0,   1,  0,  24, { 68,  -1,   0,   0}}, /* 0 */
    {0,   1,  0,  81, {  0, 179,  -1,   0}}, /* 1 */
    {0,   1,  0,  25, { 20,  -1,   0,   0}}, /* 2 */
    {0,   1,  0,  26, {  0,  -1,   0,   0}}, /* 3 */
    {0,   1,  0,  27, { 68,  -1,   0,   0}}, /* 4 */
    {0,   1,  0,  28, { 21,  -1,   0,   0}}, /* 5 */
    {0,   7,  0,  29, { 22, 183,  -1,   0}}, /* 6 */
    {0,   1,  0,  30, { 66,  -1,   0,   0}}, /* 7 */
    {0,   1,  0,  75, { 71,  -1,   0,   0}}, /* 8 */
    {0,   1,  0,  31, { 23,  -1,   0,   0}}, /* 9 */
    {0,   1,  0,  76, { 70,  -1,   0,   0}}, /* 10 */
    {0,   1,  0,  32, {  0,  -1,   0,   0}}, /* 11 */
    {0,   1,  0,  33, { 67,  -1,   0,   0}}, /* 12 */
    {0,   1,  0,  34, { 65,  -1,   0,   0}}, /* 13 */
    {0,   1,  0,  35, { 64, 176, 177,  -1}}, /* 14 */
    {0,   1,  0, 255, {  0,  -1,   0,   0}}, /* 15 */
    {0,   1,  0,  36, { 69,  -1,   0,   0}}, /* 16 */
    {0,   1,  0,  37, { 72, 169,  -1,   0}}, /* 17 */
    {0,   1,  0,  24, { 68,  -1,   0,   0}}, /* 18 */
    {0,   1,  0,  24, { 68,  -1,   0,   0}}, /* 19 */
    {0,   6,  0,  37, {165, 166, 167, 168}}, /* 20 */
    {0,   6,  0,  27, {170, 171, 172, 173}}, /* 21 */
    {0,   6,  0,  24, {170, 171, 172, 173}}, /* 22 */
    {0,   8,  0,  24, {170, 171, 174, 175}}, /* 23 */
    {0,  -8,  0, 118, {178,  -1,   0,   0}}, /* 24 */
    {0,   9,  0,  26, { 98,  99, 100,  -1}}, /* 25 */
    {0,  -8,  0,  26, {180, 181, 182,  -1}}, /* 26 */
    {0,  10,  0, 119, {184, 188,  -1,   0}}, /* 27 */
    {0,   8,  0,  76, {185, 186, 187,  -1}}, /* 28 */
    {0,   9,  0,  31, {189,  -1,   0,   0}}, /* 29 */
    {0,  12,  0, 120, {190, 191,  -1,   0}}, /* 30 */
    {0,  11,  0, 121, {192, 193,  -1,   0}}, /* 31 */
    {0,  13,  0, 122, {194, 195, 196, 197}}, /* 32 */
};
DreamAuxTriggerEntry sStg03AuxTriggers[19] = {
    /* chunk          day  records */
    {CHUNK_KEY(12,  0), 0, { 5, 27, -1}},
    {CHUNK_KEY(13,  0), 0, { 9, 29, -1}},
    {CHUNK_KEY( 6,  2), 0, { 2, -1,  0}},
    {CHUNK_KEY( 8,  3), 0, {14, 24, -1}},
    {CHUNK_KEY(11,  3), 0, {11, -1,  0}},
    {CHUNK_KEY( 7,  4), 0, { 6, -1,  0}},
    {CHUNK_KEY( 2,  6), 0, {13, -1,  0}},
    {CHUNK_KEY(11,  6), 0, { 7, 32, -1}},
    {CHUNK_KEY( 3,  7), 0, {12, 31, -1}},
    {CHUNK_KEY( 4,  7), 0, { 0, 22, -1}},
    {CHUNK_KEY( 8,  7), 0, { 3, 25, 26}},
    {CHUNK_KEY( 4, 10), 0, {16, -1,  0}},
    {CHUNK_KEY(10, 10), 0, {18, 22, -1}},
    {CHUNK_KEY( 1, 11), 0, {19, 23, -1}},
    {CHUNK_KEY( 4, 12), 0, {10, 28, -1}},
    {CHUNK_KEY( 3, 13), 0, { 4, 21, -1}},
    {CHUNK_KEY( 7, 13), 0, { 8, -1,  0}},
    {CHUNK_KEY(10, 13), 0, { 1, 30, -1}},
    {CHUNK_KEY(13, 14), 0, {17, 20, -1}},
};
TriggerRecord sStg04AuxRecords[30] = {
    /* trig cond model mood  spawns */
    {0,   1,  0,  38, {  1,  -1,   0,   0}}, /* 0 */
    {0,   1,  0,  39, { 73,  74,  75,  76}}, /* 1 */
    {0,   1,  1,  44, {108,  -1,   0,   0}}, /* 2 */
    {0,   1,  0,  40, { 73,  74,  75,  76}}, /* 3 */
    {0,   1,  0, 111, {  2,  -1,   0,   0}}, /* 4 */
    {0,   1,  0,  42, {  5,   6,   7,  -1}}, /* 5 */
    {0,   1,  0,  43, {  0,  -1,   0,   0}}, /* 6 */
    {0,   1,  0,  45, { 11,  -1,   0,   0}}, /* 7 */
    {0,   1,  0,  46, { 73,  74,  75,  76}}, /* 8 */
    {0,   1,  0,  47, { 12,  -1,   0,   0}}, /* 9 */
    {0,   1,  0,  48, { 85,  -1,   0,   0}}, /* 10 */
    {0,   1,  0,  49, { 86,  -1,   0,   0}}, /* 11 */
    {0,   1,  0,  50, {  0,  -1,   0,   0}}, /* 12 */
    {0,   1,  0,  51, { 81,  -1,   0,   0}}, /* 13 */
    {0,   1,  0,  52, { 78,  74,  75,  76}}, /* 14 */
    {0,   1,  0,  53, { 82,  83,  -1,   0}}, /* 15 */
    {0,   1,  0,  40, { 73,  74,  75,  76}}, /* 16 */
    {0,   1,  0,  40, { 73,  74,  75,  76}}, /* 17 */
    {0,   1,  0,  41, {  3,  -1,   0,   0}}, /* 18 */
    {0,   1,  0,  42, {  8,   9,  10,  -1}}, /* 19 */
    {0,   1,  0,  52, { 78,  79,  75,  76}}, /* 20 */
    {0,   1,  0,  52, { 78,  74,  79,  76}}, /* 21 */
    {0,   1,  1,  54, { 84,  -1,   0,   0}}, /* 22 */
    {0,   1,  0,  39, { 77,  78,  79,  80}}, /* 23 */
    {0,   1,  0,  46, { 77,  78,  79,  80}}, /* 24 */
    {0,  14,  1, 113, {128,  -1,   0,   0}}, /* 25 */
    {0,  11,  0, 114, {129, 130, 131, 132}}, /* 26 */
    {0,  11,  0, 114, {133, 134, 135, 136}}, /* 27 */
    {0,  17,  0, 112, {137, 138, 139, 140}}, /* 28 */
    {0,  20,  0, 128, {205, 206,  -1,   0}}, /* 29 */
};
DreamAuxTriggerEntry sStg04AuxTriggers[22] = {
    /* chunk          day  records */
    {CHUNK_KEY( 1,  0), 0, { 0, 28, -1}},
    {CHUNK_KEY( 4,  0), 2, { 1, 23, -1}},
    {CHUNK_KEY( 0,  1), 2, { 3, -1,  0}},
    {CHUNK_KEY( 1,  1), 1, { 4, -1,  0}},
    {CHUNK_KEY( 2,  1), 1, { 5, -1,  0}},
    {CHUNK_KEY( 3,  1), 1, {19, -1,  0}},
    {CHUNK_KEY( 4,  1), 2, {16, -1,  0}},
    {CHUNK_KEY( 2,  0), 1, {15, -1,  0}},
    {CHUNK_KEY( 1,  2), 2, { 6,  2, -1}},
    {CHUNK_KEY( 2,  2), 1, { 7, -1,  0}},
    {CHUNK_KEY( 3,  2), 2, { 8, 24, -1}},
    {CHUNK_KEY( 4,  2), 1, { 9, -1,  0}},
    {CHUNK_KEY( 0,  3), 0, {10, 29, -1}},
    {CHUNK_KEY( 3,  3), 2, {18, -1,  0}},
    {CHUNK_KEY( 4,  3), 1, {17, -1,  0}},
    {CHUNK_KEY( 5,  3), 2, {11, -1,  0}},
    {CHUNK_KEY( 0,  4), 1, {13, 22, 25}},
    {CHUNK_KEY( 1,  4), 2, {14, -1,  0}},
    {CHUNK_KEY( 2,  4), 1, {12, -1,  0}},
    {CHUNK_KEY( 3,  4), 1, {20, -1,  0}},
    {CHUNK_KEY( 4,  4), 1, {21, -1,  0}},
    {CHUNK_KEY( 0,  4), 1, {13, 26, 27}},
};
TriggerRecord sStg05AuxRecords[27] = {
    /* trig cond model mood  spawns */
    {0,   1,  0,  82, { 93,  92,  -1,   0}}, /* 0 */
    {0,  -1,  0,  83, {  0, 198,  -1,   0}}, /* 1 */
    {0,  -1,  0,  89, { 91,  -1,   0,   0}}, /* 2 */
    {0,   1,  0, 255, {  0,  -1,   0,   0}}, /* 3 */
    {0,   1,  0,  55, { 25,  -1,   0,   0}}, /* 4 */
    {0,  -1,  0,  56, {158, 152, 153,  -1}}, /* 5 */
    {0,  -1,  0,  57, { 26,  -1,   0,   0}}, /* 6 */
    {0,   1,  0,  58, { 27,  -1,   0,   0}}, /* 7 */
    {0,   1,  0,  59, { 87,  -1,   0,   0}}, /* 8 */
    {0,   1,  0,  60, { 28,  -1,   0,   0}}, /* 9 */
    {0,   1,  0,  61, { 88,  -1,   0,   0}}, /* 10 */
    {0,  -1,  0,  62, { 89,  90,  -1,   0}}, /* 11 */
    {0,   1,  0,  85, { 30, 163, 200,  -1}}, /* 12 */
    {0,   1,  1,  86, { 29,  -1,   0,   0}}, /* 13 */
    {0,   1,  0,  84, { 31,  -1,   0,   0}}, /* 14 */
    {0,   1,  0,  63, { 32,  -1,   0,   0}}, /* 15 */
    {0,   1,  0,  64, { 86,  -1,   0,   0}}, /* 16 */
    {0,   1,  0,  64, { 67,  -1,   0,   0}}, /* 17 */
    {0,   1,  0,  64, { 86,  -1,   0,   0}}, /* 18 */
    {0,  -8,  0, 115, {146, 147,  -1,   0}}, /* 19 */
    {0,  -9,  0, 116, {148, 149, 150, 151}}, /* 20 */
    {0,   8,  0,  56, {154, 155, 156, 157}}, /* 21 */
    {0,  12,  0,  85, {159, 160, 161, 162}}, /* 22 */
    {0,  -8,  0, 117, {164,  -1,   0,   0}}, /* 23 */
    {0,   9,  0, 123, {199,  -1,   0,   0}}, /* 24 */
    {0,   8,  0, 123, {201,  -1,   0,   0}}, /* 25 */
    {0,   8,  0,  60, {202, 203, 204,  -1}}, /* 26 */
};
DreamAuxTriggerEntry sStg05AuxTriggers[17] = {
    /* chunk          day  records */
    {CHUNK_KEY( 1,  0), 1, { 1, -1,  0}},
    {CHUNK_KEY( 2,  0), 2, { 0, -1,  0}},
    {CHUNK_KEY( 4,  0), 0, { 2, -1,  0}},
    {CHUNK_KEY( 2,  1), 2, { 4, -1,  0}},
    {CHUNK_KEY( 3,  1), 1, { 5, 21, -1}},
    {CHUNK_KEY( 0,  2), 2, {17, -1,  0}},
    {CHUNK_KEY( 1,  2), 1, { 7, 19, 20}},
    {CHUNK_KEY( 3,  2), 2, { 8, -1,  0}},
    {CHUNK_KEY( 1,  3), 1, { 9, 26, -1}},
    {CHUNK_KEY( 2,  3), 2, {10, -1,  0}},
    {CHUNK_KEY( 3,  3), 1, {11, 23, -1}},
    {CHUNK_KEY( 4,  3), 2, {12, 13, 22}},
    {CHUNK_KEY( 2,  4), 2, {14, 24, 25}},
    {CHUNK_KEY( 3,  4), 1, {15, -1,  0}},
    {CHUNK_KEY( 1,  5), 0, {16, -1,  0}},
    {CHUNK_KEY( 3,  5), 1, { 6, -1,  0}},
    {CHUNK_KEY( 4,  5), 2, {18, -1,  0}},
};
TriggerRecord sStg06AuxRecords[1] = {
    {0,   1,  0, 102, { 95,  -1,   0,   0}}, /* 0 */
};
DreamAuxTriggerEntry sStg06AuxTriggers[1] = {
    {CHUNK_KEY( 0,  5), 1, { 0, -1,  0}},
};
TriggerRecord sStg07AuxRecords[7] = {
    /* trig cond model mood  spawns */
    {0,  -9,  0,  87, { 17,  -1,   0,   0}}, /* 0 */
    {0,   1,  1,  88, { 17,  -1,   0,   0}}, /* 1 */
    {0,  -6,  0,  65, { 53,  -1,   0,   0}}, /* 2 */
    {0,  -6,  0,  66, { 18,  -1,   0,   0}}, /* 3 */
    {0,  -8,  0,  68, { 53,  -1,   0,   0}}, /* 4 */
    {0,   1,  0,  67, { 18,  -1,   0,   0}}, /* 5 */
    {0,  -1,  0, 104, {102, 103, 104,  -1}}, /* 6 */
};
DreamAuxTriggerEntry sStg07AuxTriggers[5] = {
    /* chunk          day  records */
    {CHUNK_KEY( 0,  0), 1, { 0,  1, -1}},
    {CHUNK_KEY( 1,  0), 2, { 2,  6, -1}},
    {CHUNK_KEY( 2,  0), 1, { 3, -1,  0}},
    {CHUNK_KEY( 3,  0), 2, { 4, -1,  0}},
    {CHUNK_KEY( 4,  0), 1, { 5, -1,  0}},
};
TriggerRecord sStg08AuxRecords[6] = {
    /* trig cond model mood  spawns */
    {0,   5,  0,  78, { 46,  47,  -1,   0}}, /* 0 */
    {0,  -1,  0,  95, { 48,  -1,   0,   0}}, /* 1 */
    {0,  -1,  0,  96, { 49,  50,  51,  52}}, /* 2 */
    {0,   1,  0,  96, {113, 114, 115, 116}}, /* 3 */
    {0,  16,  0,  95, {117, 118, 119, 120}}, /* 4 */
    {0,  -8,  0, 129, {228,  -1,   0,   0}}, /* 5 */
};
DreamAuxTriggerEntry sStg08AuxTriggers[3] = {
    /* chunk          day  records */
    {CHUNK_KEY( 0,  0), 0, { 0,  5, -1}},
    {CHUNK_KEY( 0,  1), 0, { 1,  4, -1}},
    {CHUNK_KEY( 0,  2), 0, { 2,  3, -1}},
};
TriggerRecord sStg09AuxRecords[2] = {
    {0,   1,  0,  69, { 33,  -1,   0,   0}}, /* 0 */
    {0,  -1,  0,  79, { 34,  -1,   0,   0}}, /* 1 */
};
DreamAuxTriggerEntry sStg09AuxTriggers[2] = {
    {CHUNK_KEY( 0,  0), 1, { 1, -1,  0}},
    {CHUNK_KEY( 0,  1), 1, { 0, -1,  0}},
};
TriggerRecord sStg10AuxRecords[2] = {
    {0,   1,  0,  73, { 19,  -1,   0,   0}}, /* 0 */
    {0,   1,  1,  74, { 19,  -1,   0,   0}}, /* 1 */
};
DreamAuxTriggerEntry sStg10AuxTriggers[1] = {
    {CHUNK_KEY( 2,  0), 0, { 0,  1, -1}},
};
TriggerRecord sStg11AuxRecords[2] = {
    {0,  -1,  0,  70, { 37, 101,  -1,   0}}, /* 0 */
    {0,  -1,  0,  70, {106, 107,  -1,   0}}, /* 1 */
};
DreamAuxTriggerEntry sStg11AuxTriggers[2] = {
    {CHUNK_KEY( 1,  1), 2, { 0, -1,  0}},
    {CHUNK_KEY( 3,  1), 2, { 1, -1,  0}},
};
TriggerRecord sStg12AuxRecords[2] = {
    {0,   1,  0,  71, {  0,  -1,   0,   0}}, /* 0 */
    {0,  10,  0, 108, {  0,  -1,   0,   0}}, /* 1 */
};
DreamAuxTriggerEntry sStg12AuxTriggers[1] = {
    {CHUNK_KEY( 1,  2), 0, { 0,  1, -1}},
};
TriggerRecord sStg13AuxRecords[1] = {
    {0,   1,  0, 103, { 94,  -1,   0,   0}}, /* 0 */
};
DreamAuxTriggerEntry sStg13AuxTriggers[1] = {
    {CHUNK_KEY( 0,  1), 1, { 0, -1,  0}},
};

/* The per-stage tables and their lengths. */
TriggerRecord *sDreamAuxGroupRecords[DREAM_AUX_STAGE_COUNT] = {
    sStg00AuxRecords, sStg01AuxRecords, sStg02AuxRecords, sStg03AuxRecords,
    sStg04AuxRecords, sStg05AuxRecords, sStg06AuxRecords, sStg07AuxRecords,
    sStg08AuxRecords, sStg09AuxRecords, sStg10AuxRecords, sStg11AuxRecords,
    sStg12AuxRecords, sStg13AuxRecords,
};
s8 sDreamAuxGroupCounts[DREAM_AUX_STAGE_COUNT] = {15, 3, 30, 33, 30, 27, 1, 7, 6, 2, 2, 2, 2, 1};
DreamAuxTriggerEntry *sDreamAuxTriggerEntries[DREAM_AUX_STAGE_COUNT] = {
    sStg00AuxTriggers, sStg01AuxTriggers, sStg02AuxTriggers, sStg03AuxTriggers,
    sStg04AuxTriggers, sStg05AuxTriggers, sStg06AuxTriggers, sStg07AuxTriggers,
    sStg08AuxTriggers, sStg09AuxTriggers, sStg10AuxTriggers, sStg11AuxTriggers,
    sStg12AuxTriggers, sStg13AuxTriggers,
};
s8 sDreamAuxTriggerCounts[DREAM_AUX_STAGE_COUNT] = {5, 2, 17, 19, 22, 17, 1, 5, 3, 2, 1, 2, 1, 1};
/* clang-format on */

/* The two ModelData files InitDreamAux can load. With one slot, only
 * SYMSPY.MOM is ever requested. */
const char sMomPathSymSpy[] = "ETC\\SYMSPY.MOM";
const char sMomPathSymDog[] = "ETC\\SYMDOG.MOM";

void InitDreamAux(void) {
    ResourceRequest req;
    u32 i;
    s32 record;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxGroupRecords); i++) {
        for (record = 0; record < sDreamAuxGroupCounts[i]; record++) {
            sDreamAuxGroupRecords[i][record].triggered = 0;
        }
    }

    ResourceRequest__Set(&req, 0, (char *)sMomPathSymSpy, 1);

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        sDreamAuxSlots[i].model = New_ModelData(&req.src);
        req.src.name = (char *)sMomPathSymDog;
    }
}

void ReleaseDreamAuxModels(void) {
    DreamAuxSlot *slot = sDreamAuxSlots;
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        ModelData *model = slot->model;

        if (model != NULL) {
            slot->model = model->methods->release(model);
        }
        slot++;
    }
}

/* What SetDreamAuxWorld installs: the stage, its StageMap (the parent every
 * Entity here attaches to), the player (each Entity's peer), the sound bank
 * each Entity is built with and the FrameClock each attaches as its
 * companion. */
extern s32 sDreamAuxStage;
extern StageMap *sDreamAuxStageMap;
extern DreamSys *sDreamAuxWorld;
extern struct VabStreamObj *sDreamAuxSound;
extern struct FrameClock *sDreamAuxFrameClock;

void SetTeleportsEnabled(s32 stage);

void SetDreamAuxWorld(s32 stage, StageMap *stageMap, DreamSys *world, struct VabStreamObj *sound,
                      struct FrameClock *frameClock) {
    DreamAuxSlot *slot = sDreamAuxSlots;
    u32 i;

    sDreamAuxStage = stage;
    sDreamAuxStageMap = stageMap;
    sDreamAuxWorld = world;
    sDreamAuxSound = sound;
    sDreamAuxFrameClock = frameClock;

    for (i = 0; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        s32 desc[4]; /* New_Entity's descriptor: word +0x00C the ModelData */
        desc[3] = (s32)slot->model;
        slot->entity = New_Entity(i + DREAM_AUX_FIRST_MOOD, desc, sDreamAuxSound);
        slot++;
    }
    SetTeleportsEnabled(stage);
}

void SetTeleportsEnabled(s32 stage) {
    SetInstantTeleportersEnabled(stage == 11 || stage == 3);
}

/* Mood rows 11, 56, 78 and 93 turn the instant teleporters on. */
void EnableTeleportsForKind(s32 moodIndex) {
    switch (moodIndex) {
        case 11:
        case 56:
        case 78:
        case 93:
            SetInstantTeleportersEnabled(1);
            break;
    }
}

void ReleaseDreamAuxEntities(void) {
    u32 i;
    DreamAuxSlot *slot;

    /* MATCHING: assignments, not initializers, so i and slot are set up in retail's order */
    i = 0;
    slot = sDreamAuxSlots;

    for (; i < ARRAY_COUNT(sDreamAuxSlots); i++) {
        Entity *entity = slot->entity;

        if (entity != NULL) {
            slot->entity = entity->methods->release(entity);
        }
        slot++;
    }
}

DreamAuxTriggerEntry *LookupDreamAuxTrigger(s16 *chunkKey);
bool CheckTriggerDayParity(s32 day, DreamAuxTriggerEntry *trigger);
TriggerWorld *FireDreamAuxTriggerEntries(s32 day, DreamAuxTriggerEntry *trigger, s32 data);
void PlaceDreamAuxEntityByPlayer(DreamAuxSlot *slot);

s32 TryDreamAuxTrigger(s32 data, s16 *chunkKey, s32 day) {
    DreamAuxTriggerEntry *trigger = LookupDreamAuxTrigger(chunkKey);

    if (trigger != NULL) {
        if (CheckTriggerDayParity(day, trigger)) {
            return (s32)FireDreamAuxTriggerEntries(day, trigger, data);
        }
        if (sDreamAuxStage != 0 && rand() % 12 == 0 && (day & 1) == 0) {
            PlaceDreamAuxEntityByPlayer(sDreamAuxSlots);
        }
    }
    return 0;
}

DreamAuxTriggerEntry *RemapTriggerForDreamColor(DreamAuxTriggerEntry *trigger, s32 index);

DreamAuxTriggerEntry *LookupDreamAuxTrigger(s16 *chunkKey) {
    s32 stage = sDreamAuxStage;
    s32 count = sDreamAuxTriggerCounts[stage];
    DreamAuxTriggerEntry *trigger = sDreamAuxTriggerEntries[stage];
    s32 i;

    for (i = 0; i < count; i++) {
        if (*chunkKey == trigger->key) {
            return RemapTriggerForDreamColor(trigger, i);
        }
        trigger++;
    }
    return NULL;
}

/* On stage 4, a red dream swaps the stage's trigger 16 for trigger 21. */
DreamAuxTriggerEntry *RemapTriggerForDreamColor(DreamAuxTriggerEntry *trigger, s32 index) {
    s32 stage = sDreamAuxStage;

    if (stage == 4 && index == 16) {
        DreamSys *player = sDreamAuxWorld;
        s32 color = player->methods->getDreamColor(player);

        if (color == DREAM_COLOR_RED) {
            trigger += 5;
        }
    }
    return trigger;
}

/* Whether `trigger` may fire on `day` (see DreamAuxTriggerEntry.dayParity). */
bool CheckTriggerDayParity(s32 day, DreamAuxTriggerEntry *trigger) {
    bool result = true;

    if (trigger->dayParity != 0) {
        day = day % 2 + 1;
        result = trigger->dayParity != day;
    }
    return result;
}

bool ProcessDreamAuxTriggerRecord(s32 day, DreamAuxTriggerEntry *trigger, TriggerRecord *record,
                                  TriggerWorld *world);

TriggerWorld *FireDreamAuxTriggerEntries(s32 day, DreamAuxTriggerEntry *trigger, s32 data) {
    /* MATCHING: only src.buffer is set, but a bare ResourceSource shrinks
     * the frame by 8. */
    ResourceRequest req;
    TriggerWorld *world;

    req.src.buffer = (void *)data;
    world = New_TriggerWorld(&req.src);

    if (world != NULL) {
        TriggerRecord *records = sDreamAuxGroupRecords[sDreamAuxStage];
        s8 *next = trigger->recordIndices;
        s8 *end = trigger->recordIndices + ARRAY_COUNT(trigger->recordIndices);

        while (next < end) {
            s8 index = *next;

            if (index == -1) {
                break;
            }
            ProcessDreamAuxTriggerRecord(day, trigger, &records[index], world);
            next++;
        }
        return world;
    }
    return NULL;
}

/* Spawns `record`'s Entities if its condition holds, and follows a
 * TRIGGER_CHAIN_MOOD_ROW record's chain. True only when an Entity could not be made. */
bool ProcessDreamAuxTriggerRecord(s32 day, DreamAuxTriggerEntry *trigger, TriggerRecord *record,
                                  TriggerWorld *world) {
    s8 *spawn;
    s8 *end;
    ModelData *model;
    s32 desc[4]; /* New_Entity's descriptor: word +0x00C the ModelData */

    if (!CheckDreamAuxTriggerCondition(day, record)) {
        return false;
    }

    EnableTeleportsForKind(record->moodIndex);

    spawn = record->spawnIndices;
    end = record->spawnIndices + ARRAY_COUNT(record->spawnIndices);
    model = world->methods->getModelData(world, record->modelIndex);
    desc[3] = (s32)model;

    if (model != NULL) {
        while (spawn < end) {
            if (*spawn == -1) {
                break;
            }
            if (SpawnDreamAuxTriggerEntity(record->moodIndex, desc, trigger, (u8)*spawn)) {
                return true;
            }
            spawn++;
        }
    }

    if (record->moodIndex != TRIGGER_CHAIN_MOOD_ROW) {
        return false;
    }
    return ProcessDreamAuxTriggerRecord(day, trigger, record + TRIGGER_CHAIN_STRIDE, world);
}

/* Tests `record`'s condition against `day` (enum TriggerCondition) and
 * latches `triggered` when it passes. */
bool CheckDreamAuxTriggerCondition(s32 day, TriggerRecord *record) {
    s8 condition = record->condition;
    s32 id;

    if (condition == TRIGGER_COND_ALWAYS) {
        record->triggered = 1;
        return true;
    }

    /* MATCHING: gotos; retail lays the negating arm out after the plain one, which no if/else gives */
    if (condition < 0) {
        if (record->triggered == 0) {
            goto negate;
        }
        return false;
    }
    id = condition;
    goto have_idx;

negate:
    id = ~condition + 1; /* MATCHING: not -condition; retail complements and adds one */

have_idx:

    switch (id) {
        case TRIGGER_COND_PERIOD_PHASE_1:
        case TRIGGER_COND_PERIOD_PHASE_2:
        case TRIGGER_COND_PERIOD_PHASE_3:
            if (!IsDayInPeriodPhase(day, id - 1)) {
                return false;
            }
            break;
        case TRIGGER_COND_DAY_MOD3_IS_0:
            if (day % 3 != 0) {
                return false;
            }
            break;
        case TRIGGER_COND_DAY_MOD3_NOT_0:
            if (day % 3 == 0) {
                return false;
            }
            break;
        case TRIGGER_COND_STYLE_VARIANT_EVEN:
            if (!IsStyleVariantEven()) {
                return false;
            }
            break;
        case TRIGGER_COND_DAY_MOD3_IS_1:
        case TRIGGER_COND_DAY_MOD3_IS_2:
            if (day % 3 != id - 7) {
                return false;
            }
            break;
        case TRIGGER_COND_EVEN_DAY:
            if ((day & 1) != 0) {
                return false;
            }
            break;
        case TRIGGER_COND_ODD_DAY:
            if ((day & 1) == 0) {
                return false;
            }
            break;
        default:
            if (id >= TRIGGER_COND_DREAM_COLOR_FIRST) {
                if (!IsCurrentDreamColor(id)) {
                    return false;
                }
            }
            break;
    }

    record->triggered = 1;
    return true;
}

/* Whether the player's dream colour is sSpecialColors' entry for trigger
 * condition `condition` (10..17). */
bool IsCurrentDreamColor(s32 condition) {
    DreamSys *player = sDreamAuxWorld;
    s32 color = sSpecialColors[condition - TRIGGER_COND_DREAM_COLOR_FIRST];
    s32 current = player->methods->getDreamColor(player);

    return color == current;
}

/* The length of the periods IsDayInPeriodPhase counts, in days. */
#define DREAM_PERIOD_DAYS 30

/* Whether `day`'s 30-day period, counted from 1, is phase, phase + 3,
 * phase + 6 or phase + 9: with phase 1..3, every third period of the 12. */
bool IsDayInPeriodPhase(s32 day, s32 phase) {
    s32 period = (day - 1) / DREAM_PERIOD_DAYS + 1;
    s32 i;

    for (i = 0; i < 4; i++) {
        if (period == phase) {
            return true;
        }
        phase += 3;
    }
    return false;
}

/* Makes an Entity of mood row `moodIndex`, turns it and attaches it at
 * placement `spawnIndex` of `trigger`'s chunk. True when New_Entity failed. */
bool SpawnDreamAuxTriggerEntity(s32 moodIndex, void *desc, DreamAuxTriggerEntry *trigger, s32 spawnIndex) {
    Entity *entity = New_Entity(moodIndex, desc, sDreamAuxSound);

    if (entity != NULL) {
        DreamAuxSpawnInfo *spawn;

        CellKeyDesc cellDesc;

        s32 worldPos[4];

        cellDesc.key.chunk = trigger->key;
        spawn = &sDreamAuxSpawnInfo[spawnIndex];
        cellDesc.key.cell = spawn->cell;
        cellDesc.offset = sDreamAuxPosTable[spawn->offsetIndex];

        sDreamAuxStageMap->methods->computeCellOffsets(sDreamAuxStageMap, worldPos, &cellDesc);
        entity->methods->updateRotation(entity, 1, sDreamAuxSpawnRotations[spawn->rotationIndex]);
        ((TodActorAttachToParentFn)entity->methods->attachToParent)(
            (TodActor *)entity, (TodActor *)sDreamAuxWorld, sDreamAuxFrameClock,
            (void *)sDreamAuxStageMap, worldPos);
        return false;
    }
    return true;
}

/* Re-attaches `slot`'s entity at its offset from the player, facing the
 * player. */
void PlaceDreamAuxEntityByPlayer(DreamAuxSlot *slot) {
    if (slot->entity != NULL) {
        s32 worldPos[3];

        slot->entity->methods->detachFromParent(slot->entity);
        SceneNode__LocalOffsetToWorldPos((SceneNode *)sDreamAuxWorld, worldPos, slot->pos, 0);
        ((TodActorAttachToParentFn)slot->entity->methods->attachToParent)(
            (TodActor *)slot->entity, (TodActor *)sDreamAuxWorld, sDreamAuxFrameClock,
            (void *)sDreamAuxStageMap, worldPos);
        SceneNode__FaceTarget((SceneNode *)slot->entity, (SceneNode *)sDreamAuxWorld, 1, 0, NULL);
    }
}
