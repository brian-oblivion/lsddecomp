/*
 * ObjM's methods (include/objm.h: the TimedTask DayTask starts for a day's
 * scene, which runs its world, its style, its pause overlay and the links
 * that end it), in ROM order: the allocator, ctor, finalize and onNotify,
 * which dispatches on the sender's class id, then its table's slots in
 * order (include/objm.h documents each; NoOpResetCounters and
 * NoOpOnTimedOut are empty), ending with its getter GetObjMMethods.
 *
 * The style layer (include/style_layer.h) is not ObjM's, but ObjM is its
 * client. What the ObjM states stand for in the game is not established;
 * the names describe mechanics.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "timed_task.h"
#include "text_row.h"
#include "tim_image.h"
#include "objm.h"
#include "vab_stream_obj.h"
#include "pad.h"
#include "fade_box.h"
#include "dream_sys.h"
#include "entity.h"
#include "stage_map.h"
#include "node_guarded_viewport.h"
#include "tim_block_src.h"
#include "wbgm.h"
#include "lbd_file.h"
#include "frame_clock.h"
#include "bmem_pmgr.h"
#include "dream_aux.h"
#include "style_layer.h"

/* onInit's gridSpan when it is passed 0: sDefaultGridSpan's value, the one
 * the StageMap starts with (10 half-cells: StageMap::gridHalfCells is
 * gridSpan >> 12). */
#define DEFAULT_GRID_SPAN 40960

/* ObjM's small data, in address order. */

/* MATCHING: two grid spans nothing reads open retail's .sdata. */
static s32 sObjMUnusedGridSpanA SDATA = DEFAULT_GRID_SPAN;
static s32 sObjMUnusedGridSpanB SDATA = DEFAULT_GRID_SPAN;

/* Added to the viewport's projection distance; 0, and never written. */
static s32 sObjMProjectionBias SDATA = 0;

/* ObjM__AdvancePauseSetup's literals, all reached by address: the TextRow's
 * position (attachToParent, percent of half the screen from the centre),
 * its colour, red (setColor), and the text. */
static ScreenSpritePos sPauseTextPos SDATA = {-20, -50};
static ColorRgb sPauseTextColor SDATA = {255, 0, 0};
static char sPauseText[] SDATA = "Pause";

/* ObjM's data, in address order. A method-table slot whose function is
 * declared for another class's `self` takes a `void *` cast. */

/* ObjM's method table, class id 0x2F230: TimedTask's slots, with ObjM's
 * overrides, then its own. */
/* clang-format off */
ObjMMethods gObjMMethods = {
    /* +0x000 header */ 0x2F230,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ ObjM__ObjM,
    /* +0x00C finalize */ ObjM__Finalize,
    /* +0x010 addChild */ (void *)BasicClass__AddChild,
    /* +0x014 removeChild */ (void *)BasicClass__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)BasicClass__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)ObjM__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ (void *)ObjM__NoOpResetCounters,
    /* +0x044 init */ (void *)ObjM__AttachTarget,
    /* +0x048 deinit */ ObjM__DetachTarget,
    /* +0x04C onInit */ (void *)ObjM__InitStyleAndWorld,
    /* +0x050 onDeinit */ ObjM__TeardownStyle,
    /* +0x054 onDrawSystemEvent */ (void *)ObjM__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ (void *)ObjM__DispatchPadEvent,
    /* +0x05C update */ (void *)ObjM__Update,
    /* +0x060 setState */ (void *)TimedTask__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setTimeout */ (void *)TimedTask__SetTimeout,
    /* +0x070 playSound */ (void *)TimedTask__PlaySound,
    /* +0x074 togglePause */ ObjM__TogglePause,
    /* +0x078 slot78 */ NULL,
    /* +0x07C onTimedOut */ (void *)ObjM__NoOpOnTimedOut,
    /* +0x080 setupSceneStyle */ ObjM__SetupSceneStyle,
    /* +0x084 exitSceneStyle */ ObjM__ExitSceneStyle,
    /* +0x088 enterStyleSession */ ObjM__EnterStyleSession,
    /* +0x08C tickStyle */ ObjM__TickStyle,
    /* +0x090 onDreamSysNotify */ ObjM__OnDreamSysNotify,
    /* +0x094 enterTimeUp */ ObjM__EnterTimeUp,
    /* +0x098 enterLinkDynamic */ ObjM__EnterLinkDynamic,
    /* +0x09C enterLinkWall */ ObjM__EnterLinkWall,
    /* +0x0A0 enterLinkFlashback */ ObjM__EnterLinkFlashback,
    /* +0x0A4 enterLinkTunnel */ ObjM__EnterLinkTunnel,
    /* +0x0A8 enterLinkStageTimer */ ObjM__EnterLinkStageTimer,
    /* +0x0AC notifyLinkTeleport */ ObjM__NotifyLinkTeleport,
    /* +0x0B0 onFadeNotify */ ObjM__OnFadeNotify,
    /* +0x0B4 onStageMapNotify */ ObjM__OnStageMapNotify,
    /* +0x0B8 checkAuxTrigger */ ObjM__CheckAuxTrigger,
    /* +0x0BC slotBC */ ObjM__NoOpSlotBC,
    /* +0x0C0 updateCloseReadyFlag */ ObjM__UpdateCloseReadyFlag,
    /* +0x0C4 clearCloseReadyFlag */ ObjM__ClearCloseReadyFlag,
    /* +0x0C8 closeAndNotifyNewGame */ ObjM__CloseAndNotifyNewGame,
    /* +0x0CC closeAndNotify */ ObjM__CloseAndNotify,
    /* +0x0D0 advancePauseSetup */ ObjM__AdvancePauseSetup,
    /* +0x0D4 teardownPauseOverlay */ ObjM__TeardownPauseOverlay,
};
/* clang-format on */

/* The StageMap's accepted tags (setAcceptedTags), 0-terminated. */
s32 sObjMAcceptedClassIds[3] = {DREAMSYS_CLASS_ID, ENTITY_CLASS_ID, 0};

/* The value ObjM__InitStyleAndWorld hands DreamSys's setPendingExtra, per
 * stage. */
/* clang-format off */
s32 sStagePendingExtras[14] = {
    /* stages 0..6  */ 0x080, 0x400, 0x080, 0x100, 0x100, 0x100, 0x000,
    /* stages 7..13 */ 0x100, 0x100, 0x080, 0x000, 0x000, 0x400, 0x400,
};
/* clang-format on */

/* The StageMap's bounds in Bright Moon Cottage: columns 0..8, rows 0..9. */
CellBounds sStage0Bounds = {0, 0, 8, 9};

/* The viewport's view point and reference point (attachViewChild). */
LongVec3 sObjMViewPoint = {0, -1200, 0};
LongVec3 sObjMViewRefPoint = {0, -1200, 10000};

ObjM *New_ObjM(BasicClass *sound, struct WBgm *bgm, TimImage *etcTim,
               struct LinkResource *dreamerTmd, s32 stage) {
    ObjM *self;
    ObjMMethods *methods;

    self = BMemPMgrAlloc(sizeof(ObjM));
    if (self != NULL) {
        methods = GetObjMMethods();
        methods->ctor(self, sound, bgm, etcTim, dreamerTmd, stage);
        return self;
    }
    return NULL;
}

void ObjM__ObjM(ObjM *self, BasicClass *sound, struct WBgm *bgm, TimImage *etcTim,
                struct LinkResource *dreamerTmd, s32 stage) {
    GetTimedTaskMethods()->ctor((TimedTask *)self, 0, sound);
    self->methods = GetObjMMethods();
    self->loadsComplete = 0;
    self->inSession = 0;
    self->timBlockPending = 1;
    self->bgm = bgm;
    self->stage = stage;
    self->ctorSound = sound;
    self->etcTim = etcTim;
    self->dreamerTmd = dreamerTmd;
    self->pauseSetupStep = 0;
    self->closeReady = 0;
    self->methods->resetCounters(self);
}

void ObjM__Finalize(ObjM *self) {
    GetTimedTaskMethods()->finalize((TimedTask *)self);
}

void ObjM__OnNotify(ObjM *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetTimedTaskMethods()->onNotify((TimedTask *)self, sender, event);
    tag = sender->methods->header;
    if ((tag & CLASS_ID_LEVEL3_MASK) == STAGEMAP_CLASS_ID) {
        self->methods->onStageMapNotify(self, sender, event);
    } else if ((tag & CLASS_ID_LEVEL3_MASK) == FADEBOX_CLASS_ID) {
        self->methods->onFadeNotify(self, (struct FadeBox *)sender, event);
    } else if ((tag & CLASS_ID_LEVEL4_MASK) == DREAMSYS_CLASS_ID) {
        self->methods->onDreamSysNotify(self, sender, event);
    }
}

void ObjM__NoOpResetCounters(void) {}

/* init. `args` is the building DayTask's init args: args->lightRig is its
 * StageMap (IntermediateBase__Init keeps it as lightRig), whose callback
 * becomes ObjM__GetGridRecord. */
void ObjM__AttachTarget(ObjM *self, IntermediateBaseInitArgs *args, DreamSys *dreamSys) {
    ((StageMap *)args->lightRig)
        ->methods->setCallback((StageMap *)args->lightRig, (ChunkFileFn)ObjM__GetGridRecord, self);
    self->dreamSys = dreamSys;
    GetTimedTaskMethods()->init((TimedTask *)self, args, INTERMEDIATEBASE_INIT_ATTACH);
    self->methods->addChild(self, (BasicClass *)dreamSys);
}

/* The StageMap's chunkFileFn: a chunk's file record, by linear cell index,
 * or by x/y when the index is negative. */
CdFileEntry *ObjM__GetGridRecord(ObjM *self, s32 cell, s32 x, s32 y) {
    CdFileEntry *record;

    if (cell >= 0) {
        record = GetStageMapChunkRecord(self->stage, cell);
    } else {
        record = GetStageMapChunkRecordXY(self->stage, x, y);
    }
    return record;
}

void ObjM__DetachTarget(ObjM *self) {
    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    GetTimedTaskMethods()->deinit((TimedTask *)self);
}

/* onInit (IntermediateBase__Init passes 0, 0, 0). */
void ObjM__InitStyleAndWorld(ObjM *self, s32 gridSpan, StyleConfig *style, s32 initOption) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    CdFileEntry *record;
    s32 day;
    s32 flag;

    vp->methods->detachViewChild(vp);
    self->timBlockPending = 1;
    record = PickStageBgm(self->stage, 0);
    self->bgm->methods->setSeq(self->bgm, record->name);

    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    record = PickStageTexture(self->stage, 0, day);
    self->timBlockSrc = New_TimBlockSrc(record->name);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sObjMViewPoint,
                                 &sObjMViewRefPoint, 0);

    self->cachedViewport = vp;
    day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    self->styleConfig = (StyleConfig *)RegisterStyleConfig((s32)self->lightRig, self->stage,
                                                           (s32)&self->ctorSound, day, 0);
    if (style != 0) {
        self->styleConfig = style;
    }

    self->initOption = initOption;
    if (self->stage != STAGE_BRIGHT_MOON_COTTAGE) {
        s32 stage;

        /* MATCHING: the volatile read keeps retail's second load of self->stage. */
        stage = *(s32 volatile *)&self->stage;
        self->tickPeriod = 16;
        flag = (stage == STAGE_VIOLENCE_DISTRICT);
        self->moveMode = 3;
        if (stage == STAGE_MOONLIGHT_TOWER) {
            flag = 1;
        }
        if (stage == STAGE_NATURAL_WORLD) {
            flag = 1;
        }
        ((StageMap *)self->lightRig)->methods->setBounds((StageMap *)self->lightRig, 0);
    } else {
        self->tickPeriod = 16;
        self->moveMode = 2;
        flag = 1;
        ((StageMap *)self->lightRig)->methods->setBounds((StageMap *)self->lightRig, &sStage0Bounds);
    }

    self->gridSpan = gridSpan;
    if (gridSpan == 0) {
        self->gridSpan = DEFAULT_GRID_SPAN;
    }
    GetSetHitHeightGate(flag);

    self->dreamSys->methods->setPendingExtra(self->dreamSys, sStagePendingExtras[self->stage]);
    self->state = OBJM_STATE_LINK_DYNAMIC;
}

/* onDeinit. */
void ObjM__TeardownStyle(ObjM *self) {
    self->methods->exitSceneStyle(self);
    ReleaseDreamAuxEntities();
    StyleTeardown();
    self->bgm->methods->stop(self->bgm);
}

void ObjM__OnDrawSystemEvent(ObjM *self, void *sender, s32 event) {
    if (event == DRAWSYSTEM_EVENT_VSYNC) {
        ObjM__PollTimBlockLoad(self, self->timBlockSrc);
    }
}

/* `src` is always self->timBlockSrc. Loaded, its CLUT rows fade to a
 * styleConfig colour; failed or loaded, it is released and the scene set
 * up (a failure also adds 30 seconds to the DreamSys's time limit). With
 * nothing pending and the StageMap idle, the style session starts. */
void ObjM__PollTimBlockLoad(ObjM *self, TimBlockSrc *src) {
    s32 timer;
    s32 colorMode;
    ColorRgb *color;
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
        if (((StageMap *)self->lightRig)->pendingLoadCount == 0 && self->inSession == 0) {
            self->loadsComplete = 1;
            self->methods->enterStyleSession(self);
        }
    }
}

/* onPadEvent, only in session: Start pressed toggles the pause, Select
 * held and released sets and clears the close-ready flag, triangle pressed
 * closes (closeAndNotifyNewGame). */
void ObjM__DispatchPadEvent(ObjM *self, void *sender, s32 code) {
    ObjMMethods *m = self->methods;
    void (*fn)(ObjM *);

    if (self->inSession == 0) {
        return;
    }
    switch (code) { /* MATCHING: the cases stay in this order; retail lays their bodies out in it */
        case PAD_EVENT_PRESSED + PAD_BUTTON_START:
            fn = m->togglePause;
            break;
        case PAD_EVENT_HELD + PAD_BUTTON_SELECT:
            fn = m->updateCloseReadyFlag;
            break;
        case PAD_EVENT_PRESSED + PAD_BUTTON_RUP:
            fn = m->closeAndNotifyNewGame;
            break;
        case PAD_EVENT_RELEASED + PAD_BUTTON_SELECT:
            fn = m->clearCloseReadyFlag;
            break;
        default:
            return;
    }
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

void ObjM__NoOpOnTimedOut(void) {}

void ObjM__SetupSceneStyle(ObjM *self) {
    NodeGuardedViewport *vp = (NodeGuardedViewport *)self->viewport;
    StyleConfig *style = self->styleConfig;
    DrawSystem *drawSystem;
    s32 width;
    StageMap *rig;

    vp->methods->detachViewChild(vp);

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    width = drawSystem->methods->getDims(drawSystem, NULL)->width;
    vp->methods->setProjection(vp, width / 2 * 5 / 3 + sObjMProjectionBias);

    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sObjMViewPoint,
                                 &sObjMViewRefPoint, 0);

    SetDreamAuxWorld(self->stage, (StageMap *)self->lightRig, self->dreamSys,
                     (struct VabStreamObj *)self->sound, (struct FrameClock *)self->frameClock);

    rig = (StageMap *)self->lightRig;
    self->methods->addChild(self, (BasicClass *)rig);

    rig->methods->setAmbientColor(rig, (ColorRgb *)style->ambientColor, 0);
    rig->methods->setChildParams(rig, 3, style->lightDirs, style->lightColors);
    rig->methods->setConfig(rig, GetStageGridDimensions(self->stage));
    ((DreamSysAttachToParentFn)self->dreamSys->methods->attachToParent)(self->dreamSys, rig);
    rig->methods->setGridSpan(rig, self->gridSpan);
    rig->methods->setAcceptedTags(rig, sObjMAcceptedClassIds);
}

void ObjM__ExitSceneStyle(ObjM *self) {
    self->methods->teardownPauseOverlay(self);
    self->dreamSys->methods->blockMovement(self->dreamSys);
    self->dreamSys->methods->detachFromParent(self->dreamSys);
    ((NodeGuardedViewport *)self->viewport)->methods->detachViewChild((NodeGuardedViewport *)self->viewport);
    self->methods->removeChild(self, self->lightRig);
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
    ((StageMap *)self->lightRig)->methods->enable((StageMap *)self->lightRig);

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
    vp->methods->setExtraSwap(vp, 0);
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
    m2->startFadeDown(fade, self->frameClock, channels, 0);
}

void ObjM__TickStyle(ObjM *self) {
    TickStyle(((StageMap *)self->lightRig)->methods->getTargetDescriptor((StageMap *)self->lightRig, 0, 0),
              0, 0);
}

/* While ObjM is OBJM_STATE_IDLE each DreamSys link code runs its enterState slot
 * (DREAMSYS_LINK_DAY_START none); otherwise any code from 9 up clears the
 * DreamSys's own state. */
void ObjM__OnDreamSysNotify(ObjM *self, BasicClass *sender, s32 code) {
    if (self->state == OBJM_STATE_IDLE) {
        switch (code) {
            case DREAMSYS_TIME_UP:
                self->methods->enterTimeUp(self);
                break;
            case DREAMSYS_LINK_DAY_START:
                break;
            case DREAMSYS_LINK_DYNAMIC:
                self->methods->enterLinkDynamic(self);
                break;
            case DREAMSYS_LINK_WALL:
                self->methods->enterLinkWall(self);
                break;
            case DREAMSYS_LINK_FLASHBACK:
                self->methods->enterLinkFlashback(self);
                break;
            case DREAMSYS_LINK_TUNNEL:
                self->methods->enterLinkTunnel(self);
                break;
            case DREAMSYS_LINK_STAGE_TIMER:
                self->methods->enterLinkStageTimer(self);
                break;
            case DREAMSYS_LINK_TELEPORT:
                self->methods->notifyLinkTeleport(self);
                break;
        }
    } else if (code >= 9) {
        self->dreamSys->state = DREAMSYS_NO_LINK;
    }
}

void ObjM__EnterTimeUp(ObjM *self) {
    DreamColors color;
    s32 phase;
    s32 t;
    s32 step;

    self->state = OBJM_STATE_TIME_UP;
    if (self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1) == 0) {
        phase = (self->frameCounter + self->stage) & 3;
        t = phase; /* MATCHING: tested through a copy; testing phase compiles differently. */
        if (t == 0) {
            self->methods->notifyParents(self, OBJM_STATE_TIME_UP);
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

void ObjM__EnterLinkDynamic(ObjM *self) {
    s32 color;

    if (self->dreamSys->currentStage < 0) {
        self->methods->enterLinkWall(self);
    } else {
        self->state = OBJM_STATE_LINK_DYNAMIC;
        color = self->dreamSys->methods->getDreamColor(self->dreamSys);
        ObjM__StartFadeUp(self, color, 0, 10, 1);
        self->dreamSys->methods->blockMovement(self->dreamSys);
    }
}

void ObjM__EnterLinkWall(ObjM *self) {
    s32 color;

    self->state = OBJM_STATE_LINK_WALL;
    color = self->dreamSys->methods->getDreamColor(self->dreamSys);
    ObjM__StartFadeUp(self, color, 0, 30, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

void ObjM__EnterLinkFlashback(ObjM *self) {
    DreamColors color;
    self->state = OBJM_STATE_LINK_FLASHBACK;
    self->dreamSys->methods->getSetFlashbackSession(self->dreamSys, &color, -1);
    ObjM__StartFadeUp(self, color, 0, 5, 1);
    self->dreamSys->methods->blockMovement(self->dreamSys);
}

void ObjM__EnterLinkTunnel(ObjM *self) {
    self->state = OBJM_STATE_LINK_TUNNEL;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_FORCED);
}

void ObjM__EnterLinkStageTimer(ObjM *self) {
    self->state = OBJM_STATE_LINK_STAGE_TIMER;
    ObjM__StartFadeUp(self, DREAM_COLOR_BLACK, 0, 6, 1);
    self->dreamSys->methods->selectMoveCallback(self->dreamSys, MOVE_CALLBACK_TICK_DRIFT);
    self->dreamSys->methods->setMoveOverride(self->dreamSys, MOVE_OVERRIDE_HELD);
}

void ObjM__NotifyLinkTeleport(ObjM *self) {
    self->methods->notifyParents(self, OBJM_NOTIFY_LINK_TELEPORT);
}

/* The fade box is the viewport's (IntermediateBase::viewport, a
 * NodeGuardedViewport: getFadeBox); IntermediateBase::frameClock, the FrameClock,
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
    fade->methods->startFadeUp(fade, self->frameClock, channels, fadeMode);
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
                ->methods->setClearColor((NodeGuardedViewport *)self->viewport, (ColorRgb *)color);
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
    struct MapChunk coord; /* SplitChunkIndex writes the column, then the row */
    s32 held;
    ChunkSlot *slot = ((StageMap *)self->lightRig)
                          ->methods->getLastEventSlotChunk((StageMap *)self->lightRig, &coord.col);
    s32 day = self->dreamSys->methods->getCurrentDayAndYear(self->dreamSys, 0);
    held = TryDreamAuxTrigger((s32)slot->loader->dataBuffer, (s16 *)&coord, day); /* the coord is the trigger key */
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

void ObjM__CloseAndNotifyNewGame(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE_NEW_GAME);
    }
}

void ObjM__CloseAndNotify(ObjM *self) {
    if (self->closeReady) {
        self->methods->teardownPauseOverlay(self);
        self->methods->notifyParents(self, OBJM_NOTIFY_CLOSE);
    }
}

/* Called each update while the overlay is up. Step 0 builds the "Pause"
 * TextRow under the StageMap; the fourth call after it hides the viewport
 * and pauses the FrameClock, the WBgm and the VabStreamObj
 * (IntermediateBase::frameClock, bgm, TimedTask::sound). */
void ObjM__AdvancePauseSetup(ObjM *self) {
    s32 step = self->pauseSetupStep;
    if (step == 0) {
        self->pauseText = New_TextRow(self->etcTim, 5, sPauseText);
        self->pauseText->methods->attachToParent(self->pauseText, (SceneNode *)self->lightRig,
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
    ((FrameClock *)self->frameClock)->methods->pause((FrameClock *)self->frameClock);
    self->bgm->methods->pause(self->bgm);
    ((VabStreamObj *)self->sound)->methods->mute((VabStreamObj *)self->sound);
}

void ObjM__TeardownPauseOverlay(ObjM *self) {
    if (self->pauseSetupStep != 0) {
        self->pauseText->methods->release(self->pauseText);
    }
    ((VabStreamObj *)self->sound)->methods->unmute((VabStreamObj *)self->sound);
    self->bgm->methods->resume(self->bgm);
    ((FrameClock *)self->frameClock)->methods->resume((FrameClock *)self->frameClock);
    ((NodeGuardedViewport *)self->viewport)->methods->setDrawEnabled((NodeGuardedViewport *)self->viewport, 1);
    self->pauseSetupStep = 0;
}

ObjMMethods *GetObjMMethods(void) {
    return &gObjMMethods;
}
