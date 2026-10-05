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
#include "link_resource.h"
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

/* sStyleDecorVariant: whether there is a decoration set, and where it sits.
 * PickStyleFallbackConfig picks it for variant 0's records 0..5. */
#define STYLE_DECOR_NONE 0    /* no bands */
#define STYLE_DECOR_UPPER 1   /* records 0..3: the set at sStyleDecorPos */
#define STYLE_DECOR_LOWERED 2 /* records 4..5: STYLE_DECOR_VARIANT2_DROP lower */

/* How far down (pixels) STYLE_DECOR_LOWERED draws the set. */
#define STYLE_DECOR_VARIANT2_DROP 30

/* StyleUpdateDecorSet's fade step: the viewport's view-point y less its
 * reference-point y, per step. */
#define STYLE_DECOR_FADE_HEIGHT 600

/* Kind-0 plus kind-1 effects built for style variant 2. */
#define STYLE_VARIANT2_EFFECTS 16

/* The palette entry that, as a variant-0 config's decor colour, selects
 * sStyleDecorColorsB instead of sStyleDecorColorsA (PickStyleFallbackConfig). */
#define STYLE_DECOR_B_PALETTE_INDEX 18

/* StyleCueSlot is defined with the cue functions below; this only clears the slots. */
typedef struct StyleCueSlot StyleCueSlot;

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

/* StyleLayer's small data, in address order: .sdata, then .sbss. */

/* RegisterStyleConfig's grid; it registers only while this is NULL. */
static StageMap *sStyleGrid SDATA = NULL;
static s32 sStyleDecorVariant SDATA = STYLE_DECOR_NONE;
static const u8 *sStyleDecorColor SDATA = NULL;
/* The fade box ApplyStyleDecorationIfSet builds: at (-100, -100), as
 * Viewport's own fade box, 320 x 240, the screen. */
static BoxFillPos sStyleDecorBoxPos SDATA = {-100, -100};
static s32 sStyleDecorBoxSize[2] SDATA = {320, 240};
/* The decoration set's place and size (StyleBuildDecorSet). Each is one
 * struct, not two words: the code copies both words through the first, so
 * the second must follow it on every platform. */
static BoxFillPos sStyleDecorPos SDATA = {-100, -60};
static BoxFillSize sStyleDecorSize SDATA = {320, 144};
/* The two greys sStyleConfig points at (+0x008 ambientColor and +0x010). */
static s32 sStyleAmbientGrey SDATA = 0x808080;
static s32 sStyleGrey10 SDATA = 0x808080;
/* The fixed configs of the stages whose config is in .sdata. */
static StyleStageConfig sStyleStage06Configs[] SDATA = {{0, 0, 0, 0}};
static StyleStageConfig sStyleStage11Configs[] SDATA = {{0, 0, 0, 0}, {0, 2, 3, 12}};
static StyleStageConfig sStyleStage08Configs[] SDATA = {{0, 2, 11, 11}};
static StyleStageConfig sStyleStage10Configs[] SDATA = {{0, 2, 3, 3}};

static s32 sStyleStage SBSS = 0;
static s32 sStyleTickCount SBSS = 0;
static s32 sStyleDay SBSS = 0;
static s32 sStyleUnreadArg SBSS = 0;
static struct StyleSceneRefs *sStyleSceneRefs SBSS = NULL;
static s32 sStyleVariant SBSS = 0;
static s32 sStyleConfigIndex SBSS = 0;
static s32 sStyleEffectSlotCount SBSS = 0;
static const u8 *sStyleDecorColors SBSS = NULL;
static const u8 *sStyleClearColor SBSS = NULL;
static BoxFill *sStyleDecorObj SBSS = NULL;
static s32 sStyleCueRecordIndex SBSS = 0;
static StyleCueSlot *sStyleCueSlots[2] SBSS = {NULL, NULL};

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

/** @brief The spawn heights the kinds pick from, then Violence District's
 * fixed configs, as one block: StyleFillEffectKind0 picks height 1..4, and
 * 4 is the word after the heights, the first config read as an s32
 * (0x0A0A0200, far below the stage). */
typedef struct StyleSpawnYBlock {
    s32 choices[4];                     /**< the spawn heights */
    StyleStageConfig stage05Configs[2]; /**< Violence District's fixed configs */
} StyleSpawnYBlock;

/* The style layer's data, in address order. */
#include "style_layer_tables.inc"

void *RegisterStyleConfig(StageMap *grid, s32 stage, void *sceneRefs, s32 day, s32 unreadArg) {
    StyleCueSlot **slot;
    s32 i;

    if (sStyleGrid == NULL) {
        i = ARRAY_COUNT(sStyleCueSlots) - 1; /* MATCHING: set here; set in the for header it differs */
        slot = &sStyleCueSlots[ARRAY_COUNT(sStyleCueSlots) - 1];
        sStyleGrid = grid;
        sStyleStage = stage;
        sStyleSceneRefs = sceneRefs;
        sStyleVariant = -1;
        sStyleDay = day;
        sStyleUnreadArg = unreadArg;
        sStyleTickCount = 0;
        for (; i >= 0; i--) {
            *slot = 0;
            slot--;
        }
        return ApplyStyleConfig();
    }
    return NULL;
}

extern void *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg);

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
    void *sound; /**< +0x000, ObjM::ctorSound: the sound object the cue functions take first */
    LinkResource *dreamerTmd; /**< +0x004, ObjM::dreamerTmd */
    void *etcTim;             /**< +0x008, ObjM::etcTim */
    Viewport *viewport;       /**< +0x00C, ObjM::cachedViewport */
} StyleSceneRefs;

void ApplyStyleDecorationIfSet(void) {
    SceneNode *fadeBox;

    if (sStyleDecorColor != 0) {
        sStyleDecorObj = New_BoxFill(sStyleDecorBoxSize, (ColorRgb *)sStyleDecorColor, 0);
        sStyleDecorObj->methods->setSemiTransOn(sStyleDecorObj, 1);
        sStyleDecorObj->methods->setSemiTransRate(sStyleDecorObj, 0);

        fadeBox = sStyleSceneRefs->viewport->methods->getFadeBox(sStyleSceneRefs->viewport);

        ((BoxFillAttachToParentFn)sStyleDecorObj->methods->attachToParent)(sStyleDecorObj, fadeBox,
                                                                           &sStyleDecorBoxPos);
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
 * The target is the grid's target cell as a world position; from Kyoto to
 * Violence District StyleScrollVramStrips also rotates a VRAM strip. What a variant, an
 * effect kind or a cue stands for in the game is not established; the names
 * describe mechanics.
 */

/* Releases the decoration box, if ApplyStyleDecorationIfSet made one. */
void StyleFlushDecoration(void) {
    if (sStyleDecorColor != 0) {
        sStyleDecorObj->methods->release(sStyleDecorObj);
        sStyleDecorColor = 0;
    }
}

/* The config for a stage without a fixed one: the variant from
 * sStyleVariantPicks[(day + stage) & 0xF], then record (day + stage) % count
 * of that variant's table. For variant 0 it also sets the clear colour, the
 * band colours and, for records 0..5, the decor variant (STYLE_DECOR_UPPER,
 * or STYLE_DECOR_LOWERED for 4..5). */
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
            sStyleDecorVariant = STYLE_DECOR_UPPER;
        } else if (index < 6) {
            sStyleDecorVariant = STYLE_DECOR_LOWERED;
        }
    }
    return config;
}

extern BoxFill *sStyleDecorSlots[STYLE_DECOR_BANDS];
#ifdef HOST_BUILD
/* Placed by address in the PS1 link; defined here for the host. */
BoxFill *sStyleDecorSlots[STYLE_DECOR_BANDS];
#endif

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

    if (sStyleDecorVariant == STYLE_DECOR_NONE) {
        return;
    }
    pos = sStyleDecorPos;
    if (sStyleDecorVariant == STYLE_DECOR_LOWERED) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    size = sStyleDecorSize;
    sStyleDecorSlots[0] = New_BoxFill(&size, (void *)sStyleDecorColors, STYLE_DECOR_PRI);
    for (i = 1; i < STYLE_DECOR_BANDS; i++) {
        band = New_BoxFill(&size, (void *)(sStyleDecorColors + i * 3), STYLE_DECOR_PRI);
        sStyleDecorSlots[i] = band;
        ((BoxFillAttachToParentFn)band->methods->attachToParent)(
            band, (SceneNode *)sStyleDecorSlots[0], &pos);
        pos.y += 3;
        size.h -= 7;
    }

    viewport = sStyleSceneRefs->viewport;
    parent = viewport->methods->getFadeBox(viewport);
    ((BoxFillAttachToParentFn)sStyleDecorSlots[0]->methods->attachToParent)(sStyleDecorSlots[0],
                                                                            parent, &pos);
}

void AdjustRgbByDelta(u8 *dst, const u8 *src, s32 delta);

/* Every tick, once the view point's y exceeds the reference point's by a
 * fade step: each band's colour
 * and the clear colour lose `fade` red and green and gain `fade` blue, and
 * the set moves down 3 pixels per unit of fade. */
void StyleUpdateDecorSet(void) {
    Viewport *viewport;
    s32 height;
    s32 fade;
    u8 rgb[8]; /* MATCHING: 8 bytes, not the 3 it uses, to place pos where retail's stack has it */
    BoxFillPos pos;
    s32 colorOfs;
    s32 i;
    BoxFill **slot;
    BoxFill *band;

    if (sStyleDecorVariant == STYLE_DECOR_NONE) {
        return;
    }
    viewport = sStyleSceneRefs->viewport;
    height = viewport->refView.vp.y - viewport->refView.vr.y;
    fade = (height / STYLE_DECOR_FADE_HEIGHT) * 3;
    if (fade <= 0) {
        return;
    }
    pos = sStyleDecorPos;
    i = 0; /* MATCHING: set here; set in the for header the code differs */
    if (sStyleDecorVariant == STYLE_DECOR_LOWERED) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    slot = sStyleDecorSlots;
    colorOfs = 0;
    pos.y += fade * 3;
    for (; i < STYLE_DECOR_BANDS; i++) {
        AdjustRgbByDelta(rgb, colorOfs + sStyleDecorColors, fade);
        band = *slot;
        band->methods->setColor(band, 1, rgb);
        band = *slot;
        colorOfs += 3;
        band->methods->setPosition(band, &pos);
        pos.y += 3;
        slot++;
    }
    AdjustRgbByDelta(rgb, sStyleClearColor, fade);
    viewport->methods->setClearColor(viewport, (ColorRgb *)rgb);
}

/* dst = src with red and green less `delta`, blue more. */
void AdjustRgbByDelta(u8 *dst, const u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

/* Releases the bands, if StyleBuildDecorSet made them. */
void StyleReleaseDecorSet(void) {
    if (sStyleDecorVariant != STYLE_DECOR_NONE) {
        ReleaseBasicClassArray((BasicClass **)sStyleDecorSlots, ARRAY_COUNT(sStyleDecorSlots));
        sStyleDecorVariant = STYLE_DECOR_NONE;
    }
}

extern StyleEffect *sStyleEffectSlots[];
#ifdef HOST_BUILD
/* Placed by address in the PS1 link; defined here for the host. At most
 * every kind-0 and kind-1 effect of variant 2, and one kind-2 or kind-3. */
StyleEffect *sStyleEffectSlots[STYLE_VARIANT2_EFFECTS + 1];
#endif
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
    refs = sStyleSceneRefs;
    SetStyleEffectSources(sStyleVariant, refs->dreamerTmd, refs->etcTim, refs->viewport);
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
#ifdef HOST_BUILD
        /* An empty slot is flushed too. The PS1 reads and writes low memory
         * for it without faulting; the host skips it. */
        if (sStyleCueSlots[i] == NULL) {
            continue;
        }
#endif
        sStyleCueSlots[i] = FlushStyleCue(sStyleCueSlots[i]);
    }
    if (sStyleGrid != NULL) {
        sStyleGrid = NULL; /* RegisterStyleConfig registers only while this is NULL */
    }
}

#ifdef HOST_BUILD
/* The StyleEffectParams block every effect is built from. The PS1 image has
 * it as nine separate symbols at fixed addresses, sStyleSpawnOffsetX ..
 * sStyleSpawnColors, which the code lays the block over; the host has no
 * such layout, so the names are the block's fields. sStyleSpawnColors is
 * its two colour pointers, color and altColor, addressed from the block. */
static StyleEffectParams sStyleSpawnParams;
#define sStyleSpawnOffsetX (sStyleSpawnParams.offset.x)
#define sStyleSpawnOffsetY (sStyleSpawnParams.offset.y)
#define sStyleSpawnOffsetZ (sStyleSpawnParams.offset.z)
#define sStyleSpawnRotation (sStyleSpawnParams.rotation)
#define sStyleSpawnScale (sStyleSpawnParams.scale)
#define sStyleSpawnModelLayout (sStyleSpawnParams.modelChildLayout)
#define sStyleSpawnTableIndex (sStyleSpawnParams.tableIndex)
#define sStyleSpawnColors \
    ((ColorRgb **)((u8 *)&sStyleSpawnParams + offsetof(StyleEffectParams, color)))
#else
extern Ratio16 *sStyleSpawnScale;
extern s32 sStyleSpawnTableIndex;
/* The first word of the StyleEffectParams block every effect is built from
 * (sStyleSpawnOffsetX .. sStyleSpawnColors, separate symbols in the image). */
extern s32 sStyleSpawnOffsetX;
#endif
extern void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY);
extern void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY);

/* Fills `count` slots with kind-0 effects: a table index and scale for all
 * of them, an offset y (0: each setup picks one; a pick of 4 reads the word
 * after the heights, the first Violence District config), and per slot
 * SetupStyleSpawnParamsRandom, or SetupStyleSpawnParamsDayMod7 on every
 * seventh day. Returns the next slot. */
StyleEffect **StyleFillEffectKind0(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;
    void (*setup)(LongVec3 *, s32);

    sStyleSpawnTableIndex = rand() % 7;
    sStyleSpawnScale = sStyleSpawnScales[(u32)rand() % 5];
    offsetY = (u32)rand() % 5;
    if (offsetY != 0) {
        offsetY = ((s32 *)&sStyleSpawnY)[offsetY]; /* 1..4, so through the block */
    }
    setup = SetupStyleSpawnParamsDayMod7;
    if (sStyleDay % 7 != 0) {
        setup = SetupStyleSpawnParamsRandom;
    }
    for (i = 0; i < count; i++) {
        setup(pos, offsetY);
        *slots = New_StyleEffect(STYLE_EFFECT_MODEL_ROW, (StyleEffectParams *)&sStyleSpawnOffsetX,
                                 (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

/* Fills `count` slots with kind-1 effects: sStyleKind1Scale, offset y
 * sStyleSpawnY.choices[2]. */
StyleEffect **StyleFillEffectKind1(StyleEffect **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;

    offsetY = sStyleSpawnY.choices[2];
    sStyleSpawnScale = sStyleKind1Scale[0];
    for (i = 0; i < count; i++) {
        SetupStyleSpawnParamsRandom(pos, offsetY);
        *slots = New_StyleEffect(STYLE_EFFECT_MODEL, (StyleEffectParams *)&sStyleSpawnOffsetX,
                                 (SceneNode *)sStyleGrid, pos);
        slots++;
    }
    return slots;
}

#ifndef HOST_BUILD
extern ColorRgb *sStyleSpawnColors[];
extern Ratio16 *sStyleSpawnRotation;
extern s32 sStyleSpawnOffsetY;
extern s32 sStyleSpawnOffsetZ;
#endif

/** @brief sStyleSpawnRotation seen as a one-field struct, through which
 * StyleFillEffectKind3 stores the effect's rotation. */
/* MATCHING: the rotation store goes through a one-field struct, so the
 * sStyleGrid read can come before it; a plain pointer store keeps it after. */
typedef struct PtrBoxK3 {
    Ratio16 *p; /**< +0x000 the spawn rotation */
} PtrBoxK3;

/* Appends one kind-3 effect. With decor variant active and band colours B
 * its offset and colour are fixed; otherwise its z offset is folded to
 * -15..0 cells and its colour is random. */
StyleEffect **StyleFillEffectKind3(StyleEffect **slots, LongVec3 *pos) {
    s32 *offsetZ;
    PtrBoxK3 *rotation;

    SetupStyleSpawnParamsRandom(pos, sStyleSpawnY.choices[2]);
    if (sStyleDecorVariant != STYLE_DECOR_NONE && sStyleDecorColors == sStyleDecorColorsB) {
        sStyleSpawnOffsetX = -22 * STAGE_CELL_SIZE;
        sStyleSpawnOffsetY = -4 * STAGE_CELL_SIZE;
        sStyleSpawnOffsetZ = 0;
        sStyleSpawnColors[0] = (ColorRgb *)sStyleKind3Colors[1];
    } else {
        offsetZ = &sStyleSpawnOffsetZ;
        if (*offsetZ > 0) {
            *offsetZ = -*offsetZ;
        }
        if (*offsetZ < -15 * STAGE_CELL_SIZE) {
            *offsetZ = -15 * STAGE_CELL_SIZE;
        }
        sStyleSpawnColors[0] = (ColorRgb *)sStyleKind3Colors[(u32)rand() % 3];
    }
    rotation = (PtrBoxK3 *)&sStyleSpawnRotation;
    rotation->p = sStyleSpawnRotations[0];
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots =
        New_StyleEffect(STYLE_EFFECT_JITTER_SPRITES,
                        (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
                        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

/** @brief One entry of sStyleSpawnColors seen as a one-field struct, through
 * which StyleFillEffectKind2 stores the effect's two colours. */
/* MATCHING: the first colour store goes through a one-field struct, as
 * PtrBoxK3's does, so the sStyleDay read can come before it. */
typedef struct ColorPtrBoxK2 {
    ColorRgb *v; /**< +0x000 a colour: first an RGB triple, then sStyleConfig.clearColor or NULL */
} ColorPtrBoxK2;

/* Appends one kind-2 effect with a random colour and, except on every
 * twentieth day, sStyleConfig.clearColor as its alternate colour. */
StyleEffect **StyleFillEffectKind2(StyleEffect **slots, LongVec3 *pos) {
    s32 r;
    s32 day;
    ColorRgb *altColor;
    ColorPtrBoxK2 *color;
    Ratio16 **rotation;
    s32 offsetY;

    r = rand();
    color = (ColorPtrBoxK2 *)sStyleSpawnColors;
    color->v = (ColorRgb *)sStyleKind2Colors[(u32)r % 3];
    color++;
    day = (sStyleDay / 20) * 20; /* MATCHING: not `sStyleDay % 20`, which compiles differently */
    if (sStyleDay != day) {
        altColor = sStyleConfig.clearColor;
    } else {
        altColor = NULL;
    }
    /* MATCHING: the height is read before the colour is stored */
    offsetY = sStyleSpawnY.choices[2];
    color->v = altColor;
    SetupStyleSpawnParamsRandom(pos, offsetY);
    rotation = &sStyleSpawnRotation;
    *rotation = sStyleSpawnRotations[0];
    sStyleSpawnTableIndex = rand() % 6;
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots =
        New_StyleEffect(STYLE_EFFECT_SPRITES,
                        (StyleEffectParams *)((u8 *)rotation - offsetof(StyleEffectParams, rotation)),
                        (SceneNode *)sStyleGrid, pos);
    slots++;
    return slots;
}

#ifndef HOST_BUILD
extern s32 sStyleSpawnModelLayout;
#endif

/* Randomises the spawn parameters: offset y (offsetY, or a random choice
 * when 0), x and z offsets of 0..22 steps of 2048 either side, a rotation
 * and a model layout. `pos` is unused. */
/* MATCHING: sStyleSpawnOffsetX is declared a scalar, not an array. */
void SetupStyleSpawnParamsRandom(LongVec3 *pos, s32 offsetY) {
    if (offsetY == 0) {
        offsetY = sStyleSpawnY.choices[rand() & 3];
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

/* The every-seventh-day setup: fixed offset y, x of 0..19 cells, z
 * by day % 3 (20, -20 or 1 cell), then the same rotation and layout
 * picks as SetupStyleSpawnParamsRandom. Both parameters are unused; it has
 * that function's signature because StyleFillEffectKind0 calls either
 * through one pointer. */
/* MATCHING: each rand() is used inline; one local for all three is a word longer per call. */
void SetupStyleSpawnParamsDayMod7(LongVec3 *pos, s32 offsetY) {
    s32 dayMod3;

    rand();
    sStyleSpawnOffsetY = sStyleSpawnY.choices[1];
    sStyleSpawnOffsetX = (rand() % 20) << STAGE_CELL_SHIFT;
    dayMod3 = sStyleDay % 3;
    sStyleSpawnOffsetZ = 20 * STAGE_CELL_SIZE;
    if (dayMod3 == 1) {
        sStyleSpawnOffsetZ = -20 * STAGE_CELL_SIZE;
    } else if (dayMod3 == 2) {
        sStyleSpawnOffsetZ = STAGE_CELL_SIZE;
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
        InitSoundCueSet(sStyleSceneRefs->sound, &slot->cueSet, entry->cue, slot,
                        sStyleCueCallbacks[entry->cue]);
        if (entry->cue == *lastCue) {
            *lastCue = -entry->cue;
        }
        entry->cue = -entry->cue;
        return slot;
    }
    return 0;
}

/* From sStyleCueRecordIndex on, the first free record of the stage's list
 * whose X+Z distance from the target is under its cue's distance; each
 * record looked at advances the index, so the next slot's search this tick
 * goes on from there.
 * Writes the record's world position and distance. */
StyleCueRecord *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target) {
    s32 j, remaining;
    StyleCueRecord *records;
    StyleCueRecord *entry;
    CellKeyDesc buf;
    s32 dx, dz, dist;
    StageMap *grid;

    if (target != 0) {
        records = sStyleCueRecordLists[sStyleStage];
        remaining = sStyleCueRecordCounts[sStyleStage] - sStyleCueRecordIndex;
        entry = (StyleCueRecord *)(sStyleCueRecordIndex * 8 + (intptr_t)records); /* MATCHING: operand order */
        for (j = 0; j < remaining; j++, entry++) {
            sStyleCueRecordIndex++;
            if (entry->cue > 0) {
                buf.key = entry->key;
                buf.offset = sStyleCueOffsets[entry->offsetIndex];
                grid = sStyleGrid;
                grid->methods->computeCellOffsets(grid, pos, &buf);
                dx = pos->x - target->x;
                if (dx < 0) {
                    dx = ~dx + 1; /* MATCHING: not -dx; retail complements and adds one */
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
    }
    return 0;
}

/* Stops the slot's cue and frees its record. Returns NULL for the slot. */
StyleCueSlot *FlushStyleCue(StyleCueSlot *slot) {
    FlushSoundCueSet(sStyleSceneRefs->sound, &slot->cueSet);
    slot->entry->cue = -slot->entry->cue;
    return 0;
}

extern s32 IsStyleCueNear(StyleCueSlot *slot, LongVec3 *target);

/* One service pass of the slot's cue while the target is in range; 0 otherwise. */
s32 ServiceStyleCueIfNear(StyleCueSlot *slot, LongVec3 *target, void *unused) {
    if (IsStyleCueNear(slot, target) != 0) {
        ServiceSoundCueSet(sStyleSceneRefs->sound, &slot->cueSet);
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
        dx = ~dx + 1; /* MATCHING: not -dx; retail complements and adds one */
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
        dist = 1; /* MATCHING: not `return 1`, which folds into the compare's result */
        return dist;
    }
    return 0;
}

extern void ApplyStyleDecorationIfSet(void); /* above */
extern void StyleBuildDecorSet(void);
extern void StyleUpdateDecorSet(void);
extern void StyleScrollVramStrips(void);
extern StyleCueSlot sStyleCueSlotPool[];
#ifdef HOST_BUILD
/* Placed by address in the PS1 link; defined here for the host. One per
 * sStyleCueSlots entry. */
StyleCueSlot sStyleCueSlotPool[ARRAY_COUNT(sStyleCueSlots)];
#endif
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
        sStyleGrid->methods->computeCellOffsets(sStyleGrid, target, cell);
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
            /* MATCHING: a no-op pair; without it target and i are kept differently */
            i++;
            i--;
        } else {
            sStyleCueSlots[i] = TryStartStyleCue(&sStyleCueSlotPool[i], &lastCue, target, unused);
        }
    }
    return lastCue;
}

/* One step of RotateVramRectRight's one-column VRAM rotation: Kyoto on the
 * strip at y 496, The Natural World to Violence District on the one at y 504. */
void StyleScrollVramStrips(void) {
    DrawRect *rect;
    DrawPoint *scratch;
    s32 count;

    if (sStyleStage == STAGE_KYOTO) {
        rect = &sStyleStripRectA;
        scratch = (DrawPoint *)&sStyleStripScratchA;
        count = 1; /* MATCHING: a local set in each branch, not a literal argument */
    } else if (sStyleStage >= STAGE_NATURAL_WORLD && sStyleStage <= STAGE_VIOLENCE_DISTRICT) {
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
