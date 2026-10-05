/* The style layer's .data tables, in address order. style_layer.c includes
 * this file in place; it is never compiled alone. */
/* clang-format off */
/* The effects' spawn rotations (x, y, z degrees) and scales. */
Ratio16 sStyleSpawnRotations[7][3] = {
    {{0, 1}, {0, 1}, {0, 1}},
    {{0, 1}, {60, 1}, {0, 1}},
    {{0, 1}, {120, 1}, {0, 1}},
    {{0, 1}, {180, 1}, {0, 1}},
    {{0, 1}, {230, 1}, {0, 1}},
    {{-5, 1}, {0, 1}, {0, 1}},
    {{-3, 1}, {180, 1}, {0, 1}},
};
Ratio16 sStyleSpawnScales[5][3] = {
    {{7, 1}, {1, 1}, {7, 1}},
    {{6, 1}, {1, 1}, {7, 1}},
    {{7, 1}, {1, 1}, {6, 1}},
    {{7, 1}, {1, 1}, {2, 1}},
    {{2, 1}, {1, 1}, {7, 1}},
};
Ratio16 sStyleKind1Scale[2][3] = {
    {{1, 1}, {1, 1}, {1, 1}},
    {{2, 1}, {1, 1}, {2, 1}},
};

/* The kind-3 and kind-2 effects' colours. */
u8 sStyleKind3Colors[4][3] = {
    {255, 230, 180}, {255, 180, 180}, {255, 0, 0}, {0, 0, 0},
};
u8 sStyleKind2Colors[4][3] = {
    {255, 128, 0}, {200, 200, 255}, {128, 128, 255}, {0, 0, 0},
};

/* The decoration's band colours, one RGB triple per band: A, or B for a
 * variant-0 config whose decor colour is STYLE_DECOR_B_PALETTE_INDEX. */
u8 sStyleDecorColorsA[18 * 3] = {
     55, 119, 148,   60, 121, 151,   65, 123, 154,   70, 125, 157,   75, 127, 160,   80, 129, 163,
     85, 131, 166,   90, 133, 169,   95, 135, 172,  100, 137, 175,  105, 139, 178,  110, 141, 181,
    115, 143, 184,  120, 150, 187,  123, 152, 190,  126, 153, 192,  128, 154, 193,  130, 155, 194,
};
u8 sStyleDecorColorsB[18 * 3] = {
     55, 119, 148,   60, 121, 151,   65, 123, 154,   70, 125, 157,   75, 127, 160,   80, 129, 163,
     90, 131, 166,  110, 133, 169,  130, 135, 172,  150, 137, 175,  170, 139, 178,  190, 141, 181,
    210, 143, 184,  230, 150, 187,  235, 152, 190,  240, 153, 192,  245, 154, 193,  250, 155, 194,
};

/* The StageMap's light directions and colours (sStyleConfig's first two
 * words, setChildParams). */
s16 sStyleLightDirs[10] = {50, 300, 60, 0, 500, -60, -300, 20, 20, 0};
u8 sStyleLightColors[3][3] = {
    {200, 200, 200}, {200, 240, 240}, {255, 230, 230},
};

/* 24 RGB triples (a greyscale ramp first: 0, 64, 128, 255) the configs'
 * colour indices pick, and six fogNear distances, 26624 down to 2048. */
/* MATCHING: sStylePalette is indexed as u8[][3], for the stride-3 address arithmetic. */
u8 sStylePalette[24][3] = {
    {0, 0, 0}, {64, 64, 64}, {128, 128, 128}, {255, 255, 255}, {64, 0, 0}, {8, 32, 8},
    {32, 32, 8}, {24, 8, 8}, {24, 24, 8}, {24, 24, 128}, {8, 8, 24}, {255, 0, 0},
    {100, 140, 180}, {50, 119, 145}, {130, 155, 194}, {0, 100, 190}, {0, 50, 30}, {70, 50, 0},
    {250, 155, 194}, {0, 255, 0}, {120, 140, 180}, {120, 0, 180}, {180, 0, 120}, {0, 0, 0},
};
s32 sStyleFogNears[6] = {26624, 20480, 14336, 8192, 4096, 2048};

/* Kind 0's effect counts. */
s8 sStyleKind0Counts[4] = {0, 3, 8, 16};

/* The fixed configs, {colorMode, fogLevel, farColorIndex, clearColorIndex}:
 * Violence District's (in sStyleSpawnY; the others' are .sdata, declared
 * above), then the four fallback variants PickStyleFallbackConfig picks
 * from, by day and stage. */
StyleSpawnYBlock sStyleSpawnY = {
    {-6144, -10240, -14336, -20480},
    {{0, 2, 10, 10}, {0, 1, 14, 13}},
};
s8 sStyleVariant0Configs[7][4] = {
    {0, 1, 14, 13},
    {0, 2, 14, 13},
    {0, 1, 18, 13},
    {0, 2, 18, 13},
    {0, 1, 14, 13},
    {0, 2, 14, 13},
    {0, 2, 20, 12},
};
s8 sStyleVariant1Configs[10][4] = {
    {0, 3,  3,  3},
    {0, 2,  3,  3},
    {0, 1,  3,  3},
    {0, 0, 21, 21},
    {0, 1, 19, 19},
    {0, 2,  2,  2},
    {0, 3,  2,  2},
    {0, 4,  2,  2},
    {0, 3, 21, 22},
    {0, 4,  1,  1},
};
s8 sStyleVariant2Configs[12][4] = {
    {0, 2,  3,  0},
    {1, 2,  3,  0},
    {2, 2,  3,  0},
    {0, 2,  7, 10},
    {1, 2,  7, 10},
    {2, 2,  7, 10},
    {0, 2,  8, 10},
    {1, 2,  8, 10},
    {2, 2,  8, 10},
    {0, 2,  6,  6},
    {0, 2,  5,  5},
    {0, 1,  4,  4},
};
s8 sStyleVariant3Configs[5][4] = {
    {0, 3,  0,  0},
    {0, 3,  9,  9},
    {0, 3, 22, 17},
    {0, 3, 16, 16},
    {0, 3, 17, 17},
};
s8 *sStyleVariantConfigs[4] = {sStyleVariant0Configs[0], sStyleVariant1Configs[0], sStyleVariant2Configs[0], sStyleVariant3Configs[0]};
s8 sStyleVariantConfigCounts[4] = {7, 10, 12, 5};
s8 sStyleVariantPicks[16] = {0, 1, 2, 3, 1, 2, 3, 0, 0, 0, 2, 2, 1, 3, 1, 0};

/* Each stage's fixed configs, or NULL for the fallback. */
StyleStageConfig *sStyleStageConfigs[14] = {
    NULL, NULL, NULL, NULL,
    NULL, sStyleSpawnY.stage05Configs, sStyleStage06Configs, sStyleStage06Configs,
    sStyleStage08Configs, sStyleStage06Configs, sStyleStage10Configs, sStyleStage11Configs,
    sStyleStage11Configs, NULL,
};

/* The one StyleConfig the layer fills (FillStyleFromConfig writes the last
 * four words; clearColor is also the kind-2 effects' alternate colour). */
StyleConfig sStyleConfig = {
    sStyleLightDirs, sStyleLightColors, &sStyleAmbientGrey, NULL,
    &sStyleGrey10, -1, NULL, 0,
};

/* StyleScrollVramStrips' two VRAM strips and their one-column scratch
 * rects (RotateVramRectRight reads only a scratch rect's corner). */
DrawRect sStyleStripRectA = {0x0, 0x1F0, 248, 8};
DrawRect sStyleStripScratchA = {0x100, 0x1F0, 1, 8};
DrawRect sStyleStripRectB = {0x0, 0x1F8, 248, 8};
DrawRect sStyleStripScratchB = {0x100, 0x1F8, 1, 8};

/* Per cue index, the distance within which it starts, and its callback. */
s32 sStyleCueDistanceTable[15] = {0, 81920, 40960, 16384, 40960, 61440, 81920, 81920, 40960, 40960, 20480, 40960, 61440, 40960, 4096};
SoundCueCallbackFn sStyleCueCallbacks[15] = {
    NULL,
    (SoundCueCallbackFn)StyleCue00, (SoundCueCallbackFn)StyleCue01, (SoundCueCallbackFn)StyleCue02,
    (SoundCueCallbackFn)StyleCue03, (SoundCueCallbackFn)StyleCue04, (SoundCueCallbackFn)StyleCue05,
    (SoundCueCallbackFn)StyleCue06, (SoundCueCallbackFn)StyleCue07, (SoundCueCallbackFn)StyleCue08,
    (SoundCueCallbackFn)StyleCue09, (SoundCueCallbackFn)StyleCue10, (SoundCueCallbackFn)StyleCue11,
    (SoundCueCallbackFn)StyleCue12, (SoundCueCallbackFn)StyleCue13,
};

/* The in-cell offsets a record's offsetIndex picks. */
CellOffset sStyleCueOffsets[1] = {{{0, 0}, 0}};

/* Each stage's positional cues: the chunk and cell (column in the low
 * byte, row in the high), the offset index and the cue index. */
StyleCueRecord sStyleStage00Cues[1] = {
    {{0x0200, 0x0806}, 0, 0, 14, 0},
};
StyleCueRecord sStyleStage02Cues[9] = {
    /* chunk   cell   off  cue */
    {{0x0304, 0x040B}, 0, 0,  9, 0},
    {{0x0003, 0x0910}, 0, 0,  4, 0},
    {{0x0104, 0x0206}, 0, 0,  4, 0},
    {{0x0104, 0x0E05}, 0, 0,  4, 0},
    {{0x0203, 0x050F}, 0, 0,  4, 0},
    {{0x0203, 0x110F}, 0, 0,  4, 0},
    {{0x0304, 0x0905}, 0, 0,  4, 0},
    {{0x0403, 0x0110}, 0, 0,  4, 0},
    {{0x0403, 0x0D10}, 0, 0,  4, 0},
};
StyleCueRecord sStyleStage03Cues[29] = {
    /* chunk   cell   off  cue */
    {{0x0407, 0x0607}, 0, 0,  1, 0},
    {{0x040A, 0x0B0E}, 0, 0,  1, 0},
    {{0x0105, 0x0A03}, 0, 0,  2, 0},
    {{0x0309, 0x0600}, 0, 0,  5, 0},
    {{0x000A, 0x0A09}, 0, 0,  5, 0},
    {{0x000A, 0x0A09}, 0, 0,  5, 0},
    {{0x000A, 0x0A09}, 0, 0,  5, 0},
    {{0x0002, 0x0909}, 0, 0, 12, 0},
    {{0x000A, 0x0A09}, 0, 0, 12, 0},
    {{0x000A, 0x0A09}, 0, 0, 12, 0},
    {{0x0104, 0x0009}, 0, 0,  1, 0},
    {{0x0104, 0x1308}, 0, 0,  1, 0},
    {{0x0208, 0x0C0A}, 0, 0,  1, 0},
    {{0x0407, 0x0507}, 0, 0,  1, 0},
    {{0x040A, 0x0C0E}, 0, 0,  1, 0},
    {{0x0502, 0x0B02}, 0, 0,  1, 0},
    {{0x0705, 0x0713}, 0, 0,  1, 0},
    {{0x0003, 0x0A03}, 0, 0, 12, 0},
    {{0x000A, 0x1301}, 0, 0, 12, 0},
    {{0x0105, 0x0713}, 0, 0, 12, 0},
    {{0x010C, 0x0C00}, 0, 0, 12, 0},
    {{0x0207, 0x0304}, 0, 0, 12, 0},
    {{0x0209, 0x0011}, 0, 0, 12, 0},
    {{0x0507, 0x080F}, 0, 0, 12, 0},
    {{0x0706, 0x0805}, 0, 0, 12, 0},
    {{0x0805, 0x0B07}, 0, 0, 12, 0},
    {{0x0D0D, 0x0F0F}, 0, 0, 12, 0},
    {{0x0E01, 0x0F01}, 0, 0, 12, 0},
    {{0x0E0D, 0x0909}, 0, 0, 12, 0},
};
StyleCueRecord sStyleStage04Cues[5] = {
    /* chunk   cell   off  cue */
    {{0x0305, 0x0902}, 0, 0,  3, 0},
    {{0x0200, 0x1110}, 0, 0,  8, 0},
    {{0x0401, 0x0D10}, 0, 0,  6, 0},
    {{0x0401, 0x0011}, 0, 0,  6, 0},
    {{0x0402, 0x020E}, 0, 0,  6, 0},
};
StyleCueRecord sStyleStage05Cues[9] = {
    /* chunk   cell   off  cue */
    {{0x0504, 0x0600}, 0, 0, 11, 0},
    {{0x0504, 0x0600}, 0, 0,  7, 0},
    {{0x0502, 0x0202}, 0, 0,  7, 0},
    {{0x0400, 0x0F12}, 0, 0,  7, 0},
    {{0x0301, 0x0D08}, 0, 0,  7, 0},
    {{0x0200, 0x1112}, 0, 0,  7, 0},
    {{0x0101, 0x1307}, 0, 0,  7, 0},
    {{0x0000, 0x1212}, 0, 0,  7, 0},
    {{0x0000, 0x0512}, 0, 0,  7, 0},
};
StyleCueRecord sStyleStage10Cues[1] = {
    {{0x0002, 0x0A08}, 0, 0, 10, 0},
};
StyleCueRecord sStyleStage11Cues[1] = {
    {{0x0101, 0x080C}, 0, 0, 10, 0},
};
StyleCueRecord sStyleStage13Cues[1] = {
    {{0x0000, 0x0903}, 0, 0, 13, 0},
};

/* The per-stage cue lists and their lengths. */
StyleCueRecord *sStyleCueRecordLists[14] = {
    sStyleStage00Cues, NULL, sStyleStage02Cues, sStyleStage03Cues,
    sStyleStage04Cues, sStyleStage05Cues, NULL, NULL,
    NULL, NULL, sStyleStage10Cues, sStyleStage11Cues,
    NULL, sStyleStage13Cues,
};
u8 sStyleCueRecordCounts[14] = {1, 0, 9, 29, 5, 9, 0, 0, 0, 0, 1, 1, 0, 1};
/* clang-format on */
