/*
 * class_3bb8c_n -- the style layer's per-scene objects: what TickStyle
 * builds on a scene's first tick, updates on every tick, and StyleTeardown
 * releases.
 *
 * RegisterStyleConfig (class_3bb8c_m.c), called by ObjM__InitStyleAndWorld
 * and a no-op until StyleTeardown clears gStyleGrid, sets the state read here: gStyleGrid (the
 * scene's Class866E8), gStyleStage (ObjM's stage), gStyleSceneRefs (ObjM's
 * sound, resources and viewport; StyleSceneRefs below) and gStyleDay (the
 * DreamSys day). ApplyStyleConfig then takes the stage's fixed config or,
 * with none, PickStyleFallbackConfig's: a variant (gStyleVariant, 0..3) and
 * a config record picked from day + stage.
 *
 * What TickStyle keeps, each built on the first tick:
 *  - the decoration box, gStyleDecorObj, when the config has a colour for
 *    it (class_3bb8c_m builds it, StyleFlushDecoration releases it);
 *  - the decor set: STYLE_DECOR_BANDS BoxFill bands (gStyleDecorSlots)
 *    coloured from gStyleDecorColors and attached under the viewport's sub
 *    handle; every tick StyleUpdateDecorSet shifts their colours, their
 *    position and the viewport's clear colour by the view point's y offset
 *    from its reference point;
 *  - the effect slots: Class876FC objects of kinds 0..3 (gStyleEffectSlots)
 *    built from one parameter block, gStyleSpawnOffsetX..gStyleSpawnColors,
 *    that StyleFillEffectKindN and SetupStyleSpawnParamsA/B fill in; each
 *    tick updates them with the target position;
 *  - two positional sound cues (gStyleCueSlots, in gStyleCueSlotPool): a
 *    free slot claims the next record of the stage's cue list that lies
 *    within its cue's distance of the target and starts the record's
 *    SoundCueSet callback (gStyleCueCallbacks, class_3bb8c_r.c); a claimed
 *    slot is serviced while the target stays in range and flushed when it
 *    leaves.
 * StyleScrollVramStrips also rotates a VRAM strip one column per tick on
 * stages 2 to 5.
 *
 * The target is the grid's target cell (ObjM__TickStyle passes
 * getTargetDescriptor) turned into a world position. What the style layer
 * is in the game -- what a variant, an effect kind or a cue stands for -- is
 * not established; the names describe mechanics. Evidence and tiers are in
 * each function's match report, `## Naming`.
 */

#include "common.h"
#include "Actor.h"
#include "Class876FC.h"
#include "BoxFill.h"
#include "Viewport.h"
#include "Class866E8.h"
#include "SoundCueSet.h"

/* The decoration set: this many BoxFill bands, stacked 3 pixels apart. */
#define STYLE_DECOR_BANDS 18

/* Every band's draw priority: the largest value BoxFill's default 13-bit
 * priority mask admits (BoxFill__Reset calls setMask(13)). */
#define STYLE_DECOR_PRI 0x1FFF

/* How far down (pixels) decor variant 2 draws the set. */
#define STYLE_DECOR_VARIANT2_DROP 30

/* StyleUpdateDecorSet's fade step: the viewport's view-point y less its
 * reference-point y, per step. */
#define STYLE_DECOR_FADE_HEIGHT 600

/* Kind-0 plus kind-1 effects built for style variant 2. */
#define STYLE_VARIANT2_EFFECTS 16

/* The palette entry that, as a variant-0 config's decor colour, selects
 * gStyleDecorColorsB instead of gStyleDecorColorsA (PickStyleFallbackConfig). */
#define STYLE_DECOR_B_PALETTE_INDEX 18

/* What gStyleSceneRefs points at: ObjM's +0x06C..+0x07B block
 * (ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it; include/ObjM.h). */
typedef struct StyleSceneRefs {
    void *sound;        /* +0x000, ObjM::ctorSound: the sound object the cue functions take first */
    void *dreamerTmd;   /* +0x004, ObjM::dreamerTmd */
    void *etcTim;       /* +0x008, ObjM::etcTim */
    Viewport *viewport; /* +0x00C, ObjM::cachedViewport */
} StyleSceneRefs;

extern const u8 *gStyleDecorColor;
extern s32 gStyleDecorObj; /* a BoxFill; class_3bb8c_m.c declares it s32 too */

/* Releases the decoration box, if ApplyStyleDecorationIfSet made one. */
void StyleFlushDecoration(void) {
    if (gStyleDecorColor != 0) {
        ((BoxFill *)gStyleDecorObj)->methods->release((BoxFill *)gStyleDecorObj);
        gStyleDecorColor = 0;
    }
}

extern s32 gStyleDay;
extern s32 gStyleStage;
extern s8 gStyleVariantPicks[];
extern s32 gStyleVariant;
extern s8 gStyleVariantConfigCounts[];
extern s32 gStyleConfigIndex;
extern s8 *gStyleVariantConfigs[];
extern const u8 *gStyleClearColor;
extern u8 gStyleDecorColorsB[];
extern u8 gStylePalette[][3];
extern const u8 *gStyleDecorColors;
extern u8 gStyleDecorColorsA[];
extern s32 gStyleDecorVariant;

/* The config for a stage without a fixed one: the variant from
 * gStyleVariantPicks[(day + stage) & 0xF], then record (day + stage) % count
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

    seed = gStyleDay + gStyleStage;
    variant = gStyleVariantPicks[seed & 0xF];
    gStyleVariant = variant;
    count = gStyleVariantConfigCounts[variant];
    index = seed % count;
    gStyleConfigIndex = index;
    config = gStyleVariantConfigs[variant] + index * 4;
    if (variant == 0) {
        clearIndex = config[3];
        gStyleClearColor = gStylePalette[clearIndex];
        decorIndex = config[2];
        decorColors = gStyleDecorColorsB;
        if (decorIndex != STYLE_DECOR_B_PALETTE_INDEX) {
            decorColors = gStyleDecorColorsA;
        }
        gStyleDecorColors = decorColors;
        if (index < 4) {
            gStyleDecorVariant = 1;
        } else if (index < 6) {
            gStyleDecorVariant = 2;
        }
    }
    return config;
}

extern s32 gStyleDecorPosX;
extern s32 gStyleDecorPosY;
extern s32 gStyleDecorSizeW;
extern s32 gStyleDecorSizeH;
extern BoxFill *gStyleDecorSlots[STYLE_DECOR_BANDS];
extern s32 gStyleSceneRefs; /* a StyleSceneRefs *; class_3bb8c_m.c declares it s32 too */

/* gStyleDecorPosX/Y and gStyleDecorSizeW/H are adjacent word pairs.
 * MATCHING: copied whole, never field by field (a BLKmode copy makes cse
 * drop cached memory values; scalar copies lose retail's reloads). */
typedef struct PairXY PairXY;

struct PairXY {
    s32 x; /* +0x000 */
    s32 y; /* +0x004 */
};

/* Builds the bands: band 0 in gStyleDecorColors' first colour, bands 1..17
 * attached under it, each 3 pixels lower and 7 shorter than the one before;
 * band 0 then goes under the viewport's sub handle. */
void StyleBuildDecorSet(void) {
    PairXY pos;
    PairXY size;
    s32 i;
    BoxFill *band;
    Viewport *viewport;
    SceneNode *parent;

    if (gStyleDecorVariant == 0) {
        return;
    }
    pos = *(PairXY *)&gStyleDecorPosX;
    if (gStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    size = *(PairXY *)&gStyleDecorSizeW;
    gStyleDecorSlots[0] = New_BoxFill(&size, (void *)gStyleDecorColors, STYLE_DECOR_PRI);
    for (i = 1; i < STYLE_DECOR_BANDS; i++) {
        band = New_BoxFill(&size, (void *)(gStyleDecorColors + i * 3), STYLE_DECOR_PRI);
        gStyleDecorSlots[i] = band;
        ((BoxFillAttachToParentFn)band->methods->attachToParent)(
            band, (SceneNode *)gStyleDecorSlots[0], (Pair32E99C *)&pos);
        pos.y += 3;
        size.y -= 7;
    }

    viewport = ((StyleSceneRefs *)gStyleSceneRefs)->viewport;
    parent = viewport->methods->getSubHandle(viewport);
    ((BoxFillAttachToParentFn)gStyleDecorSlots[0]->methods->attachToParent)(
        gStyleDecorSlots[0], parent, (Pair32E99C *)&pos);
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
    PairXY pos;
    s32 colorOfs;
    s32 i;
    BoxFill **slot;
    BoxFill *band;

    if (gStyleDecorVariant == 0) {
        return;
    }
    viewport = ((StyleSceneRefs *)gStyleSceneRefs)->viewport;
    height = viewport->refView.vp.y - viewport->refView.vr.y;
    fade = (height / STYLE_DECOR_FADE_HEIGHT) * 3;
    if (fade <= 0) {
        return;
    }
    pos = *(PairXY *)&gStyleDecorPosX;
    i = 0;
    if (gStyleDecorVariant == 2) {
        pos.y += STYLE_DECOR_VARIANT2_DROP;
    }
    slot = gStyleDecorSlots;
    colorOfs = 0;
    pos.y += fade * 3;
    do {
        AdjustRgbByDelta(rgb, (u8 *)(colorOfs + gStyleDecorColors), fade);
        band = *slot;
        band->methods->setColor(band, 1, rgb);
        band = *slot;
        i++;
        colorOfs += 3;
        band->methods->setPosition(band, (Pair32E99C *)&pos);
        pos.y += 3;
        slot++;
    } while (i < STYLE_DECOR_BANDS);
    AdjustRgbByDelta(rgb, (u8 *)gStyleClearColor, fade);
    viewport->methods->setClearColor(viewport, (ViewportRgb *)rgb);
}

/* dst = src with red and green less `delta`, blue more. */
void AdjustRgbByDelta(u8 *dst, u8 *src, s32 delta) {
    dst[0] = src[0] - delta;
    dst[1] = src[1] - delta;
    dst[2] = src[2] + delta;
}

extern void ReleaseBasicClassArray(void **array, s32 count);
extern s32 gStyleDecorVariant;
extern BoxFill *gStyleDecorSlots[STYLE_DECOR_BANDS];

/* Releases the bands, if StyleBuildDecorSet made them. */
void StyleReleaseDecorSet(void) {
    if (gStyleDecorVariant != 0) {
        ReleaseBasicClassArray((void **)gStyleDecorSlots, ARRAY_COUNT(gStyleDecorSlots));
        gStyleDecorVariant = 0;
    }
}

extern s32 gStyleVariant;
extern s32 gStyleSceneRefs;
extern s32 rand(void);
extern s8 gStyleKind0Counts[];
extern s32 gStyleEffectSlotCount;
extern Class876FC *gStyleEffectSlots[];
extern Class876FC **StyleFillEffectKind0(Class876FC **slots, s32 count, LongVec3 *pos);
extern Class876FC **StyleFillEffectKind1(Class876FC **slots, s32 count, LongVec3 *pos);
extern Class876FC **StyleFillEffectKind3(Class876FC **slots, LongVec3 *pos);
extern Class876FC **StyleFillEffectKind2(Class876FC **slots, LongVec3 *pos);

/* Hands the variant and ObjM's resources to Actor__func_56f5c, then builds
 * the effect slots for the variant: gStyleKind0Counts' pick of
 * kind 0, kind 1 up to STYLE_VARIANT2_EFFECTS for variant 2, then one kind-3
 * (variant 0) or kind-2 (variant 2). */
void StyleBuildEffectSlots(LongVec3 *pos) {
    StyleSceneRefs *refs;
    s32 kind0Count;
    s32 kind1Count;
    Class876FC **next;

    if (gStyleVariant < 0) {
        return;
    }
    refs = (StyleSceneRefs *)gStyleSceneRefs;
    Actor__func_56f5c(gStyleVariant, (Actor *)refs->dreamerTmd, (s32)refs->etcTim, (s32)refs->viewport);
    kind0Count = gStyleKind0Counts[rand() & 3];
    kind1Count = (gStyleVariant == 2) ? STYLE_VARIANT2_EFFECTS - kind0Count : 0;
    gStyleEffectSlotCount = kind0Count + kind1Count;
    next = StyleFillEffectKind0(gStyleEffectSlots, kind0Count, pos);
    next = StyleFillEffectKind1(next, kind1Count, pos);
    if (gStyleVariant == 0) {
        StyleFillEffectKind3(next, pos);
    } else if (gStyleVariant == 2) {
        StyleFillEffectKind2(next, pos);
    } else {
        return;
    }
    gStyleEffectSlotCount = gStyleEffectSlotCount + 1;
}

extern s32 gStyleVariant;
extern s32 gStyleEffectSlotCount;

/* Each slot's +0x0EC is Class876FC__Update, called with the position
 * (include/Class876FC.h: the slot keeps Actor's setPendingExtra type). */
void StyleUpdateEffectSlots(LongVec3 *pos) {
    s32 i;
    Class876FC *slot;

    if (gStyleVariant < 0) {
        return;
    }
    for (i = 0; i < gStyleEffectSlotCount; i++) {
        slot = gStyleEffectSlots[i];
        ((Class876FCUpdateFn)slot->methods->setPendingExtra)(slot, pos);
    }
}

extern s32 gStyleVariant;
extern s32 gStyleEffectSlotCount;

/* Releases the effect slots, if StyleBuildEffectSlots ran. */
void StyleReleaseEffectSlots(void) {
    if (gStyleVariant >= 0) {
        ReleaseBasicClassArray((void **)gStyleEffectSlots, gStyleEffectSlotCount);
    }
}

/* A cue record's view here: `cue` is its cue index (the gStyleCueCallbacks
 * and gStyleCueDistanceTable row, InitSoundCueSet's tag), negated while a
 * slot holds the record. EntrySlot, below, is the whole 8-byte record. */
typedef struct StyleCueEntryView StyleCueEntryView;

struct StyleCueEntryView {
    u8 pad0[0x6];
    s8 cue; /* +0x006 */
};

/* One of the two positional cues: the record it holds, the record's world
 * position, the last distance to the target, and its sound cue. */
typedef struct StyleCueSlot StyleCueSlot;

struct StyleCueSlot {
    StyleCueEntryView *entry; /* +0x000 */
    LongVec3 pos;             /* +0x004 */
    s32 lastDist;             /* +0x010 */
    SoundCueSet cueSet;       /* +0x014 */
}; /* 0x68 bytes */

extern StyleCueSlot *FlushStyleCue(StyleCueSlot *slot);

extern s32 gStyleGrid; /* a Class866E8; class_3bb8c_m.c declares it s32 too */
extern StyleCueSlot *gStyleCueSlots[2];

/* Releases everything TickStyle built and unregisters the scene. */
void StyleTeardown(void) {
    s32 i;

    StyleFlushDecoration();
    StyleReleaseDecorSet();
    StyleReleaseEffectSlots();
    for (i = 0; i < ARRAY_COUNT(gStyleCueSlots); i++) {
        gStyleCueSlots[i] = FlushStyleCue(gStyleCueSlots[i]);
    }
    if (gStyleGrid != 0) {
        gStyleGrid = 0; /* RegisterStyleConfig registers only while this is 0 */
    }
}

extern Ratio16 gStyleSpawnScales[][3];
extern s32 gStyleSpawnYChoices[];
extern Ratio16 *gStyleSpawnScale;
extern s32 gStyleSpawnTableIndex;
/* The first word of the Class876FCParams block every effect is built from
 * (gStyleSpawnOffsetX .. gStyleSpawnColors, separate symbols in the image). */
extern s32 gStyleSpawnOffsetX;
extern void SetupStyleSpawnParamsA(LongVec3 *pos, s32 offsetY);
extern void SetupStyleSpawnParamsB(LongVec3 *pos, s32 offsetY);

/* Fills `count` slots with kind-0 effects: a table index and scale for all
 * of them, an offset y (0: each setup picks one; a pick of 4 reads the word
 * after gStyleSpawnYChoices, as retail does), and per slot
 * SetupStyleSpawnParamsA, or B on every seventh day. Returns the next slot. */
Class876FC **StyleFillEffectKind0(Class876FC **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;
    void (*setup)(LongVec3 *, s32);

    gStyleSpawnTableIndex = rand() % 7;
    gStyleSpawnScale = gStyleSpawnScales[(u32)rand() % 5];
    offsetY = (u32)rand() % 5;
    if (offsetY != 0) {
        offsetY = gStyleSpawnYChoices[offsetY];
    }
    setup = SetupStyleSpawnParamsB;
    if (gStyleDay % 7 != 0) {
        setup = SetupStyleSpawnParamsA;
    }
    for (i = 0; i < count; i++) {
        setup(pos, offsetY);
        *slots =
            New_Class876FC(0, (Class876FCParams *)&gStyleSpawnOffsetX, (SceneNode *)gStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 gStyleSpawnYChoice2;
extern Ratio16 gStyleKind1Scale[];

/* Fills `count` slots with kind-1 effects: gStyleKind1Scale, offset y
 * gStyleSpawnYChoice2. */
Class876FC **StyleFillEffectKind1(Class876FC **slots, s32 count, LongVec3 *pos) {
    s32 i;
    s32 offsetY;

    offsetY = gStyleSpawnYChoice2;
    gStyleSpawnScale = gStyleKind1Scale;
    for (i = 0; i < count; i++) {
        SetupStyleSpawnParamsA(pos, offsetY);
        *slots =
            New_Class876FC(1, (Class876FCParams *)&gStyleSpawnOffsetX, (SceneNode *)gStyleGrid, pos);
        slots++;
    }
    return slots;
}

extern s32 gStyleSpawnYChoice2;
extern void SetupStyleSpawnParamsA(LongVec3 *pos, s32 offsetY);
extern s32 gStyleSpawnColors[];
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnOffsetY;
extern s32 gStyleSpawnOffsetZ;
extern u8 gStyleKind3Colors[][3];

/* MATCHING: the rotation store goes through a one-field struct, so the
 * gStyleGrid load may schedule above it (a plain pointer store blocks it). */
typedef struct PtrBoxK3 {
    Ratio16 *p; /* +0x000 */
} PtrBoxK3;

/* Appends one kind-3 effect. With decor variant active and band colours B
 * its offset and colour are fixed; otherwise its z offset is folded to
 * -30720..0 and its colour is random. */
Class876FC **StyleFillEffectKind3(Class876FC **slots, LongVec3 *pos) {
    s32 *offsetZ;
    PtrBoxK3 *rotation;

    SetupStyleSpawnParamsA(pos, gStyleSpawnYChoice2);
    if (gStyleDecorVariant != 0 && gStyleDecorColors == gStyleDecorColorsB) {
        gStyleSpawnOffsetX = -45056;
        gStyleSpawnOffsetY = -8192;
        gStyleSpawnOffsetZ = 0;
        gStyleSpawnColors[0] = (s32)gStyleKind3Colors[1];
    } else {
        offsetZ = &gStyleSpawnOffsetZ;
        if (*offsetZ > 0) {
            *offsetZ = -*offsetZ;
        }
        if (*offsetZ < -30720) {
            *offsetZ = -30720;
        }
        gStyleSpawnColors[0] = (s32)gStyleKind3Colors[(u32)rand() % 3];
    }
    rotation = (PtrBoxK3 *)&gStyleSpawnRotation;
    rotation->p = gStyleSpawnRotations[0];
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots =
        New_Class876FC(3, (Class876FCParams *)((u8 *)rotation - offsetof(Class876FCParams, rotation)),
                       (SceneNode *)gStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 gStyleKind2AltColor;
extern u8 gStyleKind2Colors[][3];
extern s32 gStyleSpawnColors[];
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnTableIndex;

/* MATCHING: the first colour store goes through a one-field struct, as
 * PtrBoxK3's does, so the gStyleDay load may schedule above it. */
typedef struct S32BoxK2 {
    s32 v; /* +0x000 */
} S32BoxK2;

/* Appends one kind-2 effect with a random colour and, except on every
 * twentieth day, gStyleKind2AltColor as its alternate colour. */
Class876FC **StyleFillEffectKind2(Class876FC **slots, LongVec3 *pos) {
    s32 r;
    s32 altColor;
    S32BoxK2 *color;
    Ratio16 **rotation;

    r = rand();
    color = (S32BoxK2 *)gStyleSpawnColors;
    color->v = (s32)gStyleKind2Colors[(u32)r % 3];
    color++;
    altColor = (gStyleDay / 20) * 20; /* MATCHING: not `% 20`, which jump.c folds */
    if (gStyleDay != altColor) {
        altColor = gStyleKind2AltColor;
    } else {
        altColor = 0;
    }
    color->v = altColor;
    SetupStyleSpawnParamsA(pos, gStyleSpawnYChoice2);
    rotation = &gStyleSpawnRotation;
    *rotation = gStyleSpawnRotations[0];
    gStyleSpawnTableIndex = rand() % 6;
    /* MATCHING: the block's address is taken back from its rotation member */
    *slots =
        New_Class876FC(2, (Class876FCParams *)((u8 *)rotation - offsetof(Class876FCParams, rotation)),
                       (SceneNode *)gStyleGrid, pos);
    slots++;
    return slots;
}

extern s32 gStyleSpawnOffsetY;
extern s32 gStyleSpawnOffsetZ;
extern Ratio16 *gStyleSpawnRotation;
extern Ratio16 gStyleSpawnRotations[][3];
extern s32 gStyleSpawnModelLayout;

/* Randomises the spawn parameters: offset y (offsetY, or a random choice
 * when 0), x and z offsets of 0..22 steps of 2048 either side, a rotation
 * and a model layout. `pos` is unused.
 * MATCHING: gStyleSpawnOffsetX is declared a scalar, not an array (an array
 * decay is kept in a saved register across the rand() calls). */
void SetupStyleSpawnParamsA(LongVec3 *pos, s32 offsetY) {
    if (offsetY == 0) {
        offsetY = gStyleSpawnYChoices[rand() & 3];
    }
    gStyleSpawnOffsetY = offsetY;
    gStyleSpawnOffsetX = (rand() % 23) << 11;
    if (rand() & 1) {
        gStyleSpawnOffsetX = -gStyleSpawnOffsetX;
    }
    gStyleSpawnOffsetZ = (rand() % 23) << 11;
    if (rand() & 1) {
        gStyleSpawnOffsetZ = -gStyleSpawnOffsetZ;
    }
    gStyleSpawnRotation = gStyleSpawnRotations[(u32)rand() % 7];
    gStyleSpawnModelLayout = rand() % 5;
}

extern s32 gStyleSpawnYChoice1;
extern s32 gStyleSpawnModelLayout;

/* The every-seventh-day setup: fixed offset y, x of 0..19 steps of 2048, z
 * by day % 3 (40960, -40960, 2048), then the same rotation and layout
 * picks as A. Both parameters are unused; it has A's signature because
 * StyleFillEffectKind0 calls either through one pointer.
 * MATCHING: each rand() is used inline; one local for all three adds a move
 * after every call. */
void SetupStyleSpawnParamsB(LongVec3 *pos, s32 offsetY) {
    s32 dayMod3;

    rand();
    gStyleSpawnOffsetY = gStyleSpawnYChoice1;
    gStyleSpawnOffsetX = (rand() % 20) << 11;
    dayMod3 = gStyleDay % 3;
    gStyleSpawnOffsetZ = 40960;
    if (dayMod3 == 1) {
        gStyleSpawnOffsetZ = -40960;
    } else if (dayMod3 == 2) {
        gStyleSpawnOffsetZ = 2048;
    }
    gStyleSpawnRotation = gStyleSpawnRotations[(u32)rand() % 7];
    gStyleSpawnModelLayout = rand() % 5;
}

extern s32 gStyleSceneRefs;
extern void *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target);
extern SoundCueCallbackFn gStyleCueCallbacks[];
extern s32 InitSoundCueSet(void *sound, SoundCueSet *set, s32 tag, void *owner,
                           SoundCueCallbackFn callback);

/* Claims the next record in range for `slot` and starts its cue. A started
 * cue equal to *lastCue is reported back negated. Returns the slot, or NULL. */
StyleCueSlot *TryStartStyleCue(StyleCueSlot *slot, s32 *lastCue, LongVec3 *target, void *unused) {
    StyleCueEntryView *entry;

    entry = (StyleCueEntryView *)FindNextStyleCueInRange(&slot->pos, &slot->lastDist, target);
    if (entry != 0) {
        slot->entry = entry;
        InitSoundCueSet(((StyleSceneRefs *)gStyleSceneRefs)->sound, &slot->cueSet, entry->cue, slot,
                        gStyleCueCallbacks[entry->cue]);
        if (entry->cue == *lastCue) {
            *lastCue = -entry->cue;
        }
        entry->cue = -entry->cue;
        return slot;
    }
    return 0;
}

extern s32 gStyleStage;
extern s32 gStyleCueRecordIndex;
extern u8 *gStyleCueRecordLists[];
extern u8 gStyleCueRecordCounts[];
extern s32 gStyleCueDistanceTable[];

/* The cell-key halves: a record's four cell bytes and gStyleCueOffsets' s16
 * x/y/z, copied whole into a 10-byte cell key (Class866E8's Descriptor10
 * shape) for computeCellOffsets. */
typedef struct Pos4 Pos4;

struct Pos4 {
    s16 hi;
    s16 lo;
};

typedef struct TabEntry TabEntry;

struct TabEntry {
    Pos4 head;
    s16 tail;
};

extern TabEntry gStyleCueOffsets[];

typedef struct EntrySlot EntrySlot;

struct EntrySlot {
    Pos4 pos;       /* +0x0 */
    u8 offsetIndex; /* +0x4 */
    u8 pad5;        /* +0x5 */
    s8 cue;         /* +0x6 */
    u8 pad7;        /* +0x7 */
};

typedef struct LocalBuf LocalBuf;

struct LocalBuf {
    Pos4 pos;
    TabEntry tab;
};

/* From gStyleCueRecordIndex on, the first free record of the stage's list
 * whose X+Z distance from the target is under its cue's distance; each
 * record looked at advances the index, so the next slot's search this tick
 * goes on from there.
 * Writes the record's world position and distance. */
void *FindNextStyleCueInRange(LongVec3 *pos, s32 *outDist, LongVec3 *target) {
    s32 j, remaining;
    u8 *records;
    EntrySlot *entry;
    LocalBuf buf;
    s32 dx, dz, dist;
    Class866E8 *grid;

    if (target == 0) {
        goto fail;
    }
    records = gStyleCueRecordLists[gStyleStage];
    remaining = gStyleCueRecordCounts[gStyleStage] - gStyleCueRecordIndex;
    entry = (EntrySlot *)(gStyleCueRecordIndex * 8 + (s32)records); /* MATCHING: operand order */
    for (j = 0; j < remaining; j++, entry++) {
        gStyleCueRecordIndex++;
        if (entry->cue > 0) {
            buf.pos = entry->pos;
            buf.tab = gStyleCueOffsets[entry->offsetIndex];
            grid = (Class866E8 *)gStyleGrid;
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
            if (dist < gStyleCueDistanceTable[entry->cue]) {
                return entry;
            }
        }
    }
fail:
    return 0;
}

extern s32 gStyleSceneRefs;
extern void FlushSoundCueSet(void *sound, SoundCueSet *set);

/* Stops the slot's cue and frees its record. Returns NULL for the slot. */
StyleCueSlot *FlushStyleCue(StyleCueSlot *slot) {
    FlushSoundCueSet(((StyleSceneRefs *)gStyleSceneRefs)->sound, &slot->cueSet);
    slot->entry->cue = -slot->entry->cue;
    return 0;
}

extern s32 IsStyleCueNear(StyleCueSlot *slot, LongVec3 *target);
extern void ServiceSoundCueSet(void *sound, SoundCueSet *set);

/* One service pass of the slot's cue while the target is in range; 0 otherwise. */
s32 ServiceStyleCueIfNear(StyleCueSlot *slot, LongVec3 *target, void *unused) {
    if (IsStyleCueNear(slot, target) != 0) {
        ServiceSoundCueSet(((StyleSceneRefs *)gStyleSceneRefs)->sound, &slot->cueSet);
        return 1;
    }
    return 0;
}

extern s32 gStyleCueDistanceTable[];

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
    if (dist < gStyleCueDistanceTable[-cue]) {
        dist = 1; /* MATCHING: not `return 1` (jump.c folds that to slt) */
        return dist;
    }
    return 0;
}

extern void ApplyStyleDecorationIfSet(void); /* class_3bb8c_m.c */
extern void StyleBuildDecorSet(void);
extern void StyleUpdateDecorSet(void);
extern void StyleScrollVramStrips(void);
extern s32 gStyleTickCount;
extern s32 gStyleCueRecordIndex;
extern StyleCueSlot gStyleCueSlotPool[];
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
        ((Class866E8 *)gStyleGrid)->methods->computeCellOffsets((Class866E8 *)gStyleGrid, target, cell);
    }
    if (gStyleTickCount++ == 0) {
        ApplyStyleDecorationIfSet();
        StyleBuildDecorSet();
        StyleBuildEffectSlots(target);
    }
    StyleUpdateDecorSet();
    StyleUpdateEffectSlots(target);
    StyleScrollVramStrips();
    gStyleCueRecordIndex = 0;
    for (i = 0; i < ARRAY_COUNT(gStyleCueSlots); i++) {
        if (gStyleCueSlots[i] != 0) {
            if (ServiceStyleCueIfNear(gStyleCueSlots[i], target, unused) == 0) {
                gStyleCueSlots[i] = FlushStyleCue(gStyleCueSlots[i]);
            }
            /* MATCHING: a no-op pair that gives target/i retail's registers */
            i++;
            i--;
        } else {
            gStyleCueSlots[i] = TryStartStyleCue(&gStyleCueSlotPool[i], &lastCue, target, unused);
        }
    }
    return lastCue;
}

extern s32 gStyleStage;
extern void func_8003B624(DrawRect *rect, s32 count, DrawRect *scratch);
extern DrawRect gStyleStripRectA;
extern DrawRect gStyleStripScratchA;
extern DrawRect gStyleStripRectB;
extern DrawRect gStyleStripScratchB;

/* One step of func_8003B624's one-column VRAM rotation: stage 2 on the
 * strip at y 496, stages 3..5 on the one at y 504. */
void StyleScrollVramStrips(void) {
    DrawRect *rect, *scratch;
    s32 count;

    if (gStyleStage == 2) {
        rect = &gStyleStripRectA;
        scratch = &gStyleStripScratchA;
        count = 1; /* MATCHING: a local set in each branch, not a literal argument */
    } else if ((u32)(gStyleStage - 3) < 3) {
        count = 1;
        rect = &gStyleStripRectB;
        scratch = &gStyleStripScratchB;
    } else {
        return;
    }
    func_8003B624(rect, count, scratch);
}
