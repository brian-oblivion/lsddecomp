/* DreamSys's .data tables, in address order, after gDreamSysMethods.
 * dream_sys.c includes this file in place; it is never compiled alone. */
/* clang-format off */
/* (0, 180, 0) degrees: DreamSys__ResetSessionState's absolute
 * updateRotation. */
Ratio16 sRotationYaw180[3] = {{0, 1}, {180, 1}, {0, 1}};

/* CalcDreamColor's 3 x 3 table, [dynamicClass][upperClass], each axis
 * classed 0..2 first. */
s8 sDreamColorTable[9] = {6, 7, 2, 4, 7, 1, 3, 0, 5};

/* DreamSys__ApplyMoveCommand's tables: sMoveCommandSigns[command] times
 * sMoveModeSpeeds[moveMode] is the signed step, and
 * sMoveCommandDispatch[command] (Actor +0x0D0/+0x0D4) takes it. Command 0
 * returns before reaching them. */
s32 sMoveModeSpeeds[5] = {0, 24, 64, 128, 384};
s8 sMoveCommandSigns[8] = {0, 1, -1, -1, 1, 0, 0, 0};
void (*sMoveCommandDispatch[5])(DreamSys *self, s32 val, void *extra) = {
    NULL, (void *)Actor__MoveLocalZOrFindLink, (void *)Actor__MoveLocalZOrFindLink,
    (void *)Actor__MoveLocalXOrFindLink, (void *)Actor__MoveLocalXOrFindLink,
};

/* The look steps and their limits: DreamSys__StepLookOffset's, indexed by
 * lookOffsetCommand, and DreamSys__StepLookYaw's, by lookYawCommand. Index 0
 * is unused; 1 and 2 are the negative and positive step. */
s32 sLookOffsetSteps[3] = {0, -600, 600};
s32 sLookOffsetLimits[3] = {0, 9000, 9000};
s32 sLookYawSteps[3] = {0, -45, 45};
s32 sLookYawLimits[3] = {0, 181, 181};

/* The relative turns: row 0 is StepLookYaw's, whose yaw numerator it
 * overwrites with its step before each use; rows 1 and 2, (0, -6, 0) and
 * (0, 6, 0), are DreamSys__ApplyPendingTurn's for turnCommand 1 and 2. */
Ratio16 sTurnRotations[3][3] = {
    {{0, 1}, {45, 1}, {0, 1}},
    {{0, 1}, {-6, 1}, {0, 1}},
    {{0, 1}, {6, 1}, {0, 1}},
};

/* DreamSys__TickDrift's per-tick addTranslation (+0x0BC) step. */
LongVec3 sDriftStep = {0, 512, 0};

/* Indexed by voiceSelect (0..0x17, DreamSys__NotifyLinkAttempt bounds
 * it): DreamSys__StartVoice plays program sVoiceBySelect[voiceSelect] at
 * octave offset sVoicePitchBySelect[voiceSelect]. */
s8 sVoiceBySelect[0x18] = {0, 3, 29, 18, 25, 18, 28, 29, 3, 30, 30, 3, 3, 21, 16, 16, 10, 28, 18, 6, 13, 19, 12, 30};
s8 sVoicePitchBySelect[0x18] = {0, 0, -1, -2, 2, -1, 0, 0, -1, 0, 0, -1, 1, 0, 2, -2, 2, 2, -1, 1, 2, 2, 1, -2};

/* DreamSys__ProjectPointAtDistance's local offset (0, 0, dist): it writes
 * z and hands the vector to SceneNode__LocalOffsetToWorldPos. */
LongVec3 sProjectOffset = {0, 0, 0};

/* The staircase walks, indexed by GetLastSpawnExtra() and stored in
 * staircaseTickFn by DreamSys__TryStaircaseLink. */
s32 (*sStaircaseTickFns[4])(DreamSys *self) = {
    DreamSys__TickStaircaseYawPlus90, DreamSys__TickStaircaseYawMinus135,
    DreamSys__TickStaircaseYawPlus45, DreamSys__TickStaircaseYawMinus90,
};

/* (0, +45, 0) and (0, -45, 0) degrees: the staircase walks' relative
 * updateRotation. */
Ratio16 sRotationYawPlus45[3] = {{0, 1}, {45, 1}, {0, 1}};
Ratio16 sRotationYawMinus45[3] = {{0, 1}, {-45, 1}, {0, 1}};

/* Each stage's time limit, TryStageTimerLink's. */
s16 sStageTimeLimits[STAGE_COUNT] = {240, 180, 480, 420, 120, 600, 60, 300, 240, 300, 240, 180, 120, 480};

/* The positions relative to a tile a spawn lands at, by
 * StageSpawn::adjustment. */
struct RelativePos sSpawnPosAdjust[37] = {
    /*  0 */ {0, 0, 0},
    /*  1 */ {0, -2048, 0},
    /*  2 */ {0, -4096, 0},
    /*  3 */ {0, -6144, 0},
    /*  4 */ {0, -8192, 0},
    /*  5 */ {0, -512, 0},
    /*  6 */ {0, -1024, 0},
    /*  7 */ {0, -1536, 0},
    /*  8 */ {0, -3072, 0},
    /*  9 */ {0, -10240, 0},
    /* 10 */ {0, -2048, 0},
    /* 11 */ {-1024, 512, 0},
    /* 12 */ {1024, 0, 0},
    /* 13 */ {1024, 0, 1024},
    /* 14 */ {-1024, 0, 1024},
    /* 15 */ {-256, 0, 0},
    /* 16 */ {0, -380, 0},
    /* 17 */ {0, -700, 0},
    /* 18 */ {-256, 0, 1024},
    /* 19 */ {-256, 0, 128},
    /* 20 */ {0, 0, -1024},
    /* 21 */ {0, -768, 0},
    /* 22 */ {-896, -7311, -393},
    /* 23 */ {0, 0, 256},
    /* 24 */ {512, 0, 160},
    /* 25 */ {0, -2048, 1280},
    /* 26 */ {0, -4096, 1280},
    /* 27 */ {0, -6144, 1280},
    /* 28 */ {0, -8192, 1024},
    /* 29 */ {0, -256, 0},
    /* 30 */ {0, -2489, 0},
    /* 31 */ {0, 3728, 0},
    /* 32 */ {0, 512, 0},
    /* 33 */ {0, -4800, 0},
    /* 34 */ {0, 1024, 0},
    /* 35 */ {0, 2048, 0},
    /* 36 */ {0, -14336, 0},
};

/* Each stage's spawn points, GenerateInitialSpawn's and
 * GetRandomSpawnFromStage's: a chunk, a tile in it, an sSpawnPosAdjust
 * index and the extra byte. */
StageSpawn sStg00SpawnPoints[8] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 6,  3},  0,  0},
    {{ 0,  0}, { 1,  8},  0,  0},
    {{ 0,  0}, { 6,  8},  0,  0},
    {{ 0,  1}, { 7,  9},  1,  0},
    {{ 0,  2}, { 5,  8},  2,  0},
    {{ 0,  3}, { 7,  9},  3,  0},
    {{ 0,  3}, { 4,  8},  3,  0},
    {{ 0,  4}, { 4,  8},  4,  0},
};
StageSpawn sStg01SpawnPoints[3] = {
    /* chunk      tile     adj extra */
    {{ 2,  1}, {12,  9},  0,  0},
    {{ 1,  0}, {12, 14},  0,  0},
    {{ 1,  1}, {18, 15},  0,  0},
};
StageSpawn sStg02SpawnPoints[30] = {
    /* chunk      tile     adj extra */
    {{ 0,  1}, {19,  0},  0,  0},
    {{ 1,  0}, {18,  4},  0,  0},
    {{ 2,  0}, {11, 16},  0,  0},
    {{ 4,  0}, { 4,  8},  0,  0},
    {{ 1,  1}, {11, 14},  0,  0},
    {{ 2,  1}, {11, 19},  0,  0},
    {{ 2,  1}, { 4, 14},  0,  0},
    {{ 3,  1}, {18,  9},  0,  0},
    {{ 4,  1}, {18, 19},  0,  0},
    {{ 0,  2}, {15,  6},  0,  0},
    {{ 1,  2}, {15, 12},  0,  0},
    {{ 2,  2}, {11, 19},  0,  0},
    {{ 2,  2}, {16, 11},  0,  0},
    {{ 3,  2}, {14,  6},  0,  0},
    {{ 5,  2}, { 2,  7},  0,  0},
    {{ 0,  3}, {13, 19},  0,  0},
    {{ 1,  3}, {19,  1}, 29,  0},
    {{ 2,  3}, { 2, 19}, 29,  0},
    {{ 3,  3}, {13,  3},  0,  0},
    {{ 3,  3}, { 7,  8},  0,  0},
    {{ 0,  4}, {10, 13}, 29,  0},
    {{ 2,  4}, {11,  6},  0,  0},
    {{ 3,  4}, {14,  8},  0,  0},
    {{ 4,  4}, {14,  8},  0,  0},
    {{ 4,  4}, {13,  0},  0,  0},
    {{ 2,  5}, {16, 13},  0,  0},
    {{ 3,  5}, { 1, 16},  0,  0},
    {{ 4,  5}, { 6,  0},  6,  0},
    {{ 1,  2}, {12, 19}, 29,  0},
    {{ 4,  0}, { 2, 19},  0,  0},
};
StageSpawn sStg03SpawnPoints[123] = {
    /* chunk      tile     adj extra */
    {{ 2,  0}, { 9,  9},  0,  0},
    {{ 3,  0}, { 6, 10},  0,  0},
    {{ 6,  0}, {13, 13},  1,  0},
    {{13,  0}, { 3, 16},  5,  0},
    {{ 4,  1}, { 9,  1},  0,  0},
    {{ 5,  1}, {10,  9},  1,  0},
    {{ 6,  1}, {16, 19},  0,  0},
    {{ 7,  1}, { 6, 15},  0,  0},
    {{10,  1}, { 7, 18},  6,  0},
    {{11,  1}, { 7,  0},  0,  0},
    {{ 5,  2}, { 8, 18},  0,  0},
    {{ 6,  2}, { 9, 12},  0,  0},
    {{ 7,  2}, { 0, 19},  0,  0},
    {{ 8,  2}, {15, 13},  0,  0},
    {{12,  2}, { 7,  5},  0,  0},
    {{14,  2}, { 5, 10}, 30,  0},
    {{ 3,  3}, {19, 10},  0,  0},
    {{ 4,  3}, {12, 10},  0,  0},
    {{ 5,  3}, {19, 19},  0,  0},
    {{ 7,  3}, { 4, 12}, 30,  0},
    {{ 8,  3}, { 5,  5},  0,  0},
    {{ 9,  3}, {15,  6},  0,  0},
    {{10,  3}, { 9, 19},  0,  0},
    {{11,  3}, { 9, 16},  0,  0},
    {{12,  3}, {15, 12},  1,  0},
    {{13,  3}, { 0, 10},  6,  0},
    {{15,  3}, { 6, 16},  0,  0},
    {{ 4,  4}, { 7,  4},  0,  0},
    {{ 5,  4}, { 2,  0},  0,  0},
    {{ 6,  4}, { 8,  0},  0,  0},
    {{ 7,  4}, {14,  6},  0,  0},
    {{ 8,  4}, {17, 17},  0,  0},
    {{ 9,  4}, { 0,  5},  0,  0},
    {{10,  4}, { 9, 13},  0,  0},
    {{11,  4}, {19, 19},  0,  0},
    {{12,  4}, { 4,  0},  0,  0},
    {{13,  4}, {15, 14},  7,  0},
    {{ 1,  5}, {16, 15},  0,  0},
    {{ 2,  5}, {13, 16},  6,  0},
    {{ 3,  5}, { 8, 15},  6,  0},
    {{ 4,  5}, { 3, 18},  0,  0},
    {{ 5,  5}, { 8, 12},  5,  0},
    {{ 7,  5}, {15,  8},  0,  0},
    {{ 8,  5}, {13, 10},  1,  0},
    {{ 9,  5}, {14, 19},  0,  0},
    {{11,  5}, {19, 18},  2,  0},
    {{12,  5}, { 4, 14},  2,  0},
    {{14,  5}, { 8,  6},  0,  0},
    {{ 2,  6}, {10,  8},  0,  0},
    {{ 3,  6}, {13, 13},  0,  0},
    {{ 4,  6}, {16,  4},  0,  0},
    {{ 6,  6}, { 9, 10},  6,  0},
    {{ 8,  6}, {13, 14},  0,  0},
    {{ 9,  6}, { 7,  4}, 31,  0},
    {{10,  6}, { 1,  5},  0,  0},
    {{11,  6}, {18, 12},  2,  0},
    {{12,  6}, { 2,  7},  2,  0},
    {{14,  6}, {10, 11},  0,  0},
    {{ 2,  7}, {11,  8},  0,  0},
    {{ 3,  7}, {10,  0},  0,  0},
    {{ 4,  7}, {12, 16},  0,  0},
    {{ 5,  7}, { 3,  9},  0,  0},
    {{ 6,  7}, {19,  1},  0,  0},
    {{ 7,  7}, { 6, 15},  0,  0},
    {{ 8,  7}, {16,  2},  0,  0},
    {{ 9,  7}, { 9, 15},  5,  0},
    {{10,  7}, {13,  4},  0,  0},
    {{11,  7}, {11, 15},  0,  0},
    {{12,  7}, { 6,  9},  0,  0},
    {{13,  7}, {11, 12},  0,  0},
    {{14,  7}, { 6, 19},  0,  0},
    {{ 2,  8}, { 1,  7},  0,  0},
    {{ 3,  8}, { 2,  1},  0,  0},
    {{ 4,  8}, {13, 12},  0,  0},
    {{ 6,  8}, { 3,  5},  0,  0},
    {{ 9,  8}, { 1, 11},  0,  0},
    {{10,  8}, {18, 14},  0,  0},
    {{ 2,  9}, {13,  7},  0,  0},
    {{ 4,  9}, {11, 13},  0,  0},
    {{ 5,  9}, { 1,  3},  0,  0},
    {{ 6,  9}, {12,  3},  0,  0},
    {{ 7,  9}, {17, 19},  0,  0},
    {{10,  9}, {18, 12},  0,  0},
    {{ 1, 10}, {19, 19},  0,  0},
    {{ 3, 10}, {19, 13},  0,  0},
    {{ 4, 10}, { 5, 15},  0,  0},
    {{ 5, 10}, {10, 13},  0,  0},
    {{ 9, 10}, { 4, 12},  0,  0},
    {{10, 10}, { 9,  5},  0,  0},
    {{11, 10}, { 7, 17},  0,  0},
    {{ 1, 11}, {13, 10}, 32,  0},
    {{ 2, 11}, {16,  3},  0,  0},
    {{ 4, 11}, { 8, 11},  7,  0},
    {{ 5, 11}, { 5, 12},  0,  0},
    {{ 7, 11}, { 2, 19},  0,  0},
    {{11, 11}, { 7, 11},  0,  0},
    {{ 1, 12}, {19, 11},  0,  0},
    {{ 2, 12}, {19, 19},  0,  0},
    {{ 3, 12}, { 1, 19},  0,  0},
    {{ 4, 12}, { 9, 18},  0,  0},
    {{ 5, 12}, { 8, 18},  0,  0},
    {{ 6, 12}, {13, 11},  3,  0},
    {{ 6, 12}, { 5,  7},  0,  0},
    {{ 9, 12}, {11, 17},  0,  0},
    {{10, 12}, { 8,  6},  0,  0},
    {{ 2, 13}, { 9, 12},  0,  0},
    {{ 3, 13}, {15, 14},  0,  0},
    {{ 4, 13}, { 9,  1},  0,  0},
    {{ 5, 13}, {12, 11},  2,  0},
    {{ 6, 13}, {10, 10},  1,  0},
    {{ 7, 13}, {15,  1},  8,  0},
    {{ 8, 13}, { 9,  9},  1,  0},
    {{ 9, 13}, {16, 17},  0,  0},
    {{10, 13}, { 1,  0},  0,  0},
    {{11, 13}, {17,  7},  0,  0},
    {{13, 13}, {15, 18},  0,  0},
    {{14, 13}, { 9,  6},  0,  0},
    {{ 4, 14}, {12,  5},  1,  0},
    {{ 6, 14}, { 4, 13},  0,  0},
    {{ 9, 14}, {14,  8},  0,  0},
    {{13, 14}, {15, 12},  0,  0},
    {{14, 15}, { 5,  6},  0,  0},
    {{ 9,  4}, { 8, 11}, 21,  0},
};
StageSpawn sStg04SpawnPoints[20] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 2,  3},  0,  0},
    {{ 1,  0}, {10, 19},  0,  0},
    {{ 2,  0}, { 3, 15},  0,  0},
    {{ 4,  0}, {15, 19},  0,  0},
    {{ 1,  1}, {10, 16},  6,  0},
    {{ 3,  1}, { 0,  3},  0,  0},
    {{ 4,  1}, {12, 14},  0,  0},
    {{ 1,  2}, { 4,  9},  0,  0},
    {{ 2,  2}, {10,  8}, 34,  0},
    {{ 3,  2}, { 9,  2},  0,  0},
    {{ 4,  2}, { 9,  8},  0,  0},
    {{ 0,  3}, {14,  8},  0,  0},
    {{ 1,  3}, {10, 11},  0,  0},
    {{ 2,  3}, { 5, 11},  0,  0},
    {{ 5,  3}, { 5,  9}, 16,  0},
    {{ 0,  4}, { 2,  8},  0,  0},
    {{ 1,  4}, {16, 16},  0,  0},
    {{ 2,  4}, {13,  1},  0,  0},
    {{ 3,  4}, { 0, 11},  0,  0},
    {{ 4,  4}, { 4,  5},  0,  0},
};
StageSpawn sStg05SpawnPoints[21] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, {18, 10},  1,  0},
    {{ 1,  0}, {16, 12},  0,  0},
    {{ 2,  0}, { 9, 16},  0,  0},
    {{ 3,  0}, {19, 19},  0,  0},
    {{ 1,  1}, {16, 18},  0,  0},
    {{ 2,  1}, { 2, 19},  0,  0},
    {{ 4,  1}, { 0, 14},  5,  0},
    {{ 0,  2}, {19, 19},  1,  0},
    {{ 1,  2}, {14, 10},  5,  0},
    {{ 3,  2}, { 9,  2},  0,  0},
    {{ 1,  3}, {10, 12}, 32,  0},
    {{ 3,  3}, { 3, 14},  0,  0},
    {{ 4,  3}, { 5, 18},  0,  0},
    {{ 1,  4}, {10, 19},  0,  0},
    {{ 2,  4}, { 9,  5},  0,  0},
    {{ 3,  4}, {11, 17},  0,  0},
    {{ 4,  4}, { 4,  9},  0,  0},
    {{ 2,  5}, { 2,  4},  0,  0},
    {{ 3,  5}, { 9,  2},  6,  0},
    {{ 4,  1}, { 0, 14}, 32,  0},
    {{ 3,  1}, {18, 16}, 36,  0},
};
StageSpawn sStg06SpawnPoints[2] = {
    {{ 0,  0}, { 8,  7},  0,  0},
    {{ 0,  5}, { 9,  6},  9,  0},
};
StageSpawn sStg07SpawnPoints[5] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 6, 13},  0,  0},
    {{ 1,  0}, {10,  8},  0,  0},
    {{ 2,  0}, {18, 12},  0,  0},
    {{ 3,  0}, {12, 13}, 32,  0},
    {{ 4,  0}, { 5, 10},  0,  0},
};
StageSpawn sStg08SpawnPoints[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 9,  2},  0,  0},
    {{ 0,  1}, {18, 12},  0,  0},
    {{ 0,  2}, {10,  5},  0,  0},
};
StageSpawn sStg09SpawnPoints[4] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 3, 17},  0,  0},
    {{ 0,  1}, {13,  2},  0,  0},
    {{ 0,  1}, {13,  8}, 34,  0},
    {{ 0,  0}, { 4, 12}, 35,  0},
};
StageSpawn sStg10SpawnPoints[5] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 1,  9},  0,  0},
    {{ 1,  0}, { 6,  9},  0,  0},
    {{ 2,  0}, {10,  9},  0,  0},
    {{ 2,  0}, {14, 10}, 34,  0},
    {{ 1,  0}, { 6,  9},  5,  0},
};
StageSpawn sStg11SpawnPoints[4] = {
    /* chunk      tile     adj extra */
    {{ 2,  1}, { 8,  9},  0,  0},
    {{ 1,  1}, { 9,  8},  0,  0},
    {{ 2,  1}, { 8,  9}, 34,  0},
    {{ 3,  1}, { 1,  8},  6,  0},
};
StageSpawn sStg12SpawnPoints[8] = {
    /* chunk      tile     adj extra */
    {{ 1,  1}, { 9,  5},  0,  0},
    {{ 2,  1}, { 4, 10},  6,  0},
    {{ 0,  2}, {16,  6},  6,  0},
    {{ 1,  2}, { 9,  8},  6,  0},
    {{ 2,  2}, { 6,  9},  0,  0},
    {{ 1,  3}, { 4, 10},  6,  0},
    {{ 2,  3}, { 8, 17},  0,  0},
    {{ 1,  2}, { 9,  8}, 10,  0},
};
StageSpawn sStg13SpawnPoints[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 9,  7},  0,  0},
    {{ 0,  1}, {14, 16},  0,  0},
    {{ 1,  1}, { 0,  1},  0,  0},
};
StageSpawn *sStageSpawnPoints[STAGE_COUNT] = {
    sStg00SpawnPoints, sStg01SpawnPoints, sStg02SpawnPoints, sStg03SpawnPoints,
    sStg04SpawnPoints, sStg05SpawnPoints, sStg06SpawnPoints, sStg07SpawnPoints,
    sStg08SpawnPoints, sStg09SpawnPoints, sStg10SpawnPoints, sStg11SpawnPoints,
    sStg12SpawnPoints, sStg13SpawnPoints,
};

/* MATCHING: u8; as s8, GenerateInitialSpawn's loop guard gains a `blez` retail does not have. */
u8 sStageSpawnPointsCount[STAGE_COUNT] = {8, 3, 30, 123, 20, 21, 2, 5, 3, 4, 5, 4, 8, 3};

/* The fixed links, TestForStaticLink's GetStaticSpawn tables: per stage,
 * the spawns, then the triggers (a chunk, a tile or ANY_TILE, and the
 * destination stage and spawn), then their counts. The same shape serves
 * the tunnels, teleporters and staircases below. */
StageSpawn sStg00PermalinkSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  2}, { 4,  9},  2,  0},
    {{ 0,  2}, { 7,  9},  2,  1},
    {{ 0,  2}, { 6,  9},  2,  2},
};
StageSpawn sStg02PermalinkSpawns[2] = {
    {{ 4,  3}, { 8, 14},  0,  3},
    {{ 3,  1}, { 2, 19},  0,  4},
};
StageSpawn sStg03PermalinkSpawns[2] = {
    {{ 6,  4}, { 2,  3},  0,  5},
    {{ 7, 13}, {10,  9}, 22,  6},
};
StageSpawn sStg04PermalinkSpawns[1] = {
    {{ 5,  3}, {11,  9}, 16,  7},
};
StageSpawn sStg05PermalinkSpawns[1] = {
    {{ 1,  3}, {17, 10},  0,  8},
};
StageSpawn sStg06PermalinkSpawns[1] = {
    {{ 0,  0}, { 8,  7},  0,  9},
};
StageSpawn sStg09PermalinkSpawns[1] = {
    {{ 0,  0}, { 3, 17},  0, 10},
};
StageSpawn sStg10PermalinkSpawns[1] = {
    {{ 0,  0}, {11,  9},  0, 11},
};
StageSpawn sStg11PermalinkSpawns[1] = {
    {{ 1,  1}, { 9,  9},  0, 12},
};
StageSpawn sStg12PermalinkSpawns[1] = {
    {{ 1,  2}, {10, 12},  0, 13},
};
StageSpawn *sStagePermalinkSpawns[STAGE_COUNT] = {
    sStg00PermalinkSpawns, NULL, sStg02PermalinkSpawns, sStg03PermalinkSpawns,
    sStg04PermalinkSpawns, sStg05PermalinkSpawns, sStg06PermalinkSpawns, NULL,
    NULL, sStg09PermalinkSpawns, sStg10PermalinkSpawns, sStg11PermalinkSpawns,
    sStg12PermalinkSpawns, NULL,
};
StaticLinkTrigger sStg00PermalinkTriggers[3] = {
    /* chunk     tile         stage                      spawn */
    {{ 0,  2}, {{  4,   9}}, STAGE_LONG_HALLWAY,        0},
    {{ 0,  2}, {{  7,   8}}, STAGE_KYOTO,               0},
    {{ 0,  2}, {{  6,   8}}, STAGE_NATURAL_WORLD,       0},
};
StaticLinkTrigger sStg02PermalinkTriggers[2] = {
    {{ 2,  1}, ANY_TILE,     STAGE_MOONLIGHT_TOWER,     0},
    {{ 5,  3}, {{  2,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 1},
};
StaticLinkTrigger sStg03PermalinkTriggers[1] = {
    {{ 6,  4}, {{  1,   2}}, STAGE_BRIGHT_MOON_COTTAGE, 2},
};
StaticLinkTrigger sStg04PermalinkTriggers[4] = {
    /* chunk     tile         stage                      spawn */
    {{ 2,  1}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
    {{ 3,  1}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
    {{ 4,  1}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
    {{ 4,  2}, ANY_TILE,     STAGE_SUN_FACES_HEAVE,     0},
};
StaticLinkTrigger sStg05PermalinkTriggers[2] = {
    {{ 1,  3}, ANY_TILE,     STAGE_CLOCKWORK_MACHINES,  0},
    {{ 2,  3}, ANY_TILE,     STAGE_CLOCKWORK_MACHINES,  0},
};
StaticLinkTrigger sStg06PermalinkTriggers[1] = {
    {{ 0,  5}, {{  5,   5}}, STAGE_KYOTO,               1},
};
StaticLinkTrigger sStg09PermalinkTriggers[1] = {
    {{ 0,  1}, {{ 13,  14}}, STAGE_VIOLENCE_DISTRICT,   0},
};
StaticLinkTrigger sStg10PermalinkTriggers[1] = {
    {{ 0,  0}, {{  0,  10}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
};
StaticLinkTrigger sStg11PermalinkTriggers[1] = {
    {{ 1,  1}, {{  9,   9}}, STAGE_HAPPY_TOWN,          0},
};
StaticLinkTrigger sStg12PermalinkTriggers[1] = {
    {{ 1,  2}, {{ 10,  14}}, STAGE_NATURAL_WORLD,       1},
};
StaticLinkTrigger *sStagePermalinkTriggers[STAGE_COUNT] = {
    sStg00PermalinkTriggers, NULL, sStg02PermalinkTriggers, sStg03PermalinkTriggers,
    sStg04PermalinkTriggers, sStg05PermalinkTriggers, sStg06PermalinkTriggers, NULL,
    NULL, sStg09PermalinkTriggers, sStg10PermalinkTriggers, sStg11PermalinkTriggers,
    sStg12PermalinkTriggers, NULL,
};
s8 sStagePermalinkTriggersCount[STAGE_COUNT] = {3, 0, 2, 1, 4, 2, 1, 0, 0, 1, 1, 1, 1, 0};

/* The four cardinal rotations, yaw 0, 90, 180 and 270 degrees.
 * CheckTunnelHeading and CheckStaircaseHeading store an entry's address in
 * enterRotation / exitRotation, which SetMoveOverride, SpawnAtLink and
 * TryStaircaseLink apply. */
Ratio16 sCardinalRotations[4][3] = {
    {{0, 1}, {0, 1}, {0, 1}},
    {{0, 1}, {90, 1}, {0, 1}},
    {{0, 1}, {180, 1}, {0, 1}},
    {{0, 1}, {270, 1}, {0, 1}},
};

/* The tunnels, TestForTunnelLinks' tables, with a heading per spawn and per
 * trigger (an sCardinalRotations index): the player must face the
 * trigger's to take the tunnel (DreamSys__CheckTunnelHeading) and leaves
 * facing the spawn's. */
StageSpawn sStg00TunnelSpawns[1] = {
    {{ 0,  0}, { 8,  0}, 15, 14},
};
u8 sStg00TunnelExitHeadings[1] = {0};
StageSpawn sStg01TunnelSpawns[2] = {
    {{ 1,  0}, { 8,  0}, 18, 15},
    {{ 1,  1}, {18, 19}, 20, 16},
};
u8 sStg01TunnelExitHeadings[2] = {0, 2};
StageSpawn sStg02TunnelSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 2,  0}, {10,  0}, 19, 17},
    {{ 3,  5}, {18, 17},  0, 18},
    {{ 4,  2}, { 8,  6}, 12, 19},
};
u8 sStg02TunnelExitHeadings[3] = {0, 3, 2};
StageSpawn sStg03TunnelSpawns[5] = {
    /* chunk      tile     adj extra */
    {{ 7,  8}, { 8,  6}, 12, 20},
    {{ 3,  9}, { 8,  6}, 12, 21},
    {{13,  6}, { 9,  6}, 12, 22},
    {{11, 12}, { 8,  6}, 12, 23},
    {{ 3,  4}, { 7,  6}, 12, 24},
};
u8 sStg03TunnelExitHeadings[5] = {2, 2, 2, 2, 2};
StageSpawn sStg04TunnelSpawns[1] = {
    {{ 2,  0}, { 9,  0}, 23, 25},
};
u8 sStg04TunnelExitHeadings[1] = {0};
StageSpawn sStg05TunnelSpawns[1] = {
    {{ 2,  0}, { 8,  0}, 24, 26},
};
u8 sStg05TunnelExitHeadings[1] = {0};
StageSpawn sStg07TunnelSpawns[1] = {
    {{ 0,  0}, { 9,  0}, 23, 27},
};
u8 sStg07TunnelExitHeadings[1] = {0};
StageSpawn sStg08TunnelSpawns[1] = {
    {{ 0,  0}, { 9,  0}, 23, 28},
};
u8 sStg08TunnelExitHeadings[1] = {0};
StageSpawn sStg13TunnelSpawns[1] = {
    {{ 0,  0}, { 8,  0}, 18, 29},
};
u8 sStg13TunnelExitHeadings[1] = {0};
StageSpawn *sTunnelSpawns[STAGE_COUNT] = {
    sStg00TunnelSpawns, sStg01TunnelSpawns, sStg02TunnelSpawns, sStg03TunnelSpawns,
    sStg04TunnelSpawns, sStg05TunnelSpawns, NULL, sStg07TunnelSpawns,
    sStg08TunnelSpawns, NULL, NULL, NULL,
    NULL, sStg13TunnelSpawns,
};
u8 *sTunnelExitHeadings[STAGE_COUNT] = {
    sStg00TunnelExitHeadings, sStg01TunnelExitHeadings, sStg02TunnelExitHeadings, sStg03TunnelExitHeadings,
    sStg04TunnelExitHeadings, sStg05TunnelExitHeadings, NULL, sStg07TunnelExitHeadings,
    sStg08TunnelExitHeadings, NULL, NULL, NULL,
    NULL, sStg13TunnelExitHeadings,
};
StaticLinkTrigger sStg00TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       0},
    {{ 0,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       0},
};
u8 sStg00TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg01TunnelTriggers[4] = {
    /* chunk     tile         stage                      spawn */
    {{ 1,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       1},
    {{ 1,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       1},
    {{ 1,  1}, {{ 18,  17}}, STAGE_FLESH_TUNNELS,       0},
    {{ 1,  1}, {{ 19,  17}}, STAGE_FLESH_TUNNELS,       0},
};
u8 sStg01TunnelEnterHeadings[4] = {2, 2, 0, 0};
StaticLinkTrigger sStg02TunnelTriggers[6] = {
    /* chunk     tile         stage                      spawn */
    {{ 2,  0}, {{ 10,   1}}, STAGE_NATURAL_WORLD,       2},
    {{ 2,  0}, {{ 11,   1}}, STAGE_NATURAL_WORLD,       2},
    {{ 3,  5}, {{ 14,  16}}, STAGE_TEMPLE_DOJO,         0},
    {{ 3,  5}, {{ 14,  17}}, STAGE_TEMPLE_DOJO,         0},
    {{ 4,  2}, {{  8,   8}}, STAGE_MONUMENT_PARK,       0},
    {{ 4,  2}, {{  9,   8}}, STAGE_MONUMENT_PARK,       0},
};
u8 sStg02TunnelEnterHeadings[6] = {2, 2, 1, 1, 0, 0};
StaticLinkTrigger sStg03TunnelTriggers[10] = {
    /* chunk     tile         stage                      spawn */
    {{ 7,  8}, {{  8,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
    {{ 7,  8}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
    {{ 3,  9}, {{  8,   8}}, STAGE_PIT_AND_TEMPLE,      0},
    {{ 3,  9}, {{  9,   8}}, STAGE_PIT_AND_TEMPLE,      0},
    {{13,  6}, {{  9,   8}}, STAGE_KYOTO,               0},
    {{13,  6}, {{ 10,   8}}, STAGE_KYOTO,               0},
    {{11, 12}, {{  8,   8}}, STAGE_HAPPY_TOWN,          0},
    {{11, 12}, {{  9,   8}}, STAGE_HAPPY_TOWN,          0},
    {{ 3,  4}, {{  7,   8}}, STAGE_VIOLENCE_DISTRICT,   0},
    {{ 3,  4}, {{  8,   8}}, STAGE_VIOLENCE_DISTRICT,   0},
};
u8 sStg03TunnelEnterHeadings[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
StaticLinkTrigger sStg04TunnelTriggers[2] = {
    {{ 2,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       3},
    {{ 2,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       3},
};
u8 sStg04TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg05TunnelTriggers[2] = {
    {{ 2,  0}, {{  8,   1}}, STAGE_NATURAL_WORLD,       4},
    {{ 2,  0}, {{  9,   1}}, STAGE_NATURAL_WORLD,       4},
};
u8 sStg05TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg07TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   3}}, STAGE_KYOTO,               1},
    {{ 0,  0}, {{  9,   3}}, STAGE_KYOTO,               1},
};
u8 sStg07TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg08TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   1}}, STAGE_PIT_AND_TEMPLE,      1},
    {{ 0,  0}, {{  9,   1}}, STAGE_PIT_AND_TEMPLE,      1},
};
u8 sStg08TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger sStg13TunnelTriggers[2] = {
    {{ 0,  0}, {{  8,   1}}, STAGE_KYOTO,               2},
    {{ 0,  0}, {{  9,   1}}, STAGE_KYOTO,               2},
};
u8 sStg13TunnelEnterHeadings[2] = {2, 2};
StaticLinkTrigger *sTunnelTriggers[STAGE_COUNT] = {
    sStg00TunnelTriggers, sStg01TunnelTriggers, sStg02TunnelTriggers, sStg03TunnelTriggers,
    sStg04TunnelTriggers, sStg05TunnelTriggers, NULL, sStg07TunnelTriggers,
    sStg08TunnelTriggers, NULL, NULL, NULL,
    NULL, sStg13TunnelTriggers,
};
u8 *sTunnelEnterHeadings[STAGE_COUNT] = {
    sStg00TunnelEnterHeadings, sStg01TunnelEnterHeadings, sStg02TunnelEnterHeadings, sStg03TunnelEnterHeadings,
    sStg04TunnelEnterHeadings, sStg05TunnelEnterHeadings, NULL, sStg07TunnelEnterHeadings,
    sStg08TunnelEnterHeadings, NULL, NULL, NULL,
    NULL, sStg13TunnelEnterHeadings,
};
s8 sTunnelTriggersCount[STAGE_COUNT] = {2, 4, 6, 10, 2, 2, 0, 2, 2, 0, 0, 0, 0, 2};

/* The instant teleporters, TestForInstantTeleporters' tables. */
StageSpawn sStg02TeleportSpawns[9] = {
    /* chunk      tile     adj extra */
    {{ 1,  1}, { 4,  8},  0,  0},
    {{ 1,  0}, { 7, 14},  0,  0},
    {{ 4,  1}, { 1,  3},  0,  0},
    {{ 4,  3}, { 3, 10}, 10,  0},
    {{ 3,  4}, {16,  3}, 11,  0},
    {{ 2,  3}, { 2, 16},  0,  0},
    {{ 4,  4}, { 0, 18},  0,  0},
    {{ 2,  1}, { 4, 14},  0,  0},
    {{ 5,  1}, { 1, 14},  0,  0},
};
StageSpawn sStg03TeleportSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 9,  4}, { 9, 18}, 21,  0},
    {{ 9,  4}, { 9,  3}, 21,  0},
    {{ 7,  4}, { 7,  0},  0,  0},
};
StageSpawn sStg05TeleportSpawns[5] = {
    /* chunk      tile     adj extra */
    {{ 4,  1}, { 0, 14},  5,  0},
    {{ 3,  3}, { 8, 10},  0,  0},
    {{ 1,  2}, {13, 14},  0,  0},
    {{ 2,  2}, { 9, 19},  0,  0},
    {{ 1,  2}, {13,  5},  0,  0},
};
StageSpawn sStg08TeleportSpawns[3] = {
    /* chunk      tile     adj extra */
    {{ 0,  0}, { 9,  5}, 17,  0},
    {{ 0,  1}, {11, 12},  0,  0},
    {{ 0,  1}, {17,  3},  0,  0},
};
StageSpawn *sTeleportSpawns[STAGE_COUNT] = {
    NULL, NULL, sStg02TeleportSpawns, sStg03TeleportSpawns,
    NULL, sStg05TeleportSpawns, NULL, NULL,
    sStg08TeleportSpawns, NULL, NULL, NULL,
    NULL, NULL,
};
StaticLinkTrigger sStg02TeleportTriggers[8] = {
    /* chunk     tile         stage                      spawn */
    {{ 1,  1}, {{  4,  10}}, STAGE_KYOTO,               0},
    {{ 0,  0}, {{  7,  17}}, STAGE_KYOTO,               1},
    {{ 4,  1}, {{  7,   0}}, STAGE_KYOTO,               2},
    {{ 4,  3}, {{  4,  18}}, STAGE_KYOTO,               3},
    {{ 1,  2}, {{  1,  14}}, STAGE_KYOTO,               5},
    {{ 2,  1}, {{ 13,  14}}, STAGE_KYOTO,               7},
    {{ 4,  4}, {{ 13,   3}}, STAGE_KYOTO,               6},
    {{ 4,  1}, {{ 11,   0}}, STAGE_KYOTO,               8},
};
StaticLinkTrigger sStg03TeleportTriggers[1] = {
    {{ 7,  4}, {{  0,   5}}, STAGE_NATURAL_WORLD,       2},
};
StaticLinkTrigger sStg05TeleportTriggers[5] = {
    /* chunk     tile         stage                      spawn */
    {{ 3,  1}, {{ 18,  16}}, STAGE_VIOLENCE_DISTRICT,   0},
    {{ 2,  3}, {{ 11,  11}}, STAGE_VIOLENCE_DISTRICT,   1},
    {{ 0,  2}, {{ 18,  10}}, STAGE_VIOLENCE_DISTRICT,   2},
    {{ 2,  2}, {{  9,  16}}, STAGE_VIOLENCE_DISTRICT,   3},
    {{ 1,  2}, {{ 13,  16}}, STAGE_VIOLENCE_DISTRICT,   4},
};
StaticLinkTrigger sStg08TeleportTriggers[3] = {
    /* chunk     tile         stage                      spawn */
    {{ 0,  0}, {{  8,  11}}, STAGE_FLESH_TUNNELS,       0},
    {{ 0,  0}, {{  9,   4}}, STAGE_FLESH_TUNNELS,       1},
    {{ 0,  1}, {{ 17,   8}}, STAGE_FLESH_TUNNELS,       2},
};
StaticLinkTrigger *sTeleportTriggers[STAGE_COUNT] = {
    NULL, NULL, sStg02TeleportTriggers, sStg03TeleportTriggers,
    NULL, sStg05TeleportTriggers, NULL, NULL,
    sStg08TeleportTriggers, NULL, NULL, NULL,
    NULL, NULL,
};
s8 sTeleportTriggersCount[STAGE_COUNT] = {0, 0, 8, 1, 0, 5, 0, 0, 3, 0, 0, 0, 0, 0};

/* The staircases, TestForStaircaseNodes' tables (Bright Moon Cottage only), with
 * headings as for the tunnels (DreamSys__CheckStaircaseHeading). */
StageSpawn sStg00StaircaseSpawns[8] = {
    /* chunk      tile     adj extra */
    {{ 0,  1}, { 8,  7}, 25,  0},
    {{ 0,  2}, { 8,  7}, 26,  0},
    {{ 0,  3}, { 8,  7}, 27,  0},
    {{ 0,  4}, { 9,  7}, 28,  2},
    {{ 0,  0}, { 9,  9},  0,  1},
    {{ 0,  1}, { 9,  9},  1,  1},
    {{ 0,  2}, { 9,  9},  2,  1},
    {{ 0,  3}, { 9,  9},  3,  3},
};
u8 sStg00StaircaseExitHeadings[8] = {0, 0, 0, 3, 3, 3, 3, 3};
StageSpawn *sStaircaseSpawns[STAGE_COUNT] = {
    sStg00StaircaseSpawns, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
u8 *sStaircaseExitHeadings[STAGE_COUNT] = {
    sStg00StaircaseExitHeadings, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
StaticLinkTrigger sStg00StaircaseTriggers[8] = {
    /* chunk     tile         stage                      spawn */
    {{ 0,  0}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 0},
    {{ 0,  1}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 1},
    {{ 0,  2}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 2},
    {{ 0,  3}, {{  9,   9}}, STAGE_BRIGHT_MOON_COTTAGE, 3},
    {{ 0,  1}, {{  8,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 4},
    {{ 0,  2}, {{  8,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 5},
    {{ 0,  3}, {{  8,   8}}, STAGE_BRIGHT_MOON_COTTAGE, 6},
    {{ 0,  4}, {{  8,   7}}, STAGE_BRIGHT_MOON_COTTAGE, 7},
};
u8 sStg00StaircaseEnterHeadings[8] = {2, 2, 2, 2, 2, 2, 2, 1};
StaticLinkTrigger *sStaircaseTriggers[STAGE_COUNT] = {
    sStg00StaircaseTriggers, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
u8 *sStaircaseEnterHeadings[STAGE_COUNT] = {
    sStg00StaircaseEnterHeadings, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL,
};
s8 sStaircaseTriggersCount[STAGE_COUNT] = {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/* IsDaySpecial's table of special days. */
s16 sSpecialDays[42] = {2, 15, 43, 81, 120, 126, 202, 259, 267, 284, 308, 328, 358, 7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 77, 84, 91, 98, 105, 112, 119, 126, 134, 141, 148, 155, 162, 169, 176, 183, 190, 197, 204};
/* clang-format on */
