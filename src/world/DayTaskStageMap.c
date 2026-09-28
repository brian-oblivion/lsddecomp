/*
 * DayTaskStageMap -- the dream day's task and the stage map it builds: DayTask,
 * RegisterRecordTableFiles, TimedTask (DayTask's parent, and ObjM's) and
 * StageMap, in that ROM order.
 *
 * DayTask (include/DayTask.h), New_DayTask through GetDayTaskMethods: the
 * task GameApplication__RunDayTask runs for one dream day. Its ctor loads
 * the day's shared resources (ETC\ETC.TIM, ETC\DREAMER.TMD, the week's
 * BGM) and fills the init args' viewport (a NodeGuardedViewport), frame
 * clock and light rig (a StageMap); on each DrawSystem VSync
 * DayTask__AdvancePhase starts the day or replaces the running ObjM, and
 * DayTask__OnObjMNotify turns ObjM's states into the next phase or the
 * task's result.
 *
 * RegisterRecordTableFiles: registers gRecordTable's file entries with the
 * CD driver in at most two batches (DayTask's ctor, and the loader-task
 * callback in GameApplicationFileResource.c).
 *
 * TimedTask (include/TimedTask.h), New_TimedTask through
 * GetTimedTaskMethods: an IntermediateBase with a frame timeout, a sound
 * object and a result.
 *
 * StageMap (include/StageMap.h, whose banner describes the class),
 * New_StageMap through GetStageMapMethods: the loaded part of a stage's
 * map, seven chunk slots each laid out as a lattice of GridCells. Its
 * methods fall in three runs, each introduced below: life and the command
 * path; placing, loading and querying the slots; the drawn window and the
 * scale ramp. Its data tables and SplitCoord2 are in include/class_3bb8c.h.
 *
 * NodeGuardedViewport and GridCell, which DayTask and StageMap use, are
 * defined in TitleMenuTaskObjF.c.
 */
#include "common.h"
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "DayTaskStageMap.h"
#include "VabStreamObj.h"
#include "NodeGuardedViewport.h"
#include "StageMap.h"
#include "WBgm.h"
#include "TimImage.h"
#include "FrameClock.h"
#include "DreamSys.h"
#include "LinkResource.h"
#include "ObjM.h"
#include "Actor.h"
#include "LightRig.h"
#include "TimedTask.h"
#include "DrawSystem.h"
#include "PlacementGrid.h"
#include "LbdFile.h"
#include "GridCell.h"
#include "FlatLightObj.h"
#include "BMemPMgr.h"
#include "class_3bb8c.h"

/* The viewpoint and view-reference points DayTask__OnInit hands the
 * viewport's attachViewChild: (0, -1200, 0) and (0, -1200, 10000). */
extern LongVec3 sDayViewPoint;
extern LongVec3 sDayViewRef;

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
    /* MATCHING: mode is never set, but a bare ResourceSource shrinks the
     * frame by 8. */
    ResourceRequest req;
    s32 vabPath;

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
    self->bgm = New_WBgm((char *)vabPath, NULL, 1);
    RegisterRecordTableFiles(1);
    SetActiveDataSourceDriverMode(syncDriver == 0, 1, 1);
    self->initArgs = initArgs;
    initArgs->viewport = (BasicClass *)New_NodeGuardedViewport();
    initArgs->frameClock = (BasicClass *)New_FrameClock();
    initArgs->lightRig = (BasicClass *)New_StageMap(NULL, 1);
    self->dreamSys = dreamSys;
    self->methods->addChild(self, (BasicClass *)dreamSys);
    dreamSys->methods->setSoundObj(dreamSys, (s32)self->sound);
    dreamSys->methods->setEtcTim(dreamSys, (s32)self->etcTim);
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
    if ((tag & 0xFFFF) == DREAMSYS_CLASS_ID) {
        self->methods->onDreamSysNotify(self, sender, event);
    } else if ((tag & 0xFFFFF) == OBJM_CLASS_ID) {
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
    return GetTimedTaskMethods()->init((TimedTask *)self, self->initArgs, 0);
}

void DayTask__Deinit(DayTask *self) {
    DreamSys *dreamSys = self->dreamSys;

    GetTimedTaskMethods()->deinit((TimedTask *)self);
    dreamSys->methods->setViewport(dreamSys, 0);
    dreamSys->methods->removeChild(dreamSys, self->initArgs->pad);
    dreamSys->methods->removeChild(dreamSys, self->unk10);
}

void DayTask__OnInit(DayTask *self) {
    SubObjE *drawSystem;
    Viewport *vp;
    SceneNode *fadeBox;
    ViewportSize *size;

    drawSystem = (SubObjE *)self->initArgs->drawSystem;
    vp = (Viewport *)self->viewport;
    size = drawSystem->methods->getDims(drawSystem, 0);
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

/* onTag1Notify: on each DrawSystem VSync, starts the day's first ObjM
 * (READY) or replaces the ObjM a link state ended (REPLACE_OBJM). A day
 * startDay refuses ends at once. */
void DayTask__AdvancePhase(DayTask *self, BasicClass *sender, s32 event) {
    s32 result;

    GetTimedTaskMethods()->onTag1Notify((TimedTask *)self, sender, event);
    if (event == DRAWSYSTEM_EVENT_VSYNC && self->phase != DAYTASK_PHASE_RUNNING) {
        switch (self->phase) {
            case DAYTASK_PHASE_READY:
                result = self->dreamSys->methods->startDay(self->dreamSys);
                if (result < 0) {
                    self->dreamSys->methods->endDay(self->dreamSys, 0);
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

void DayTask__OnState4(void) {}

void DayTask__OnDreamSysNotify(void) {}

void DayTask__OnObjMNotify(DayTask *self, BasicClass *sender, s32 event) {
    CinematicCall cinematic;
    s32 result;

    switch (event) {
        case OBJM_STATE_TIME_UP:
            self->objM->methods->deinit(self->objM);
            self->objM->methods->release(self->objM);
            result = self->dreamSys->methods->endDay(self->dreamSys, 0);
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
            /* endDay(1) on CLOSE, endDay(2) (a new game) on CLOSE_NEW_GAME */
            self->dreamSys->methods->endDay(self->dreamSys, event != OBJM_NOTIFY_CLOSE ? 2 : 1);
            self->result = DAYTASK_RESULT_CLOSED;
            self->methods->setState(self, INTERMEDIATEBASE_STATE_STOP);
            break;
    }
}

DayTaskMethods *GetDayTaskMethods(void) {
    return &gDayTaskMethods;
}

/* src/cd/GameFiles.c: returns gRecordTable and writes its record count to
 * *out. */
extern void *GetRecordTable(s32 *out);
/* src/app/GameApplicationFileResource.c: appends `count` records of `table` to the CD driver's
 * file table and resolves them; returns 0 to be retried, and 1 when the CD
 * driver is not the active data source. */
extern s32 RegisterFileTableEntries(void *table, s32 count);

/* How many times RegisterRecordTableFiles has run (a call with `all` set
 * counts as two), and how many records its first, half-table batch took. */
extern s32 sRecordRegisterCalls;
extern s32 sRecordFirstBatchCount;

/* Registers gRecordTable's records with the CD driver, retrying until it
 * accepts them. The first call registers the whole table when `all` is set,
 * else its first half; the second call registers the rest; any later call
 * registers nothing. */
s32 RegisterRecordTableFiles(s32 all) {
    s32 count;
    void *table;
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
    Get_vtable_IntermediateBase()->ctor((IntermediateBase *)self);
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
    Get_vtable_IntermediateBase()->finalize((IntermediateBase *)self);
}

void TimedTask__CancelTimeout(TimedTask *self) {
    self->methods->setTimeout(self, -1);
}

s32 TimedTask__Init(TimedTask *self, IntermediateBaseInitArgs *args, s32 mode) {
    self->result = 0;
    Get_vtable_IntermediateBase()->init((IntermediateBase *)self, args, mode);
    return self->result;
}

void TimedTask__Deinit(TimedTask *self) {
    Get_vtable_IntermediateBase()->deinit((IntermediateBase *)self);
}

void TimedTask__NoOpSlot58(void) {}

void TimedTask__CheckTimeout(TimedTask *self, BasicClass *sender, s32 event) {
    Get_vtable_IntermediateBase()->update((IntermediateBase *)self, sender, event);
    if ((u32)self->frameCounter > (u32)self->timeoutFrames) {
        self->methods->setState(self, TIMEDTASK_STATE_TIMED_OUT);
    }
}

void TimedTask__SetState(TimedTask *self, s32 state) {
    Get_vtable_IntermediateBase()->setState((IntermediateBase *)self, state);
    if (state == TIMEDTASK_STATE_TIMED_OUT) {
        self->result = TIMEDTASK_RESULT_TIMED_OUT;
        self->methods->onState4(self);
    }
}

void TimedTask__SetTimeout(TimedTask *self, s32 timeout) {
    self->timeoutFrames = (timeout < 0) ? timeout : timeout * TIMEDTASK_TIMEOUT_UNIT_FRAMES;
}

/*
 * TimedTask's last two functions, then the front third of StageMap's
 * methods.
 *
 * This third holds the object's life and its command path: the allocator
 * and ctor (seven slots, each an LbdFile, a placement list, a cellParent
 * GridCell attached at `origin` and STAGE_SLOT_CELLS cells STAGE_CELL_SIZE
 * apart, row stride STAGE_CHUNK_CELLS), Finalize, OnNotify, Reset,
 * OnSlotEvent, the per-tick update (UpdateIfEnabled: footprint tracking,
 * then the scale ramp), UnloadAllSlots, the setters ObjM configures it
 * through (SetChildParams, SetCallback, SetAcceptedTags, SetGridSpan,
 * SetConfig), and the path a command takes to the cells:
 * DispatchLinkCommand passes on an Actor sender's, ForwardAcceptedCommand
 * filters the sender against acceptedTags, ApplyToSenderFootprint turns
 * the sender's position into a 3 x 3 cell footprint (SetFootprintFromCell
 * or SetFootprintRect), and DispatchToRectCells hands the command to every
 * cell in it and every cell chained behind each (NotifyGridCell).
 * StageMap__NoOpSlotD8 is the empty +0x0D8 slot; nothing calls it.
 */

/* TimedTask::sound is BasicClass * (it may be the ctor's own argument); when
 * it is a New_VabStreamObj object, +0x080 is VabStreamObj__PlayTone. Plays
 * `tone` at full volume (127) throughout. */
void TimedTask__PlaySound(TimedTask *self, s32 tone) {
    VabStreamObj *sound = (VabStreamObj *)self->sound;

    if (sound != NULL) {
        sound->methods->playTone(sound, tone, 127, 127);
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

extern LongVec3 gDefaultOrigin;

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
        self->origin = gDefaultOrigin;
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

        /* MATCHING: `end` from `cells` before `cursor = cells`, or the load
         * of slot->cells no longer goes through $v0. */
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

/* A sender of DrawSystem's class (id nibble 0x1) goes on to onNotifyTag1. */
void StageMap__OnNotify(StageMap *self, BasicClass *sender, s32 command) {
    GetSceneNodeMethods()->onNotify((SceneNode *)self, sender, command);

    if ((sender->methods->header & CLASS_ID_ROOT_MASK) == DRAWSYSTEM_CLASS_ID) {
        self->methods->onNotifyTag1(self, sender, command);
    }
}

extern s32 gDefaultGridSpan;

void StageMap__Reset(StageMap *self) {
    self->config = NULL;
    self->acceptedTags = NULL;
    self->rectCount = 0;
    self->methods->setGridSpan(self, gDefaultGridSpan);
    self->unk1CC = -1;
    self->unk1D0 = -1;
    self->unk1D4 = -1;
    self->unk1D8 = -1;
}

/* Records the slot and passes a slot event on to the parents; a
 * STAGEMAP_EVENT_SLOT_RELEASE also releases the slot's heldObj. */
void StageMap__OnSlotEvent(StageMap *self, s32 command, ChunkSlot *slot) {
    GetSceneNodeMethods()->notifyWithHull((SceneNode *)self, command);

    /* MATCHING: two literal if+goto tests; an if/else-if chain inverts the
     * branches. */
    if (command == STAGEMAP_EVENT_SLOT_RELEASE)
        goto release;
    if (command == STAGEMAP_EVENT_SLOT_DATA_READY)
        goto record;
    return;

release:
    if (slot->heldObj != NULL) {
        slot->heldObj = slot->heldObj->methods->release(slot->heldObj);
    }

record:
    self->lastEventSlot = slot;
    self->methods->notifyParents(self, command);
}

void StageMap__UpdateIfEnabled(StageMap *self) {
    if (self->enabled) {
        self->methods->updateFootprintTracking(self);
        self->methods->stepScaleRamp(self);
    }
}

void StageMap__DispatchLinkCommand(StageMap *self, BasicClass *sender, s32 command) {
    if ((u8)sender->methods->header == ACTOR_CLASS_ID) {
        self->methods->forwardAcceptedCommand(self, sender, command);
    }
}

/* Cancel every slot's load, clear and release what it holds, then zero
 * the load counters and end the scale ramp. */
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
        light->methods->setColor(light, 1, (FlatLightColor *)colors);
        colors += sizeof(FlatLightColor);
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
    u8 unused[24]; /* MATCHING: retail's frame is 24 bytes larger than the locals need */

    switch (command) {
        case 2:
        case 3:
        case 5:
        case 6:
        case 7:
        case 8:
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

/* Clamp a span x span footprint centred on desc's cell to the chunk's
 * STAGE_CHUNK_CELLS x STAGE_CHUNK_CELLS lattice: a cell on the low edge (0)
 * or the high edge loses one row/column there.
 * MATCHING: the edge tests read copies taken before the decrement, and the
 * height is `span` itself. */
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

/* Notify every cell of every rectangle, and every object chained behind
 * each cell, with the cell's key in curCell while it is notified.
 * MATCHING: the comma increments go `rect++, i++` and `cell++, col++`. */
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
                    /* MATCHING: b0/b1 as one halfword; two byte copies are two lb/sb */
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

/* Hands the command to a cell flagged GRIDCELL_FLAG_TAKES_COMMANDS. sender
 * and command arrive as DispatchToRectCells' own. */
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
 * The middle of StageMap's methods: placing a cell descriptor in the world,
 * loading the seven chunk slots around a centre chunk, linking a loaded
 * chunk into its slot's cells, and the queries that turn a position back
 * into a slot and cell.
 *
 *  - SetTargetAndLoadChunks, ComputeCellOffsets, ComputeCellWorldOffsets:
 *    a cell descriptor to a world position and the chunk it lies in.
 *  - Enable/Disable, UpdateFootprintTracking: the per-tick tracking of the
 *    target.
 *  - LoadChunksAround, ComputeNeighbourMask, ComputeChunkLoadEntry,
 *    ApplyChunkLoads, CountPendingLoads: moving the slots around a centre
 *    chunk and starting (or cancelling) each slot's LbdFile load.
 *  - OnNotifyTag1: on the DrawSystem's per-VSync notification, finishing
 *    the loads that have completed.
 *  - PopulateSlotCells / ClearSlotCells: linking a loaded chunk's
 *    placements and models into its slot's cells, and clearing them.
 *  - GetTargetDescriptor, ComputeFootprintDescriptor, SplitChunkIndex,
 *    GetLastEventSlotChunk, FindSlotByNeighbour, FindSlotForPosition:
 *    queries. A slot's position is its cellParent's GsCOORDINATE2, read
 *    through SplitCoord2 (include/class_3bb8c.h).
 *
 * Positions are in world units: a cell is STAGE_CELL_SIZE square, a chunk
 * STAGE_CHUNK_SIZE (include/StageMap.h).
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

/* A cell descriptor to world positions: writes the chunk's centre to
 * chunkPos and the cell point (cell column/row plus the offset inside the
 * cell, measured from the cell's centre) to outPos, and returns the chunk's
 * index (0 in a vertical grid). The grid is centred on `origin`; odd rows sit
 * half a chunk to -x. */
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
    halfCell = STAGE_CELL_SIZE / 2; /* MATCHING: a local, set here, places retail's constant load */
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

/* The per-tick tracking: re-reads the target's descriptor; in a flat grid,
 * reloads the slots the target's slot selects (sFootprintResultRemap) around
 * its chunk; moves the drawn window; notifies the parents when the chunk
 * changed. Returns the selected spec's index (0 for the centre slot, which
 * changes nothing). */
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

/* Moves each slot `specs` marks for loading to the centre position plus
 * its neighbour key's offset (a vertical grid: to its layer), builds a
 * ChunkLoadEntry for it, gives every slot its new key, then starts the
 * loads (applyChunkLoads). */
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
            slot = &self->slots[i]; /* MATCHING: the first loop's `slot`; a second local swaps a register */
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

/* Fills `out` for the slot taking neighbour key `neighbour` of centreChunk:
 * the neighbour's chunk index and the chunk's file record from the
 * callback, or a NULL file when that neighbour lies off the grid. Returns 1
 * for a file, 0 for none. chunkIndex is written as a whole word (see
 * ChunkLoadEntry). MATCHING: one `step` local carries every addend. */
s32 StageMap__ComputeChunkLoadEntry(StageMap *self, ChunkLoadEntry *out, s32 columns, s32 oddRow,
                                    s32 centreChunk, s32 onGridMask, s32 neighbour) {
    s32 bit = sNeighbourBits[neighbour];
    s32 result;

    if ((onGridMask & bit) == 0) {
        result = 0;
        goto nullCase;
    }

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
        *(s32 *)((u8 *)out + 4) = chunk;
    } else {
        *(s32 *)((u8 *)out + 4) = centreChunk + neighbour;
    }

    out->file = self->chunkFileFn(self->chunkFileCtx, *(s32 *)((u8 *)out + 4), 0, 0);
    do { /* MATCHING: removing it drifts the image */
    } while (0);
    result = 1;
    goto storeKey;

nullCase:
    out->file = NULL;

storeKey:
    out->neighbour = neighbour;
    return result;
}

/* Starts each entry's load in the slot holding its neighbour key (after
 * clearing the cells of a chunk already linked there), or cancels the slot's
 * load for a NULL file; then counts the slots left pending.
 * MATCHING: `tail` is taken from `entry` inside the loop and `entry` itself
 * advances; a copy of the parameter swaps two saved registers. */
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

void StageMap__OnNotifyTag1(StageMap *self, void *sender, s32 command) {
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

/* Links a loaded chunk into its slot: points the slot's PlacementGrid at the
 * header's placement records, replaces its LinkResource with one over the
 * header's model block, then resolves record after record until the grid
 * returns 0. A record with no model (-1) hides its lattice cell; one with a
 * model links it into the next lattice cell (a chained record: the next
 * overflow cell), sets the cell's position, y rotation and flags, and hides
 * it until the drawn window shows it. While a record has `next` set, the
 * cell's nextInCell is the overflow cell the following record takes.
 * MATCHING: `header` and `header2` are two locals (one changes the
 * allocation), and the cells are walked by byte offset (an index changes the
 * code). */
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
    /* MATCHING: mode is never set, but a bare ResourceSource shrinks the
     * frame by 8. */
    ResourceRequest req;

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
            /* MATCHING: keeps the rec.x/.y/.z loads below the coord2 load */
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

/* The descriptor of world position `pos`: the slot holding it, that slot's
 * chunk index and column/row, the chunk's centre, the position relative to
 * it, and the cell column/row and the offset from the cell's centre. Returns
 * 0, or 1 when no slot holds the position.
 * MATCHING: cellCol/cellRow re-read the stored bytes, the half cell sits
 * inside the subtracted group, and `out->slot` is stored last. */
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

ChunkSlot *StageMap__FindSlotByNeighbour(StageMap *self, s32 neighbour) {
    s32 i;
    ChunkSlot *slot;

    for (i = 0; i < ARRAY_COUNT(self->slots); i++) {
        slot = &self->slots[i];
        if (slot->loader->elemKey == neighbour) {
            return slot;
        }
    }
}

/* The slot whose chunk holds `pos` in x and z (in a vertical grid, also
 * the layer holding y), or NULL.
 * MATCHING: `edge` is assigned inside each upper-bound test. */
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
 * StageMap's drawn window and scale ramp: the last of the class's methods.
 *
 *  - FindSlotIndexByNeighbour, FindSlotIndexByChunk: which of the seven
 *    slots holds a neighbour key, or a loaded chunk.
 *  - The footprint, the window of cells that is drawn. Once every chunk is
 *    loaded, RefreshFootprint hides the cells of the current `rects`, and
 *    every cell chained behind them, rebuilds `rects` and shows the cells of
 *    the new ones (SetFootprintVisible: GsDOFF in each cell's `attribute`).
 *    `rects` is up to four CellRects, one per slot the window overlaps. In
 *    a flat grid the window lies ahead of the target along the axis nearest
 *    its facing and is shifted sideways toward where it looks
 *    (ComputeFootprintFromRotation); BuildFootprintRects and
 *    SplitFootprintRect clip it at the chunk's right and bottom edges into
 *    the neighbouring slots. In a vertical grid it is whole chunks: the
 *    target's, the previous one, and the next one when the target's cell
 *    lies outside `bounds` (SetFootprintFromQuery, InitFootprintRect,
 *    IsPointOutOfBounds; SetBounds).
 *  - The scale ramp: StartScaleRamp picks a step and a tick count,
 *    StepScaleRamp adds the step to every cell's scale once a tick, and
 *    EndScaleRamp sets every cell back to 1/1 (AddScaleStepToCell and
 *    ResetCellScale, run on every cell of every slot by ForEachSlot and
 *    ForEachSlotCell).
 *  - GetUnk1CC, and GetStageMapMethods.
 */

/* The four scale steps (Ratio16[3], x/y/z) StartScaleRamp picks for
 * `scaleStep`: y +1/64 and +1/4 for a positive rate (fast 0, nonzero),
 * -1/64 and -1/4 otherwise; x and z 0/1. */
extern Ratio16 sScaleStepUpSlow[3];
extern Ratio16 sScaleStepUpFast[3];
extern Ratio16 sScaleStepDownSlow[3];
extern Ratio16 sScaleStepDownFast[3];

/* 1/1, 1/1, 1/1: the scale ResetCellScale sets on every cell. */
extern Ratio16 sScaleOne[3];

/* The index of the slot whose chunk is at neighbour key `key`; 0 when
 * none is. */
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

/* The index of the slot holding chunk chunkIndex with its header read; -1
 * when none does. */
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

/* The flat grid's window, from the target's cell and its y rotation: a
 * window aheadCells deep along whichever of x and z the target faces
 * (within 45 degrees), starting at the target's cell and running the way it
 * faces, and acrossCells wide, centred on the target and shifted by the
 * off-axis part of a gridSpan-long facing vector, in cells, kept inside half
 * the grid. The window goes to BuildFootprintRects as footprintCol/Row and
 * footprintWidth/Height. MATCHING: the (u16) casts are retail's lhu. */
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
    rot = &param->rotate; /* MATCHING: here, before the call, so it lives across it */
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

    /* Facing +-x, then facing +-z. MATCHING: the second test is retail's; the
     * two cover every angle. */
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

/* Splits the window (footprintCol/Row, footprintWidth/Height) into
 * `rects`: a window that starts left of the centre chunk or above it starts
 * in that neighbour's slot, with its column and row moved into that chunk
 * (rows above are staggered by half a chunk); a part past the right edge
 * goes to the slot of key + 1, and SplitFootprintRect splits off the part
 * past the bottom edge. MATCHING: one `rect` pointer reused for the second
 * rectangle, `over` its own local, and `count += 1` in each arm and again at
 * the join. */
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
            /* MATCHING: stored in both arms (cross-jumping merges them, and
             * the join keeps the col reload below after it). */
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
            /* MATCHING: two statements; one expression shares span - 20
             * with the store below. */
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

/* 1 when there are no bounds or the point (cell column, row) lies outside
 * them, else 0. MATCHING: the in-range test returns 0 and `return 1`
 * follows it; the other order allocates differently. */
s32 IsPointOutOfBounds(CellBounds *bounds, s8 *point) {
    if (bounds != NULL && point[0] >= bounds->minCol && bounds->maxCol >= point[0] &&
        point[1] >= bounds->minRow && bounds->maxRow >= point[1]) {
        return 0;
    }
    return 1;
}

/* rects[index] becomes the whole of the slot holding chunkIndex; returns the
 * next index. */
s32 StageMap__InitFootprintRect(StageMap *self, s32 unused, s32 index, s32 chunkIndex) {
    CellRect *rect;

    rect = &self->rects.e[index];
    *rect = gFullSlotRect;
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

/* Picks the scale step by the sign of `rate` and by `fast`, and runs the
 * ramp for |rate| times the step's y denominator ticks (scaleStep[1].den).
 * MATCHING: the goto ladder (retail stores scaleStep on the fast positive
 * path and once for the other three), `scale` loaded once before the sign
 * test, and `val` set in an if/else; `~rate + 1` is retail's negation. */
void StageMap__StartScaleRamp(StageMap *self, s32 rate, s32 fast) {
    Ratio16 *table;
    s32 val;
    s32 scale;

    if (rate <= 0) {
        goto rate_le;
    }
    table = sScaleStepUpSlow;
    if (fast == 0) {
        goto store;
    }
    self->scaleStep = sScaleStepUpFast;
    goto merge;
rate_le:
    table = sScaleStepDownSlow;
    if (fast == 0) {
        goto store;
    }
    table = sScaleStepDownFast;
store:
    self->scaleStep = table;
merge:
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
    cell = slot->cells;
    for (; cell < end; cell++) {
        cellFn(self, *cell);
    }
}

StageMapMethods *GetStageMapMethods(void) {
    return &gStageMapMethods;
}
