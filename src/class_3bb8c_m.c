/*
 * class_3bb8c_m -- seventh carved slice of the class_3bb8c block
 * (0x44518..0x44F14, vram 0x80053D18..0x80054714), 20 functions, ALL 20
 * MATCHED (0 INCLUDE_ASM, 0 NON_MATCHING). Carved round 15; all four former
 * toolchain-blocker functions matched round 23/44, once `addiu_at` and
 * `gp_rel` were resolved project-wide (CLAUDE.md, "Open toolchain
 * blockers"). This unit owns no switch jump table.
 *
 * ObjM's methods from +0x0A0 to the end of its table (gObjMMethods,
 * include/ObjM.h; track 4, round 89 unified the class_3bb8c_k/_l/_m views
 * there), and its getter: EnterState7/8/A and NotifyParentsCodeB (the
 * DreamSys codes 0xE..0x11, which set IntermediateBase::state and start a
 * fade), StartFadeUp (the viewport's FadeBox fade box), the fade box's
 * and the StageMap's notification handlers (OnFadeNotify: 5 fade down
 * done, 6 fade up done; OnStageMapNotify: 7 runs CheckAuxTrigger), and
 * the "Pause" overlay: AdvancePauseSetup builds the TextRow and, four
 * calls later, pauses the FrameClock, the WBgm and the VabStreamObj and
 * hides the viewport; TeardownPauseOverlay undoes it; the close-ready flag
 * and CloseAndNotifyC/D report 0xC/0xD to the parent (DayTask's
 * onObjMNotify). NoOpSlotBC is empty. What the state codes mean in the
 * game is not established.
 *
 * Then four free functions (RegisterStyleConfig / ApplyStyleConfig /
 * FillStyleFromConfig / ApplyStyleDecorationIfSet) that read and write a
 * small set of `.sdata`/`.sbss` style globals and fill a `StyleM`
 * colour/config descriptor (struct defined below, own comment). They are
 * not ObjM methods, but ObjM is their client: ObjM__InitStyleAndWorld
 * (class_3bb8c_l) calls RegisterStyleConfig and keeps its result as
 * ObjM::styleConfig, and the `sceneRefs` it passes, kept as
 * gStyleSceneRefs, points at ObjM's +0x06C block (StyleSceneRefs, below;
 * ApplyStyleDecorationIfSet attaches its BoxFill to that block's
 * viewport's fade box).
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "class_3bb8c.h"
#include "DreamSys.h"
#include "LbdFile.h"
#include "BoxFill.h"
#include "TextRow.h"
#include "ObjM.h"
#include "StageMap.h"
#include "NodeGuardedViewport.h"
#include "FadeBox.h"
#include "FrameClock.h"
#include "WBgm.h"
#include "VabStreamObj.h"

/* StageMap__SplitChunkIndex's output: the chunk's column and row. */
typedef struct ChunkCoord {
    u8 column;
    u8 row;
} ChunkCoord;

/* src/code_4cd08.c; it reads `coord` as one s16 trigger key. */
extern s32 TryDreamAuxTrigger(s32 data, ChunkCoord *coord, s32 day);

/* ObjM__AdvancePauseSetup's literals, all reached by address: the "Pause"
 * text, the TextRow's position (attachToParent) and its colour (setColor). */
extern char sPauseText[];             /* "Pause" */
extern ScreenSpritePos sPauseTextPos; /* (-20, -50) */
extern SpriteRgb sPauseTextColor;     /* red: (255, 0, 0) */

void ObjM__EnterState7(ObjM *self) {
    DreamColors color;
    self->state = 7;
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1);
    ObjM__StartFadeUp(self, color, 0, 5, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

void ObjM__EnterState8(ObjM *self) {
    self->state = 8;
    ObjM__StartFadeUp(self, 0, 0, 6, 1);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, 1);
}

void ObjM__EnterStateA(ObjM *self) {
    self->state = 0xA;
    ObjM__StartFadeUp(self, 0, 0, 6, 1);
    self->dreamSys->methods->selectCallback98(self->dreamSys, 2);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, 2);
}

void ObjM__NotifyParentsCodeB(ObjM *self) {
    self->methods->notifyParents(self, 0xB);
}

/* The viewport (IntermediateBase::viewport, a NodeGuardedViewport) hands out its fade
 * box (getFadeBox, Viewport's New_FadeBox). */
void ObjM__StartFadeUp(ObjM *self, s32 channels, s32 fadeMode, s32 step, s32 addChild) {
    FadeBox *fade = (FadeBox *)((NodeGuardedViewport *)self->viewport)
                        ->methods->getFadeBox((NodeGuardedViewport *)self->viewport);
    if (step != 0) {
        fade->methods->setStep(fade, step);
    }
    if (addChild != 0) {
        self->methods->addChild(self, (BasicClass *)fade);
    }
    fade->methods->startFadeUp(fade, self->unk10, channels, fadeMode);
}

void ObjM__OnFadeNotify(ObjM *self, FadeBox *sender, s32 event) {
    void *color;
    switch (event) {
        case 5:
            self->methods->removeChild(self, (BasicClass *)sender);
            self->dreamSys->methods->setMoveOverride(self->dreamSys, 0);
            self->state = 0;
            break;
        case 6:
            self->methods->removeChild(self, (BasicClass *)sender);
            color = sender->methods->getColor(sender);
            ((NodeGuardedViewport *)self->viewport)
                ->methods->setClearColor((NodeGuardedViewport *)self->viewport, (ViewportRgb *)color);
            if (self->state != 5 && self->state != 8 && self->state == 0xA) {
                self->dreamSys->methods->stopDrift(self->dreamSys, 1);
                self->dreamSys->methods->setMoveOverride(self->dreamSys, 0);
                self->state = 4;
            }
            self->methods->notifyParents(self, self->state);
            break;
    }
}

void ObjM__OnStageMapNotify(ObjM *self, BasicClass *sender, s32 event) {
    if (event == 7) {
        self->methods->checkAuxTrigger(self);
    }
}

/* The StageMap's last event slot's data block goes to TryDreamAuxTrigger
 * with the slot's chunk coordinates and the day; the block is released
 * unless that returns an object, which the slot keeps as heldObj. */
s32 ObjM__CheckAuxTrigger(ObjM *self) {
    ChunkCoord coord;
    s32 held;
    ChunkSlot *slot =
        ((StageMap *)self->unk14)->methods->getLastEventSlotChunk((StageMap *)self->unk14, &coord.column);
    s32 day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    held = TryDreamAuxTrigger((s32)slot->loader->dataBuffer, &coord, day);
    slot->heldObj = (BasicClass *)held;
    if (held != 0) {
        return 0;
    }
    slot->loader->methods->releaseDataBlock(slot->loader);
    return 1;
}

void ObjM__NoOpSlotBC(void) {}

void ObjM__UpdateCloseReadyFlag(ObjM *self) {
    if (self->pauseSetupStep != 0 && self->state == 0) {
        self->closeReady = 1;
    }
}

void ObjM__ClearCloseReadyFlag(ObjM *self) {
    self->closeReady = 0;
}

void ObjM__CloseAndNotifyD(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, 0xD);
    }
}

void ObjM__CloseAndNotifyC(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, 0xC);
    }
}

/* The pause: step 0 builds the "Pause" TextRow under the StageMap; the
 * fourth call after it hides the viewport and pauses the FrameClock, the
 * WBgm and the VabStreamObj (IntermediateBase::unk10, bgm, TimedTask::sound). */
void ObjM__AdvancePauseSetup(ObjM *self) {
    s32 step = self->pauseSetupStep;
    if (step == 0) {
        self->pauseText = New_TextRow(self->etcTim, 5, sPauseText);
        self->pauseText->methods->attachToParent(self->pauseText, (SceneNode *)self->unk14,
                                                 (LongVec3 *)&sPauseTextPos);
        self->pauseText->methods->setColor(self->pauseText, &sPauseTextColor);
        self->pauseSetupStep = step + 1;
        return;
    }
    self->pauseSetupStep = step + 1;
    if (step != 4) {
        return;
    }
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 0);
    ((FrameClock *)self->unk10)->methods->pause((FrameClock *)self->unk10);
    self->bgm->methods->pause(self->bgm);
    ((VabStreamObj *)self->sound)->methods->mute((VabStreamObj *)self->sound);
}

void ObjM__TeardownPauseOverlay(ObjM *self) {
    if (self->pauseSetupStep != 0) {
        self->pauseText->methods->release(self->pauseText);
    }
    ((VabStreamObj *)self->sound)->methods->unmute((VabStreamObj *)self->sound);
    self->bgm->methods->resume(self->bgm);
    ((FrameClock *)self->unk10)->methods->resume((FrameClock *)self->unk10);
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 1);
    self->pauseSetupStep = 0;
}

ObjMMethods *GetObjMMethods(void) {
    return &gObjMMethods;
}

extern s32 gStyleGrid;
extern s32 gStyleStage;
extern s32 gStyleTickCount;
extern s32 gStyleDay;
extern s32 sStyleUnreadArg;
extern s32 gStyleSceneRefs;
extern s32 gStyleVariant;
extern void *gStyleCueSlots[2];

extern void *ApplyStyleConfig(void);

s32 RegisterStyleConfig(s32 grid, s32 stage, s32 sceneRefs, s32 day, s32 unreadArg) {
    void **slot;
    s32 i;

    if (gStyleGrid == 0) {
        i = ARRAY_COUNT(gStyleCueSlots) - 1;
        slot = &gStyleCueSlots[ARRAY_COUNT(gStyleCueSlots) - 1];
        gStyleGrid = grid;
        gStyleStage = stage;
        gStyleSceneRefs = sceneRefs;
        gStyleVariant = -1;
        gStyleDay = day;
        sStyleUnreadArg = unreadArg;
        gStyleTickCount = 0;
        do {
            *slot = 0;
            i--;
            slot--;
        } while (i >= 0);
        return (s32)ApplyStyleConfig();
    }
    return 0;
}

/* A stage's style config: four signed bytes, from sStyleStageConfigs or
 * PickStyleFallbackConfig (class_3bb8c_n), which FillStyleFromConfig turns
 * into sStyleConfig's last four words. */
typedef struct StyleStageConfig {
    s8 colorMode; /* StyleConfig::colorMode */
    s8 fogLevel; /* sStyleFogNears index; STYLE_DECOR_FOG_LEVEL and up also build the decoration box */
    s8 farColorIndex; /* gStylePalette index: StyleConfig::farColor, and the decoration box's colour */
    s8 clearColorIndex; /* gStylePalette index: StyleConfig::clearColor */
} StyleStageConfig;

/* The fog levels whose config also gets a decoration box (ApplyStyleConfig):
 * this one and denser, sStyleFogNears' last two (fogNear 4096 and 2048). */
#define STYLE_DECOR_FOG_LEVEL 4

extern StyleConfig sStyleConfig;
extern StyleStageConfig *sStyleStageConfigs[];
extern StyleStageConfig *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg);
extern u8 gStylePalette[][3];
extern const u8 *gStyleDecorColor;

/* The stage's fixed config, or with none PickStyleFallbackConfig's, into
 * sStyleConfig. */
void *ApplyStyleConfig(void) {
    StyleStageConfig *cfg = sStyleStageConfigs[gStyleStage];

    if (cfg == 0) {
        cfg = PickStyleFallbackConfig();
    }
    FillStyleFromConfig(&sStyleConfig, cfg);
    if (cfg->fogLevel >= STYLE_DECOR_FOG_LEVEL) {
        gStyleDecorColor = gStylePalette[cfg->farColorIndex];
    }
    return &sStyleConfig;
}

/* gStylePalette is 24 RGB triples (a greyscale ramp first: 0, 64, 128, 255).
 * MATCHING: indexed as `u8[][3]`, for retail's `i*2 + i + base` stride-3
 * address arithmetic. sStyleFogNears is six fogNear distances, 26624 down to
 * 2048. */
extern s32 sStyleFogNears[];

void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg) {
    style->clearColor = gStylePalette[cfg->clearColorIndex];
    style->farColor = gStylePalette[cfg->farColorIndex];
    style->fogNear = sStyleFogNears[cfg->fogLevel];
    style->colorMode = cfg->colorMode;
}

/* gStyleDecorObj is a BoxFill (include/BoxFill.h), kept in an s32 global
 * (track 4b's to retype). */

/* What gStyleSceneRefs points at: ObjM's +0x06C..+0x07B block
 * (ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it; include/ObjM.h). The same view as class_3bb8c_n.c's, field for
 * field; this unit reads only +0x00C, ObjM::cachedViewport, whose +0x0AC slot
 * is Viewport's getFadeBox. */
typedef struct StyleSceneRefs {
    void *sound;        /* +0x000, ObjM::ctorSound */
    void *dreamerTmd;   /* +0x004, ObjM::dreamerTmd */
    void *etcTim;       /* +0x008, ObjM::etcTim */
    Viewport *viewport; /* +0x00C, ObjM::cachedViewport */
} StyleSceneRefs;

extern s32 gStyleDecorObj;
extern s32 sStyleDecorBoxSize[2];    /* 320 x 240, the screen */
extern BoxFillPos sStyleDecorBoxPos; /* (-100, -100), as Viewport's own fade box */

void ApplyStyleDecorationIfSet(void) {
    SceneNode *fadeBox;

    if (gStyleDecorColor != 0) {
        gStyleDecorObj = (s32)New_BoxFill(sStyleDecorBoxSize, (BoxFillRgb *)gStyleDecorColor, 0);
        ((BoxFill *)gStyleDecorObj)->methods->setSemiTransOn((BoxFill *)gStyleDecorObj, 1);
        ((BoxFill *)gStyleDecorObj)->methods->setSemiTransRate((BoxFill *)gStyleDecorObj, 0);

        fadeBox = ((StyleSceneRefs *)gStyleSceneRefs)
                      ->viewport->methods->getFadeBox(((StyleSceneRefs *)gStyleSceneRefs)->viewport);

        ((BoxFillAttachToParentFn)((BoxFill *)gStyleDecorObj)->methods->attachToParent)(
            (BoxFill *)gStyleDecorObj, fadeBox, &sStyleDecorBoxPos);
    }
}
