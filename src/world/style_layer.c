/*
 * The style layer: the free functions that give a day's scene its style,
 * with ObjM (src/world/objm.c) as their client. ObjM__InitStyleAndWorld
 * calls RegisterStyleConfig once per scene and keeps the result,
 * sStyleConfig, as ObjM::styleConfig. In ROM order, under section banners
 * after the first:
 *  - its setup: RegisterStyleConfig stores the scene (grid, stage, ObjM's
 *    scene references, day) in the sStyle globals the rest reads;
 *    ApplyStyleConfig takes the stage's four-byte StyleStageConfig, or
 *    PickStyleFallbackConfig's, and FillStyleFromConfig turns it into
 *    StyleConfig colours and a fog distance; ApplyStyleDecorationIfSet
 *    builds the decoration box, a full-screen semi-transparent BoxFill
 *    under the viewport's fade box, when the config asked for one;
 *  - its per-scene objects (StyleFlushDecoration to StyleScrollVramStrips);
 *  - its sound cues (StyleCue00..13 and ComputeStyleCueFalloff), then
 *    IsStyleVariantEven.
 * What the style configs stand for in the game is not established; the
 * names describe mechanics.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "objm.h"
#include "stage_map.h"
#include "box_fill.h"
#include "viewport.h"
#include "style_effect.h"
#include "sound_cue_set.h"
#include "tim_image.h"
#include "dream_aux.h"
#include "style_layer.h"
#include "bmem_pmgr.h"
#include <rand.h>

/* The fog levels whose config also gets a decoration box (ApplyStyleConfig):
 * this one and up, sStyleFogNears' two nearest (fogNear 4096 and 2048). */
#define STYLE_DECOR_FOG_LEVEL 4

/* The decoration set: this many BoxFill bands, stacked 3 pixels apart. */
#define STYLE_DECOR_BANDS 18

/* Every band's draw priority, which Viewport__DrawNode hands to
 * GsSortBoxFill unmasked. */
#define STYLE_DECOR_PRI 0x1FFF

/* How far down (pixels) decor variant 2 draws the set. */
#define STYLE_DECOR_VARIANT2_DROP 30

/* StyleUpdateDecorSet's fade step: the viewport's view-point y less its
 * reference-point y, per step. */
#define STYLE_DECOR_FADE_HEIGHT 600

/* Kind-0 plus kind-1 effects built for style variant 2. */
#define STYLE_VARIANT2_EFFECTS 16

/* The palette entry that, as a variant-0 config's decor colour, selects
 * sStyleDecorColorsB instead of sStyleDecorColorsA (PickStyleFallbackConfig). */
#define STYLE_DECOR_B_PALETTE_INDEX 18

extern s32 sStyleGrid;
extern s32 sStyleStage;
extern s32 sStyleTickCount;
extern s32 sStyleDay;
extern s32 sStyleUnreadArg;
extern s32 sStyleSceneRefs; /* a StyleSceneRefs * (below) */
extern s32 sStyleVariant;
/* StyleCueSlot is defined with the cue functions below; this only clears the slots. */
typedef struct StyleCueSlot StyleCueSlot;
extern StyleCueSlot *sStyleCueSlots[2];

extern void *ApplyStyleConfig(void);

/** @brief A stage's style config: four signed bytes, from
 * sStyleStageConfigs (NULL for a stage without a fixed one) or
 * PickStyleFallbackConfig (below), which FillStyleFromConfig turns
 * into sStyleConfig's last four words. */
typedef struct StyleStageConfig {
    s8 colorMode; /**< StyleConfig::colorMode */
    s8 fogLevel; /**< sStyleFogNears index; STYLE_DECOR_FOG_LEVEL and up also build the decoration box */
    s8 farColorIndex; /**< sStylePalette index: StyleConfig::farColor, and the decoration box's colour */
    s8 clearColorIndex; /**< sStylePalette index: StyleConfig::clearColor */
} StyleStageConfig;

/** @brief One 8-byte record of a stage's cue list (sStyleCueRecordLists):
 * the cell it sits in, its in-cell offset (a sStyleCueOffsets index) and
 * `cue`, its cue index (the sStyleCueCallbacks and sStyleCueDistanceTable
 * row, InitSoundCueSet's tag), negated while a slot holds the record. */
typedef struct StyleCueRecord {
    CellKey key;    /**< +0x0 the cell the cue sits in */
    u8 offsetIndex; /**< +0x4 its offset in the cell, a sStyleCueOffsets index */
    u8 pad5;        /* +0x5 */
    s8 cue;         /**< +0x6 the cue index; negated while a slot holds the record */
    u8 pad7;        /* +0x7 */
} StyleCueRecord;

/* The fixed configs of the stages whose config is in .sdata, and the two
 * greys sStyleConfig points at (+0x008 ambientColor and +0x010). */
extern s32 sStyleAmbientGrey;
extern s32 sStyleGrey10;
extern StyleStageConfig sStyleStage06Configs[];
extern StyleStageConfig sStyleStage08Configs[];
extern StyleStageConfig sStyleStage10Configs[];
extern StyleStageConfig sStyleStage11Configs[];

/* Defined below: the cue callbacks sStyleCueCallbacks lists. */
void StyleCue00(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue01(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue02(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue03(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue04(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue05(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue06(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue07(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue08(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue09(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue10(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue11(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue12(StyleCueSlot *ctx, SoundCueSet *set);
void StyleCue13(StyleCueSlot *ctx, SoundCueSet *set);

/* The style layer's data, in address order. */
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

/* Kind 0's effect counts, and the spawn heights the kinds pick from. */
s8 sStyleKind0Counts[4] = {0, 3, 8, 16};
s32 sStyleSpawnYChoices[4] = {-6144, -10240, -14336, -20480};

/* The fixed configs, {colorMode, fogLevel, farColorIndex, clearColorIndex}:
 * stage 5's (the others' are .sdata, declared above), then the four
 * fallback variants PickStyleFallbackConfig picks from, by day and stage. */
StyleStageConfig sStyleStage05Configs[2] = {{0, 2, 10, 10}, {0, 1, 14, 13}};
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
    NULL, sStyleStage05Configs, sStyleStage06Configs, sStyleStage06Configs,
    sStyleStage08Configs, sStyleStage06Configs, sStyleStage10Configs, sStyleStage11Configs,
    sStyleStage11Configs, NULL,
};

/* The one StyleConfig the layer fills (FillStyleFromConfig writes the last
 * four words; clearColor is also the kind-2 effects' alternate colour). */
StyleConfig sStyleConfig = {
    (s32)sStyleLightDirs, (s32)sStyleLightColors, (s32)&sStyleAmbientGrey, NULL,
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
u8 *sStyleCueRecordLists[14] = {
    (u8 *)sStyleStage00Cues, NULL, (u8 *)sStyleStage02Cues, (u8 *)sStyleStage03Cues,
    (u8 *)sStyleStage04Cues, (u8 *)sStyleStage05Cues, NULL, NULL,
    NULL, NULL, (u8 *)sStyleStage10Cues, (u8 *)sStyleStage11Cues,
    NULL, (u8 *)sStyleStage13Cues,
};
u8 sStyleCueRecordCounts[14] = {1, 0, 9, 29, 5, 9, 0, 0, 0, 0, 1, 1, 0, 1};
/* clang-format on */

s32 RegisterStyleConfig(s32 grid, s32 stage, s32 sceneRefs, s32 day, s32 unreadArg) {
    StyleCueSlot **slot;
    s32 i;

    if (sStyleGrid == 0) {
        i = ARRAY_COUNT(sStyleCueSlots) - 1;
        slot = &sStyleCueSlots[ARRAY_COUNT(sStyleCueSlots) - 1];
        sStyleGrid = grid;
        sStyleStage = stage;
        sStyleSceneRefs = sceneRefs;
        sStyleVariant = -1;
        sStyleDay = day;
        sStyleUnreadArg = unreadArg;
        sStyleTickCount = 0;
        do {
            *slot = 0;
            i--;
            slot--;
        } while (i >= 0);
        return (s32)ApplyStyleConfig();
    }
    return 0;
}

extern void *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg);
extern const u8 *sStyleDecorColor;

/* The stage's fixed config, or with none PickStyleFallbackConfig's, into
 * sStyleConfig, whose first three words (the StageMap's light settings)
 * are fixed. */
void *ApplyStyleConfig(void) {
    StyleStageConfig *cfg = sStyleStageConfigs[sStyleStage];

    if (cfg == 0) {
        cfg = PickStyleFallbackConfig();
    }
    FillStyleFromConfig(&sStyleConfig, cfg);
    if (cfg->fogLevel >= STYLE_DECOR_FOG_LEVEL) {
        sStyleDecorColor = sStylePalette[cfg->farColorIndex];
    }
    return &sStyleConfig;
}

void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg) {
    style->clearColor = sStylePalette[cfg->clearColorIndex];
    style->farColor = sStylePalette[cfg->farColorIndex];
    style->fogNear = sStyleFogNears[cfg->fogLevel];
    style->colorMode = cfg->colorMode;
}

/** @brief What sStyleSceneRefs points at: ObjM's +0x06C..+0x07B block
 * (ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it; include/objm.h). */
typedef struct StyleSceneRefs {
    void *sound;      /**< +0x000, ObjM::ctorSound: the sound object the cue functions take first */
    void *dreamerTmd; /**< +0x004, ObjM::dreamerTmd */
    void *etcTim;     /**< +0x008, ObjM::etcTim */
    Viewport *viewport; /**< +0x00C, ObjM::cachedViewport */
} StyleSceneRefs;

extern s32 sStyleDecorObj;           /* a BoxFill * */
extern s32 sStyleDecorBoxSize[2];    /* 320 x 240, the screen */
extern BoxFillPos sStyleDecorBoxPos; /* (-100, -100), as Viewport's own fade box */

void ApplyStyleDecorationIfSet(void) {
    SceneNode *fadeBox;

    if (sStyleDecorColor != 0) {
        sStyleDecorObj = (s32)New_BoxFill(sStyleDecorBoxSize, (ColorRgb *)sStyleDecorColor, 0);
        ((BoxFill *)sStyleDecorObj)->methods->setSemiTransOn((BoxFill *)sStyleDecorObj, 1);
        ((BoxFill *)sStyleDecorObj)->methods->setSemiTransRate((BoxFill *)sStyleDecorObj, 0);

        fadeBox = ((StyleSceneRefs *)sStyleSceneRefs)
                      ->viewport->methods->getFadeBox(((StyleSceneRefs *)sStyleSceneRefs)->viewport);

        ((BoxFillAttachToParentFn)((BoxFill *)sStyleDecorObj)->methods->attachToParent)(
            (BoxFill *)sStyleDecorObj, fadeBox, &sStyleDecorBoxPos);
    }
}

/* ---- The style layer's per-scene objects ---------------------------------
 *
 * What TickStyle builds on a scene's first tick, updates every tick and
 * StyleTeardown releases, in sStyle globals RegisterStyleConfig set (the
 * scene's StageMap, ObjM's stage, its StyleSceneRefs and the day):
 *  - the decoration box (sStyleDecorObj), when the config has a colour;
 *  - the decor set: STYLE_DECOR_BANDS BoxFill bands (sStyleDecorSlots) under
 *    the viewport's fade box, shaded every tick by the view point's height;
 *  - the effect slots: StyleEffects of kinds 0..3 (sStyleEffectSlots), each
 *    built from the one parameter block the StyleFillEffectKindN functions
 *    fill, updated every tick with the target position;
 *  - two positional sound cues (sStyleCueSlots): a free slot claims the next
 *    record of the stage's cue list within its cue's distance of the target
 *    and starts the record's SoundCueSet callback; a claimed slot is
 *    serviced while the target stays in range and flushed when it leaves.
 * The target is the grid's target cell as a world position; on stages 2 to
 * 5 StyleScrollVramStrips also rotates a VRAM strip. What a variant, an
 * effect kind or a cue stands for in the game is not established; the names
 * describe mechanics.
 */

/* Releases the decoration box, if ApplyStyleDecorationIfSet made one. */
void StyleFlushDecoration(void) {
    if (sStyleDecorColor != 0) {
        ((BoxFill *)sStyleDecorObj)->methods->release((BoxFill *)sStyleDecorObj);
        sStyleDecorColor = 0;
    }
}

extern s32 sStyleConfigIndex;
extern const u8 *sStyleClearColor;
extern const u8 *sStyleDecorColors;
extern s32 sStyleDecorVariant;

/* The config for a stage without a fixed one: the variant from
 * sStyleVariantPicks[(day + stage) & 0xF], then record (day + stage) % count
 * of that variant's table. For variant 0 it also sets the clear colour, the
 * band colours and, for records 0..5, the decor variant (1, or 2 for 4..5). */
void *PickStyleFallbackConfig(void) {
    s32 seed;
    s32 variant;
    s32 count;
    s32 index;
    s8 *config;
    s32 clearIndex;
    s32 decorIndex;
    u8 *decorColors;

    seed = sStyleDay + sStyleStage;
    variant = sStyleVariantPicks[seed & 0xF];
    sStyleVariant = variant;
    count = sStyleVariantConfigCounts[variant];
    index = seed % count;
    sStyleConfigIndex = index;
    config = sStyleVariantConfigs[variant] + index * 4;
    if (variant == 0) {
        clearIndex = config[3];
        sStyleClearColor = sStylePalette[clearIndex];
        decorIndex = config[2];
        decorColors = sStyleDecorColorsB;
        if (decorIndex != STYLE_DECOR_B_PALETTE_INDEX) {
            decorColors = sStyleDecorColorsA;
        }
        sStyleDecorColors = decorColors;
        if (index < 4) {
            sStyleDecorVariant = 1;
        } else if (index < 6) {
            sStyleDecorVariant = 2;
        }
    }
    return config;
}

extern s32 sStyleDecorPosX;
extern s32 sStyleDecorPosY;
extern s32 sStyleDecorSizeW;
extern s32 sStyleDecorSizeH;
extern BoxFill *sStyleDecorSlots[STYLE_DECOR_BANDS];

/* sStyleDecorPosX/Y and sStyleDecorSizeW/H are adjacent word pairs, a
 * BoxFillPos and a BoxFillSize. */
/* MATCHING: both are copied whole, never field by field. */

/* Builds the bands: band 0 in sStyleDecorColors' first colour, bands 1..17
 * attached under it, each 3 pixels lower and 7 shorter than the one before;
 * band 0 then goes under the viewport's fade box. */
void StyleBuildDecorSet(void) {
    BoxFillPos pos;
    BoxFillSize size;
    s32 i;
    BoxFill *band;
    Viewport *viewport;
    SceneNode *parent;

    if (sStyleDecorVariant == 0) {
        return;
    }
    pos = *(BoxFillPos *)&sStyleDecorPosX;
    if (sStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    size = *(BoxFillSize *)&sStyleDecorSizeW;
    sStyleDecorSlots[0] = New_BoxFill(&size, (void *)sStyleDecorColors, STYLE_DECOR_PRI);
    for (i = 1; i < STYLE_DECOR_BANDS; i++) {
        band = New_BoxFill(&size, (void *)(sStyleDecorColors + i * 3), STYLE_DECOR_PRI);
        sStyleDecorSlots[i] = band;
        ((BoxFillAttachToParentFn)band->methods->attachToParent)(
            band, (SceneNode *)sStyleDecorSlots[0], &pos);
        pos.y += 3;
        size.h -= 7;
    }

    viewport = ((StyleSceneRefs *)sStyleSceneRefs)->viewport;
    parent = viewport->methods->getFadeBox(viewport);
    ((BoxFillAttachToParentFn)sStyleDecorSlots[0]->methods->attachToParent)(sStyleDecorSlots[0],
                                                                            parent, &pos);
}

void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta);

/* Every tick, once the view point's y exceeds the reference point's by a
 * fade step: each band's colour
 * and the clear colour lose `fade` red and green and gain `fade` blue, and
 * the set moves down 3 pixels per unit of fade. */
void StyleUpdateDecorSet(void) {
    Viewport *viewport;
    s32 height;
    s32 fade;
    u8 rgb[8]; /* MATCHING: 8, not 3 (the frame keeps pos at sp+0x18) */
    BoxFillPos pos;
    s32 colorOfs;
    s32 i;
    BoxFill **slot;
    BoxFill *band;

    if (sStyleDecorVariant == 0) {
        return;
    }
    viewport = ((StyleSceneRefs *)sStyleSceneRefs)->viewport;
    height = viewport->refView.vp.y - viewport->refView.vr.y;
    fade = (height / STYLE_DECOR_FADE_HEIGHT) * 3;
    if (fade <= 0) {
        return;
    }
    pos = *(BoxFillPos *)&sStyleDecorPosX;
    i = 0;
    if (sStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    slot = sStyleDecorSlots;
    colorOfs = 0;
    pos.y += fade * 3;
    do {
        AdjustRgbByDelta(rgb, (u8 *)(colorOfs + sStyleDecorColors), fade);
        band = *slot;
        band->methods->setColor(band, 1, rgb);
        band = *slot;
        i++;
        colorOfs += 3;
        band->methods->setPosition(band, &pos);
        pos.y += 3;
        slot++;
    } while (i < STYLE_DECOR_BANDS);
    AdjustRgbByDelta(rgb, (u8 *)sStyleClearColor, fade);
    viewport->methods->setClearColor(viewport, (ColorRgb *)rgb);
}

/* dst = src with red and green less `delta`, blue more. */
void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

/* Releases the bands, if StyleBuildDecorSet made them. */
void StyleReleaseDecorSet(void) {
    if (sStyleDecorVariant != 0) {
        ReleaseBasicClassArray((BasicClass **)sStyleDecorSlots, ARRAY_COUNT(sStyleDecorSlots));
        sStyleDecorVariant = 0;
    }
}

extern s32 sStyleEffectSlotCount;
extern StyleEffect *sStyleEffectSlots[];
extern StyleEffect **StyleFillEffectKind0(StyleEffect **slots, s32 count, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos);
extern StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos);

/* Hands the variant and ObjM's resources to SetStyleEffectSources, then builds
 * the effect slots for the variant: sStyleKind0Counts' pick of
 * kind 0, kind 1 up to STYLE_VARIANT2_EFFECTS for variant 2, then one kind-3
 * (variant 0) or kind-2 (variant 2). */
void StyleBuildEffectSlots(LongVec3 *pos) {
    StyleSceneRefs *refs;
    s32 kind0Count;
    s32 kind1Count;
    StyleEffect **next;

    if (sStyleVariant < 0) {
        return;
    }
    refs = (StyleSceneRefs *)sStyleSceneRefs;
    SetStyleEffectSources(sStyleVariant, (Actor *)refs->dreamerTmd, (s32)refs->etcTim,
                          (s32)refs->viewport);
    kind0Count = sStyleKind0Counts[rand() & 3];
    kind1Count = (sStyleVariant == 2) ? STYLE_VARIANT2_EFFECTS - kind0Count : 0;
    sStyleEffectSlotCount = kind0Count + kind1Count;
    next = StyleFillEffectKind0(sStyleEffectSlots, kind0Count, pos);
    next = StyleFillEffectKind1(next, kind1Count, pos);
    if (sStyleVariant == 0) {
        StyleFillEffectKind3(next, pos);
    } else if (sStyleVariant == 2) {
        StyleFillEffectKind2(next, pos);
    } else {
        return;
    }
    sStyleEffectSlotCount = sStyleEffectSlotCount + 1;
}

/* Each slot's +0x0EC is StyleEffect__Update, called with the position
 * (include/style_effect.h: the slot keeps Actor's setPendingExtra type). */
void StyleUpdateEffectSlots(LongVec3 *pos) {
    s32 i;
    StyleEffect *slot;

    if (sStyleVariant < 0) {
        return;
    }
    for (i = 0; i < sStyleEffectSlotCount; i++) {
        slot = sStyleEffectSlots[i];
        ((StyleEffectUpdateFn)slot->methods->setPendingExtra)(slot, pos);
    }
}

/* Releases the effect slots, if StyleBuildEffectSlots ran. */
void StyleReleaseEffectSlots(void) {
    if (sStyleVariant >= 0) {
        ReleaseBasicClassArray((BasicClass **)sStyleEffectSlots, sStyleEffectSlotCount);
    }
}

/** @brief One of the two positional cues: the record it holds, the
 * record's world position, the last distance to the target, and its sound
 * cue. */
struct StyleCueSlot {
    StyleCueRecord *entry; /**< +0x000 the record the slot plays */
    LongVec3 pos;          /**< +0x004 the record's world position */
    s32 lastDist;          /**< +0x010 the last distance measured to the target */
    SoundCueSet cueSet;    /**< +0x014 the slot's sound cue */
}; /* 0x68 bytes */

extern StyleCueSlot *FlushStyleCue(StyleCueSlot *slot);

/* Releases everything TickStyle built and unregisters the scene. */
void StyleTeardown(void) {
    s32 i;

    StyleFlushDecoration();
    StyleReleaseDecorSet();
    StyleReleaseEffectSlots();
    for (i = 0; i < ARRAY_COUNT(sStyleCueSlots); i++) {
        sStyleCueSlots[i] = FlushStyleCue(sStyleCueSlots[i]);
    }
    if (sStyleGrid != 0) {
        sStyleGrid = 0; /* RegisterStyleConfig registers only while this is 0 */
    }
}

extern Ratio16 *sStyleSpawnScale;
extern s32 sStyleSpawnTableIndex;
/* The first word of the StyleEffectParams block every effect is built from
 * (sStyleSpawnOffsetX .. sStyleSpawnColors, separate symbols in the image). */
extern s32 sStyleSpawnOffsetX;
extern void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY);
extern void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY);

/* Fills `count` slots with kind-0 effects: a table index and scale for all
 * of them, an offset y (0: each setup picks one; a pick of 4 reads the word
 * after sStyleSpawnYChoices), and per slot SetupStyleSpawnParamsRandom, or
 * SetupStyleSpawnParamsDayMod7 on every seventh day. Returns the next slot. */
StyleEffect **StyleFillEffectKind0(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;
    void (*setup)(LongVec3 *, s32);

    sStyleSpawnTableIndex = rand() % 7;
    sStyleSpawnScale = sStyleSpawnScales[(u32)rand() % 5];
    offsetY = (u32)rand() % 5;
    if (offsetY != 0) {
        offsetY = sStyleSpawnYChoices[offsetY];
    }
    setup = SetupStyleSpawnParamsDayMod7;
    if (sStyleDay % 7 != 0) {
        setup = SetupStyleSpawnParamsRandom;
    }
    for (i = 0; i < count; i++) {
        setup(pos, offsetY);
        *slots =
            New_StyleEffect(0, (StyleEffectParams *)&sStyleSpawnOffsetX, (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

/* Fills `count` slots with kind-1 effects: sStyleKind1Scale, offset y
 * sStyleSpawnYChoices[2]. */
StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;

    offsetY = sStyleSpawnYChoices[2];
    sStyleSpawnScale = sStyleKind1Scale[0];
    for (i = 0; i < count; i++) {
        SetupStyleSpawnParamsRandom(pos, offsetY);
        *slots =
            New_StyleEffect(1, (StyleEffectParams *)&sStyleSpawnOffsetX, (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 sStyleSpawnColors[];
extern Ratio16 *sStyleSpawnRotation;
extern s32 sStyleSpawnOffsetY;
extern s32 sStyleSpawnOffsetZ;

/** @brief sStyleSpawnRotation seen as a one-field struct, through which
 * StyleFillEffectKind3 stores the effect's rotation. */
/* MATCHING: the rotation store goes through a one-field struct, so the
 * sStyleGrid load may schedule above it (a plain pointer store blocks it). */
typedef struct PtrBoxK3 {
    Ratio16 *p; /**< +0x000 the spawn rotation */
} PtrBoxK3;

/* Appends one kind-3 effect. With decor variant active and band colours B
 * its offset and colour are fixed; otherwise its z offset is folded to
 * -30720..0 and its colour is random. */
StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos) {
    s32 *offsetZ;
    PtrBoxK3 *rotation;

    SetupStyleSpawnParamsRandom(pos, sStyleSpawnYChoices[2]);
    if (sStyleDecorVariant != 0 && sStyleDecorColors == sStyleDecorColorsB) {
        sStyleSpawnOffsetX = -45056;
        sStyleSpawnOffsetY = -8192;
        sStyleSpawnOffsetZ = 0;
        sStyleSpawnColors[0] = (s32)sStyleKind3Colors[1];
    } else {
        offsetZ = &sStyleSpawnOffsetZ;
        if (*offsetZ > 0) {
            *offsetZ = -*offsetZ;
        }
        if (*offsetZ < -30720) {
            *offsetZ = -30720;
        }
        sStyleSpawnColors[0] = (s32)sStyleKind3Colors[(u32)rand() % 3];
    }
    rotation = (PtrBoxK3 *)&sStyleSpawnRotation;
    rotation->p = sStyleSpawnRotations[0];
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        3, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

/** @brief One word of sStyleSpawnColors seen as a one-field struct, through
 * which StyleFillEffectKind2 stores the effect's two colours. */
/* MATCHING: the first colour store goes through a one-field struct, as
 * PtrBoxK3's does, so the sStyleDay load may schedule above it. */
typedef struct S32BoxK2 {
    s32 v; /**< +0x000 a colour: first an RGB triple's address, then sStyleConfig.clearColor or 0 */
} S32BoxK2;

/* Appends one kind-2 effect with a random colour and, except on every
 * twentieth day, sStyleConfig.clearColor as its alternate colour. */
StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos) {
    s32 r;
    s32 altColor;
    S32BoxK2 *color;
    Ratio16 **rotation;
    s32 offsetY;

    r = rand();
    color = (S32BoxK2 *)sStyleSpawnColors;
    color->v = (s32)sStyleKind2Colors[(u32)r % 3];
    color++;
    altColor = (sStyleDay / 20) * 20; /* MATCHING: not `% 20`, which jump.c folds */
    if (sStyleDay != altColor) {
        altColor = (s32)sStyleConfig.clearColor;
    } else {
        altColor = 0;
    }
    /* MATCHING: the height is read before the colour is stored */
    offsetY = sStyleSpawnYChoices[2];
    color->v = altColor;
    SetupStyleSpawnParamsRandom(pos, offsetY);
    rotation = &sStyleSpawnRotation;
    *rotation = sStyleSpawnRotations[0];
    sStyleSpawnTableIndex = rand() % 6;
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots = New_StyleEffect(
        2, (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 sStyleSpawnModelLayout;

/* Randomises the spawn parameters: offset y (offsetY, or a random choice
 * when 0), x and z offsets of 0..22 steps of 2048 either side, a rotation
 * and a model layout. `pos` is unused. */
/* MATCHING: sStyleSpawnOffsetX is declared a scalar, not an array. */
void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY) {
    if (offsetY == 0) {
        offsetY = sStyleSpawnYChoices[rand() & 3];
    }
    sStyleSpawnOffsetY = offsetY;
    sStyleSpawnOffsetX = (rand() % 23) << 11;
    if (rand() & 1) {
        sStyleSpawnOffsetX = -sStyleSpawnOffsetX;
    }
    sStyleSpawnOffsetZ = (rand() % 23) << 11;
    if (rand() & 1) {
        sStyleSpawnOffsetZ = -sStyleSpawnOffsetZ;
    }
    sStyleSpawnRotation = sStyleSpawnRotations[(u32)rand() % 7];
    sStyleSpawnModelLayout = rand() % 5;
}

/* The every-seventh-day setup: fixed offset y, x of 0..19 steps of 2048, z
 * by day % 3 (40960, -40960, 2048), then the same rotation and layout
 * picks as SetupStyleSpawnParamsRandom. Both parameters are unused; it has
 * that function's signature because StyleFillEffectKind0 calls either
 * through one pointer. */
/* MATCHING: each rand() is used inline; one local for all three adds a move after every call. */
void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY) {
    s32 dayMod3;

    rand();
    sStyleSpawnOffsetY = sStyleSpawnYChoices[1];
    sStyleSpawnOffsetX = (rand() % 20) << 11;
    dayMod3 = sStyleDay % 3;
    sStyleSpawnOffsetZ = 40960;
    if (dayMod3 == 1) {
        sStyleSpawnOffsetZ = -40960;
    } else if (dayMod3 == 2) {
        sStyleSpawnOffsetZ = 2048;
    }
    sStyleSpawnRotation = sStyleSpawnRotations[(u32)rand() % 7];
    sStyleSpawnModelLayout = rand() % 5;
}

extern StyleCueRecord *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target);

/* Claims the next record in range for `slot` and starts its cue. A started
 * cue equal to *lastCue is reported back negated. Returns the slot, or NULL. */
StyleCueSlot *TryStartStyleCue(StyleCueSlot *slot, s32 *lastCue, LongVec3 *target, void *unused) {
    StyleCueRecord *entry;

    entry = FindNextStyleCueInRange(&slot->pos, &slot->lastDist, target);
    if (entry != 0) {
        slot->entry = entry;
        InitSoundCueSet(((StyleSceneRefs *)sStyleSceneRefs)->sound, &slot->cueSet, entry->cue, slot,
                        sStyleCueCallbacks[entry->cue]);
        if (entry->cue == *lastCue) {
            *lastCue = -entry->cue;
        }
        entry->cue = -entry->cue;
        return slot;
    }
    return 0;
}

extern s32 sStyleCueRecordIndex;

/* From sStyleCueRecordIndex on, the first free record of the stage's list
 * whose X+Z distance from the target is under its cue's distance; each
 * record looked at advances the index, so the next slot's search this tick
 * goes on from there.
 * Writes the record's world position and distance. */
StyleCueRecord *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target) {
    s32 j, remaining;
    u8 *records;
    StyleCueRecord *entry;
    CellKeyDesc buf;
    s32 dx, dz, dist;
    StageMap *grid;

    if (target == 0) {
        goto fail;
    }
    records = sStyleCueRecordLists[sStyleStage];
    remaining = sStyleCueRecordCounts[sStyleStage] - sStyleCueRecordIndex;
    entry = (StyleCueRecord *)(sStyleCueRecordIndex * 8 + (s32)records); /* MATCHING: operand order */
    for (j = 0; j < remaining; j++, entry++) {
        sStyleCueRecordIndex++;
        if (entry->cue > 0) {
            buf.key = entry->key;
            buf.offset = sStyleCueOffsets[entry->offsetIndex];
            grid = (StageMap *)sStyleGrid;
            grid->methods->computeCellOffsets(grid, pos, &buf);
            dx = pos->x - target->x;
            if (dx < 0) {
                dx = ~dx + 1; /* MATCHING: not -dx (the nor fills the delay slot) */
            }
            dz = pos->z - target->z;
            if (dz >= 0) {
                dist = dx + dz;
            } else {
                dist = dx - dz;
            }
            *outDist = dist;
            if (dist < sStyleCueDistanceTable[entry->cue]) {
                return entry;
            }
        }
    }
fail:
    return 0;
}

/* Stops the slot's cue and frees its record. Returns NULL for the slot. */
StyleCueSlot *FlushStyleCue(StyleCueSlot *slot) {
    FlushSoundCueSet(((StyleSceneRefs *)sStyleSceneRefs)->sound, &slot->cueSet);
    slot->entry->cue = -slot->entry->cue;
    return 0;
}

extern s32 IsStyleCueNear(StyleCueSlot *slot, LongVec3 *target);

/* One service pass of the slot's cue while the target is in range; 0 otherwise. */
s32 ServiceStyleCueIfNear(StyleCueSlot *slot, LongVec3 *target, void *unused) {
    if (IsStyleCueNear(slot, target) != 0) {
        ServiceSoundCueSet(((StyleSceneRefs *)sStyleSceneRefs)->sound, &slot->cueSet);
        return 1;
    }
    return 0;
}

/* Whether the target is within the held cue's distance (X+Z); keeps the distance. */
s32 IsStyleCueNear(StyleCueSlot *slot, LongVec3 *target) {
    s32 dx, dz, dist;
    s8 cue;

    if (target == 0) {
        return 0;
    }
    dx = slot->pos.x - target->x;
    if (dx < 0) {
        dx = ~dx + 1; /* MATCHING: not -dx (the nor fills the delay slot) */
    }
    dz = slot->pos.z - target->z;
    if (dz >= 0) {
        dist = dx + dz;
    } else {
        dist = dx - dz;
    }
    slot->lastDist = dist;
    cue = slot->entry->cue;
    if (dist < sStyleCueDistanceTable[-cue]) {
        dist = 1; /* MATCHING: not `return 1` (jump.c folds that to slt) */
        return dist;
    }
    return 0;
}

extern void ApplyStyleDecorationIfSet(void); /* above */
extern void StyleBuildDecorSet(void);
extern void StyleUpdateDecorSet(void);
extern void StyleScrollVramStrips(void);
extern StyleCueSlot sStyleCueSlotPool[];
extern StyleCueSlot *TryStartStyleCue(StyleCueSlot *slot, s32 *lastCue, LongVec3 *target, void *unused);
extern s32 ServiceStyleCueIfNear(StyleCueSlot *slot, LongVec3 *target, void *unused);

/* The per-tick entry (ObjM__TickStyle): the target position from `cell`,
 * first-tick builds, then the updates and the two cue slots. Returns
 * lastCue, negated if a slot started that cue this tick. */
s32 TickStyle(Descriptor10 *cell, void *unused, s32 lastCue) {
    LongVec3 *target;
    LongVec3 targetPos;
    s32 i;

    target = 0;
    if (cell != 0) {
        target = &targetPos;
        ((StageMap *)sStyleGrid)->methods->computeCellOffsets((StageMap *)sStyleGrid, target, cell);
    }
    if (sStyleTickCount++ == 0) {
        ApplyStyleDecorationIfSet();
        StyleBuildDecorSet();
        StyleBuildEffectSlots(target);
    }
    StyleUpdateDecorSet();
    StyleUpdateEffectSlots(target);
    StyleScrollVramStrips();
    sStyleCueRecordIndex = 0;
    for (i = 0; i < ARRAY_COUNT(sStyleCueSlots); i++) {
        if (sStyleCueSlots[i] != 0) {
            if (ServiceStyleCueIfNear(sStyleCueSlots[i], target, unused) == 0) {
                sStyleCueSlots[i] = FlushStyleCue(sStyleCueSlots[i]);
            }
            /* MATCHING: a no-op pair that gives target/i retail's registers */
            i++;
            i--;
        } else {
            sStyleCueSlots[i] = TryStartStyleCue(&sStyleCueSlotPool[i], &lastCue, target, unused);
        }
    }
    return lastCue;
}

/* One step of RotateVramRectRight's one-column VRAM rotation: stage 2 on the
 * strip at y 496, stages 3..5 on the one at y 504. */
void StyleScrollVramStrips(void) {
    DrawRect *rect;
    DrawPoint *scratch;
    s32 count;

    if (sStyleStage == 2) {
        rect = &sStyleStripRectA;
        scratch = (DrawPoint *)&sStyleStripScratchA;
        count = 1; /* MATCHING: a local set in each branch, not a literal argument */
    } else if ((u32)(sStyleStage - 3) < 3) {
        count = 1;
        rect = &sStyleStripRectB;
        scratch = (DrawPoint *)&sStyleStripScratchB;
    } else {
        return;
    }
    RotateVramRectRight(rect, count, scratch);
}

/* ---- The style layer's sound cues ---------------------------------------
 *
 *  - StyleCue00..StyleCue13, the 14 rows of sStyleCueCallbacks, and their
 *    helper ComputeStyleCueFalloff. Each is a SoundCueSet callback
 *    (include/sound_cue_set.h): TryStartStyleCue starts a style-cue slot's
 *    embedded set with the claimed cue record's index as the tag and that
 *    row of the table as the callback, as Entity does with the
 *    Entity__Cue* handlers of sEntityMoodTable's rows. Every tick a
 *    callback sets the set's attenuation from the slot's distance to the
 *    target and, on the ticks its pattern selects, requests VAB programs on
 *    the three voices; most restart the pattern by setting `tick` to -1
 *    once it passes a limit.
 *  - IsStyleVariantEven (include/dream_aux.h).
 */

/* ------------------------------------------------------------------ *
 * sStyleCueCallbacks's 14 slots (StyleCue00..StyleCue13) plus the shared
 * helper ComputeStyleCueFalloff they all call first.
 * ------------------------------------------------------------------ */

/* The owner every StyleCueNN callback receives is its StyleCueSlot:
 * TryStartStyleCue passes the slot as InitSoundCueSet's owner and its
 * embedded `cueSet` as the set, so a callback's `set` is `&ctx->cueSet`. */

/* Every callback calls this first; it is defined after them, in ROM order. */
s32 ComputeStyleCueFalloff(StyleCueSlot *ctx);

void StyleCue00(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 7;
        set->slots[0].octave = 0;
    } else if (tick == 2) {
        set->slots[1].program = 7;
        set->slots[1].octave = 0;
    } else if (tick == 5) {
        set->slots[2].program = 7;
        set->slots[2].octave = 0;
    } else if (tick >= 8) {
        set->tick = -1;
    }
}

void StyleCue01(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 24;
        set->slots[0].octave = -2;
    } else if (tick >= 1025) {
        set->tick = -1;
    }
}

void StyleCue02(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 12;
        set->slots[0].octave = 2;
    } else if (tick >= 5) {
        set->tick = -1;
    }
}

void StyleCue03(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 30;
        set->slots[0].vol = 32;
        set->slots[0].octave = 0;
        set->slots[0].endVol = 10;
    }
    if (set->tick % 400 == 0) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].vol = 32;
    set->slots[2].octave = 0;
    set->slots[2].endVol = 10;
}

void StyleCue04(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 3 == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = 0;
    }
    if (set->tick % 5 == 0) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
        set->slots[1].vol = 24;
        set->slots[1].endVol = 24;
    }
    if (set->tick % 7 == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = 0;
    }
    set->slots[2].program = 6;
    set->slots[2].octave = 1;
    set->slots[2].vol = 42;
    set->slots[2].endVol = 10;
}

void StyleCue05(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 30;
        set->slots[0].octave = -1;
    } else if (set->tick < 50 && set->tick % 5 == 4) {
        set->slots[1].program = 30;
        set->slots[1].octave = 0;
        set->slots[1].vol = set->slots[1].vol - set->tick * 2;
        set->slots[1].endVol = set->slots[1].vol;
    } else if (set->tick >= 101 && set->tick < 110) {
        set->slots[2].program = 13;
        set->slots[2].octave = 1;
    } else if (set->tick >= 201) {
        set->tick = -1;
    }
}

void StyleCue06(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 7;
        set->slots[0].octave = 2;
    } else if (tick >= 27) {
        set->tick = -1;
    }
}

void StyleCue07(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 20;
        set->slots[0].octave = 1;
    } else if (tick == 3) {
        set->slots[0].octave = 2;
        set->slots[0].vol = 24;
        set->slots[0].program = 3;
        set->slots[0].endVol = 20;
    } else if (tick >= 51) {
        set->tick = -1;
    }
}

void StyleCue08(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = 0;
        set->slots[0].vol = 64;
        set->slots[0].endVol = 64;
    }
}

void StyleCue09(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick % 20 == 0) {
        set->slots[0].program = 9;
        set->slots[0].octave = -2;
    }
}

void StyleCue10(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 rem;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    rem = set->tick % 20;
    if (rem == 1) {
        set->slots[0].program = 9;
        set->slots[0].octave = -2;
    } else if (rem == 16) {
        set->slots[1].program = 9;
        set->slots[1].octave = -2;
    }
}

void StyleCue11(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 rem;

    StyleCue10(ctx, set);
    rem = set->tick % 70;
    if (rem == 50) {
        set->slots[2].program = 20;
        set->slots[2].octave = 1;
    } else if (rem >= 54 && rem < 59) {
        set->slots[2].program = 13;
        set->slots[2].octave = 1;
    } else if (rem == 61) {
        set->slots[2].program = 9;
        set->slots[2].octave = -1;
    }
}

void StyleCue12(StyleCueSlot *ctx, SoundCueSet *set) {
    s32 tick;

    set->attenuation = ComputeStyleCueFalloff(ctx);
    tick = set->tick;
    if (tick == 0) {
        set->slots[0].program = 20;
        set->slots[0].octave = -2;
        set->slots[1].program = 20;
        set->slots[1].octave = -2;
    } else if (tick == 4) {
        set->slots[1].program = 20;
        set->slots[1].octave = -2;
    } else if (tick == 20) {
        set->slots[0].program = 16;
        set->slots[0].octave = -2;
        set->slots[1].program = 18;
        set->slots[1].octave = -2;
    } else if (tick >= 201) {
        set->tick = -1;
    }
}

void StyleCue13(StyleCueSlot *ctx, SoundCueSet *set) {
    set->attenuation = ComputeStyleCueFalloff(ctx);
    if (set->tick == 0) {
        set->slots[0].program = 24;
        set->slots[0].octave = 0;
    }
}

/* One range per cue record (15), indexed by the record's cue index: the
 * negative of `cue` while a slot has the record claimed. IsStyleCueNear
 * tests the slot's distance against the same row. */

s32 ComputeStyleCueFalloff(StyleCueSlot *ctx) {
    s32 range = sStyleCueDistanceTable[-ctx->entry->cue];
    s32 stepDist = range / ctx->cueSet.attenuationSteps;

    return ctx->lastDist / stepDist;
}

s32 IsStyleVariantEven(void) {
    return (sStyleVariant & 1) ^ 1;
}
