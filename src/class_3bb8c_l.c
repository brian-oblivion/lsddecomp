/*
 * class_3bb8c_l -- sixth carved slice of the class_3bb8c block
 * (0x435E0..0x44518, vram 0x80052DE0..0x80053D18), 20 functions, ALL
 * MATCHED. Carved round 15; fully matched by round 45.
 *
 * This slice is entirely ObjM's own methods (gObjMMethods, include/ObjM.h;
 * track 4, round 89 unified the class_3bb8c_k/_l/_m views there), slots
 * +0x040..+0x09C in table order: the init/deinit pair (AttachTarget keeps
 * the DreamSys and hooks the StageMap's callback, DetachTarget), onInit
 * and onDeinit (InitStyleAndWorld, TeardownStyle), onTag1Notify with the
 * TimBlockSrc poll it runs (PollTimBlockLoad), onPadEvent
 * (DispatchPadEvent), update, togglePause (ObjM__TogglePause), the style scene
 * slots +0x080..+0x08C, the DreamSys notification dispatcher
 * (OnDreamSysNotify, owning `jtbl_8001174C`) and EnterState4/5/6, which
 * set IntermediateBase::state and start a fade (ObjM__StartFadeUp,
 * class_3bb8c_m). NoOpSlot40 and NoOpSlot7C are empty.
 *
 * include/class_3bb8c.h is SHARED with every other class_3bb8c_* slice.
 * Header edits must be strictly ADDITIVE.
 */
#include "common.h"
#include "class_3bb8c.h"
#include "TimedTask.h"
#include "DreamSys.h"
#include "ObjM.h"
#include "StageMap.h"
#include "NodeGuardedViewport.h"
#include "FadeBox.h"
#include "TimBlockSrc.h"
#include "WBgm.h"
#include "Pad.h"

void ObjM__NoOpSlot40(void) {}

/* init. `args` is the building DayTask's init args: args->unkC is its
 * StageMap (IntermediateBase__Init keeps it as unk14), whose callback
 * becomes ObjM__GetGridRecord. */
void ObjM__AttachTarget(ObjM *self, IntermediateBaseInitArgs *args, DreamSys *dreamSys) {
    ((StageMap *)args->lightRig)
        ->methods->setCallback((StageMap *)args->lightRig, (ChunkFileFn)ObjM__GetGridRecord, self);
    self->dreamSys = dreamSys;
    GetTimedTaskMethods()->init((TimedTask *)self, args, 1);
    self->methods->addChild(self, (BasicClass *)dreamSys);
}

void ObjM__GetGridRecord(ObjM *self, s32 cell, s32 x, s32 y) {
    if (cell >= 0) {
        GetGridRecordAt(self->stage, cell);
    } else {
        GetGridRecordXY(self->stage, x, y);
    }
}

void ObjM__DetachTarget(ObjM *self) {
    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    GetTimedTaskMethods()->deinit((TimedTask *)self);
}

/* Cross-unit helpers, src/code_39094.c (PickVariant, PickDailyVariant:
 * `Rec1C *(s32 index, ...)`, read here as the value handed on),
 * src/code_d294_c.c (func_8001EF60) and class_3bb8c_m (RegisterStyleConfig,
 * whose third argument is kept as gStyleSceneRefs). */
extern s32 PickVariant(s32 stage, s32 unused);
extern s32 PickDailyVariant(s32 stage, s32 unused, s32 day);
extern s32 func_8001EF60(s32 value);
extern s32 RegisterStyleConfig(void *grid, s32 stage, s32 *sceneRefs, s32 day, s32 arg4);

/* Data reached by address: the viewport's view point and view reference
 * (attachViewChild), the StageMap's bounds; and gStagePendingExtras, one
 * setPendingExtra value per stage. */
extern LongVec3 gObjMViewPoint;
extern LongVec3 gObjMViewRefPoint;
extern s32 gStagePendingExtras[];
extern CellBounds gStage0Bounds;

/* onInit's gridSpan when it is passed 0: gDefaultGridSpan's value, the one
 * the StageMap starts with (10 half-cells: StageMap::gridHalfCells is
 * gridSpan >> 12). */
#define DEFAULT_GRID_SPAN 40960

/* onInit (IntermediateBase__Init passes 0, 0, 0). */
void ObjM__InitStyleAndWorld(ObjM *self, s32 gridSpan, StyleConfig *style, s32 arg3) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    s32 record;
    s32 day;
    s32 flag;

    vp->methods->detachViewChild(vp);
    self->timBlockPending = 1;
    record = PickVariant(self->stage, 0);
    self->bgm->methods->setSeq(self->bgm, (char *)record);

    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    record = PickDailyVariant(self->stage, 0, day);
    self->timBlockSrc = (TimBlockSrc *)New_TimBlockSrc(record);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &gObjMViewPoint,
                                 &gObjMViewRefPoint, 0);

    self->cachedViewport = vp;
    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    self->styleConfig =
        (StyleConfig *)RegisterStyleConfig(self->unk14, self->stage, (s32 *)&self->ctorSound, day, 0);
    if (style != 0) {
        self->styleConfig = style;
    }

    self->unk4C = arg3;
    if (self->stage != 0) {
        s32 stage;
        s32 three;

        /* Retail reloads self->stage here even though the outer `if`
         * just read it and nothing wrote it in between -- a volatile-
         * qualified POINTER TYPE at the read site (not a volatile
         * object) forces the reload without changing the field's own
         * declared type, the same idiom code_179d8_m.c documents for
         * D_8008EA26's `*(u8 *)&sym`, used here in the opposite
         * direction (forcing a reload instead of permitting a fold). */
        stage = *(s32 volatile *)&self->stage;
        self->tickPeriod = 16;
        three = 3;
        /* Order-only: without this barrier the scheduler moves `three`'s
         * `li` past the `self->tickPeriod` store; removing it does not change
         * which register holds which value. */
        __asm__("");
        flag = (stage == 5);
        if (stage == 6) {
            flag = 1;
        }
        self->moveMode = three;
        if (stage == three) {
            flag = 1;
        }
        ((StageMap *)self->unk14)->methods->setBounds((StageMap *)self->unk14, 0);
    } else {
        self->tickPeriod = 16;
        self->moveMode = 2;
        flag = 1;
        ((StageMap *)self->unk14)->methods->setBounds((StageMap *)self->unk14, &gStage0Bounds);
    }

    self->gridSpan = gridSpan;
    if (gridSpan == 0) {
        self->gridSpan = DEFAULT_GRID_SPAN;
    }
    func_8001EF60(flag);

    self->dreamSys->methods->setPendingExtra(self->dreamSys, gStagePendingExtras[self->stage]);
    self->state = 5;
}

/* onDeinit. */
void ObjM__TeardownStyle(ObjM *self) {
    self->methods->exitSceneStyle(self);
    TickDreamAuxSlots2();
    StyleTeardown();
    self->bgm->methods->stop(self->bgm);
}

void ObjM__OnTag1Notify(ObjM *self, void *sender, s32 event) {
    if (event == 2) {
        ObjM__PollTimBlockLoad(self, self->timBlockSrc);
    }
}

/* `src` is always self->timBlockSrc. Loaded, its CLUT rows fade to a
 * styleConfig colour; failed or loaded, it is released and the scene set
 * up (a failure also adds 0x1E to the DreamSys's time limit). With nothing
 * pending and the StageMap idle, the style session starts. */
void ObjM__PollTimBlockLoad(ObjM *self, TimBlockSrc *src) {
    s32 timer;
    s32 colorMode;
    TimBlockSrcColor *color;
    TimBlockSrcMethods *m;

    if (self->timBlockPending != 0) {
        if (src->failed != 0) {
            src->methods->release(src);
            self->timBlockPending = 0;
            self->methods->setupSceneStyle(self);
            timer = self->dreamSys->methods->getDreamTimerScaled(self->dreamSys);
            self->dreamSys->methods->getSetDreamTimeLimit(self->dreamSys, timer + 30);
        } else if (src->loaded != 0) {
            colorMode = self->styleConfig->colorMode;
            m = src->methods;
            if (colorMode != 2) {
                color = self->styleConfig->farColor;
            } else {
                color = self->styleConfig->clearColor;
            }
            m->fadeAllEntries(src, color);
            src->methods->release(src);
            self->timBlockPending = 0;
            self->methods->setupSceneStyle(self);
        }
    }
    if (self->timBlockPending == 0) {
        if (((StageMap *)self->unk14)->pendingLoadCount == 0 && self->inSession == 0) {
            self->unk64 = 1;
            self->methods->enterStyleSession(self);
        }
    }
}

/* onPadEvent: 0x21 togglePause, 0xC updateCloseReadyFlag, 0x2C
 * clearCloseReadyFlag, 0x16 closeAndNotifyD; only in session. */
void ObjM__DispatchPadEvent(ObjM *self, void *sender, s32 code) {
    ObjMMethods *m = self->methods;
    void (*fn)(ObjM *);

    if (self->inSession == 0) {
        return;
    }
    if (code == PAD_EVENT_PRESSED + PAD_BUTTON_RUP) {
        goto closeAndNotify;
    }
    if (code <= PAD_EVENT_PRESSED + PAD_BUTTON_RUP) {
        if (code == PAD_EVENT_HELD + PAD_BUTTON_SELECT) {
            goto updateCloseReady;
        }
        return;
    }
    if (code == PAD_EVENT_PRESSED + PAD_BUTTON_START) {
        goto togglePause;
    }
    if (code == PAD_EVENT_RELEASED + PAD_BUTTON_SELECT) {
        goto clearCloseReady;
    }
    return;
togglePause:
    fn = m->togglePause;
    goto call;
updateCloseReady:
    fn = m->updateCloseReadyFlag;
    goto call;
closeAndNotify:
    fn = m->closeAndNotifyD;
    goto call;
clearCloseReady:
    fn = m->clearCloseReadyFlag;
call:
    fn(self);
}

/* update, the FrameClock case of onNotify. */
void ObjM__Update(ObjM *self) {
    void (*fn)(ObjM *);

    if (self->inSession != 0) {
        self->frameCounter++;
        if (self->pauseSetupStep != 0) {
            fn = self->methods->advancePauseSetup;
        } else {
            fn = self->methods->tickStyle;
        }
        fn(self);
    }
}

/* togglePause. */
void ObjM__TogglePause(ObjM *self) {
    ObjMMethods *m = self->methods;

    if (self->pauseSetupStep != 0) {
        m->clearCloseReadyFlag(self);
        m->teardownPauseOverlay(self);
    } else {
        m->advancePauseSetup(self);
    }
}

void ObjM__NoOpSlot7C(void) {}

/* code_4cd08.c's (MATCHED round 43); no header declares it. `world` is the
 * DreamSys it installs as gDreamAuxWorld (track 4, round 88). */
extern void SetDreamAuxWorld(s32 a0, s32 a1, DreamSys *world, s32 a3, s32 a4);

/* GetStageGridDimensions comes from include/StageGrid.h, through
 * DreamSys.h. */

/* A plain `s32` bias added to the projection distance (%gp_rel value). */
extern s32 gObjMProjectionBias;

/* The StageMap's accepted tags (setAcceptedTags), an opaque .data block
 * (asm/data/76DC8.data.s) reached by address. */
extern s32 gObjMAcceptedClassIds[];

void ObjM__SetupSceneStyle(ObjM *self) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    StyleConfig *style = self->styleConfig;
    DrawSystem *drawSystem;
    s32 width;
    StageMap *rig;

    vp->methods->detachViewChild(vp);

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    width = drawSystem->methods->getDims(drawSystem, NULL)->w;
    vp->methods->setProjection(vp, width / 2 * 5 / 3 + gObjMProjectionBias);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &gObjMViewPoint,
                                 &gObjMViewRefPoint, 0);

    SetDreamAuxWorld(self->stage, (s32)self->unk14, self->dreamSys, (s32)self->sound, (s32)self->unk10);

    rig = (StageMap *)self->unk14;
    self->methods->addChild(self, (BasicClass *)rig);

    rig->methods->setAmbientColor(rig, (LightRigRgb *)style->ambientColor, 0);
    rig->methods->setChildParams(rig, 3, style->lightDirs, style->lightColors);
    rig->methods->setConfig(rig, GetStageGridDimensions(self->stage));
    ((DreamSysAttachToParentFn)self->dreamSys->methods->attachToParent)(self->dreamSys, rig);
    rig->methods->setGridSpan(rig, self->gridSpan);
    rig->methods->setAcceptedTags(rig, gObjMAcceptedClassIds);
}

void ObjM__ExitSceneStyle(ObjM *self) {
    self->methods->teardownPauseOverlay(self);
    self->dreamSys->methods->blockMovement(self->dreamSys);
    self->dreamSys->methods->detachFromParent(self->dreamSys);
    ((NodeGuardedViewport *)self->viewport)->methods->detachViewChild((NodeGuardedViewport *)self->viewport);
    self->methods->removeChild(self, self->unk14);
}

void ObjM__EnterStyleSession(ObjM *self) {
    NodeGuardedViewportMethods *m;
    FadeBoxMethods *m2;
    NodeGuardedViewport *vp;
    FadeBox *fade;
    StyleConfig *style;
    DreamColors flashColor;
    s32 flashback;
    s32 channels;
    void *farColor;

    self->inSession = 1;
    self->dreamSys->methods->resetLinkState(self->dreamSys, self->moveMode, self->tickPeriod);
    ((StageMap *)self->unk14)->methods->enable((StageMap *)self->unk14);

    vp = (NodeGuardedViewport *)self->viewport;
    style = self->styleConfig;
    vp->methods->setLightMode(vp, 1); /* GsFOG */
    vp->methods->setClearColor(vp, style->clearColor);
    vp->methods->setFogNear(vp, style->fogNear);
    m = vp->methods;
    if (style->colorMode != 1) {
        farColor = style->farColor;
    } else {
        farColor = style->clearColor;
    }
    m->setFarColor(vp, farColor);
    vp->methods->setUnkB4(vp, 0);
    vp->methods->setDrawEnabled(vp, 1);

    fade = (FadeBox *)vp->methods->getFadeBox(vp);
    self->methods->addChild(self, (BasicClass *)fade);

    flashback = self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &flashColor, -1);
    fade->methods->setDivisorMode(fade, flashback, (flashback != 0) ? 3 : 0);
    m2 = fade->methods;
    if (flashback == 0) {
        channels = -1;
    } else {
        channels = flashColor;
    }
    m2->startFadeDown(fade, self->unk10, channels, 0);
}

void ObjM__TickStyle(ObjM *self) {
    TickStyle(((StageMap *)self->unk14)->methods->getTargetDescriptor((StageMap *)self->unk14, 0, 0),
              0, 0);
}

/* The DreamSys's codes 0xA..0x11 run enterState4..notifyParentsCodeB while
 * the state is 0; any code from 9 up otherwise clears the DreamSys's own
 * state (Actor::state). */
void ObjM__OnDreamSysNotify(ObjM *self, BasicClass *sender, s32 code) {
    if (self->state == 0) {
        switch (code) {
            case DREAMSYS_TIME_UP:
                self->methods->enterState4(self);
                break;
            case DREAMSYS_LINK_DAY_START:
                break;
            case DREAMSYS_LINK_DYNAMIC:
                self->methods->enterState5(self);
                break;
            case DREAMSYS_LINK_WALL:
                self->methods->enterState6(self);
                break;
            case DREAMSYS_LINK_FLASHBACK:
                self->methods->enterState7(self);
                break;
            case DREAMSYS_LINK_TUNNEL:
                self->methods->enterState8(self);
                break;
            case DREAMSYS_LINK_STAGE_TIMER:
                self->methods->enterStateA(self);
                break;
            case DREAMSYS_LINK_TELEPORT:
                self->methods->notifyParentsCodeB(self);
                break;
        }
    } else if (code >= 9) {
        self->dreamSys->state = DREAMSYS_NO_LINK;
    }
}

void ObjM__EnterState4(ObjM *self) {
    DreamColors color;
    s32 phase;
    s32 t;
    s32 step;

    self->state = 4;
    if (self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1) == 0) {
        phase = (self->frameCounter + self->stage) & 3;
        t = phase;
        if (t == 0) {
            self->methods->notifyParents(self, 4);
            return;
        }
        step = 10;
        switch (t) {
            case 1:
                color = DREAM_COLOR_BLACK;
                break;
            case 2:
                color = DREAM_COLOR_RED;
                break;
            case 3:
                color = DREAM_COLOR_WHITE;
                step = 5;
                break;
        }
        ObjM__StartFadeUp(self, color, 0, step, 1);
        return;
    }
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 5, 1);
}

void ObjM__EnterState5(ObjM *self) {
    s32 color;

    if (self->dreamSys->currentStage < 0) {
        self->methods->enterState6(self);
    } else {
        self->state = 5;
        color = self->dreamSys->methods->getDreamColor(self->dreamSys);
        ObjM__StartFadeUp(self, color, 0, 10, 1);
        self->dreamSys->methods->blockMovement(self->dreamSys);
    }
}

void ObjM__EnterState6(ObjM *self) {
    s32 color;

    self->state = 6;
    color = self->dreamSys->methods->getDreamColor(self->dreamSys);
    ObjM__StartFadeUp(self, color, 0, 30, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}
