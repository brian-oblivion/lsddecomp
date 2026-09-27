/*
 * class_3bb8c_m -- ObjM's methods from +0x0A0 to the end of its table
 * (gObjMMethods, include/ObjM.h) and its getter, then the style layer's
 * setup: the four free functions that pick the stage's scene style.
 *
 * ObjM, in ROM order:
 *  - EnterState7/8/A and NotifyParentsCodeB, the DreamSys link codes
 *    ObjM__OnDreamSysNotify hands on (flashback, tunnel, stage timer,
 *    teleport): each sets IntermediateBase::state (enum ObjMState) and
 *    fades up through StartFadeUp, or notifies its parent at once.
 *  - StartFadeUp: the viewport's fade box (a FadeBox), optionally added as
 *    a child, fades up in the given channels.
 *  - OnFadeNotify: fade down done returns ObjM to IDLE; fade up done sets
 *    the viewport's clear colour to the fade's and notifies the state (a
 *    stage-timer link turns into TIME_UP first). OnStageMapNotify runs
 *    CheckAuxTrigger when a slot's data block is ready.
 *  - The pause overlay: AdvancePauseSetup builds the "Pause" TextRow and,
 *    four calls later, hides the viewport and pauses the FrameClock, the
 *    WBgm and the VabStreamObj; TeardownPauseOverlay undoes it. While it is
 *    up and ObjM is IDLE, the close-ready flag arms CloseAndNotifyC/D,
 *    which tear it down and notify a close (DayTask ends the day).
 *    NoOpSlotBC is empty.
 *
 * The style setup is not ObjM's, but ObjM is its client:
 * ObjM__InitStyleAndWorld (class_3bb8c_l) calls RegisterStyleConfig once
 * per scene and keeps the result, sStyleConfig, as ObjM::styleConfig.
 * RegisterStyleConfig stores the scene (grid, stage, ObjM's scene
 * references, day) in the gStyle globals class_3bb8c_n reads;
 * ApplyStyleConfig takes the stage's four-byte StyleStageConfig, or
 * PickStyleFallbackConfig's, and FillStyleFromConfig turns it into
 * StyleConfig colours and a fog distance. ApplyStyleDecorationIfSet builds
 * the decoration box, a full-screen semi-transparent BoxFill under the
 * viewport's fade box, when the config asked for one.
 *
 * What the ObjM states and the style configs stand for in the game is not
 * established; the names describe mechanics.
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
    self->state = OBJM_STATE_LINK_FLASHBACK;
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1);
    ObjM__StartFadeUp(self, color, 0, 5, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

void ObjM__EnterState8(ObjM *self) {
    self->state = OBJM_STATE_LINK_TUNNEL;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_FORCED);
}

void ObjM__EnterStateA(ObjM *self) {
    self->state = OBJM_STATE_LINK_STAGE_TIMER;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->selectCallback98(self->dreamSys, MOVE_CALLBACK_TICK_DRIFT);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_HELD);
}

void ObjM__NotifyParentsCodeB(ObjM *self) {
    self->methods->notifyParents(self, OBJM_NOTIFY_LINK_TELEPORT);
}

/* The fade box is the viewport's (IntermediateBase::viewport, a
 * NodeGuardedViewport: getFadeBox); IntermediateBase::unk10, the FrameClock,
 * drives it. A zero step keeps the box's own. */
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
        case FADEBOX_EVENT_FADE_DOWN_DONE:
            self->methods->removeChild(self, (BasicClass *)sender);
            self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_NONE);
            self->state = OBJM_STATE_IDLE;
            break;
        case FADEBOX_EVENT_FADE_UP_DONE:
            self->methods->removeChild(self, (BasicClass *)sender);
            color = sender->methods->getColor(sender);
            ((NodeGuardedViewport *)self->viewport)
                ->methods->setClearColor((NodeGuardedViewport *)self->viewport, (ViewportRgb *)color);
            /* MATCHING: the two dead `!=` tests are retail's compares. */
            if (self->state != OBJM_STATE_LINK_DYNAMIC && self->state != OBJM_STATE_LINK_TUNNEL &&
                self->state == OBJM_STATE_LINK_STAGE_TIMER) {
                self->dreamSys->methods->stopDrift(self->dreamSys, 1);
                self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_NONE);
                self->state = OBJM_STATE_TIME_UP;
            }
            self->methods->notifyParents(self, self->state);
            break;
    }
}

void ObjM__OnStageMapNotify(ObjM *self, BasicClass *sender, s32 event) {
    if (event == STAGEMAP_EVENT_SLOT_DATA_READY) {
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
    if (self->pauseSetupStep != 0 && self->state == OBJM_STATE_IDLE) {
        self->closeReady = 1;
    }
}

void ObjM__ClearCloseReadyFlag(ObjM *self) {
    self->closeReady = 0;
}

void ObjM__CloseAndNotifyD(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE_NEW_GAME);
    }
}

void ObjM__CloseAndNotifyC(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE);
    }
}

/* Called each update while the overlay is up. Step 0 builds the "Pause"
 * TextRow under the StageMap; the fourth call after it hides the viewport
 * and pauses the FrameClock, the WBgm and the VabStreamObj
 * (IntermediateBase::unk10, bgm, TimedTask::sound). */
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
extern s32 gStyleSceneRefs; /* a StyleSceneRefs * (below) */
extern s32 gStyleVariant;
/* class_3bb8c_n.c defines StyleCueSlot; this unit only clears the slots. */
typedef struct StyleCueSlot StyleCueSlot;
extern StyleCueSlot *gStyleCueSlots[2];

extern void *ApplyStyleConfig(void);

s32 RegisterStyleConfig(s32 grid, s32 stage, s32 sceneRefs, s32 day, s32 unreadArg) {
    StyleCueSlot **slot;
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

/* A stage's style config: four signed bytes, from sStyleStageConfigs (NULL
 * for a stage without a fixed one) or PickStyleFallbackConfig
 * (class_3bb8c_n), which FillStyleFromConfig turns into sStyleConfig's last
 * four words. */
typedef struct StyleStageConfig {
    s8 colorMode; /* StyleConfig::colorMode */
    s8 fogLevel; /* sStyleFogNears index; STYLE_DECOR_FOG_LEVEL and up also build the decoration box */
    s8 farColorIndex; /* gStylePalette index: StyleConfig::farColor, and the decoration box's colour */
    s8 clearColorIndex; /* gStylePalette index: StyleConfig::clearColor */
} StyleStageConfig;

/* The fog levels whose config also gets a decoration box (ApplyStyleConfig):
 * this one and up, sStyleFogNears' two nearest (fogNear 4096 and 2048). */
#define STYLE_DECOR_FOG_LEVEL 4

extern StyleConfig sStyleConfig;
extern StyleStageConfig *sStyleStageConfigs[];
extern StyleStageConfig *PickStyleFallbackConfig(void);
extern void FillStyleFromConfig(StyleConfig *style, StyleStageConfig *cfg);
extern u8 gStylePalette[][3];
extern const u8 *gStyleDecorColor;

/* The stage's fixed config, or with none PickStyleFallbackConfig's, into
 * sStyleConfig, whose first three words (the StageMap's light settings)
 * are fixed. */
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

/* What gStyleSceneRefs points at: ObjM's +0x06C..+0x07B block
 * (ObjM__InitStyleAndWorld passes &ctorSound to RegisterStyleConfig, which
 * keeps it; include/ObjM.h). The same view as class_3bb8c_n.c's, field for
 * field; this unit reads only the viewport. */
typedef struct StyleSceneRefs {
    void *sound;        /* +0x000, ObjM::ctorSound */
    void *dreamerTmd;   /* +0x004, ObjM::dreamerTmd */
    void *etcTim;       /* +0x008, ObjM::etcTim */
    Viewport *viewport; /* +0x00C, ObjM::cachedViewport */
} StyleSceneRefs;

extern s32 gStyleDecorObj;           /* a BoxFill *; class_3bb8c_n.c declares it s32 too */
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
