/*
 * dream_day.c -- the dream day's task and the stage map it builds, in
 * address order: DayTask (include/day_task.h), RegisterRecordTableFiles
 * (include/dream_day.h), TimedTask, DayTask's and ObjM's parent
 * (include/timed_task.h), and StageMap (include/stage_map.h). Each class's
 * header says what it is and how it lives; StageMap's methods fall in three
 * runs, each introduced below. NodeGuardedViewport and GridCell, which
 * DayTask and StageMap use, are defined in src/ui/title_menu.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "dream_day.h"
#include "vab_stream_obj.h"
#include "node_guarded_viewport.h"
#include "stage_map.h"
#include "wbgm.h"
#include "tim_image.h"
#include "frame_clock.h"
#include "dream_sys.h"
#include "link_resource.h"
#include "objm.h"
#include "actor.h"
#include "light_rig.h"
#include "timed_task.h"
#include "draw_system.h"
#include "placement_grid.h"
#include "lbd_file.h"
#include "grid_cell.h"
#include "flat_light_obj.h"
#include "bmem_pmgr.h"
#include "game_files.h"
#include "data_source.h"
#include "dream_aux.h"

/* "ETC\\ETC.TIM" and "ETC\\DREAMER.TMD", the files DayTask's ctor loads. */
extern const char sEtcTimPath[];
extern const char sDreamerTmdPath[];

/* The data of the three classes, in address order: DayTask's, TimedTask's
 * and StageMap's. A method-table slot whose function is declared for
 * another class's `self` (a parent's method, or an override that keeps the
 * parent's parameter types) takes a `void *` cast. */

/* DayTask's method table, class id 0x1F230. */
/* clang-format off */
DayTaskMethods gDayTaskMethods = {
    /* +0x000 header */ 0x1F230,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ DayTask__DayTask,
    /* +0x00C finalize */ DayTask__Finalize,
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
    /* +0x038 onNotify */ (void *)DayTask__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ DayTask__ResetPhase,
    /* +0x044 init */ (void *)DayTask__Init,
    /* +0x048 deinit */ DayTask__Deinit,
    /* +0x04C onInit */ (void *)DayTask__OnInit,
    /* +0x050 onDeinit */ DayTask__OnDeinit,
    /* +0x054 onDrawSystemEvent */ DayTask__AdvancePhase,
    /* +0x058 onPadEvent */ (void *)TimedTask__NoOpOnPadEvent,
    /* +0x05C update */ (void *)TimedTask__CheckTimeout,
    /* +0x060 setState */ (void *)TimedTask__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setTimeout */ (void *)TimedTask__SetTimeout,
    /* +0x070 playSound */ (void *)TimedTask__PlaySound,
    /* +0x074 togglePause */ NULL,
    /* +0x078 slot78 */ NULL,
    /* +0x07C onTimedOut */ (void *)DayTask__OnTimedOut,
    /* +0x080 onDreamSysNotify */ (void *)DayTask__OnDreamSysNotify,
    /* +0x084 onObjMNotify */ DayTask__OnObjMNotify,
};
/* clang-format on */

/* The viewpoint and view-reference points DayTask__OnInit hands the
 * viewport's attachViewChild. */
LongVec3 sDayViewPoint = {0, -1200, 0};
LongVec3 sDayViewRef = {0, -1200, 10000};

/* TimedTask's method table, class id 0x230: +0x074..+0x07C are NULL here
 * and filled by its subclasses. */
/* clang-format off */
TimedTaskMethods gTimedTaskMethods = {
    /* +0x000 header */ 0x230,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ TimedTask__TimedTask,
    /* +0x00C finalize */ TimedTask__Finalize,
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
    /* +0x038 onNotify */ (void *)IntermediateBase__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 resetCounters */ TimedTask__CancelTimeout,
    /* +0x044 init */ TimedTask__Init,
    /* +0x048 deinit */ TimedTask__Deinit,
    /* +0x04C onInit */ NULL,
    /* +0x050 onDeinit */ NULL,
    /* +0x054 onDrawSystemEvent */ (void *)IntermediateBase__OnDrawSystemEvent,
    /* +0x058 onPadEvent */ (void *)TimedTask__NoOpOnPadEvent,
    /* +0x05C update */ TimedTask__CheckTimeout,
    /* +0x060 setState */ TimedTask__SetState,
    /* +0x064 onStart */ (void *)IntermediateBase__OnStart,
    /* +0x068 onStop */ (void *)IntermediateBase__OnStop,
    /* +0x06C setTimeout */ TimedTask__SetTimeout,
    /* +0x070 playSound */ TimedTask__PlaySound,
    /* +0x074 togglePause */ NULL,
    /* +0x078 slot78 */ NULL,
    /* +0x07C onTimedOut */ NULL,
};
/* clang-format on */

/* StageMap's method table, class id 0x114. */
/* clang-format off */
StageMapMethods gStageMapMethods = {
    /* +0x000 header */ 0x114,
    /* +0x004 release */ (void *)BasicClass__Release,
    /* +0x008 ctor */ (void *)StageMap__StageMap,
    /* +0x00C finalize */ StageMap__Finalize,
    /* +0x010 addChild */ (void *)SceneNode__AddChild,
    /* +0x014 removeChild */ (void *)SceneNode__RemoveChild,
    /* +0x018 removeAllChildren */ (void *)SceneNode__RemoveAllChildren,
    /* +0x01C getNextChild */ (void *)BasicClass__GetNextChild,
    /* +0x020 addParentRef */ (void *)BasicClass__AddParentRef,
    /* +0x024 removeParentRef */ (void *)BasicClass__RemoveParentRef,
    /* +0x028 clearParentRefs */ (void *)BasicClass__ClearParentRefs,
    /* +0x02C getNextParentRef */ (void *)BasicClass__GetNextParentRef,
    /* +0x030 notifyParents */ (void *)BasicClass__NotifyParents,
    /* +0x034 slot34 */ BasicClass__NoOpSlot34,
    /* +0x038 onNotify */ (void *)StageMap__OnNotify,
    /* +0x03C slot3C */ NULL,
    /* +0x040 reset */ StageMap__Reset,
    /* +0x044 updateRotation */ (void *)SceneNode__UpdateRotation,
    /* +0x048 updateScale */ (void *)SceneNode__UpdateScale,
    /* +0x04C attachToParent */ (void *)SceneNode__AttachToParent,
    /* +0x050 detachFromParent */ (void *)SceneNode__DetachFromParent,
    /* +0x054 detachAttachedChildren */ (void *)SceneNode__DetachAttachedChildren,
    /* +0x058 getNextAttachedChild */ (void *)SceneNode__GetNextAttachedChild,
    /* +0x05C finalizeHook */ (void *)SceneNode__NoOpFinalizeHook,
    /* +0x060 setDisplay */ (void *)SceneNode__SetDisplay,
    /* +0x064 setSemiTransOn */ (void *)SceneNode__SetSemiTrans,
    /* +0x068 setSemiTransRate */ (void *)SceneNode__SetSemiTransRate,
    /* +0x06C setLighting */ (void *)SceneNode__SetLighting,
    /* +0x070 setLightMode */ (void *)SceneNode__SetLightMode,
    /* +0x074 setLightDim */ (void *)SceneNode__SetLightDim,
    /* +0x078 setUseZ */ (void *)SceneNode__SetUseZ,
    /* +0x07C setSubdivision */ (void *)SceneNode__SetSubdivision,
    /* +0x080 setBackClip */ (void *)SceneNode__SetBackClip,
    /* +0x084 getRotMatrix */ (void *)SceneNode__GetRotMatrix,
    /* +0x088 notifyWithHull */ (void *)StageMap__OnSlotEvent,
    /* +0x08C getModelHull */ (void *)SceneNode__GetModelHull,
    /* +0x090 transformAndNotifyParents */ (void *)SceneNode__TransformAndNotifyParents,
    /* +0x094 onPadEvent */ (void *)SceneNode__OnPadEvent,
    /* +0x098 update */ (void *)StageMap__UpdateIfEnabled,
    /* +0x09C dispatchLinkCommand */ (void *)StageMap__DispatchLinkCommand,
    /* +0x0A0 tryAttachNearby */ (void *)SceneNode__TryAttachNearby,
    /* +0x0A4 composeAndApplyRotation */ (void *)SceneNode__ComposeAndApplyRotation,
    /* +0x0A8 checkBoundsOverlap */ (void *)SceneNode__CheckBoundsOverlap,
    /* +0x0AC raycastHullAgainstFaces */ (void *)SceneNode__RaycastHullAgainstFaces,
    /* +0x0B0 slotB0 */ SceneNode__NoOpSlotB0,
    /* +0x0B4 addToActorParents */ (void *)SceneNode__AddToActorParents,
    /* +0x0B8 getLight */ (void *)LightRig__GetLight,
    /* +0x0BC setAmbientColor */ (void *)LightRig__SetAmbientColor,
    /* +0x0C0 unloadAllSlots */ StageMap__UnloadAllSlots,
    /* +0x0C4 setChildParams */ StageMap__SetChildParams,
    /* +0x0C8 setCallback */ StageMap__SetCallback,
    /* +0x0CC setAcceptedTags */ StageMap__SetAcceptedTags,
    /* +0x0D0 forwardAcceptedCommand */ StageMap__ForwardAcceptedCommand,
    /* +0x0D4 getCurrentCellKey */ (void *)StageMap__GetCurrentCellKey,
    /* +0x0D8 slotD8 */ StageMap__NoOpSlotD8,
    /* +0x0DC setGridSpan */ StageMap__SetGridSpan,
    /* +0x0E0 setConfig */ StageMap__SetConfig,
    /* +0x0E4 setTargetAndLoadChunks */ StageMap__SetTargetAndLoadChunks,
    /* +0x0E8 computeCellOffsets */ StageMap__ComputeCellOffsets,
    /* +0x0EC enable */ StageMap__Enable,
    /* +0x0F0 disable */ StageMap__Disable,
    /* +0x0F4 updateFootprintTracking */ StageMap__UpdateFootprintTracking,
    /* +0x0F8 loadChunksAround */ (void *)StageMap__LoadChunksAround,
    /* +0x0FC applyChunkLoads */ StageMap__ApplyChunkLoads,
    /* +0x100 onDrawSystemEvent */ StageMap__OnDrawSystemEvent,
    /* +0x104 populateSlotCells */ StageMap__PopulateSlotCells,
    /* +0x108 clearSlotCells */ StageMap__ClearSlotCells,
    /* +0x10C getTargetDescriptor */ StageMap__GetTargetDescriptor,
    /* +0x110 computeFootprintDescriptor */ StageMap__ComputeFootprintDescriptor,
    /* +0x114 getLastEventSlotChunk */ StageMap__GetLastEventSlotChunk,
    /* +0x118 findSlotByNeighbour */ StageMap__FindSlotByNeighbour,
    /* +0x11C findSlotForPosition */ StageMap__FindSlotForPosition,
    /* +0x120 findSlotIndexByNeighbour */ StageMap__FindSlotIndexByNeighbour,
    /* +0x124 findSlotIndexByChunk */ StageMap__FindSlotIndexByChunk,
    /* +0x128 refreshFootprint */ StageMap__RefreshFootprint,
    /* +0x12C applyToSenderFootprint */ StageMap__ApplyToSenderFootprint,
    /* +0x130 getUnk1CC */ StageMap__GetUnk1CC,
    /* +0x134 setBounds */ StageMap__SetBounds,
    /* +0x138 startScaleRamp */ StageMap__StartScaleRamp,
    /* +0x13C stepScaleRamp */ StageMap__StepScaleRamp,
    /* +0x140 endScaleRamp */ StageMap__EndScaleRamp,
};
/* clang-format on */

/* The origin StageMap's ctor takes when it is given none. */
LongVec3 sDefaultOrigin = {0, 0, 0};

/* clang-format off */
/* Indexed by neighbour key (ChunkSlotSpec::neighbour, enum ChunkNeighbour)
 * in StageMap__LoadChunksAround: the world offset of that neighbour's
 * cellParent from the centre position. */
LongVec3 sNeighbourOffsets[CHUNK_NEIGHBOUR_COUNT] = {
    /* PREV_ROW_LO */ {-40960, 0, -61440},
    /* PREV_ROW_HI */ {     0, 0, -61440},
    /* PREV_COL    */ {-61440, 0, -20480},
    /* CENTRE      */ {-20480, 0, -20480},
    /* NEXT_COL    */ { 20480, 0, -20480},
    /* NEXT_ROW_LO */ {-40960, 0,  20480},
    /* NEXT_ROW_HI */ {     0, 0,  20480},
};

/* CHUNK_NEIGHBOUR_BIT(key) for each neighbour key: what
 * StageMap__ComputeChunkLoadEntry tests against ComputeNeighbourMask's
 * result. */
s32 sNeighbourBits[CHUNK_NEIGHBOUR_COUNT] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40};

/* The chunk-index steps to the seven chunks around a centre chunk, by
 * neighbour key (ChunkNeighbourDelta, include/stage_map.h). */
ChunkNeighbourDelta sChunkNeighbourDeltas[CHUNK_NEIGHBOUR_COUNT] = {
    /*               rows  odd  even */
    /* PREV_ROW_LO */ {-1,  -1,  0},
    /* PREV_ROW_HI */ {-1,   0,  1},
    /* PREV_COL    */ { 0,  -1, -1},
    /* CENTRE      */ { 0,   0,  0},
    /* NEXT_COL    */ { 0,   1,  1},
    /* NEXT_ROW_LO */ { 1,  -1,  0},
    /* NEXT_ROW_HI */ { 1,   0,  1},
};

/* Indexed by the neighbour key (LbdFile::elemKey) of the slot holding the
 * target, read signed by StageMap__UpdateFootprintTracking: 0 for the centre
 * (no reload), else which of sFootprintResultPtrTable's spec tables to
 * load. The byte is also UpdateFootprintTracking's return value. */
s8 sFootprintResultRemap[8] = {1, 2, 3, 0, 4, 5, 6, 0};

/* The spec table SetTargetAndLoadChunks passes to loadChunksAround, one
 * {neighbour, load} per slot: every slot (re)loaded. */
ChunkSlotSpec sDefaultTargetSpecs[CHUNK_NEIGHBOUR_COUNT] = {
    {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}, {6, 1},
};

/* The spec tables UpdateFootprintTracking passes when the target has moved
 * into the named neighbour: each slot's new neighbour key, and whether it
 * (re)loads; three slots load in each, the four whose chunk is still in
 * reach do not. */
ChunkSlotSpec sFootprintSpecsPrevRowLo[CHUNK_NEIGHBOUR_COUNT] = {
    {3, 0}, {4, 0}, {5, 0}, {6, 0}, {0, 1}, {1, 1}, {2, 1},
};
ChunkSlotSpec sFootprintSpecsPrevRowHi[CHUNK_NEIGHBOUR_COUNT] = {
    {2, 0}, {3, 0}, {0, 1}, {5, 0}, {6, 0}, {1, 1}, {4, 1},
};
ChunkSlotSpec sFootprintSpecsPrevCol[CHUNK_NEIGHBOUR_COUNT] = {
    {1, 0}, {0, 1}, {3, 0}, {4, 0}, {2, 1}, {6, 0}, {5, 1},
};
ChunkSlotSpec sFootprintSpecsNextCol[CHUNK_NEIGHBOUR_COUNT] = {
    {1, 1}, {0, 0}, {4, 1}, {2, 0}, {3, 0}, {6, 1}, {5, 0},
};
ChunkSlotSpec sFootprintSpecsNextRowLo[CHUNK_NEIGHBOUR_COUNT] = {
    {2, 1}, {5, 1}, {0, 0}, {1, 0}, {6, 1}, {3, 0}, {4, 0},
};
ChunkSlotSpec sFootprintSpecsNextRowHi[CHUNK_NEIGHBOUR_COUNT] = {
    {4, 1}, {5, 1}, {6, 1}, {0, 0}, {1, 0}, {2, 0}, {3, 0},
};

/* By UpdateFootprintTracking's result (sFootprintResultRemap). */
ChunkSlotSpec *sFootprintResultPtrTable[7] = {
    NULL,
    sFootprintSpecsPrevRowLo, sFootprintSpecsPrevRowHi, sFootprintSpecsPrevCol,
    sFootprintSpecsNextCol, sFootprintSpecsNextRowLo, sFootprintSpecsNextRowHi,
};

/* The rectangle StageMap__InitFootprintRect copies into rects[index] before
 * setting its slotIndex: no slot (-1), the whole 20 x 20 cells from (0, 0). */
CellRect sFullSlotRect = {-1, 0, 0, 20, 20};

/* The four scale steps (x/y/z) StartScaleRamp picks for `scaleStep`: y
 * +1/64 and +1/4 for a positive rate (fast 0, nonzero), -1/64 and -1/4
 * otherwise; x and z 0/1. */
Ratio16 sScaleStepUpSlow[3]   = {{0, 1}, { 1, 64}, {0, 1}};
Ratio16 sScaleStepUpFast[3]   = {{0, 1}, { 1,  4}, {0, 1}};
Ratio16 sScaleStepDownSlow[3] = {{0, 1}, {-1, 64}, {0, 1}};
Ratio16 sScaleStepDownFast[3] = {{0, 1}, {-1,  4}, {0, 1}};

/* The scale ResetCellScale sets on every cell. */
Ratio16 sScaleOne[3] = {{1, 1}, {1, 1}, {1, 1}};
/* clang-format on */

DayTask *New_DayTask(IntermediateBaseInitArgs *initArgs, DreamSys *dreamSys, s32 syncDriver) {
    DayTask *self;

    self = BMemPMgrAlloc(sizeof(DayTask));
    if (self != NULL) {
        GetDayTaskMethods()->ctor(self, initArgs, dreamSys, syncDriver);
        return self;
    }
    return NULL;
}

void DayTask__DayTask(DayTask *self, IntermediateBaseInitArgs *initArgs, DreamSys *dreamSys,
                      s32 syncDriver) {
    ResourceRequest req; /* MATCHING: mode is never set; a bare ResourceSource shrinks the frame */
    char *vabPath;

    GetTimedTaskMethods()->ctor((TimedTask *)self, GetSoundEffectDir(0), 0);
    self->methods = GetDayTaskMethods();
    InitDreamAux();
    self->etcTim = New_TimImage((char *)sEtcTimPath);
    ((TimImageUploadFn)self->etcTim->methods->processBuffer)(self->etcTim);
    self->etcTim->methods->freeBuffer(self->etcTim);
    req.src.buffer = NULL;
    req.src.name = (char *)sDreamerTmdPath;
    self->dreamerTmd = New_LinkResource(&req.src);
    vabPath = PickSoundBank(0);
    self->bgm = New_WBgm(vabPath, NULL, 1);
    RegisterRecordTableFiles(1);
    SetActiveDataSourceDriverMode(syncDriver == 0, 1, 1);
    self->initArgs = initArgs;
    initArgs->viewport = (BasicClass *)New_NodeGuardedViewport();
    initArgs->frameClock = (BasicClass *)New_FrameClock();
    initArgs->lightRig = (BasicClass *)New_StageMap(NULL, 1);
    self->dreamSys = dreamSys;
    self->methods->addChild(self, (BasicClass *)dreamSys);
    dreamSys->methods->setSoundObj(dreamSys, (VabStreamObj *)self->sound);
    dreamSys->methods->setEtcTim(dreamSys, self->etcTim);
    self->methods->resetCounters(self);
}

void DayTask__Finalize(DayTask *self) {
    IntermediateBaseInitArgs *args = self->initArgs;
    BasicClass *obj;

    self->methods->removeChild(self, (BasicClass *)self->dreamSys);
    obj = args->lightRig;
    args->lightRig = obj->methods->release(obj);
    obj = args->frameClock;
    args->frameClock = obj->methods->release(obj);
    obj = args->viewport;
    args->viewport = obj->methods->release(obj);
    self->bgm->methods->release(self->bgm);
    self->dreamerTmd->methods->release(self->dreamerTmd);
    self->etcTim->methods->release(self->etcTim);
    ReleaseDreamAuxModels();
    GetTimedTaskMethods()->finalize((TimedTask *)self);
}

void DayTask__OnNotify(DayTask *self, BasicClass *sender, s32 event) {
    s32 tag;

    GetTimedTaskMethods()->onNotify((TimedTask *)self, sender, event);
    tag = sender->methods->header;
    if ((tag & CLASS_ID_LEVEL4_MASK) == DREAMSYS_CLASS_ID) {
        self->methods->onDreamSysNotify(self, sender, event);
    } else if ((tag & CLASS_ID_LEVEL5_MASK) == OBJM_CLASS_ID) {
        self->methods->onObjMNotify(self, sender, event);
    }
}

void DayTask__ResetPhase(DayTask *self) {
    self->phase = DAYTASK_PHASE_IDLE;
}

s32 DayTask__Init(DayTask *self) {
    DreamSys *dreamSys = self->dreamSys;

    dreamSys->methods->addChild(dreamSys, self->initArgs->pad);
    dreamSys->methods->addChild(dreamSys, self->initArgs->frameClock);
    dreamSys->methods->setViewport(dreamSys, (Viewport *)self->initArgs->viewport);
    return GetTimedTaskMethods()->init((TimedTask *)self, self->initArgs, INTERMEDIATEBASE_INIT_RUN);
}

void DayTask__Deinit(DayTask *self) {
    DreamSys *dreamSys = self->dreamSys;

    GetTimedTaskMethods()->deinit((TimedTask *)self);
    dreamSys->methods->setViewport(dreamSys, 0);
    dreamSys->methods->removeChild(dreamSys, self->initArgs->pad);
    dreamSys->methods->removeChild(dreamSys, self->frameClock);
}

void DayTask__OnInit(DayTask *self) {
    DrawSystem *drawSystem;
    Viewport *vp;
    SceneNode *fadeBox;
    ScreenDims *size;

    drawSystem = (DrawSystem *)self->initArgs->drawSystem;
    vp = (Viewport *)self->viewport;
    size = drawSystem->methods->getDims(drawSystem, NULL);
    vp->methods->setScreenSize(vp, size);
    fadeBox = vp->methods->getFadeBox(vp);
    fadeBox->methods->setDisplay(fadeBox, 1);
    vp->methods->setMaxPackets(vp, 1200);
    vp->methods->attachViewChild(vp, (BasicClass *)self->dreamSys, &sDayViewPoint, &sDayViewRef, 0);
    vp->methods->initOt(vp);
    self->phase = DAYTASK_PHASE_READY;
}

void DayTask__OnDeinit(DayTask *self) {
    Viewport *vp = (Viewport *)self->viewport;

    vp->methods->deinitOt(vp);
    vp->methods->detachViewChild(vp);
}

void DayTask__AdvancePhase(DayTask *self, BasicClass *sender, s32 event) {
    s32 result;

    GetTimedTaskMethods()->onDrawSystemEvent((TimedTask *)self, sender, event);
    if (event == DRAWSYSTEM_EVENT_VSYNC && self->phase != DAYTASK_PHASE_RUNNING) {
        switch (self->phase) {
            case DAYTASK_PHASE_READY:
                result = self->dreamSys->methods->startDay(self->dreamSys);
                if (result < 0) {
                    self->dreamSys->methods->endDay(self->dreamSys, DAY_OUTCOME_ENDED);
                    self->result = DAYTASK_RESULT_CINEMATIC;
                    self->methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
                    return;
                }
                DayTask__StartObjM(self, result);
                break;
            case DAYTASK_PHASE_RUNNING: /* MATCHING: unreachable (phase != RUNNING above), but removing it changes the switch's code */
                break;
            case DAYTASK_PHASE_REPLACE_OBJM:
                self->objM->methods->deinit(self->objM);
                self->objM->methods->release(self->objM);
                result = self->dreamSys->methods->getCurrentStage(self->dreamSys);
                DayTask__StartObjM(self, result);
                break;
        }
    }
}

void DayTask__StartObjM(DayTask *self, s32 stage) {
    self->objM = New_ObjM(self->sound, self->bgm, self->etcTim, self->dreamerTmd, stage);
    self->methods->addChild(self, (BasicClass *)self->objM);
    self->objM->methods->init(self->objM, self->initArgs, (s32)self->dreamSys);
    self->phase = DAYTASK_PHASE_RUNNING;
}

void DayTask__OnTimedOut(void) {}

void DayTask__OnDreamSysNotify(void) {}

void DayTask__OnObjMNotify(DayTask *self, BasicClass *sender, s32 event) {
    CinematicCall cinematic;
    s32 result;

    switch (event) {
        case OBJM_STATE_TIME_UP:
            self->objM->methods->deinit(self->objM);
            self->objM->methods->release(self->objM);
            result = self->dreamSys->methods->endDay(self->dreamSys, DAY_OUTCOME_ENDED);
            if (result == 0) {
                cinematic = self->dreamSys->methods->getCinematic(self->dreamSys);
                self->result = cinematic.entry < 0 ? DAYTASK_RESULT_ENDED : DAYTASK_RESULT_CINEMATIC;
            } else {
                self->result = DAYTASK_RESULT_CLOSED;
            }
            self->methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
            break;
        case OBJM_STATE_LINK_DYNAMIC:
        case OBJM_STATE_LINK_WALL:
        case OBJM_STATE_LINK_FLASHBACK:
        case OBJM_STATE_LINK_TUNNEL:
        case OBJM_STATE_LINK_STAGE_TIMER:
            self->phase = DAYTASK_PHASE_REPLACE_OBJM;
            break;
        case OBJM_NOTIFY_CLOSE:
        case OBJM_NOTIFY_CLOSE_NEW_GAME:
            self->objM->methods->deinit(self->objM);
            self->objM->methods->release(self->objM);
            self->dreamSys->methods->endDay(
                self->dreamSys, event != OBJM_NOTIFY_CLOSE ? DAY_OUTCOME_NEW_GAME : DAY_OUTCOME_CLOSED);
            self->result = DAYTASK_RESULT_CLOSED;
            self->methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
            break;
    }
}

DayTaskMethods *GetDayTaskMethods(void) {
    return &gDayTaskMethods;
}

/* How many times RegisterRecordTableFiles has run (a call with `all` set
 * counts as two), and how many records its first, half-table batch took. */
extern s32 sRecordRegisterCalls;
extern s32 sRecordFirstBatchCount;

s32 RegisterRecordTableFiles(s32 all) {
    s32 count;
    CdFileEntry *table;
    s32 prev;
    s32 result;

    table = GetRecordTable(&count);
    prev = sRecordRegisterCalls;
    sRecordRegisterCalls = prev + 1;

    switch (prev + 1) {
        case 1:
            if (all != 0) {
                sRecordRegisterCalls = prev + 2;
            } else {
                count = count / 2;
                sRecordFirstBatchCount = count;
            }
            break;
        case 2:
            count = count - sRecordFirstBatchCount;
            break;
        default:
            count = 0;
            break;
    }

    while ((result = RegisterFileTableEntries(table, count)) == 0) {
    }
    return result;
}

TimedTask *New_TimedTask(char *soundBankPath, BasicClass *sound) {
    TimedTask *self;

    self = BMemPMgrAlloc(sizeof(TimedTask));
    if (self != NULL) {
        GetTimedTaskMethods()->ctor(self, soundBankPath, sound);
        return self;
    }
    return NULL;
}

void TimedTask__TimedTask(TimedTask *self, char *soundBankPath, BasicClass *sound) {
    GetIntermediateBaseMethods()->ctor((IntermediateBase *)self);
    self->methods = GetTimedTaskMethods();
    if (soundBankPath != NULL) {
        self->sound = (BasicClass *)New_VabStreamObj(soundBankPath);
    } else {
        self->sound = sound;
    }
    self->soundBankPath = soundBankPath;
    self->methods->resetCounters(self);
}

void TimedTask__Finalize(TimedTask *self) {
    if (self->soundBankPath != NULL) {
        self->sound->methods->release(self->sound);
    }
    GetIntermediateBaseMethods()->finalize((IntermediateBase *)self);
}

void TimedTask__CancelTimeout(TimedTask *self) {
    self->methods->setTimeout(self, -1);
}

s32 TimedTask__Init(TimedTask *self, IntermediateBaseInitArgs *args, s32 mode) {
    self->result = TIMEDTASK_RESULT_DONE;
    GetIntermediateBaseMethods()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

void TimedTask__Deinit(TimedTask *self) {
    GetIntermediateBaseMethods()->deinit((IntermediateBase *)self);
}

void TimedTask__NoOpOnPadEvent(void) {}

void TimedTask__CheckTimeout(TimedTask *self, BasicClass *sender, s32 event) {
    GetIntermediateBaseMethods()->update((IntermediateBase *)self, sender, event);
    if ((u32)self->frameCounter > (u32)self->timeoutFrames) {
        self->methods->setState(self, TIMEDTASK_STATE_TIMED_OUT);
    }
}

void TimedTask__SetState(TimedTask *self, s32 state) {
    GetIntermediateBaseMethods()->setState((IntermediateBase *)self, state);
    if (state == TIMEDTASK_STATE_TIMED_OUT) {
        self->result = TIMEDTASK_RESULT_TIMED_OUT;
        self->methods->onTimedOut(self);
    }
}

void TimedTask__SetTimeout(TimedTask *self, s32 timeout) {
    self->timeoutFrames = (timeout < 0) ? timeout : timeout * TIMEDTASK_TIMEOUT_UNIT_FRAMES;
}

/*
 * StageMap's first run of methods: its life (allocator, ctor, Finalize,
 * OnNotify, Reset, OnSlotEvent, the per-tick UpdateIfEnabled,
 * UnloadAllSlots), the setters ObjM configures it through, and the path a
 * command takes to the cells: DispatchLinkCommand, ForwardAcceptedCommand
 * (the acceptedTags filter), ApplyToSenderFootprint (a 3 x 3 cell
 * footprint around the sender) and DispatchToRectCells. TimedTask's last two
 * functions come first.
 */

/* `sound` may be the ctor's own argument; as a VabStreamObj, its +0x080 is
 * VabStreamObj__PlayTone. */
void TimedTask__PlaySound(TimedTask *self, s32 tone) {
    VabStreamObj *sound = (VabStreamObj *)self->sound;

    if (sound != NULL) {
        sound->methods->playTone(sound, tone, TIMEDTASK_TONE_VOLUME, TIMEDTASK_TONE_VOLUME);
    }
}

TimedTaskMethods *GetTimedTaskMethods(void) {
    return &gTimedTaskMethods;
}

StageMap *New_StageMap(LongVec3 *origin, s32 autoLoad) {
    StageMap *self;

    self = BMemPMgrAlloc(sizeof(StageMap));
    if (self != NULL) {
        GetStageMapMethods()->ctor(self, origin, autoLoad);
        return self;
    }
    return NULL;
}

void StageMap__StageMap(StageMap *self, LongVec3 *origin, s32 autoLoad) {
    s32 i;
    ChunkSlot *slot;
    GridCell *cell;
    GridCell **cells;
    GridCell **cursor;
    GridCell **end;
    LongVec3 pos;

    GetLightRigMethods()->ctor((LightRig *)self);
    self->methods = GetStageMapMethods();

    if (origin != NULL) {
        self->origin = *origin;
    } else {
        self->origin = sDefaultOrigin;
    }

    self->loadsPending = 0;
    self->pendingLoadCount = 0;
    self->chunksLoaded = 0;
    self->enabled = 0;
    self->target = NULL;
    self->acceptedTags = NULL;
    self->scaleRampTicks = 0;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];

        slot->loader = New_LbdFile();
        slot->loader->freeGuard = (slot->loader->buffer != NULL);
        slot->loader->elemKey = i;
        slot->loader->methods->setAutoLoadData(slot->loader, autoLoad);

        slot->heldObj = NULL;
        slot->unk18 = 0;
        slot->neighbour = i;
        slot->loadPending = 0;

        slot->placements = New_PlacementGrid(0);
        slot->cellParent = New_GridCell();
        slot->cellParent->methods->attachToParent(slot->cellParent, (SceneNode *)self, &self->origin);

        slot->cells = (GridCell **)BMemPMgrAlloc(STAGE_SLOT_CELLS * sizeof(GridCell *));
        if (slot->cells == NULL) {
            return;
        }

        pos.x = STAGE_CELL_SIZE / 2;
        pos.y = 0;
        pos.z = STAGE_CELL_SIZE / 2;

        /* MATCHING: `end` from `cells` before `cursor = cells`, or slot->cells loads differently */
        cells = slot->cells;
        end = cells + STAGE_SLOT_CELLS;
        cursor = cells;
        while (cursor < end) {
            cell = New_GridCell();
            *cursor = cell;
            cell->methods->attachToParent(cell, (SceneNode *)slot->cellParent, &pos);

            /* Cell centres, row by row; the test lets a row reach
             * STAGE_CHUNK_CELLS + 1 positions before it wraps. */
            pos.x += STAGE_CELL_SIZE;
            if (pos.x > STAGE_CHUNK_SIZE + STAGE_CELL_SIZE / 2) {
                pos.x = STAGE_CELL_SIZE / 2;
                pos.z += STAGE_CELL_SIZE;
            }

            /* MATCHING: each use reloads *cursor. */
            cell = *cursor;
            cell->methods->setLightMode(cell, 1); /* GsFOG */
            cell = *cursor;
            cursor++;
            cell->attribute |= GsDOFF;
        }
    }

    self->methods->addChild(self, (BasicClass *)GetDrawSystem());
    self->methods->reset(self);
}

void StageMap__Finalize(StageMap *self) {
    s32 i;
    ChunkSlot *slot;
    GridCell *cell;
    GridCell **cells;
    GridCell **cursor;
    GridCell **end;

    self->methods->removeChild(self, (BasicClass *)GetDrawSystem());

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);

        if (slot->loader != NULL) {
            slot->loader->methods->release(slot->loader);
        }

        if (slot->placements != NULL) {
            if (slot->placements->linkResource != NULL) {
                slot->placements->linkResource->methods->release(slot->placements->linkResource);
            }
            slot->placements = slot->placements->methods->release(slot->placements);
        }

        if (slot->cellParent != NULL) {
            slot->cellParent->methods->release(slot->cellParent);
        }

        cells = slot->cells;
        end = cells + STAGE_SLOT_CELLS;
        cursor = cells;
        while (cursor < end) {
            cell = *cursor;
            if (cell != NULL) {
                cell->methods->release(cell);
            }
            cursor++;
        }

        BMemPMgrFree(slot->cells);
    }

    GetLightRigMethods()->finalize((LightRig *)self);
}

void StageMap__OnNotify(StageMap *self, BasicClass *sender, s32 command) {
    GetSceneNodeMethods()->onNotify((SceneNode *)self, sender, command);

    if ((sender->methods->header & CLASS_ID_ROOT_MASK) == DRAWSYSTEM_CLASS_ID) {
        self->methods->onDrawSystemEvent(self, sender, command);
    }
}

extern s32 sDefaultGridSpan;

void StageMap__Reset(StageMap *self) {
    self->config = NULL;
    self->acceptedTags = NULL;
    self->rectCount = 0;
    self->methods->setGridSpan(self, sDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}

void StageMap__OnSlotEvent(StageMap *self, s32 command, ChunkSlot *slot) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, command);

    switch (command) {
        case STAGEMAP_EVENT_SLOT_RELEASE:
            if (slot->heldObj != NULL) {
                slot->heldObj = slot->heldObj->methods->release(slot->heldObj);
            }
            /* fall through */
        case STAGEMAP_EVENT_SLOT_DATA_READY:
            self->lastEventSlot = slot;
            self->methods->notifyParents(self, command);
            break;
    }
}

void StageMap__UpdateIfEnabled(StageMap *self) {
    if (self->enabled) {
        self->methods->updateFootprintTracking(self);
        self->methods->stepScaleRamp(self);
    }
}

void StageMap__DispatchLinkCommand(StageMap *self, BasicClass *sender, s32 command) {
    if ((sender->methods->header & CLASS_ID_LEVEL2_MASK) == ACTOR_CLASS_ID) {
        self->methods->forwardAcceptedCommand(self, sender, command);
    }
}

void StageMap__UnloadAllSlots(StageMap *self) {
    s32 i;
    ChunkSlot *slot;
    PlacementGrid *placements;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        slot->loader->methods->cancelRequests(slot->loader);
        slot->loadPending = 0;
        self->methods->clearSlotCells(self, slot);
        placements = slot->placements;
        if (placements->linkResource != NULL) {
            placements->linkResource =
                placements->linkResource->methods->release(placements->linkResource);
        }
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);
        slot->loader->methods->releaseDataBlock(slot->loader);
    }

    self->chunksLoaded = 0;
    self->pendingLoadCount = 0;
    self->methods->endScaleRamp(self);
}

void StageMap__SetChildParams(StageMap *self, s32 count, s32 dirs, s32 colors) {
    s32 i;
    FlatLightObj *light;

    for (i = 0; i < count; i++) {
        light = (FlatLightObj *)self->methods->getLight(self, i);
        light->methods->setColor(light, 1, (ColorRgb *)colors);
        colors += sizeof(ColorRgb);
        light->methods->setDirection(light, 1, (s16 *)dirs);
        dirs += 3 * sizeof(s16);
    }
}

void StageMap__SetCallback(StageMap *self, ChunkFileFn fn, void *ctx) {
    self->chunkFileFn = fn;
    self->chunkFileCtx = ctx;
}

void StageMap__SetAcceptedTags(StageMap *self, s32 *tags) {
    self->acceptedTags = tags;
}

void StageMap__ForwardAcceptedCommand(StageMap *self, void *sender, s32 command) {
    s32 *tag;
    u8 unused[24]; /* MATCHING: the frame is 24 bytes larger than the locals need */

    switch (command) {
        case SCENENODE_EVENT_HULL_FIRST:
        case SCENENODE_EVENT_HULL_LAST:
        case ACTOR_EVENT_UNSWEPT:
        case ACTOR_EVENT_MOVED_Z:
        case ACTOR_EVENT_MOVED_X:
        case ACTOR_EVENT_MOVED_Y:
            break;
        default:
            return;
    }

    tag = self->acceptedTags;
    if (tag == NULL)
        return;
    if (*tag == 0)
        return;

    do {
        if (*tag == ((BasicClass *)sender)->methods->header) {
            self->methods->applyToSenderFootprint(self, sender, command);
        }
        tag++;
    } while (*tag != 0);
}

void StageMap__ApplyToSenderFootprint(StageMap *self, SceneNode *sender, s32 command) {
    SplitLongVec3 *pos;
    CellRectSet savedRects;
    Descriptor10Ext desc;
    s32 savedRectCount;

    if (sender->parent != NULL) {
        pos = (SplitLongVec3 *)sender->coord2->workm.t;
    } else {
        pos = NULL;
    }

    if (self->methods->computeFootprintDescriptor(self, &desc, pos) != 0) {
        return;
    }

    savedRectCount = self->rectCount;
    savedRects = self->rects;

    if (self->config->isVertical == 0) {
        StageMap__SetFootprintFromCell(self, &desc, 3);
    } else {
        StageMap__SetFootprintRect(self, &desc, 3);
    }

    StageMap__DispatchToRectCells(self, sender, command);

    self->rectCount = savedRectCount;
    self->rects = savedRects;
}

void StageMap__SetFootprintFromCell(StageMap *self, Descriptor10Ext *desc, s32 span) {
    s16 row;

    self->footprintCol = desc->base.b2 - 1;
    row = desc->base.b3 - 1;
    self->footprintWidth = span;
    self->footprintHeight = span;
    self->footprintRow = row;
    StageMap__BuildFootprintRects(self);
}

/* A cell on the low edge (0) or the high edge loses one row/column there. */
/* MATCHING: the edge tests read copies taken before the decrement; the height is `span` */
void StageMap__SetFootprintRect(StageMap *self, Descriptor10Ext *desc, s32 span) {
    s32 col;
    s32 row;
    s32 width;
    s32 origCol;
    s32 origRow;

    width = span;
    col = desc->base.b2;
    row = desc->base.b3;
    origCol = col;
    origRow = row;

    if (col == 0) {
        width = span - 1;
    } else {
        col--;
    }
    if (origCol == STAGE_CHUNK_CELLS - 1) {
        width--;
    }

    if (origRow == 0) {
        span--;
    } else {
        row--;
    }
    if (origRow == STAGE_CHUNK_CELLS - 1) {
        span--;
    }

    self->rectCount = 1;
    self->rects.e[0].slotIndex = self->methods->findSlotIndexByChunk(self, desc->chunkIndex);
    self->rects.e[0].col = col;
    self->rects.e[0].row = row;
    self->rects.e[0].width = width;
    self->rects.e[0].height = span;
}

/* MATCHING: the comma increments go `rect++, i++` and `cell++, col++` */
void StageMap__DispatchToRectCells(StageMap *self, SceneNode *sender, s32 command) {
    s32 i;
    s32 row;
    s32 col;
    CellRect *rect;
    ChunkSlot *slot;
    GridCell **cell;
    GridCell *chained;

    rect = self->rects.e;
    for (i = 0; i < self->rectCount; rect++, i++) {
        slot = &self->slots[rect->slotIndex];
        if (slot->loader->headerReady != 0) {
            cell = (slot->cells + rect->col) + rect->row * STAGE_CHUNK_CELLS;
            for (row = 0; row < rect->height; row++) {
                for (col = 0; col < rect->width; cell++, col++) {
                    /* MATCHING: b0/b1 copied as one halfword, not as two bytes */
                    *(u16 *)&self->curCell = *(u16 *)&self->targetCell;
                    self->curCell.b2 = rect->col + col;
                    self->curCell.b3 = rect->row + row;
                    NotifyGridCell(*cell, sender, command);
                    for (chained = (*cell)->nextInCell; chained != NULL; chained = chained->nextInCell) {
                        NotifyGridCell(chained, sender, command);
                    }
                }
                cell += STAGE_CHUNK_CELLS - rect->width;
            }
        }
    }
}

void NotifyGridCell(GridCell *cell, SceneNode *sender, s32 command) {
    if (cell != NULL && (cell->flags36 & GRIDCELL_FLAG_TAKES_COMMANDS)) {
        cell->methods->onNotify(cell, sender, command);
    }
}

Descriptor10 *StageMap__GetCurrentCellKey(StageMap *self) {
    return &self->curCell;
}

void StageMap__NoOpSlotD8(void) {}

void StageMap__SetGridSpan(StageMap *self, s32 span) {
    self->gridSpan = span;
    self->gridCells = (s16)(span >> STAGE_CELL_SHIFT);
    self->gridHalfCells = (s16)(span >> (STAGE_CELL_SHIFT + 1));
}

void StageMap__SetConfig(StageMap *self, StageGridDimensions *config) {
    self->methods->reset(self);
    self->config = config;
}

/*
 * StageMap's second run of methods: placing a cell descriptor in the world
 * (SetTargetAndLoadChunks, ComputeCellOffsets, ComputeCellWorldOffsets),
 * the per-tick tracking (Enable, Disable, UpdateFootprintTracking), loading
 * the seven slots around a centre chunk (LoadChunksAround through
 * CountPendingLoads), finishing the loads on each VSync
 * (OnDrawSystemEvent), linking a loaded chunk into its slot's cells
 * (PopulateSlotCells, ClearSlotCells), and the queries that turn a position
 * back into a slot and cell (GetTargetDescriptor through
 * FindSlotForPosition). A slot's position is its cellParent's
 * GsCOORDINATE2, read through SplitCoord2.
 */

/* The height of one layer of a vertical grid: FindSlotForPosition gives
 * neighbour key i the y range (-(i + 1) * height, -i * height]. */
#define VERTICAL_LAYER_HEIGHT 2048

s32 StageMap__SetTargetAndLoadChunks(StageMap *self, void *outPos, SceneNode *target, Descriptor10 *cell) {
    s32 chunkCentre[3];
    s32 chunkIndex;

    self->target = target;
    self->targetCell.base = *cell;
    chunkIndex = ComputeCellWorldOffsets(outPos, chunkCentre, self->config, &self->origin, cell);
    return self->methods->loadChunksAround(self, chunkIndex, (LongVec3 *)chunkCentre, sDefaultTargetSpecs);
}

s32 StageMap__ComputeCellOffsets(StageMap *self, void *outPos, void *cell) {
    s32 chunkCentre[3];

    return ComputeCellWorldOffsets(outPos, chunkCentre, self->config, &self->origin, cell);
}

s32 ComputeCellWorldOffsets(s32 *outPos, s32 *chunkPos, StageGridDimensions *dims, LongVec3 *origin,
                            Descriptor10 *cell) {
    s32 row;
    s32 rowSpan;
    s32 chunkIndex;
    s32 x;
    s32 z;
    s32 halfCell;

    if (dims->isVertical == 0) {
        row = cell->b1;
        rowSpan = dims->rows;
        chunkIndex = cell->b0 + dims->columns * row;
    } else {
        rowSpan = 1;
        row = 0;
        chunkIndex = 0;
    }
    x = (origin->x - dims->columns * (STAGE_CHUNK_SIZE / 2)) + cell->b0 * STAGE_CHUNK_SIZE;
    z = origin->z - rowSpan * (STAGE_CHUNK_SIZE / 2);
    chunkPos[0] = x;
    if (row & 1) {
        chunkPos[0] = x - STAGE_CHUNK_SIZE / 2;
    }
    chunkPos[1] = origin->y;
    halfCell = STAGE_CELL_SIZE / 2; /* MATCHING: a local set here; the literal at each use differs */
    chunkPos[2] = z + row * STAGE_CHUNK_SIZE;
    outPos[0] = (cell->b2 << STAGE_CELL_SHIFT) + chunkPos[0] + (cell->h4 + halfCell);
    outPos[1] = cell->h6 + chunkPos[1];
    outPos[2] = (cell->b3 << STAGE_CELL_SHIFT) + chunkPos[2] + (cell->h8 + halfCell);
    chunkPos[0] += STAGE_CHUNK_SIZE / 2;
    chunkPos[2] = chunkPos[2] + STAGE_CHUNK_SIZE / 2;
    return chunkIndex;
}

void StageMap__Enable(StageMap *self) {
    self->enabled = 1;
}

void StageMap__Disable(StageMap *self) {
    self->methods->unloadAllSlots(self);
    self->enabled = 0;
}

s32 StageMap__UpdateFootprintTracking(StageMap *self) {
    Descriptor10Ext desc;
    ChunkSlot *slot;
    s32 neighbour;
    s32 specIndex;
    u16 oldChunk;

    if (self->methods->getTargetDescriptor(self, &desc, 0) == 0) {
        return 0;
    }

    slot = desc.slot;
    neighbour = slot->loader->elemKey;
    specIndex = sFootprintResultRemap[neighbour];

    if (self->config->isVertical == 0) {
        self->methods->loadChunksAround(self, desc.chunkIndex, &desc.chunkCentre,
                                        sFootprintResultPtrTable[specIndex]);
    }

    self->methods->refreshFootprint(self);

    oldChunk = *(u16 *)&self->targetCell;
    self->targetCell = desc;

    if ((s16)oldChunk != *(s16 *)&desc) {
        self->methods->notifyParents(self, STAGEMAP_EVENT_CHUNK_CHANGED);
    }

    return specIndex;
}

void StageMap__LoadChunksAround(StageMap *self, s32 centreChunk, LongVec3 *centrePos,
                                ChunkSlotSpec *specs) {
    s32 columns;
    s32 oddRow;
    s32 onGridMask;
    s32 count;
    s32 i;
    ChunkSlot *slot;
    SplitCoord2 *origin;
    LongVec3 *offset;
    ChunkLoadEntry loads[CHUNK_NEIGHBOUR_COUNT];

    if (specs != 0) {
        columns = self->config->columns;
        oddRow = (centreChunk / columns) & 1;
        onGridMask = StageMap__ComputeNeighbourMask(self, centreChunk, oddRow);

        count = 0;
        for (i = 0; i < CHUNK_NEIGHBOUR_COUNT; i++) {
            slot = self->methods->findSlotByNeighbour(self, i);
            slot->neighbour = specs[i].neighbour;
            if (specs[i].load != 0) {
                offset = &sNeighbourOffsets[specs[i].neighbour];
                origin = (SplitCoord2 *)slot->cellParent->coord2;
                if (self->config->isVertical == 0) {
                    origin->tx.w = centrePos->x + offset->x;
                    origin->ty = centrePos->y;
                    origin->tz.w = centrePos->z + offset->z;
                } else {
                    origin->tx.w = centrePos->x - (STAGE_CHUNK_SIZE / 2);
                    origin->ty = centrePos->y + offset->y;
                    origin->tz.w = centrePos->z - (STAGE_CHUNK_SIZE / 2);
                }
                slot->cellParent->coord2->flg = 0;
                StageMap__ComputeChunkLoadEntry(self, &loads[count], columns, oddRow, centreChunk,
                                                onGridMask, specs[i].neighbour);
                count++;
            }
        }

        for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
            slot = &self->slots[i]; /* MATCHING: the first loop's `slot`; a second local differs */
            slot->loader->elemKey = slot->neighbour;
        }

        self->methods->applyChunkLoads(self, loads, count);
    }
}

s32 StageMap__ComputeNeighbourMask(StageMap *self, s32 chunk, s32 oddRow) {
    StageGridDimensions *dims;
    s32 columns;
    s32 isVertical;
    s32 rows;
    s32 offGrid;
    s32 i;

    dims = self->config;
    columns = dims->columns;
    isVertical = dims->isVertical;
    rows = dims->rows;
    if (isVertical == 0) {
        offGrid = (chunk < columns) ? CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_LO) |
                                          CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_HI)
                                    : 0;
        if (chunk >= columns * (rows - 1)) {
            offGrid |= CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_LO) |
                       CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_HI);
        }
        if (chunk % columns == 0) {
            offGrid |= oddRow ? CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_LO) |
                                    CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_COL) |
                                    CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_LO)
                              : CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_COL);
        }
        if ((chunk + 1) % columns != 0) {
            return ~offGrid;
        }
        offGrid |= oddRow ? CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_COL)
                          : CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_PREV_ROW_HI) |
                                CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_COL) |
                                CHUNK_NEIGHBOUR_BIT(CHUNK_NEIGHBOUR_NEXT_ROW_HI);
        return ~offGrid;
    } else {
        offGrid = -1;
        for (i = 0; i < rows; i++) {
            offGrid <<= 1;
        }
        return ~offGrid;
    }
}

/* MATCHING: one `step` local carries every addend */
s32 StageMap__ComputeChunkLoadEntry(StageMap *self, ChunkLoadEntry *out, s32 columns, s32 oddRow,
                                    s32 centreChunk, s32 onGridMask, s32 neighbour) {
    s32 bit = sNeighbourBits[neighbour];
    s32 result;

    if ((onGridMask & bit) != 0) {
        if (self->config->isVertical == 0) {
            const ChunkNeighbourDelta *delta = &sChunkNeighbourDeltas[neighbour];
            s32 chunk;
            s32 step;

            if (delta->rowDelta == 0) {
                step = delta->colDeltaOddRow;
            } else {
                step = columns * delta->rowDelta;
                if (oddRow != 0) {
                    step += delta->colDeltaOddRow;
                } else {
                    step += delta->colDeltaEvenRow;
                }
            }
            chunk = centreChunk + step;
            out->chunkIndex.word = chunk;
        } else {
            out->chunkIndex.word = centreChunk + neighbour;
        }

        out->file = self->chunkFileFn(self->chunkFileCtx, out->chunkIndex.word, 0, 0);
        do { /* MATCHING: an empty do/while (0); without it the code comes out differently */
        } while (0);
        result = 1;
    } else {
        out->file = NULL;
        result = 0;
    }
    out->neighbour = neighbour;
    return result;
}

/* MATCHING: `tail` from `entry` inside the loop, `entry` itself advancing; a copy differs */
void StageMap__ApplyChunkLoads(StageMap *self, ChunkLoadEntry *entry, s32 count) {
    s32 i;
    ChunkSlot *slot;
    ChunkLoadEntryTail *tail;

    for (i = 0; i < count; i++) {
        tail = (ChunkLoadEntryTail *)&entry->chunkIndex;
        slot = self->methods->findSlotByNeighbour(self, tail->neighbour);
        ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(self, STAGEMAP_EVENT_SLOT_RELEASE,
                                                               slot, i);
        if (entry->file != 0) {
            if (slot->loader->headerReady != 0) {
                self->methods->clearSlotCells(self, slot);
            }
            slot->loader->chunkIndex = tail->chunkIndex;
            ((LbdFileLoadHeaderFn)slot->loader->methods->processBuffer)(slot->loader, entry->file);
            slot->loadPending = 1;
            self->loadsPending = 1;
        } else {
            if (slot->loader->headerReady != 0) {
                self->methods->clearSlotCells(self, slot);
            }
            if (slot->loader->loadState != 0) {
                slot->loader->methods->cancelRequests(slot->loader);
                slot->loadPending = 0;
            }
        }
        entry++;
    }
    self->pendingLoadCount = StageMap__CountPendingLoads(self);
}

s32 StageMap__CountPendingLoads(StageMap *self) {
    s32 count;
    s32 i;

    count = 0;
    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        if (self->slots[i].loadPending != 0) {
            count++;
        }
    }
    return count;
}

void StageMap__OnDrawSystemEvent(StageMap *self, void *sender, s32 command) {
    s32 i;
    ChunkSlot *slot;
    s32 pending;

    if (command != DRAWSYSTEM_EVENT_VSYNC) {
        return;
    }
    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->dataReady != 0) {
            slot->loader->dataReady = 0;
            ((StageMapOnSlotEventFn)self->methods->notifyWithHull)(
                self, STAGEMAP_EVENT_SLOT_DATA_READY, slot, i);
        }
        pending = self->loadsPending;
        if (pending == 1 && slot->loadPending != 0) {
            if (slot->loader->headerReady != 0) {
                self->methods->populateSlotCells(self, slot);
                slot->loader->headerReady = LBDFILE_HEADER_CONSUMED;
                slot->loadPending = 0;
                if (--self->pendingLoadCount == 0) {
                    self->pendingLoadCount = 0;
                    self->loadsPending = 0;
                    self->chunksLoaded = pending;
                }
            } else if (slot->loader->loadState == 0) {
                slot->loadPending = 0;
            }
        }
    }
}

/* MATCHING: `header` and `header2` are two locals; the cells are walked by byte offset */
void StageMap__PopulateSlotCells(StageMap *self, ChunkSlot *slot) {
    LbdFileHeader *header;
    LbdFileHeader *header2;
    LbdFile *loader;
    PlacementGrid *grid;
    LinkResource *oldResource;
    GridCell **cell;
    u8 *cells;
    GsCOORDINATE2 *coord;
    GsCOORD2PARAM *param;
    s32 x;
    s32 y;
    s32 z;
    s32 rotY;
    s32 model;
    s32 hidden;
    s32 i;
    s32 cellOff;
    s32 overflowOff;
    CellPlacement rec;
    ResourceRequest req; /* MATCHING: mode is never set; a bare ResourceSource shrinks the frame */

    loader = slot->loader;
    grid = slot->placements;
    header = loader->buffer;
    grid->buffer = (u8 *)header + header->placementsOffset;
    grid->bufferSize = 0;
    oldResource = grid->linkResource;
    if (oldResource != 0) {
        oldResource->methods->release(oldResource);
    }
    header2 = loader->buffer;
    req.src.buffer = (u8 *)header2 + header2->placementsOffset + header2->placementsSize;
    grid->linkResource = New_LinkResource(&req.src);
    rec.next = 0;

    i = 0;
    hidden = GsDOFF;
    cellOff = 0;
    overflowOff = STAGE_SLOT_LATTICE_CELLS * sizeof(GridCell *);
    for (;;) {
        model = ((PlacementGridResolveEntryFn)grid->methods->processBuffer)(grid, &rec, i);
        if (model == 0) {
            return;
        }
        if (model == -1) {
            s32 attr;
            cell = (GridCell **)((u8 *)slot->cells + cellOff);
            attr = (*cell)->attribute;
            (*cell)->attribute = attr | hidden;
            (*cell)->model = 0;
            (*cell)->tmd = 0;
        } else {
            cells = (u8 *)slot->cells;
            if (rec.chained != 0) {
                cell = (GridCell **)(cells + overflowOff);
                overflowOff += sizeof(GridCell *);
            } else {
                cell = (GridCell **)(cells + cellOff);
            }
            (*cell)->model = (void *)model;
            (*cell)->tmd = (s32)((TmdModel *)(*cell)->model)->object;
            GsLinkObject4((u_long)((TmdModel *)(*cell)->model)->object,
                          (GsDOBJ2 *)&(*cell)->attribute, 0);
            coord = (*cell)->coord2;
            /* MATCHING: an ordering barrier; it keeps the rec.x/.y/.z reads after coord2's */
            __asm__("");
            x = rec.x;
            y = rec.y;
            z = rec.z;
            coord->coord.t[0] = x;
            coord->coord.t[1] = y;
            coord->coord.t[2] = z;
            param = (*cell)->coord2->param;
            param->rotate.vx = 0;
            rotY = rec.rotY;
            param->rotate.vz = 0;
            param->rotate.vy = rotY;
            (*cell)->flags36 = rec.cellFlags;
            (*cell)->coord2->flg = 0;
            {
                s32 attr = (*cell)->attribute;
                (*cell)->attribute = attr | hidden;
            }
        }
        if (rec.next) {
            GridCell **overflow = (GridCell **)((u8 *)slot->cells + overflowOff);
            (*cell)->nextInCell = *overflow;
            continue;
        }
        cellOff += sizeof(GridCell *);
        (*cell)->nextInCell = 0;
        i++;
    }
}

void StageMap__ClearSlotCells(StageMap *self, ChunkSlot *slot) {
    GridCell **p;
    GridCell **end;

    if (slot->loader->chunkIndex >= 0) {
        ((LbdFileReleaseHeaderElemFn)slot->loader->methods->releaseHeader)(slot->loader, slot);
        end = slot->cells + STAGE_SLOT_CELLS;
        for (p = slot->cells; p < end; p++) {
            (*p)->attribute |= GsDOFF;
            (*p)->model = 0;
            (*p)->tmd = 0;
        }
    }
}

Descriptor10 *StageMap__GetTargetDescriptor(StageMap *self, Descriptor10Ext *desc, void **outPos) {
    void *pos;

    pos = self->target->coord2->coord.t;
    if (outPos != 0) {
        *outPos = pos;
    }
    if (desc != 0) {
        if (self->methods->computeFootprintDescriptor(self, desc, pos) != 0) {
            return 0;
        }
    }
    return &self->targetCell.base;
}

/* MATCHING: cellCol/cellRow re-read the stored bytes; the half cell sits inside the subtracted group */
/* MATCHING: `out->slot` is stored last; SplitLongVec3's unions reread x/z at the narrower width */
s32 StageMap__ComputeFootprintDescriptor(StageMap *self, Descriptor10Ext *out, SplitLongVec3 *pos) {
    ChunkSlot *slot;
    SplitCoord2 *chunkOrigin;
    SplitCoord2 *origin;
    s32 chunkIndex;
    s32 rel;
    s32 cellCol;
    s32 cellRow;

    slot = self->methods->findSlotForPosition(self, (LongVec3 *)pos);
    if (slot != 0) {
        chunkIndex = slot->loader->chunkIndex;
        out->chunkIndex = chunkIndex;
        StageMap__SplitChunkIndex(self, (u8 *)out, chunkIndex);

        chunkOrigin = (SplitCoord2 *)self->methods->findSlotByNeighbour(self, slot->loader->elemKey)
                          ->cellParent->coord2;
        out->chunkCentre.x = chunkOrigin->tx.w + STAGE_CHUNK_SIZE / 2;
        out->chunkCentre.y = chunkOrigin->ty;
        out->chunkCentre.z = chunkOrigin->tz.w + STAGE_CHUNK_SIZE / 2;

        origin = (SplitCoord2 *)slot->cellParent->coord2;
        out->relPos.x = pos->x.w - out->chunkCentre.x;
        out->relPos.y = pos->y.w;
        out->relPos.z = pos->z.w - out->chunkCentre.z;

        rel = pos->x.w - origin->tx.w;
        if (rel < 0) {
            rel += STAGE_CELL_SIZE - 1;
        }
        out->base.b2 = rel >> STAGE_CELL_SHIFT;

        rel = pos->z.w - origin->tz.w;
        if (rel < 0) {
            rel += STAGE_CELL_SIZE - 1;
        }
        out->base.b3 = rel >> STAGE_CELL_SHIFT;

        cellCol = out->base.b2;
        out->base.h4 = pos->x.h - (origin->tx.h + (cellCol << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2);
        out->base.h6 = pos->y.h;
        cellRow = out->base.b3;
        out->base.h8 = pos->z.h - (origin->tz.h + (cellRow << STAGE_CELL_SHIFT) + STAGE_CELL_SIZE / 2);
        out->slot = slot;

        return 0;
    }
    return 1;
}

void StageMap__SplitChunkIndex(StageMap *self, u8 *out, s32 chunkIndex) {
    out[0] = chunkIndex % self->config->columns;
    out[1] = chunkIndex / self->config->columns;
}

ChunkSlot *StageMap__GetLastEventSlotChunk(StageMap *self, u8 *out) {
    StageMap__SplitChunkIndex(self, out, self->lastEventSlot->loader->chunkIndex);
    return self->lastEventSlot;
}

/* The original returns NULL when no slot matches only because the loop's exit
 * test leaves 0 as the result; it has no return statement there. */
#ifdef NON_MATCHING
ChunkSlot *StageMap__FindSlotByNeighbour(StageMap *self, s32 neighbour) {
    s32 i;
    ChunkSlot *slot;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->elemKey == neighbour) {
            return slot;
        }
    }
    return NULL;
}
#else
INCLUDE_ASM("asm/nonmatchings/world/dream_day", StageMap__FindSlotByNeighbour);
#endif

/* MATCHING: `edge` is assigned inside each upper-bound test */
ChunkSlot *StageMap__FindSlotForPosition(StageMap *self, LongVec3 *pos) {
    s32 i;
    s32 span;
    s32 layerTop;
    ChunkSlot *slot;
    SplitCoord2 *origin;
    s32 edge;

    i = 0;
    span = STAGE_CHUNK_SIZE;
    layerTop = 0;
    for (; i < CHUNK_NEIGHBOUR_COUNT; i++, layerTop -= VERTICAL_LAYER_HEIGHT) {
        slot = self->methods->findSlotByNeighbour(self, i);
        origin = (SplitCoord2 *)slot->cellParent->coord2;
        if (pos->x >= origin->tx.w && pos->x < (edge = origin->tx.w) + span) {
            if (pos->z >= origin->tz.w && pos->z < (edge = origin->tz.w) + span) {
                if (self->config->isVertical == 0) {
                    return slot;
                }
                if (layerTop >= pos->y) {
                    if (layerTop - VERTICAL_LAYER_HEIGHT >= pos->y) {
                        continue;
                    }
                    return slot;
                }
            }
        }
    }
    return 0;
}

/*
 * StageMap's last run of methods: the drawn window and the scale ramp.
 * FindSlotIndexByNeighbour and FindSlotIndexByChunk name a slot by key or
 * chunk. RefreshFootprint moves the window of cells that is drawn: in a flat
 * grid ahead of the target along its facing (ComputeFootprintFromRotation,
 * split per slot by BuildFootprintRects and SplitFootprintRect), in a
 * vertical grid whole chunks (SetFootprintFromQuery, IsPointOutOfBounds,
 * InitFootprintRect), shown and hidden by SetFootprintVisible. The scale
 * ramp (StartScaleRamp, StepScaleRamp, EndScaleRamp) walks every cell with
 * ForEachSlot and ForEachSlotCell.
 */

s32 StageMap__FindSlotIndexByNeighbour(StageMap *self, s32 key) {
    s32 index;
    s32 i;
    ChunkSlot *slot;

    index = 0;
    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->elemKey == key) {
            index = i;
            break;
        }
    }
    return index;
}

s32 StageMap__FindSlotIndexByChunk(StageMap *self, s32 chunkIndex) {
    s32 i;
    ChunkSlot *slot;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->chunkIndex == chunkIndex && slot->loader->headerReady != 0) {
            return i;
        }
    }
    return -1;
}

void StageMap__RefreshFootprint(StageMap *self) {
    s32 acrossCells;

    if (self->chunksLoaded == 0) {
        return;
    }
    acrossCells = self->gridHalfCells * 2;
    StageMap__SetFootprintVisible(self, 0);
    if (self->config->isVertical == 0) {
        StageMap__ComputeFootprintFromRotation(self, acrossCells, self->gridCells);
    } else {
        StageMap__SetFootprintFromQuery(self);
    }
    StageMap__SetFootprintVisible(self, 1);
}

/* MATCHING: the (u16) casts make the angle reads unsigned */
void StageMap__ComputeFootprintFromRotation(StageMap *self, s32 acrossCells, s32 aheadCells) {
    GsCOORD2PARAM *param;
    Descriptor10Ext desc;
    s32 cellCol;
    s32 cellRow;
    u16 angle; /* MATCHING: u16; s32 makes the frame 8 bytes smaller */
    MATRIX mat;
    s32 lateral;
    s32 shiftCol;
    s32 half;
    void *rot;

    param = self->target->coord2->param;
    rot = &param->rotate; /* MATCHING: here, before the call, not after it */
    self->methods->getTargetDescriptor(self, &desc, 0);
    cellCol = desc.base.b2;
    cellRow = desc.base.b3;
    angle = param->rotate.vy;
    if (param->rotate.vy < 0) {
        angle += ONE;
    }

    /* The facing vector: (0, 0, gridSpan) turned by the target's rotation,
     * in place (t is both ApplyMatrixLV's input and its output). */
    mat = GsIDMATRIX;
    mat.t[0] = 0;
    mat.t[1] = 0;
    mat.t[2] = self->gridSpan;
    RotMatrix(rot, &mat);
    ApplyMatrixLV(&mat, (VECTOR *)mat.t, (VECTOR *)mat.t);

    /* Facing +-x, then facing +-z; the two tests cover every angle. */
    /* MATCHING: the else-if keeps its test, though every angle the first rejects passes it */
    if ((u16)(angle - ANGLE_DEG(45)) < ANGLE_DEG(90) || (u16)(angle - ANGLE_DEG(225)) < ANGLE_DEG(90)) {
        lateral = mat.t[2];
        self->footprintWidth = aheadCells;
        self->footprintHeight = acrossCells;
        self->footprintCol = (mat.t[0] > 0) ? cellCol : cellCol - acrossCells + 1;
        self->footprintRow = (mat.t[2] > 0) ? cellRow - (u16)self->gridHalfCells - 1
                                            : cellRow - (u16)self->gridHalfCells + 1;
        shiftCol = 0;
    } else if ((u16)(angle - ANGLE_DEG(135)) < ANGLE_DEG(90) ||
               (u16)(angle - ANGLE_DEG(45)) >= ANGLE_DEG(270)) {
        lateral = mat.t[0];
        self->footprintWidth = acrossCells;
        self->footprintHeight = aheadCells;
        self->footprintCol = (mat.t[0] > 0) ? cellCol - (u16)self->gridHalfCells - 1
                                            : cellCol - (u16)self->gridHalfCells + 1;
        self->footprintRow = (mat.t[2] > 0) ? cellRow : cellRow - aheadCells + 1;
        shiftCol = 1;
    }

    half = self->gridHalfCells;
    lateral >>= STAGE_CELL_SHIFT;
    if (lateral >= half) {
        lateral = half - 1;
    }
    half = -half;
    if (half >= lateral) {
        lateral = half + 1;
    }
    if (shiftCol) {
        self->footprintCol = (u16)self->footprintCol + lateral;
    } else {
        self->footprintRow = (u16)self->footprintRow + lateral;
    }
    StageMap__BuildFootprintRects(self);
}

/* MATCHING: one `rect` reused for the second rectangle, `over` its own local, `count += 1` thrice */
void StageMap__BuildFootprintRects(StageMap *self) {
    s32 wrappedCol;
    s32 col;
    s32 width;
    s32 height;
    s32 key;
    s32 row;
    CellRect *rect;
    s32 span;
    s32 count;
    s32 over;

    wrappedCol = 0;
    col = self->footprintCol;
    width = self->footprintWidth;
    height = self->footprintHeight;
    key = CHUNK_NEIGHBOUR_CENTRE;
    if (col < 0) {
        col += STAGE_CHUNK_CELLS;
        wrappedCol = 1;
        key = CHUNK_NEIGHBOUR_PREV_COL;
    }
    row = self->footprintRow;
    if (row < 0) {
        row += STAGE_CHUNK_CELLS;
        if (wrappedCol != 0) {
            col -= STAGE_CHUNK_HALF_CELLS;
            key = CHUNK_NEIGHBOUR_PREV_ROW_LO;
        } else if (col < STAGE_CHUNK_HALF_CELLS) {
            col += STAGE_CHUNK_HALF_CELLS;
            key = CHUNK_NEIGHBOUR_PREV_ROW_LO;
        } else {
            col -= STAGE_CHUNK_HALF_CELLS;
            key = CHUNK_NEIGHBOUR_PREV_ROW_HI;
        }
    }
    rect = &self->rects.e[0];
    rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, key);
    rect->col = (col >= 0) ? col : 0;
    rect->row = row;
    span = col + width;
    if (span > STAGE_CHUNK_CELLS) {
        over = span - STAGE_CHUNK_CELLS;
        rect->width = width - over;
        count = StageMap__SplitFootprintRect(self, rect, 0, key, col, row, width, height);
        count += 1;
        rect = &self->rects.e[count];
        rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, key + 1);
        rect->col = 0;
        rect->row = self->rects.e[0].row;
        rect->width = over;
        rect->height = self->rects.e[0].height;
    } else {
        rect->width = width;
        count = StageMap__SplitFootprintRect(self, rect, 0, key, col, row, width, height);
    }
    count += 1;
    self->rectCount = count;
}

s32 StageMap__SplitFootprintRect(StageMap *self, CellRect *rect, s32 count, s32 key, s32 col,
                                 s32 row, s32 width, s32 height) {
    s32 rowsBelow;
    s32 span;
    s32 belowKey;
    s32 widthLeft;

    if (row + height > STAGE_CHUNK_CELLS) {
        /* The rectangle runs past the chunk's bottom edge: clip this rect
         * and open a new one, in the slot below, for the rest. */
        rowsBelow = (row + height) - STAGE_CHUNK_CELLS;
        span = rowsBelow;
        rect->height = height - rowsBelow;
        count = count + 1;
        rect = &self->rects.e[count];

        if (col < STAGE_CHUNK_HALF_CELLS) {
            belowKey = key + 2;
            rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, belowKey);
            rect->col = col + STAGE_CHUNK_HALF_CELLS;
            /* MATCHING: stored in both arms; merged after the if, col is read back elsewhere */
            rect->height = span;
        } else {
            belowKey = key + 3;
            rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, belowKey);
            rect->col = col - STAGE_CHUNK_HALF_CELLS;
            rect->height = span;
        }

        span = rect->col + width;
        rect->row = 0;
        if (span > STAGE_CHUNK_CELLS) {
            /* ...and past the right edge too. */
            count = count + 1;
            /* MATCHING: two statements; one expression shares span - 20 with the store below */
            widthLeft = width + STAGE_CHUNK_CELLS;
            widthLeft = widthLeft - span;
            rect->width = widthLeft;
            rect = &self->rects.e[count];
            rect->slotIndex = self->methods->findSlotIndexByNeighbour(self, belowKey + 1);
            rect->col = 0;
            rect->row = 0;
            rect->width = span - STAGE_CHUNK_CELLS;
            rect->height = rowsBelow;
        } else {
            rect->width = width;
        }
    } else {
        rect->height = height;
    }
    return count;
}

void StageMap__SetFootprintFromQuery(StageMap *self) {
    s32 junk; /* MATCHING: never set; InitFootprintRect ignores the argument */
    Descriptor10Ext desc;

    self->methods->getTargetDescriptor(self, &desc, 0);
    self->rectCount = 0;
    self->rectCount = StageMap__InitFootprintRect(self, junk, 0, desc.chunkIndex);
    if (IsPointOutOfBounds(self->bounds, &desc.base.b2) != 0) {
        if (desc.chunkIndex + 1 < self->config->rows) {
            self->rectCount =
                StageMap__InitFootprintRect(self, junk, self->rectCount, desc.chunkIndex + 1);
        }
    }
    if (desc.chunkIndex - 1 >= 0) {
        self->rectCount = StageMap__InitFootprintRect(self, junk, self->rectCount, desc.chunkIndex - 1);
    }
}

/* MATCHING: the in-range test returns 0 and `return 1` follows; the other order differs */
s32 IsPointOutOfBounds(CellBounds *bounds, s8 *point) {
    if (bounds != NULL && point[0] >= bounds->minCol && bounds->maxCol >= point[0] &&
        point[1] >= bounds->minRow && bounds->maxRow >= point[1]) {
        return 0;
    }
    return 1;
}

s32 StageMap__InitFootprintRect(StageMap *self, s32 unused, s32 index, s32 chunkIndex) {
    CellRect *rect;

    rect = &self->rects.e[index];
    *rect = sFullSlotRect;
    rect->slotIndex = self->methods->findSlotIndexByChunk(self, chunkIndex);
    return index + 1;
}

void StageMap__SetFootprintVisible(StageMap *self, s32 visible) {
    s32 i;
    s32 j;
    s32 k;
    CellRect *rect;
    ChunkSlot *slot;
    GridCell **cell;
    GridCell *next;

    rect = self->rects.e;
    for (i = 0; i < self->rectCount; rect++, i++) {
        slot = &self->slots[rect->slotIndex];
        if (slot->loader->headerReady == 0) {
            continue;
        }
        cell = slot->cells + rect->col + rect->row * STAGE_CHUNK_CELLS;
        for (j = 0; j < rect->height; j++) {
            for (k = 0; k < rect->width; k++) {
                if (visible != 0) {
                    (*cell)->attribute &= ~GsDOFF;
                } else {
                    (*cell)->attribute |= GsDOFF;
                }
                next = (*cell)->nextInCell;
                while (next != NULL) {
                    if (visible != 0) {
                        next->attribute &= ~GsDOFF;
                    } else {
                        next->attribute |= GsDOFF;
                    }
                    next = next->nextInCell;
                }
                cell++;
            }
            cell += STAGE_CHUNK_CELLS - rect->width;
        }
    }
}

void *StageMap__GetUnk1CC(StageMap *self) {
    return &self->unk1CC;
}

void StageMap__SetBounds(StageMap *self, CellBounds *bounds) {
    self->bounds = bounds;
}

/* MATCHING: the up-slow table set ahead of the fast test (one store after both ifs compiles
 * differently), `scale` read before the sign test, `val` in an if/else, `~rate + 1` */
void StageMap__StartScaleRamp(StageMap *self, s32 rate, s32 fast) {
    Ratio16 *table;
    s32 val;
    s32 scale;

    if (rate > 0) {
        table = sScaleStepUpSlow;
        if (fast != 0) {
            self->scaleStep = sScaleStepUpFast;
        } else {
            self->scaleStep = table;
        }
    } else {
        table = sScaleStepDownSlow;
        if (fast != 0) {
            table = sScaleStepDownFast;
        }
        self->scaleStep = table;
    }
    scale = self->scaleStep[1].den;
    if (rate >= 0) {
        val = scale * rate;
    } else {
        val = scale * (~rate + 1);
    }
    self->scaleRampTicks = val;
}

void StageMap__StepScaleRamp(StageMap *self) {
    if (self->scaleRampTicks > 0) {
        StageMap__ForEachSlot(self, StageMap__AddScaleStepToCell, 0);
        self->scaleRampTicks -= 1;
        if (self->scaleRampTicks == 0) {
            self->scaleRampTicks = -1;
        }
    }
}

void StageMap__EndScaleRamp(StageMap *self) {
    if (self->scaleRampTicks != 0) {
        StageMap__ForEachSlot(self, StageMap__ResetCellScale, 0);
        self->scaleRampTicks = 0;
    }
}

void StageMap__AddScaleStepToCell(StageMap *self, GridCell *cell) {
    cell->methods->updateScale(cell, 0, self->scaleStep);
}

void StageMap__ResetCellScale(StageMap *self, GridCell *cell) {
    cell->methods->updateScale(cell, 1, sScaleOne);
}

void StageMap__ForEachSlot(StageMap *self, StageMapCellFn cellFn, ChunkSlotFn slotFn) {
    s32 i;
    ChunkSlot *slot;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slotFn != 0) {
            slotFn(self, slot);
        }
        StageMap__ForEachSlotCell(self, cellFn, slot);
    }
}

void StageMap__ForEachSlotCell(StageMap *self, StageMapCellFn cellFn, ChunkSlot *slot) {
    GridCell **cell;
    GridCell **end;

    end = slot->cells + STAGE_SLOT_CELLS;
    for (cell = slot->cells; cell < end; cell++) {
        cellFn(self, *cell);
    }
}

StageMapMethods *GetStageMapMethods(void) {
    return &gStageMapMethods;
}
